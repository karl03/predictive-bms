#ifndef LPF_H
#define LPF_H

#include <cmath>
// Basic low-pass filter, this implementation also known as "leaky integrator"
class LPF {
    private:
        float filtered_current_;
        int reaction_time_;

    public:
        LPF(int reaction_time, float initial_current = 0) {reaction_time_ = reaction_time, filtered_current_ = initial_current;}
        void SetInitialParams(float current) {filtered_current_ = current;}
        float Update(unsigned long time_delta, float current);
};

#endif