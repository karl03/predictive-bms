#ifndef LPF_H
#define LPF_H

#include <cmath>

class LPF {
    private:
        float filtered_current_;
        unsigned long current_time_;
        int reaction_time_;

    public:
        LPF(int reaction_time, unsigned long initial_time = 0, float initial_current = 0) {reaction_time_ = reaction_time, current_time_ = initial_time; filtered_current_ = initial_current;}
        void SetInitialParams(unsigned long time, float current) {current_time_ = time; filtered_current_ = current;}
        float Update(unsigned long time, float current);
};

#endif