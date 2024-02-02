#ifndef BATT_MONITOR_H
#define BATT_MONITOR_H

#include <cmath>
#include "batt_model.h"
#include "LPF.h"
#include "resistance_estimate.h"

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
            float filtered_current;     // Low-pass filtered current in mA
            float cell_voltages[4];     // Per-cell voltage in V
            float cell_z_scores[4];
            float mAh_used;
            float mWh_used;
            unsigned long last_update;  // Last update of values in microseconds
            flags batt_flags;
        };

        BattMonitor(State* state, BattModel* batt_model, int reaction_time);
        void updateConsumption(float time_delta, float voltage, float current_mA, float cell_voltages[4]);
        void resetFilter(float current) {lpf_->SetInitialParams(current);}
        float getResistance() {return resistance_estimate_->getResistance();}
    
    private:
        void updateCellDifferences();
        State *state_;
        BattModel *batt_model_;
        LPF *lpf_;
        ResistanceEstimate *resistance_estimate_;
};

#endif