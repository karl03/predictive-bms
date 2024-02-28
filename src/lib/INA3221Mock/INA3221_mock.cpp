#include "INA3221_mock.h"

void INA3221_mock::begin() {
    if (sd_->exists(file_path_)) {
        file_.open(file_path_, O_READ);
        num_read_on_line_ = 0;
    }
}

float INA3221_mock::getVoltage(ina3221_mock_ch_t channel) {
    if (num_read_on_line_ >= channel + 1) {
        float total = 0;
        for (int counter = 0; counter <= channel; counter++) {
            total += voltages_[channel];
        }
        return total;
    } else {
        for (int counter=num_read_on_line_; counter <= channel; counter++) {
            voltages_[counter] = readFloat();
            num_read_on_line_++;
        }
        float total = 0;
        for (int counter = 0; counter <= channel; counter++) {
            total += voltages_[counter];
        }

        return total;
    }
}

float INA3221_mock::readFloat() {
    if (file_.available()) {
        char cur_char = file_.read();
        char chars[64];
        chars[0] = '\0';
        int index = 0;

        while (cur_char != 255 && cur_char != -1 && cur_char != ',' && cur_char != '\n') {
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

void INA3221_mock::moveToNextLine() {
    for (int counter=0; counter < 4 - num_read_on_line_; counter++) {
        readFloat();
    }
    num_read_on_line_ = 0;
}