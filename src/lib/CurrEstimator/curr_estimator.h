#ifndef CURR_ESTIMATOR_H
#define CURR_ESTIMATOR_H

#include <SdFat.h>

class CurrEstimator {
    public:
        CurrEstimator(SdFs *sd, const char *file_path, unsigned int long_decay_s, unsigned int short_decay_s);
        float updateAvg(float current_mA, unsigned long time_delta_micros);
        float getLongAvg() {return long_term_avg_;}
        float getShortAvg() {return short_term_avg_;}
        int writeLongTermAvg();
    
    private:
        SdFs *sd_;
        FsFile file_;
        const char *file_path_;
        float long_term_avg_;
        float short_term_avg_;
        unsigned int long_decay_s_;
        unsigned int short_decay_s_;
        bool first_run_;
        bool use_file_;
};

#endif