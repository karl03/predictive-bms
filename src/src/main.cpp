#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <U8g2lib.h>
#include <INA226_WE.h>
#include <INA3221.h>
#include "batt_model.h"
#include "resistance_estimate.h"
#include "batt_monitor.h"
#include "defines.h"

U8G2 *u8g2 = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
INA226_WE ina226 = INA226_WE(0x40);
INA3221 ina3221(INA3221_ADDR41_VCC);
File log_file;
BattModel *simulator;
BattModel *modifiable_simulator;
BattMonitor *monitor;

bool serial_mode = false;
float busVoltage_V = 0.0;
float cell_voltages[4];
float current_mA = 0.0;
float shunt_voltage = 0.0;
float step_mAh_charged = 0;
float mAh_charged = 0;
float mWh_charged = 0;
unsigned long cur_time = 0;
unsigned long loop_time = 0;
int iter_counter = 0;
int log_counter = 0;
volatile int missed_count = 0;
volatile float v_iter = 0;
volatile float shunt_mV_iter = 0;

float SoCLookup(float voltage, const float* array) {
    if ((voltage >= MIN_VOLTAGE) && (voltage <= MAX_VOLTAGE)) {
        int index = (int)(voltage * 100) - (int)(MIN_VOLTAGE * 100);
        return array[index];
    } else if (voltage > MAX_VOLTAGE) {
        return 0;
    } else {
        return array[0];
    }
}

IRAM_ATTR void measureNow() {
    u8g2->clearBuffer();
    u8g2->setFont(u8g2_font_profont17_mr);
    u8g2->setCursor(0, u8g2->getMaxCharHeight());
    u8g2->print("Reading Caught!");
    u8g2->sendBuffer();
    missed_count ++;
    v_iter += ina226.getBusVoltage_V();
    shunt_mV_iter += shuntVoltageTomA(ina226.getShuntVoltage_mV());
    ina226.readAndClearFlags();
}

void setup() {
    if (SD_LOGGING == 1) {
        if (!SD.begin(15)) {
            while(1){};
        } else {
            while (SD.exists(String(log_counter) + ".csv")) {
                log_counter++;
            }
            log_file = SD.open((String(log_counter) + ".csv"), FILE_WRITE);
            if (!log_file) {
                while(1){};
            }
        }
    }

    // Required to run even when display is not enabled
    u8g2->begin();

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

    if(!ina226.init()) {
        if (USE_DISPLAY) {
            u8g2->clearBuffer();
            u8g2->setFont(u8g2_font_profont17_mr);
            u8g2->setCursor(0, u8g2->getMaxCharHeight());
            u8g2->print("INA226 INIT FAIL");
            u8g2->sendBuffer();
        }
        while(1){};
    }

    // Increase averaging onboard INAs to be remove need to average locally, allows time to run other processes
    // INA226 is primary for functionality, so its conversion time is set to higher than the 3221, allowing loop to be based around its readings
    ina226.setConversionTime(CONV_TIME_2116);
    ina226.setAverage(AVERAGE_16);
    ina226.enableConvReadyAlert();
    pinMode(INTERRUPT_PIN, INPUT_PULLUP);
    // Total time = 2.116ms conversion * 2 readings * 16 averages = 67.712ms
    ina3221.begin();
    ina3221.reset();
    ina3221.setShuntMeasDisable();  // INA3221 is only used for bus voltages, so shunt measurement is disabled to save time
    ina3221.setBusConversionTime(INA3221_REG_CONF_CT_1100US);
    ina3221.setAveragingMode(INA3221_REG_CONF_AVG_16);
    // Total time = 1.1ms conversion * 3 readings * 16 averages = 52.8ms

    unsigned long zero_amp_start = millis();
    cur_time = millis();
    while ((millis() - zero_amp_start) < STABILISATION_TIME_MS) {
        delay(0);   // Required to allow for ESP background processes to run
        if (USE_DISPLAY) {
            u8g2->clearBuffer();
            u8g2->setCursor(0, u8g2->getMaxCharHeight());
            u8g2->print("Stabilising");
            u8g2->setCursor(0, u8g2->getMaxCharHeight() * 2);
            u8g2->printf("Time left:%lu", STABILISATION_TIME_MS - (millis() - zero_amp_start));
            u8g2->setCursor(0, u8g2->getMaxCharHeight() * 3);
            u8g2->printf("mA:%.0f", shuntVoltageTomA(ina226.getShuntVoltage_mV()));
            u8g2->sendBuffer();
        }
        if (ina226.isBusy()) {
            ina226.waitUntilConversionCompleted();
        }
        if ((shuntVoltageTomA(ina226.getShuntVoltage_mV()) > ZERO_AMP_CUTOFF) || (shuntVoltageTomA(ina226.getShuntVoltage_mV()) < (-ZERO_AMP_CUTOFF))) {
            zero_amp_start = millis();
        }
        loop_time = millis() - cur_time;
        cur_time = millis();
    }

    // Initialise battery model/ monitor
    simulator = new BattModel(MAX_VOLTAGE, CAPACITY, NOMINAL_VOLTAGE, NOMINAL_CAPACITY, INTERNAL_RESISTANCE, EXPONENTIAL_VOLTAGE, EXPONENTIAL_CAPACITY, CURVE_CURRENT, MIN_VOLTAGE);
    modifiable_simulator = new BattModel(MAX_VOLTAGE, CAPACITY, NOMINAL_VOLTAGE, NOMINAL_CAPACITY, INTERNAL_RESISTANCE, EXPONENTIAL_VOLTAGE, EXPONENTIAL_CAPACITY, CURVE_CURRENT, MIN_VOLTAGE);

    busVoltage_V = ina226.getBusVoltage_V();
    current_mA = shuntVoltageTomA(ina226.getShuntVoltage_mV());
    cell_voltages[0] = ina3221.getVoltage(INA3221_CH1);
    cell_voltages[1] = ina3221.getVoltage(INA3221_CH2) -  ina3221.getVoltage(INA3221_CH1);
    cell_voltages[2] = ina3221.getVoltage(INA3221_CH3) - ina3221.getVoltage(INA3221_CH2);
    cell_voltages[3] = ina226.getBusVoltage_V() - ina3221.getVoltage(INA3221_CH3);
    float mAh_used = SoCLookup(busVoltage_V, MAH_AT_VOLTAGE);
    float mWh_used = SoCLookup(busVoltage_V, MWH_AT_VOLTAGE);

    monitor = new BattMonitor(busVoltage_V, current_mA, cell_voltages, mAh_used, mWh_used, micros(), simulator, modifiable_simulator, REACTION_TIME, MAX_VOLTAGE_VARIANCE, MAX_CELL_VARIANCE);
}

void loop() {
    loop_time = micros() - cur_time;
    cur_time = micros();
    // Ensures new reading on every loop, block until available if not yet available.
    // INA226 checked first, as its conversion time is greater
    detachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN));
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

    if (missed_count) {
        busVoltage_V = (ina226.getBusVoltage_V() + v_iter) / (missed_count + 1);
        // Consider also getting missed INA3221 values if possible
        cell_voltages[0] = ina3221.getVoltage(INA3221_CH1);
        cell_voltages[1] = ina3221.getVoltage(INA3221_CH2) -  ina3221.getVoltage(INA3221_CH1);
        cell_voltages[2] = ina3221.getVoltage(INA3221_CH3) - ina3221.getVoltage(INA3221_CH2);
        cell_voltages[3] = busVoltage_V - ina3221.getVoltage(INA3221_CH3);
        current_mA = shuntVoltageTomA((ina226.getShuntVoltage_mV() + shunt_mV_iter) / (missed_count + 1));
        
        missed_count = 0;
    } else {
        busVoltage_V = ina226.getBusVoltage_V();
        cell_voltages[0] = ina3221.getVoltage(INA3221_CH1);
        cell_voltages[1] = ina3221.getVoltage(INA3221_CH2) -  ina3221.getVoltage(INA3221_CH1);
        cell_voltages[2] = ina3221.getVoltage(INA3221_CH3) - ina3221.getVoltage(INA3221_CH2);
        cell_voltages[3] = busVoltage_V - ina3221.getVoltage(INA3221_CH3);
        current_mA = shuntVoltageTomA(ina226.getShuntVoltage_mV());
    }



    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), measureNow, FALLING);
    delay(100); // to test interrupt

    // monitor->updateConsumption(micros(), 60, busVoltage_V, current_mA, cell_voltages);

    // Decide how to access data for logging in main. Keep local variables or use getters?
    if (SD_LOGGING) {
        log_file = SD.open((String(log_counter) + ".csv"), FILE_WRITE);
        if (log_file) {
            // Time
            log_file.print(millis());
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
            // mAh Used
            log_file.print(mAh_charged);
            log_file.print(",");
            // mWh Used
            log_file.print(mWh_charged);
            log_file.println(",");
            log_file.close();
        } else {
            if (serial_mode) {
                Serial.print("File error!");
            } else if (USE_DISPLAY) {
                u8g2->clearBuffer();
                u8g2->setCursor(0, u8g2->getMaxCharHeight());
                u8g2->printf("File error!");
                u8g2->sendBuffer();
            }
            while(1){};
        }
    }
  
    if (serial_mode) {
        Serial.print("v1,");
        Serial.println(cell_voltages[0]);
        Serial.print("v2,");
        Serial.println(cell_voltages[1]);
        Serial.print("v3,");
        Serial.println(cell_voltages[2]);
        Serial.print("v4,");
        Serial.println(cell_voltages[3]);
    } else if (USE_DISPLAY) {
        u8g2->clearBuffer();
        u8g2->setCursor(0, u8g2->getMaxCharHeight());
        u8g2->print(cell_voltages[0], 3);
        u8g2->print("V  ");
        u8g2->print(cell_voltages[1], 3);
        u8g2->print("V");
        u8g2->setCursor(0, (u8g2->getMaxCharHeight() * 2));
        u8g2->print(cell_voltages[2], 3);
        u8g2->print("V  ");
        u8g2->print(cell_voltages[3], 3);
        u8g2->print("V");
        u8g2->setCursor(0, (u8g2->getMaxCharHeight() * 3));
        u8g2->print(current_mA, 1);
        u8g2->print("mA ");
        u8g2->sendBuffer();
    }
}