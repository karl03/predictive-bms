#include "LPF.h"

float LPF::Update(unsigned long time_delta, float current) {
    float filter_alpha = (float) time_delta / (float) reaction_time_;
    if (filter_alpha >= 1) {
        filtered_current_ = current;
    } else {
        filtered_current_ = filtered_current_ * (1 - filter_alpha) + current * filter_alpha;
    }

    return filtered_current_;
};