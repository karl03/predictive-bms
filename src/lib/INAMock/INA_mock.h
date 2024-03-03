// Enums taken from INA226_WE: https://github.com/wollewald/INA226_WE?tab=MIT-1-ov-file
// MIT License

// Copyright (c) 2020 Wolfgang (Wolle) Ewald

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
        FsFile file_;   // CSV file in format: time (ms), bus voltage (V), shunt voltage (mV)
        const char *file_path_;
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
        INA_mock(SdFs *sd, const char *file_path) {sd_ = sd; file_path_ = file_path; conversion_time_ = CONV_TIME_1100; averages_ = AVERAGE_1;}
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