#include "INA_mock.h"

int INA_mock::init() {
    if (sd_.exists(file_path_)) {
        file_.open(file_path_.c_str(), O_READ);
        last_update_ = micros();
        return 1;
    } else {
        return 0;
    }
}

int INA_mock::isBusy() {
    if (micros() - last_update_ >= conversion_time_ * averages_) {
        moveToNextLine();
        last_update_ = micros();
        return 0;
    } else {
        return 1;
    }
}

void INA_mock::waitUntilConversionCompleted() {
    if (micros() - last_update_ < conversion_time_ * averages_) {
        delayMicroseconds((conversion_time_ * averages_) - (micros() - last_update_));
    }
    moveToNextLine();
    last_update_ = micros();
}

float INA_mock::getBusVoltage_V()
{
    if (bus_read_ || shunt_read_) {
        return cur_bus_v_;
    } else {
        cur_bus_v_ = readFloat();
        bus_read_ = 1;
        return cur_bus_v_;
    }
}

float INA_mock::getShuntVoltage_mV()
{
    if (shunt_read_) {
        return cur_shunt_mv_;
    } else if (bus_read_) {
        cur_shunt_mv_ = readFloat();
        shunt_read_ = 1;
        return cur_shunt_mv_;
    } else {
        cur_bus_v_ = readFloat();
        bus_read_ = 1;
        cur_shunt_mv_ = readFloat();
        shunt_read_ = 1;
        return cur_shunt_mv_;
    }
}

float INA_mock::readFloat() {
    if (file_.available()) {
        char cur_char = file_.read();
        char chars[64];
        int index = 0;

        while (cur_char != ',' && cur_char != '\n') {
            chars[index] = cur_char;
            chars[index + 1] = '\0';
            cur_char = file_.read();
        }

        if (strlen(chars) > 0) {
            return atof(chars);
        }
    }

    return .0f;
}

void INA_mock::moveToNextLine() {
    if (shunt_read_) {
        bus_read_ = 0;
        shunt_read_ = 0;
    } else if (bus_read_) {
        bus_read_ = 0;
        readFloat();    // Move to next line
    } else {
        readFloat();
        readFloat();    // Move to next line
    }
}
/*
Have only one file ever for mocking, like mocking.csv, check if this exists
maybe with preprocessor stuff decide whether to mock or not? or just have a user define and check this
*/