#ifndef BATT_MONITOR_H
#define BATT_MONITOR_H

#include <cmath>
#include "batt_model.h"

#define A_ms_to_A_h 0.00000027777777777778

class BattMonitor {
    public:
        typedef enum {
            EMPTY = 1,
            OVERCHARGED = 2,
            IMBALANCED = 4
        } flags;

        struct State {
            float voltage;              // Voltage in V
            float current;              // Current in mA
            float cell_voltages[4];     // Per-cell voltage in V
            float cell_z_scores[4];
            float mAh_used;
            float mWh_used;
            unsigned long last_update;  // Last update of values in microseconds
            flags flags_;
        };

        BattMonitor(State* state, BattModel* batt_model) {state_ = state; batt_model_ = batt_model;};
        void update_consumption(float time_delta, float voltage, float current_mA, float cell_voltages[4]);
    
    private:
        float resistance_;
        State* state_;
        BattModel* batt_model_;
};

#endif