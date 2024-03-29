#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SdFat.h>
#include <U8g2lib.h>
#include <INA226_WE.h>
#include <INA3221.h>
#include "defines.h"

U8G2 *u8g2 = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
INA226_WE ina226 = INA226_WE(0x40);
INA3221 ina3221(INA3221_ADDR41_VCC);
SdFs sd;
FsFile log_file;

bool serial_mode = false;
float busVoltage_V = 0.0;
float voltage[3];
float current_mA = 0.0;
float shunt_voltage = 0.0;
float step_mAh_charged = 0;
float mAh_charged = 0;
float mWh_charged = 0;
unsigned long loop_time = 0;
unsigned long last_update = 0;
int log_counter = 0;
int missed_count = 0;



void setup() {
    if (SD_LOGGING == 1) {
        if (!sd.begin(15, SD_SCK_MHZ(39.9))) {
            while(1){};
        } else {
            while (sd.exists(String(log_counter) + ".csv")) {
                log_counter++;
            }
            if (!log_file.open((String(log_counter) + ".csv").c_str(), O_CREAT | O_RDWR)) {
                while(1){};
            }
            log_file.close();
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
        u8g2->setFont(u8g2_font_profont17_mr);
        u8g2->setCursor(0, u8g2->getMaxCharHeight());
        u8g2->print("Serial Mode");
        u8g2->setCursor(0, u8g2->getMaxCharHeight() * 2);
        u8g2->print("Time: ");
        u8g2->setCursor(0, u8g2->getMaxCharHeight() * 3);
        u8g2->print("Missed: ");
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
    if (serial_mode) {
        Serial.print("Cell 1,Cell 2,Cell 3,Cell 4,");
        Serial.print("Total,");
        Serial.print("Shunt Voltage,");
        Serial.print("mA Current,");
        Serial.print("mAh Charged,");
        Serial.print("mWh Charged,");
        Serial.print("Loop Time (micros),");
        Serial.print("Missed readings");
        Serial.println("");
    }

    last_update = micros();
}

void loop() {
    // Ensures new reading on every loop, block until available if not yet available.
    // INA226 checked first, as its conversion time is greater
    if (ina226.isBusy()) {
        ina226.waitUntilConversionCompleted();
    } else {
        // In-time reading missed, usually due to SD card buffer write taking place
        missed_count ++;
        // If alert pin on INA is connected to interrupt pin on ESP, could potentially count number of missed readings.
    }

    ina3221.readFlags();
    if (!ina3221.getConversionReadyFlag()) {
        while (!ina3221.getConversionReadyFlag()) {
            ina3221.readFlags();
        }
    }

    busVoltage_V = ina226.getBusVoltage_V();
    shunt_voltage = ina226.getShuntVoltage_mV();
    voltage[0] = ina3221.getVoltage(INA3221_CH1);
    voltage[1] = ina3221.getVoltage(INA3221_CH2);
    voltage[2] = ina3221.getVoltage(INA3221_CH3);
    current_mA = (shunt_voltage / CURRENT_SCALE) * 10000 + (CURRENT_OFFSET);

    loop_time = micros() - last_update;
    last_update = micros();
    step_mAh_charged = (current_mA * (loop_time * 0.001) * A_ms_to_A_h);
    mAh_charged += step_mAh_charged;
    mWh_charged += step_mAh_charged * busVoltage_V;

    if (SD_LOGGING == 1) {
        log_file.open((String(log_counter) + ".csv").c_str(), O_WRITE | O_APPEND);

        if (log_file) {
            // Time
            log_file.print(millis());
            log_file.print(",");
            // V1
            log_file.print(voltage[0]);
            log_file.print(",");
            // V2
            log_file.print(voltage[1] - voltage[0]);
            log_file.print(",");
            // V3
            log_file.print(voltage[2] - voltage[1]);
            log_file.print(",");
            // V4
            log_file.print(busVoltage_V - voltage[2]);
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
            log_file.flush();
            log_file.close();
        } else {
            while(1){};
        }
    }
  
    if (serial_mode) {
        if (USE_DISPLAY) {
            u8g2->setCursor(u8g2->getMaxCharWidth() * 6, u8g2->getMaxCharHeight() * 2);
            u8g2->print(loop_time);
            u8g2->setCursor(u8g2->getMaxCharWidth() * 8, u8g2->getMaxCharHeight() * 3);
            u8g2->print(missed_count);
            u8g2->sendBuffer();
        }
        Serial.print(voltage[0]);
        Serial.print(",");
        Serial.print(voltage[1] - voltage[0]);
        Serial.print(",");
        Serial.print(voltage[2] - voltage[1]);
        Serial.print(",");
        Serial.print(busVoltage_V - voltage[2]);
        Serial.print(",");
        Serial.print(busVoltage_V);
        Serial.print(",");
        Serial.print(shunt_voltage);
        Serial.print(",");
        Serial.print(current_mA);
        Serial.print(",");
        Serial.print(mAh_charged);
        Serial.print(",");
        Serial.print(mWh_charged);
        Serial.print(",");
        Serial.print(loop_time);
        Serial.print(",");
        Serial.print(missed_count);
        Serial.println("");
    } else if (USE_DISPLAY == 1) {
        u8g2->clearBuffer();
        u8g2->setCursor(0, u8g2->getMaxCharHeight());
        u8g2->print(voltage[0], 3);
        u8g2->print("V  ");
        u8g2->print(voltage[1] - voltage[0], 3);
        u8g2->print("V");
        u8g2->setCursor(0, (u8g2->getMaxCharHeight() * 2));
        u8g2->print(voltage[2] - voltage[1], 3);
        u8g2->print("V  ");
        u8g2->print(busVoltage_V - voltage[2], 3);
        u8g2->print("V");
        u8g2->setCursor(0, (u8g2->getMaxCharHeight() * 3));
        u8g2->print(current_mA, 1);
        u8g2->print("mA ");
        u8g2->sendBuffer();
    }
}