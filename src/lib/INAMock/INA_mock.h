#ifndef INA_MOCK_H
#define INA_MOCK_H

#include <Arduino.h>
#include <SD.h>

typedef enum INA_mock_AVERAGES{
    AVERAGE_1       = 1, 
    AVERAGE_4       = 4,
    AVERAGE_16      = 16,
    AVERAGE_64      = 64,
    AVERAGE_128     = 128,
    AVERAGE_256     = 256,
    AVERAGE_512     = 512,
    AVERAGE_1024    = 1024
} mockAverageMode;

typedef enum INA226_mock_CONV_TIME{ // Conversion time in microseconds
    CONV_TIME_140   = 140,
    CONV_TIME_204   = 204,
    CONV_TIME_332   = 332,
    CONV_TIME_588   = 588,
    CONV_TIME_1100  = 1100,
    CONV_TIME_2116  = 2116,
    CONV_TIME_4156  = 4156,
    CONV_TIME_8244  = 8244
} mockConvTime;

class INA_mock
{
private:
    File file_;
    String file_path_;
    int conversion_time_;
    int averages_;
    unsigned long last_update_;
    int bus_read_;
    int shunt_read_;
    float cur_bus_v_;
    float cur_shunt_mv_;
    float readFloat();
    void moveToNextLine();
public:
    INA_mock(String file_path) {file_path_ = file_path; conversion_time_ = CONV_TIME_1100; averages_ = AVERAGE_1;}
    int init();
    void setConversionTime(mockConvTime conversion_time) {conversion_time_ = conversion_time;}
    void setAverage(mockAverageMode averages) {averages_ = averages;}
    int isBusy();
    void waitUntilConversionCompleted();
    float getBusVoltage_V();
    float getShuntVoltage_mV();
};


#endif