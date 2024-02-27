#include "curr_estimator.h"

CurrEstimator::CurrEstimator(SdFs *sd, const char *file_path, unsigned int long_decay_s, unsigned int short_decay_s) {
    sd_ = sd;
    file_path_ = file_path;
    long_decay_s_ = long_decay_s;
    short_decay_s_ = short_decay_s;
    first_run_ = 1;
    if (sd_->exists(file_path_)) {
        file_.open(file_path_, O_READ);
        char cur_char = file_.read();
        char chars[64];
        chars[0] = '\0';
        int index = 0;

        while (cur_char != 255 && cur_char != -1 && cur_char != '\n' && cur_char != '\n') {
            chars[index] = cur_char;
            chars[index + 1] = '\0';
            cur_char = file_.read();
            index ++;
        }

        if (strlen(chars) > 0) {
            long_term_avg_ = atof(chars);
            first_run_ = 0;
        } else {
            long_term_avg_ = .0f;
            file_.print(.0f);
            file_.flush();
        }
        file_.close();
    } else {
        file_.open(file_path_, FILE_WRITE);
        file_.print(.0f);
        file_.flush();
        file_.close();
        long_term_avg_ = .0f;
    }

    short_term_avg_ = long_term_avg_;
}

float CurrEstimator::updateAvg(float current_mA, unsigned long time_delta_micros) {
    if (long_term_avg_ == .0f) {
        long_term_avg_ = current_mA;
        short_term_avg_ = current_mA;
        return current_mA;
    }
    float time_delta_s = time_delta_micros * 0.000001f;  // Microseconds to seconds
    float short_alpha = time_delta_s / short_decay_s_;
    float long_alpha = time_delta_s / long_decay_s_;
    if (long_alpha >= 1) {
        long_term_avg_ = current_mA;
        short_term_avg_ = current_mA;
    } else if (short_alpha >= 1) {
        short_term_avg_ = current_mA;
        long_term_avg_ =  long_alpha * current_mA + (1 - long_alpha) * long_term_avg_;
    } else {
        short_term_avg_ = short_alpha * current_mA + (1 - short_alpha) * short_term_avg_;
        long_term_avg_ = long_alpha * current_mA + (1 - long_alpha) * long_term_avg_;
    }

    return long_term_avg_;
}

int CurrEstimator::writeLongTermAvg() {
    if (!file_.open(file_path_, FILE_WRITE)) {
        return 0;
    }
    if (!file_.remove()) {
        return 0;
    }
    if (!file_.open(file_path_, FILE_WRITE)) {
        return 0;
    }
    if (first_run_) {
        file_.print(short_term_avg_);    
    } else {
        file_.print(long_term_avg_);
    }
    file_.flush();
    file_.close();
    return 1;
}