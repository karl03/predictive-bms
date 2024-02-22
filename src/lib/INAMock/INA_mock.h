#ifndef INA_MOCK_H
#define INA_MOCK_H

#include <Arduino.h>
#include <SdFat.h>

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
    SdFs *sd_;
    FsFile file_;
    char *file_path_;
    unsigned long conversion_time_;
    unsigned long cur_time_;
    unsigned long next_time_;
    int averages_;
    unsigned long last_update_;
    int time_read_;
    int bus_read_;
    int shunt_read_;
    float cur_bus_v_;
    float cur_shunt_mv_;
    unsigned long readUnsignedLong();
    float readFloat();
    void moveToNextLine();
public:
    INA_mock(SdFs *sd, char *file_path) {sd_ = sd; file_path_ = file_path; conversion_time_ = CONV_TIME_1100; averages_ = AVERAGE_1;}
    int init();
    void setConversionTime(mockConvTime conversion_time) {conversion_time_ = conversion_time;}
    void setAverage(mockAverageMode averages) {averages_ = averages;}
    int isBusy();
    void readAndClearFlags() {time_read_ = 0; bus_read_ = 0; shunt_read_ = 0; last_update_ = micros();}
    void waitUntilConversionCompleted();
    float getBusVoltage_V();
    float getShuntVoltage_mV();
};


#endif