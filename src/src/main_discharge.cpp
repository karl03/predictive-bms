#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <U8g2lib.h>
#include <INA226_WE.h>
#include <INA3221.h>
#include "batt_model.h"
#include "resistance_estimate.h"
#include "defines.h"

U8G2 *u8g2 = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
INA226_WE ina226 = INA226_WE(0x40);
INA3221 ina3221(INA3221_ADDR41_VCC);
File log_file;

bool serial_mode = false;
float busVoltage_V = 0.0;
float voltage[3];
// float current_A = 0.0;
float current_mA = 0.0;
float shunt_voltage = 0.0;
float step_mAh_charged = 0;
float mAh_charged = 0;
// float Wh_charged = 0;
float mWh_charged = 0;
unsigned long cur_time = 0;
unsigned long loop_time = 0;
unsigned long last_avg = 0;
unsigned long longest_loop = 0;
float v_iter = 0;
// float A_iter = 0;
float mA_iter = 0;
int iter_counter = 0;
int log_counter = 0;
int missed_count = 0;



void setup() {
    if (sd_logging == 1) {
        if (!SD.begin(15)) {
            while(1){};
        } else {
            while (SD.exists(String(log_counter) + ".csv")) {
                log_counter++;
            }
            log_file = SD.open((String(log_counter) + ".csv"), FILE_WRITE);
        }
    }

    // Required to run even when display is not enabled
    u8g2->begin();

    if (serial_timeout > 0) {
        if (use_display) {
            u8g2->clearBuffer();
            u8g2->setFont(u8g2_font_profont17_mr);
            u8g2->setCursor(0, 49);
            u8g2->print("Connecting...");
            u8g2->sendBuffer();
        }

        Serial.begin(115200);
        long start_time = millis();
        while ((millis() - start_time) < (serial_timeout * 1000)) {
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

    if (serial_mode && use_display) {
        u8g2->clearBuffer();
        u8g2->setCursor(0, 49);
        u8g2->print("Connected!");
        u8g2->sendBuffer();
        delay(1000);
        u8g2->clearBuffer();
        u8g2->sendBuffer();
    } else if (use_display) {
        u8g2->setFont(u8g2_font_inr46_mf);
        u8g2->setCursor(0, 49);
        u8g2->print("VLT");
        u8g2->sendBuffer();
        delay(500);
    }

    if(!ina226.init()) {
        if (use_display) {
            u8g2->clearBuffer();
            u8g2->setFont(u8g2_font_profont17_mr);
            u8g2->setCursor(0, 49);
            u8g2->print("INA226 INIT FAIL");
            u8g2->sendBuffer();
        }
        while(1){};
    }

    // Increase averaging onboard INAs to be remove need to average locally, allows time to run other processes
    ina226.setConversionTime(CONV_TIME_2116);
    ina226.setAverage(AVERAGE_16);

    ina3221.begin();
    ina3221.reset();
    ina3221.setShuntMeasDisable();  // INA3221 is only used for bus voltages, so shunt measurement is disabled to save time
    ina3221.setBusConversionTime(INA3221_REG_CONF_CT_2116US);
    ina3221.setAveragingMode(INA3221_REG_CONF_AVG_16);

    if (serial_mode) {
        Serial.print("Cell 1");
        Serial.print(",");
        Serial.print("Cell 2");
        Serial.print(",");
        Serial.print("Cell 3");
        Serial.print(",");
        Serial.print("Cell 4");
        Serial.print(",");
        Serial.print("Total");
        Serial.print(",");
        Serial.print("mA Current");
        Serial.print(",");
        Serial.print("mAh Charged");
        Serial.print(",");
        Serial.print("mWh Charged");
        Serial.print(",");
        Serial.print("Time (ms)");
        Serial.print(",");
        Serial.print("Missed readings");
        Serial.print(",");
        Serial.println("");
    }

    cur_time = micros();
}

void loop() {
    u8g2->clearBuffer();
    u8g2->setFont(u8g2_font_profont17_mr);
    u8g2->setCursor(0, u8g2->getMaxCharHeight());
    u8g2->print(micros() - cur_time);
    u8g2->setCursor(0, u8g2->getMaxCharHeight() * 2);
    u8g2->print(missed_count);
    u8g2->setCursor(0, u8g2->getMaxCharHeight() * 3);
    u8g2->print(longest_loop);
    u8g2->sendBuffer();
    loop_time = micros() - cur_time;
    cur_time = micros();
    if (loop_time > longest_loop) {
        longest_loop = loop_time;
    }
    // Ensures new reading on every loop, block until available if not yet available.
    // 3221 checked first, as its conversion time is greater
    ina3221.readFlags();
    if (!ina3221.getConversionReadyFlag()) {
        while (!ina3221.getConversionReadyFlag()) {
            ina3221.readFlags();
        }
    } else {
        // In-time reading missed, usually due to SD card buffer write taking place
        missed_count ++;
        // If alert pin on INA is connected to interrupt pin on ESP, could potentially count number of missed readings.
    }

    if (ina226.isBusy()) {
        ina226.waitUntilConversionCompleted();
    }

    busVoltage_V = ina226.getBusVoltage_V();
    shunt_voltage = ina226.getShuntVoltage_mV();
    voltage[0] = ina3221.getVoltage(INA3221_CH1);
    voltage[1] = ina3221.getVoltage(INA3221_CH2);
    voltage[2] = ina3221.getVoltage(INA3221_CH3);
    current_mA = (shunt_voltage / current_scale) * 10000 + (current_offset);

    step_mAh_charged = (current_mA * (loop_time * 0.001) * A_ms_to_A_h);
    mAh_charged += step_mAh_charged;
    mWh_charged += step_mAh_charged * (v_iter / iter_counter);

    if (sd_logging == 1) {
        log_file = SD.open((String(log_counter) + ".csv"), FILE_WRITE);

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
            log_file.close();
        } else {
            while(1){};
        }
    }
  
    if (serial_mode) {
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
        Serial.print(current_mA);
        Serial.print(",");
        Serial.print(mAh_charged);
        Serial.print(",");
        Serial.print(mWh_charged);
        Serial.print(",");
        Serial.print(millis());
        Serial.print(",");
        Serial.print(missed_count);
        Serial.print(",");
        Serial.println("");
    } else if (use_display == 1) {
        u8g2->clearBuffer();
        u8g2->setFont(u8g2_font_profont17_mr);
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