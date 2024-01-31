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
float current_mA = 0.0;
float shunt_voltage = 0.0;
float step_mAh_charged = 0;
float mAh_charged = 0;
float mWh_charged = 0;
unsigned long cur_time = 0;
unsigned long loop_time = 0;
unsigned long last_avg = 0;
float v_iter = 0;
float mA_iter = 0;
int iter_counter = 0;
int log_counter = 0;
int missed_count = 0;

float SoC;
// On startup, look up estimated SoC based on current voltage, assuming current is (close to) zero.
// After initial SoC has been measured begin coloumb counting.

float SoCLookup(float voltage) {
    if ((voltage >= MIN_VOLTAGE) && (voltage <= MAX_VOLTAGE)) {
        return MAH_AT_VOLTAGE[(int)(voltage * 100) - (int)(MIN_VOLTAGE * 100)];
    }
}


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
            u8g2->setCursor(0, u8g2->getMaxCharHeight());
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
        u8g2->setCursor(0, u8g2->getMaxCharHeight());
        u8g2->print("Connected!");
        u8g2->sendBuffer();
        delay(1000);
        u8g2->clearBuffer();
        u8g2->sendBuffer();
    } else if (use_display) {
        u8g2->clearBuffer();
        u8g2->setFont(u8g2_font_inr46_mf);
        u8g2->setCursor(0, 49);
        u8g2->print("VLT");
        u8g2->sendBuffer();
        u8g2->setFont(u8g2_font_profont17_mr);
        delay(500);
    }

    if(!ina226.init()) {
        if (use_display) {
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
    // Total time = 2.116ms conversion * 2 readings * 16 averages = 67.712ms
    ina3221.begin();
    ina3221.reset();
    ina3221.setShuntMeasDisable();  // INA3221 is only used for bus voltages, so shunt measurement is disabled to save time
    ina3221.setBusConversionTime(INA3221_REG_CONF_CT_1100US);
    ina3221.setAveragingMode(INA3221_REG_CONF_AVG_16);
    // Total time = 1.1ms conversion * 3 readings * 16 averages = 52.8ms
}

void loop() {
    u8g2->clearBuffer();
    u8g2->setFont(u8g2_font_profont17_mr);
    u8g2->setCursor(0, u8g2->getMaxCharHeight());
    u8g2->print(loop_time);
    u8g2->setCursor(0, u8g2->getMaxCharHeight() * 2);
    u8g2->print(missed_count);
    u8g2->sendBuffer();
    loop_time = micros() - cur_time;
    cur_time = micros();
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

    // run battery model here, as this time would otherwise be spent waiting for measurements

    busVoltage_V = ina226.getBusVoltage_V();
    shunt_voltage = ina226.getShuntVoltage_mV();
    voltage[0] = ina3221.getVoltage(INA3221_CH1);
    voltage[1] = ina3221.getVoltage(INA3221_CH2);
    voltage[2] = ina3221.getVoltage(INA3221_CH3);
    current_mA = (shunt_voltage / CURRENT_SCALE) * 10000 + (CURRENT_OFFSET);

    // Averaging could be used alongside alert pin to write an average value after missing readings
    if (last_avg == 0) {
        last_avg = cur_time;
    } else if (iter_counter > 0 && (cur_time - last_avg) > (avg_ms * 1000)) {
        step_mAh_charged = ((mA_iter / iter_counter) * ((cur_time - last_avg) * 0.001) * A_ms_to_A_h);
        mAh_charged += step_mAh_charged;
        mWh_charged += step_mAh_charged * (v_iter / iter_counter);
        last_avg = cur_time;
        v_iter = 0;
        mA_iter = 0;
        iter_counter = 0;
    } else {
        v_iter += busVoltage_V;
        mA_iter += current_mA;
        iter_counter ++;
    }

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
        Serial.print("v1,");
        Serial.println(voltage[0]);
        Serial.print("v2,");
        Serial.println(voltage[1] - voltage[0]);
        Serial.print("v3,");
        Serial.println(voltage[2] - voltage[1]);
        Serial.print("v4,");
        Serial.println(busVoltage_V - voltage[2]);
    } else if (use_display == 1) {
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