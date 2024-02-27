#include "INA_mock.h"

int INA_mock::init() {
    if (sd_->exists(file_path_)) {
        file_.open(file_path_, O_READ);
        last_update_ = micros();
        next_time_ = readUnsignedLong();
        cur_time_ = next_time_;
        time_read_ = 1;
        return 1;
    } else {
        return 0;
    }
}

int INA_mock::isBusy() {
    if ((micros() - last_update_) >= ((next_time_ - cur_time_) * 1000)) {
        moveToNextLine();
        last_update_ = micros();
        return 0;
    } else {
        return 1;
    }
}

void INA_mock::waitUntilConversionCompleted() {
    if ((micros() - last_update_) < ((next_time_ - cur_time_) * 1000)) {
        delayMicroseconds(((next_time_ - cur_time_) * 1000) - (micros() - last_update_));
    }
    moveToNextLine();
    last_update_ = micros();
}

float INA_mock::getBusVoltage_V()
{
    if (bus_read_) {
        return cur_bus_v_;
    } else if (time_read_) {
        cur_bus_v_ = readFloat();
        bus_read_ = 1;
        return cur_bus_v_;
    } else {
        cur_time_ = next_time_;
        next_time_ = readUnsignedLong();
        time_read_ = 1;
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
    } else if (time_read_) {
        cur_bus_v_ = readFloat();
        bus_read_ = 1;
        cur_shunt_mv_ = readFloat();
        shunt_read_ = 1;
        return cur_shunt_mv_;
    } else {
        cur_time_ = next_time_;
        next_time_ = readUnsignedLong();
        time_read_ = 1;
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
        chars[0] = '\0';
        int index = 0;

        while (cur_char != -1 && cur_char != ',' && cur_char != '\n') {
            chars[index] = cur_char;
            chars[index + 1] = '\0';
            cur_char = file_.read();
            index ++;
        }

        if (strlen(chars) > 0) {
            return atof(chars);
        }
    }

    return .0f;
}

unsigned long INA_mock::readUnsignedLong() {
    if (file_.available()) {
        char cur_char = file_.read();
        char chars[64];
        chars[0] = '\0';
        int index = 0;

        while (cur_char != -1 && cur_char != ',' && cur_char != '\n') {
            chars[index] = cur_char;
            chars[index + 1] = '\0';
            cur_char = file_.read();
            index ++;
        }

        if (strlen(chars) > 0) {
            return atol(chars);
        }
    }

    return 0;
}

void INA_mock::moveToNextLine() {
    if (shunt_read_) {
        cur_time_ = next_time_;
        next_time_ = readUnsignedLong();
        time_read_ = 1;
        bus_read_ = 0;
        shunt_read_ = 0;
    } else if (bus_read_) {
        bus_read_ = 0;
        readFloat();
        cur_time_ = next_time_;
        next_time_ = readUnsignedLong();
        time_read_ = 1;
    } else if (time_read_) {
        readFloat();
        readFloat();
        cur_time_ = next_time_;
        next_time_ = readUnsignedLong();
        time_read_ = 1;
    } else {
        readUnsignedLong();
        readFloat();
        readFloat();
        cur_time_ = next_time_;
        next_time_ = readUnsignedLong();
        time_read_ = 1;
    }
}
/*
Have only one file ever for mocking, like mocking.csv, check if this exists
maybe with preprocessor stuff decide whether to mock or not? or just have a user define and check this
*/