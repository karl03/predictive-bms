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
float current_A = 0.0;
float shunt_voltage = 0.0;
float Ws_charged = 0;
float Wh_charged = 0;
unsigned long cur_time = 0;
unsigned long prev_time = 0;
unsigned long last_avg = 0;
float v_iter = 0;
float A_iter = 0;
int iter_counter = 0;
int log_counter = 0;

float SoC;
// On startup, look up estimated SoC based on current voltage, assuming current is (close to) zero.
// After initial SoC has been measured begin coloumb counting.

void run_model_tests() {
  unsigned long timer = micros();
  BattModel *test_model = new BattModel(1.39, 7, 1.18, 6.25, 0.002, 1.28, 1.3, 1.3, 0.8);
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
  u8g2->print(test_model->Simulate(3.5, 15), 8);
  timer = micros() - timer;
  u8g2->setCursor(0, (u8g2->getMaxCharHeight() * 3));
  u8g2->print("Time:");
  u8g2->print(timer);
  u8g2->print("micros");
  u8g2->sendBuffer();
  delay(100000);
};

void setup() {
  run_model_tests();
  if (sd_logging == 1) {
    if (!SD.begin(15)) {
      while(1){};
    } else {
      while (SD.exists(String(log_counter) + ".csv")) {
        log_counter++;
      }
      log_file = SD.open((String(log_counter) + ".csv"), FILE_WRITE);

      if (log_file) {
        log_file.println("BMS Logging");
        log_file.close();
      }
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

  ina226.setResistorRange(0.00041229385, 200.0);
  ina226.setAverage(AVERAGE_256);

  ina3221.begin();
  ina3221.reset();
  prev_time = millis();
}

void loop() {
  cur_time = millis();

  busVoltage_V = ina226.getBusVoltage_V();
  voltage[0] = ina3221.getVoltage(INA3221_CH1);
  voltage[1] = ina3221.getVoltage(INA3221_CH2);
  voltage[2] = ina3221.getVoltage(INA3221_CH3);
  // current_mA = ina226.getCurrent_mA();
  shunt_voltage = ina226.getShuntVoltage_mV();
  current_A = (shunt_voltage / current_scale) * 10 + (current_offset / 1000);

  if (last_avg == 0) {
    last_avg = cur_time;
  } else if (iter_counter > 0 && (cur_time - last_avg) > avg_ms) {
    Wh_charged += ((A_iter / iter_counter) * (cur_time - last_avg) * A_ms_to_A_h) * (v_iter / iter_counter);
    last_avg = cur_time;
    v_iter = 0;
    A_iter = 0;
    iter_counter = 0;
  } else {
    v_iter += busVoltage_V;
    A_iter += current_A;
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
      // Shunt Voltage
      log_file.print(shunt_voltage);
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
    // u8g2->print(current_A, 3);
    // u8g2->print("A  ");
    u8g2->print((busVoltage_V * current_A), 0);
    u8g2->print("W  ");
    u8g2->print(Wh_charged, 2);
    u8g2->print("Wh");
    u8g2->sendBuffer();
  }
  
  // delay(100 - (cur_time - prev_time));

  prev_time = millis();
}