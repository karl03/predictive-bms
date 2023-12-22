#include "LPF.h"

float LPF::Update(unsigned long time, float current) {
    float filter_val = exp(- ((double) time - (double) current_time_) / (double) reaction_time_);
    current_time_ = time;

    filtered_current_ = filtered_current_ * (1 - filter_val) + current * filter_val;
    return filtered_current_;
};