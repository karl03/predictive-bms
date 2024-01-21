#include <cmath>

// Time constants for filters
#define AP_BATT_MONITOR_RES_EST_TC_1        0.5f
#define AP_BATT_MONITOR_RES_EST_TC_2        0.1f

class ResistanceEstimate {
    private:
        float filtered_current_;
        unsigned long current_time_;
        int reaction_time_;
        float current_max_amps_;
        float current_filt_amps_;
        float voltage_filt_;
        float resistance_voltage_ref_;
        float resistance_current_ref_;
        unsigned long resistance_timer_ms_;

    public:
        float resistance;
        float voltage_resting_estimate_;
        ResistanceEstimate();
        void update_resistance_estimate(float current_amps, float voltage, unsigned long now);
};