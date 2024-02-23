#ifndef REISTANCE_ESTIMATE_H
#define REISTANCE_ESTIMATE_H

#include <cmath>

// Time constants for filters
#define AP_BATT_MONITOR_RES_EST_TC_1        0.5f
#define AP_BATT_MONITOR_RES_EST_TC_2        0.1f

class ResistanceEstimate {
    private:
        unsigned long current_time_;
        float current_max_amps_;
        float current_filt_amps_;
        float voltage_filt_;
        float resistance_voltage_ref_;
        float resistance_current_ref_;
        float resistance_;
        float voltage_resting_estimate_;
        bool isPositive(float val);
        float constrainFloat(float val, float min, float max);

    public:
        float getResistanceOhms() {return resistance_;}
        ResistanceEstimate() {};
        void updateResistanceEstimate(float current_amps, float voltage, unsigned long time_delta_micros);
};

#endif