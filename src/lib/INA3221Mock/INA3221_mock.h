// Enums taken from INA3221: https://github.com/Tinyu-Zhao/INA3221
// MIT License

// Copyright (c) 2022 Tinyu Zhao

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

#ifndef INA3221_MOCK_H
#define INA3221_MOCK_H

#include <Arduino.h>
#include <SdFat.h>

// Channels
typedef enum {
    INA3221_CH1 = 0,
    INA3221_CH2,
    INA3221_CH3,
    INA3221_CH_NUM
} ina3221_mock_ch_t;

// Conversion times
typedef enum {
    INA3221_REG_CONF_CT_140US = 0,
    INA3221_REG_CONF_CT_204US,
    INA3221_REG_CONF_CT_332US,
    INA3221_REG_CONF_CT_588US,
    INA3221_REG_CONF_CT_1100US,
    INA3221_REG_CONF_CT_2116US,
    INA3221_REG_CONF_CT_4156US,
    INA3221_REG_CONF_CT_8244US
} ina3221_mock_conv_time_t;

// Averaging modes
typedef enum {
    INA3221_REG_CONF_AVG_1 = 0,
    INA3221_REG_CONF_AVG_4,
    INA3221_REG_CONF_AVG_16,
    INA3221_REG_CONF_AVG_64,
    INA3221_REG_CONF_AVG_128,
    INA3221_REG_CONF_AVG_256,
    INA3221_REG_CONF_AVG_512,
    INA3221_REG_CONF_AVG_1024
} ina3221_mock_avg_mode_t;

class INA3221_mock
{
    private:
        SdFs *sd_;
        FsFile file_;  // CSV file in format: voltage across cell 1, 2, 3, 4 (all in V)
        const char *file_path_;
        int conversion_time_;
        int averages_;
        int num_read_on_line_;
        float voltages_[4];
        float readFloat();
        void moveToNextLine();
    public:
        INA3221_mock(SdFs *sd, const char *file_path) {sd_ = sd; file_path_ = file_path; conversion_time_ = 0; averages_ = 0;}
        void begin();
        void reset() {};
        void setShuntMeasDisable() {};
        void setBusConversionTime(ina3221_mock_conv_time_t conversion_time) {conversion_time_ = conversion_time;}
        void setAveragingMode(ina3221_mock_avg_mode_t averages) {averages_ = averages;}
        void readFlags() {moveToNextLine(); num_read_on_line_ = 0;}
        bool getConversionReadyFlag() {return true;}
        float getVoltage(ina3221_mock_ch_t channel);
};


#endif