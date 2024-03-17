#include <Arduino.h>
#include <Wire.h>
#include <SdFat.h>
#include <U8g2lib.h>
#include "batt_model.h"
#include "resistance_estimate.h"
#include "batt_monitor.h"
#include "curr_estimator.h"
#include "defines.h"

U8G2 *u8g2 = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
SdFs sd;
FsFile log_file;
#ifdef MOCKING
#include "INA_mock.h"
#include "INA3221_mock.h"
INA_mock ina226 = INA_mock(&sd, MOCKING);
INA3221_mock ina3221(&sd, MOCKING_3221);
#else
#define MOCKING 0
#include <INA226_WE.h>
#include <INA3221.h>
INA226_WE ina226 = INA226_WE(0x40);
INA3221 ina3221(INA3221_ADDR41_VCC);
#endif
BattModel *simulator;
BattModel *modifiable_simulator;
BattMonitor *monitor;
CurrEstimator *curr_estimator;
CurrEstimator *watt_estimator;

bool serial_mode = false;
bool flying = false;
bool flying_low_threshold_reached = false;
unsigned long low_threshold_start = 0;
float busVoltage_V = 0.0;
float shuntVoltage_mV = 0.0;
float cell_voltages[4];
float current_mA = 0.0;
unsigned long cur_time = 0;
unsigned long loop_time = 0;
int log_counter = 0;
int missed_count = 0;
float capacity_till_empty = CAPACITY;
float total_flight_time_s = 0;
float flight_time_remaining_s = 0;
float initial_mAh_used = 0;
float initial_mWh_used = 0;

void setup() {
    // Required to run even when display is not enabled
    u8g2->begin();

    if (SD_LOGGING || MOCKING || (CURRENT_FILE && WATT_FILE)) {
        if (!sd.begin(15, SD_SCK_MHZ(39.9))) {
            if (DISPLAY) {
                u8g2->clearBuffer();
                u8g2->setFont(u8g2_font_profont17_mr);
                u8g2->setCursor(0, u8g2->getMaxCharHeight());
                u8g2->print("SD Fail");
                u8g2->sendBuffer();
            }
            while(1){yield();}
        } else if (SD_LOGGING) {
            while (sd.exists(String(log_counter) + ".csv")) {
                log_counter++;
            }
            log_file.open((String(log_counter) + ".csv").c_str(), O_CREAT | O_RDWR);
            if (!log_file.isOpen()) {
                if (DISPLAY) {
                    u8g2->clearBuffer();
                    u8g2->setFont(u8g2_font_profont17_mr);
                    u8g2->setCursor(0, u8g2->getMaxCharHeight());
                    u8g2->print("File Fail");
                    u8g2->sendBuffer();
                }
                while(1){yield();}
            }
            log_file.close();
        }
    }

    if (SERIAL_TIMEOUT > 0) {
        if (USE_DISPLAY) {
            u8g2->clearBuffer();
            u8g2->setFont(u8g2_font_profont17_mr);
            u8g2->setCursor(0, u8g2->getMaxCharHeight());
            u8g2->print("Connecting...");
            u8g2->sendBuffer();
        }

        Serial.begin(115200);
        long start_time = millis();
        while ((millis() - start_time) < (SERIAL_TIMEOUT * 1000)) {
            Serial.flush();
            if (Serial.available() <= 0) {
                Serial.print("BMS");
            } else if (Serial.readStringUntil('\n') == "hello") {
                    Serial.println("hello");
                    serial_mode = true;
                    break;
            }
        };
    }

    if (serial_mode && USE_DISPLAY) {
        u8g2->clearBuffer();
        u8g2->setCursor(0, u8g2->getMaxCharHeight());
        u8g2->print("Connected!");
        u8g2->sendBuffer();
        delay(1000);
        u8g2->clearBuffer();
        u8g2->sendBuffer();
    } else if (USE_DISPLAY) {
        u8g2->clearBuffer();
        u8g2->setFont(u8g2_font_inr46_mf);
        u8g2->setCursor(0, 49);
        u8g2->print("VLT");
        u8g2->sendBuffer();
        u8g2->setFont(u8g2_font_profont17_mr);
        delay(500);
    }

    if (!ina226.init()) {
        if (USE_DISPLAY) {
            u8g2->clearBuffer();
            u8g2->setFont(u8g2_font_profont17_mr);
            u8g2->setCursor(0, u8g2->getMaxCharHeight());
            u8g2->print("INA226 INIT FAIL");
            u8g2->sendBuffer();
        }
        while(1){yield();}
    }

    // Increase averaging onboard INAs to remove need to average locally, allows time to run other processes
    // INA226 is primary for functionality, so its conversion time is set to higher than the 3221, allowing loop to be based around its readings
    ina226.setConversionTime(CONV_TIME_2116);
    ina226.setAverage(AVERAGE_16);
    // Total time = 2.116ms conversion * 2 readings * 16 averages = 67.712ms
    ina3221.begin();
    ina3221.reset();
    ina3221.setShuntMeasDisable();  // INA3221 is only used for bus voltages, so shunt measurement is disabled to save time
    ina3221.setBusConversionTime(INA3221_REG_CONF_CT_1100US);
    ina3221.setAveragingMode(INA3221_REG_CONF_AVG_16);
    // Total time = 1.1ms conversion * 3 readings * 16 averages = 52.8ms

    // unsigned long zero_amp_start = millis();
    // cur_time = millis();
    // while ((millis() - zero_amp_start) < STABILISATION_TIME_MS) {
    //     delay(0);   // Required to allow for ESP background processes to run
    //     if (USE_DISPLAY) {
    //         u8g2->clearBuffer();
    //         u8g2->setCursor(0, u8g2->getMaxCharHeight());
    //         u8g2->print("Stabilising");
    //         u8g2->setCursor(0, u8g2->getMaxCharHeight() * 2);
    //         u8g2->printf("Time left:%lu", STABILISATION_TIME_MS - (millis() - zero_amp_start));
    //         u8g2->setCursor(0, u8g2->getMaxCharHeight() * 3);
    //         u8g2->printf("mA:%.0f", shuntVoltageTomA(ina226.getShuntVoltage_mV()));
    //         u8g2->sendBuffer();
    //     }
    //     if (ina226.isBusy()) {
    //         ina226.waitUntilConversionCompleted();
    //     }
    //     if ((shuntVoltageTomA(ina226.getShuntVoltage_mV()) > MAX_RESTING_AMPS * 1000) || (shuntVoltageTomA(ina226.getShuntVoltage_mV()) < (-MAX_RESTING_AMPS) * 1000)) {
    //         zero_amp_start = millis();
    //     }
    //     loop_time = millis() - cur_time;
    //     cur_time = millis();
    // }

    // Initialise battery model/ monitor
    simulator = new BattModel(MAX_VOLTAGE, CAPACITY, NOMINAL_VOLTAGE, NOMINAL_CAPACITY, INTERNAL_RESISTANCE, EXPONENTIAL_VOLTAGE, EXPONENTIAL_CAPACITY, CURVE_CURRENT, MIN_VOLTAGE);
    modifiable_simulator = new BattModel(MAX_VOLTAGE, CAPACITY, NOMINAL_VOLTAGE, NOMINAL_CAPACITY, INTERNAL_RESISTANCE, EXPONENTIAL_VOLTAGE, EXPONENTIAL_CAPACITY, CURVE_CURRENT, MIN_VOLTAGE);

    busVoltage_V = ina226.getBusVoltage_V();
    shuntVoltage_mV = ina226.getShuntVoltage_mV();
    current_mA = shuntVoltageTomA(shuntVoltage_mV);
    float ina3221_voltages[] = {ina3221.getVoltage(INA3221_CH1),
                                ina3221.getVoltage(INA3221_CH2),
                                ina3221.getVoltage(INA3221_CH3)};
    cell_voltages[0] = ina3221_voltages[0];
    cell_voltages[1] = ina3221_voltages[1] -  ina3221_voltages[0];
    cell_voltages[2] = ina3221_voltages[2] - ina3221_voltages[1];
    cell_voltages[3] = busVoltage_V - ina3221_voltages[2];

    while (simulator->Simulate(initial_mAh_used * 0.001, current_mA * 0.001, current_mA * 0.001) > busVoltage_V) {
        initial_mAh_used += 1;
        yield();
    }

    if (initial_mAh_used > 0) {
        unsigned int index = (sizeof(MAH_AT_VOLTAGE) / sizeof(MAH_AT_VOLTAGE[0]));
        while (index > 0) {
            index--;
            if (MAH_AT_VOLTAGE[index] > initial_mAh_used) {
                initial_mWh_used = MWH_AT_VOLTAGE[index];
                index = 0;
            }
            yield();
        }
    }

    if (DISPLAY) {
        u8g2->clearBuffer();
        u8g2->setCursor(0, u8g2->getMaxCharHeight());
        u8g2->printf("Mah:%f", initial_mAh_used);
        u8g2->setCursor(0, u8g2->getMaxCharHeight() * 2);
        u8g2->printf("Mwh:%f", initial_mWh_used);
        u8g2->sendBuffer();
        delay(1000);
    }

    monitor = new BattMonitor(busVoltage_V, current_mA, cell_voltages, initial_mAh_used, initial_mWh_used, micros(), simulator, modifiable_simulator, REACTION_TIME * 1000000, MAX_VOLTAGE_VARIANCE, MAX_CELL_VARIANCE, CAPACITY_STEP_PERCENTAGE);
    if (CURRENT_FILE && WATT_FILE) {
        curr_estimator = new CurrEstimator(&sd, CURRENT_FILE, LONG_DECAY_SECONDS, SHORT_DECAY_SECONDS);
        watt_estimator = new CurrEstimator(&sd, WATT_FILE, LONG_DECAY_SECONDS, SHORT_DECAY_SECONDS);
    } else {
        curr_estimator = new CurrEstimator(&sd, "\0", LONG_DECAY_SECONDS, SHORT_DECAY_SECONDS);
        watt_estimator = new CurrEstimator(&sd, "\0", LONG_DECAY_SECONDS, SHORT_DECAY_SECONDS);
    }
    
    monitor->resetFilter(current_mA);
}

void loop() {
    loop_time = micros() - cur_time;
    cur_time = micros();
    // Ensures new reading on every loop, block until available if not yet available.
    // INA226 checked first, as its conversion time is greater
    if (ina226.isBusy()) {
        ina226.waitUntilConversionCompleted();
    } else {
        // In-time reading missed, usually due to SD card buffer write taking place
        missed_count ++;
    }

    ina3221.readFlags();
    if (!ina3221.getConversionReadyFlag()) {
        while (!ina3221.getConversionReadyFlag()) {
            ina3221.readFlags();
        }
    }
        
    busVoltage_V = ina226.getBusVoltage_V();
    float ina3221_voltages[] = {ina3221.getVoltage(INA3221_CH1),
                                ina3221.getVoltage(INA3221_CH2),
                                ina3221.getVoltage(INA3221_CH3)};
    cell_voltages[0] = ina3221_voltages[0];
    cell_voltages[1] = ina3221_voltages[1] -  ina3221_voltages[0];
    cell_voltages[2] = ina3221_voltages[2] - ina3221_voltages[1];
    cell_voltages[3] = busVoltage_V - ina3221_voltages[2];
    shuntVoltage_mV = ina226.getShuntVoltage_mV();
    current_mA = shuntVoltageTomA(shuntVoltage_mV);

    if (!flying && current_mA >= MIN_FLYING_AMPS * 1000) {
        flying = true;
        flying_low_threshold_reached = false;
    } else if (flying && current_mA < MIN_FLYING_AMPS * 1000) {
        if (flying_low_threshold_reached) {
            if (millis() - low_threshold_start > MAX_ZERO_CURRENT_TIME_MS) {
                flying = false;
                curr_estimator->writeLongTermAvg();
                watt_estimator->writeLongTermAvg();
            }
        } else {
            flying_low_threshold_reached = true;
            low_threshold_start = millis();
        }
    } else if (flying) {
        flying_low_threshold_reached = false;
    }

    if (flying) {
        curr_estimator->updateAvg(current_mA, loop_time);
        watt_estimator->updateAvg((current_mA * 0.001) * busVoltage_V, loop_time);
        monitor->updateConsumption(micros(), busVoltage_V, current_mA, cell_voltages);

        if (modifiable_simulator->Simulate(capacity_till_empty, (watt_estimator->getShortAvg() / MIN_FLYING_VOLTAGE), (watt_estimator->getShortAvg() / MIN_FLYING_VOLTAGE)) > MIN_FLYING_VOLTAGE) {
            capacity_till_empty += capacity_till_empty * (CAPACITY_STEP_PERCENTAGE * 0.01);
        } else {
            capacity_till_empty -= capacity_till_empty * (CAPACITY_STEP_PERCENTAGE * 0.01);
        }
    }

    flight_time_remaining_s = (((capacity_till_empty * monitor->getNominalVoltage()) - (monitor->getEstmWhUsed() * 0.001)) / watt_estimator->getShortAvg()) * 3600;
    total_flight_time_s = ((capacity_till_empty * monitor->getNominalVoltage())/ watt_estimator->getShortAvg()) * 3600;


    if (SD_LOGGING) {
        log_file.open((String(log_counter) + ".csv").c_str(), O_WRITE | O_APPEND);
        if (log_file.isOpen()) {
            if (MOCKING && flying) {
                // Time
                log_file.print(millis());
                log_file.print(",");
                // Voltage
                log_file.print(busVoltage_V);
                log_file.print(",");
                // Sim voltage
                log_file.print(monitor->getSimVoltage());
                log_file.print(",");
                // Adjusted sim voltage
                log_file.print(monitor->getFittedSimVoltage());
                log_file.print(",");
                // Total estimated flight time
                log_file.print(total_flight_time_s);
                log_file.print(",");
                // Remaining estimated flight time
                log_file.print(flight_time_remaining_s);
                log_file.print(",");
                // Estimated capacity
                log_file.print(monitor->getEstimatedCapacity(), 3);
                log_file.print(",");
                // Estimated usable capacity
                log_file.print(capacity_till_empty, 3);
                log_file.print(",");
                // Used capacity
                log_file.print(monitor->getmAhUsed(), 0);
                log_file.print(",");
                // Estimated used capacity
                log_file.print(monitor->getEstmAhUsed(), 0);
                log_file.print(",");
                // Used Energy
                log_file.print(monitor->getmWhUsed(), 0);
                log_file.print(",");
                // Estimated used Energy
                log_file.print(monitor->getEstmWhUsed(), 0);
                log_file.print(",");
                // Cell performance values
                log_file.print(monitor->getCellsStatus()[0]);
                log_file.print(",");
                log_file.print(monitor->getCellsStatus()[1]);
                log_file.print(",");
                log_file.print(monitor->getCellsStatus()[2]);
                log_file.print(",");
                log_file.print(monitor->getCellsStatus()[3]);
                log_file.print(",");
                // Resistance estimate
                log_file.print(monitor->getResistanceOhms(), 5);
                log_file.println(",");
                log_file.flush();
                log_file.close();
            } else if (!MOCKING) {
                // Time
                log_file.print(millis());
                log_file.print(",");
                // Voltage
                log_file.print(busVoltage_V);
                log_file.print(",");
                // V1
                log_file.print(cell_voltages[0]);
                log_file.print(",");
                // V2
                log_file.print(cell_voltages[1]);
                log_file.print(",");
                // V3
                log_file.print(cell_voltages[2]);
                log_file.print(",");
                // V4
                log_file.print(cell_voltages[3]);
                log_file.print(",");
                // Current
                log_file.print(current_mA);
                log_file.print(",");
                // Shunt Voltage Used
                log_file.print(shuntVoltage_mV);
                log_file.println(",");
                log_file.flush();
                log_file.close();
            }
        } else {
            if (serial_mode) {
                Serial.print("File error!");
            } else if (USE_DISPLAY) {
                u8g2->clearBuffer();
                u8g2->setCursor(0, u8g2->getMaxCharHeight());
                u8g2->printf("File error!");
                u8g2->sendBuffer();
            }
            while(1){yield();}
        }
    }
  
    if (serial_mode) {
        Serial.print(cell_voltages[0]);
        Serial.print(cell_voltages[1]);
        Serial.print(cell_voltages[2]);
        Serial.print(cell_voltages[3]);
        Serial.print(",");
        Serial.print(busVoltage_V);
        Serial.print(",");
        Serial.print(monitor->getSimVoltage());
        Serial.print(",");
        Serial.print(monitor->getFittedSimVoltage());
        Serial.print(",");
        Serial.print(total_flight_time_s);
        Serial.print(",");
        Serial.print(flight_time_remaining_s);
        Serial.print(",");
        Serial.print(monitor->getEstimatedCapacity());
        Serial.print(",");
        Serial.print(capacity_till_empty);
        Serial.print(",");
        Serial.print(monitor->getmAhUsed());
        Serial.print(",");
        Serial.print(monitor->getEstmAhUsed());
        Serial.print(",");
        Serial.print(monitor->getmWhUsed());
        Serial.print(",");
        Serial.print(monitor->getEstmWhUsed());
        Serial.print(",");
        Serial.print(monitor->getCellsStatus()[0]);
        Serial.print(",");
        Serial.print(monitor->getCellsStatus()[1]);
        Serial.print(",");
        Serial.print(monitor->getCellsStatus()[2]);
        Serial.print(",");
        Serial.print(monitor->getCellsStatus()[3]);
        Serial.print(",");
        Serial.println(monitor->getResistanceOhms());
    } else if (USE_DISPLAY) {
        u8g2->clearBuffer();
        u8g2->setCursor(0, u8g2->getMaxCharHeight());
        u8g2->print(busVoltage_V, 2);
        u8g2->print("V");
        u8g2->setCursor(0, (u8g2->getMaxCharHeight() * 2));
        u8g2->print(monitor->getFittedSimVoltageDiff());
        u8g2->print("  ");
        u8g2->print(flying);
        u8g2->setCursor(0, (u8g2->getMaxCharHeight() * 3));
        u8g2->print(monitor->getEstimatedCapacity());
        u8g2->print("mA");
        u8g2->print(missed_count);
        u8g2->sendBuffer();
    }
}