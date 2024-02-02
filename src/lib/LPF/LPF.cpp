#include "LPF.h"

float LPF::Update(unsigned long time_delta, float current) {
    float filter_val = exp(- ((double) time_delta) / (double) reaction_time_);

    filtered_current_ = filtered_current_ * (1 - filter_val) + current * filter_val;
    return filtered_current_;
};