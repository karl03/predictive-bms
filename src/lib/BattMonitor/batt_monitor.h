#ifndef BATT_MONITOR_H
#define BATT_MONITOR_H

#include <cmath>
#include "batt_model.h"
#include "LPF.h"
#include "resistance_estimate.h"

#define A_ms_to_A_h 0.00000027777777777778

class BattMonitor {
    public:
        enum {
            EMPTY = 1,
            OVERCHARGED = 2,
            UNDERPERFORMING = 4,
            IMBALANCED = 8
        } flags;

        BattMonitor(float voltage, float current, float cell_voltages[4], float mAh_used, float mWh_used, unsigned long time_micros, BattModel* batt_model, int reaction_time, float max_voltage_variance, float max_cell_variance);
        void updateConsumption(unsigned long time_micros, float voltage, float current_mA, float cell_voltages[4]);
        void resetFilter(float current) {lpf_->SetInitialParams(current);}
        float getResistance() {return resistance_estimate_->getResistance();}
    
    private:
        struct State {
            float voltage;              // Voltage in V
            float current;              // Current in mA
            float filtered_current;     // Low-pass filtered current in mA
            float cell_voltages[4];     // Per-cell voltage in V
            float cell_z_scores[4];
            float mAh_used;
            float mWh_used;
            unsigned long last_update;  // Last update of values in microseconds
            int batt_flags;
        };

        void updateCellDifferences();
        State *state_;
        BattModel *batt_model_;
        LPF *lpf_;
        ResistanceEstimate *resistance_estimate_;
        float max_voltage_variance_;
        float max_cell_variance_;
};

#endif