#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <INA226_WE.h>
#include <INA3221.h>

U8G2 *u8g2 = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
INA226_WE ina226 = INA226_WE(0x40);
INA3221 ina3221(INA3221_ADDR41_VCC);

void setup() {

  u8g2->begin();
  u8g2->clearBuffer();
  u8g2->setFont(u8g2_font_inr46_mf);
  u8g2->setCursor(0, 49);
  u8g2->print("VLT");
  u8g2->sendBuffer();
  delay(500);

  if(!ina226.init()){
  u8g2->clearBuffer();
  u8g2->setFont(u8g2_font_profont17_mr);
  u8g2->setCursor(0, 49);
  u8g2->print("INA226 INIT FAIL");
  u8g2->sendBuffer();
    while(1){}
  }

  ina3221.begin();
  ina3221.reset();

}

void loop() {
  float busVoltage_V = 0.0;
  float voltage[3];

  busVoltage_V = ina226.getBusVoltage_V();
  voltage[0] = ina3221.getVoltage(INA3221_CH1);
  voltage[1] = ina3221.getVoltage(INA3221_CH2);
  voltage[2] = ina3221.getVoltage(INA3221_CH3);

  u8g2->clearBuffer();
  u8g2->setFont(u8g2_font_profont17_mr);
  u8g2->setCursor(0, u8g2->getMaxCharHeight());
  u8g2->print(voltage[0]);
  u8g2->print("V  ");
  u8g2->print(voltage[1]);
  u8g2->print("V");
  u8g2->setCursor(0, (u8g2->getMaxCharHeight() * 2));
  u8g2->print(voltage[2]);
  u8g2->print("V  ");
  u8g2->print(busVoltage_V);
  u8g2->print("V");
  u8g2->sendBuffer();

  delay(1000);
}