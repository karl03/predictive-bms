#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <U8g2lib.h>
#include <INA226_WE.h>
#include <INA3221.h>
#include "batt_model.h"
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
unsigned long last_avg = 0;
float v_iter = 0;
// float A_iter = 0;
float mA_iter = 0;
int iter_counter = 0;
int log_counter = 0;

float SoC;
// On startup, look up estimated SoC based on current voltage, assuming current is (close to) zero.
// After initial SoC has been measured begin coloumb counting.

void run_model_tests() {
  unsigned long timer = micros();
  BattModel *test_model = new BattModel(1.39, 7, 1.18, 6.25, 0.002, 1.28, 1.3, 1.3, 0.8, 30);
  timer = micros() - timer;
  u8g2->begin();
  u8g2->clearBuffer();
  u8g2->setFont(u8g2_font_profont17_mr);
  u8g2->setCursor(0, 49);
  u8g2->print("Model Tests");
  u8g2->sendBuffer();
  delay(500);
  u8g2->clearBuffer();
  u8g2->setFont(u8g2_font_profont17_mr);
  u8g2->setCursor(0, u8g2->getMaxCharHeight());
  u8g2->print("Time to initialise:");
  u8g2->setCursor(0, (u8g2->getMaxCharHeight() * 2));
  u8g2->print(timer);
  u8g2->print("micros");
  u8g2->sendBuffer();
  delay(3000);
  u8g2->clearBuffer();
  u8g2->setCursor(0, u8g2->getMaxCharHeight());
  u8g2->print("K=");
  u8g2->print(test_model->GetK(), 8);
  u8g2->setCursor(0, (u8g2->getMaxCharHeight() * 2));
  u8g2->print("test=");
  timer = micros();
  u8g2->print(test_model->Simulate(3.5, 15, 30), 8);
  timer = micros() - timer;
  u8g2->setCursor(0, (u8g2->getMaxCharHeight() * 3));
  u8g2->print("Time:");
  u8g2->print(timer);
  u8g2->print("micros");
  u8g2->sendBuffer();
  delay(100000);
};

void setup() {
  // run_model_tests();
  if (sd_logging == 1) {
    if (!SD.begin(15)) {
      while(1){};
    } else {
      while (SD.exists(String(log_counter) + ".csv")) {
        log_counter++;
      }
      log_file = SD.open((String(log_counter) + ".csv"), FILE_WRITE);

      // if (log_file) {
      //   log_file.println("BMS Logging");
      //   log_file.close();
      // }
    }
  }

  if (use_display == 1) {
    u8g2->begin();
    u8g2->clearBuffer();

    if (serial_timeout > 0) {
      u8g2->setFont(u8g2_font_profont17_mr);
      u8g2->setCursor(0, 49);
      u8g2->print("Connecting...");
      u8g2->sendBuffer();

      Serial.begin(115200);
      long start_time = millis();
      while ((millis() - start_time) < (serial_timeout * 1000)) {
        if (Serial.readString() == "hello") {
          serial_mode = true;
          break;
        }
      };
    }

    if (serial_mode) {
      Serial.println("BMS");
      u8g2->clearBuffer();
      u8g2->setCursor(0, 49);
      u8g2->print("Connected!");
      u8g2->sendBuffer();
      delay(1000);
      u8g2->clearBuffer();
      u8g2->sendBuffer();
    } else {
      u8g2->setFont(u8g2_font_inr46_mf);
      u8g2->setCursor(0, 49);
      u8g2->print("VLT");
      u8g2->sendBuffer();
      delay(500);
    }

    if(!ina226.init()) {
      u8g2->clearBuffer();
      u8g2->setFont(u8g2_font_profont17_mr);
      u8g2->setCursor(0, 49);
      u8g2->print("INA226 INIT FAIL");
      u8g2->sendBuffer();
      while(1){}
    }
  } else {
    // seemingly required to run in all cases
    u8g2->begin();

    if (serial_timeout > 0) {
      Serial.begin(115200);
      long start_time = millis();
      while ((millis() - start_time) < (serial_timeout * 1000)) {
        if (Serial.readString() == "hello") {
          serial_mode = true;
          break;
        }
      };
    }

    if (serial_mode) {
      Serial.println("BMS");
    }

    if(!ina226.init()) {
      while(1){}
    }
  }

  ina226.setConversionTime(CONV_TIME_2116);
  ina226.setAverage(AVERAGE_16);

  ina3221.begin();
  ina3221.reset();
  ina3221.setBusConversionTime(INA3221_REG_CONF_CT_2116US);
  ina3221.setAveragingMode(INA3221_REG_CONF_AVG_16);
}

void loop() {
  u8g2->clearBuffer();
  u8g2->setFont(u8g2_font_profont17_mr);
  u8g2->setCursor(0, 49);
  u8g2->print(micros() - cur_time);
  u8g2->sendBuffer();
  cur_time = micros();
  // If simulation is running, timing may be off for averaging, as certain readings will be skipped, so maybe increase averaging onboard ina226 to be nearer to time to run simulation
  ina226.waitUntilConversionCompleted();  // Ensures at least 1 reading per addition to average
  busVoltage_V = ina226.getBusVoltage_V();
  shunt_voltage = ina226.getShuntVoltage_mV();
  voltage[0] = ina3221.getVoltage(INA3221_CH1);
  voltage[1] = ina3221.getVoltage(INA3221_CH2);
  voltage[2] = ina3221.getVoltage(INA3221_CH3);
  current_mA = (shunt_voltage / current_scale) * 10000 + (current_offset);

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
    // u8g2->print((busVoltage_V * current_A), 0);
    // u8g2->print("W  ");
    // u8g2->print(Wh_charged, 2);
    // u8g2->print("Wh");
    u8g2->sendBuffer();
  }
}