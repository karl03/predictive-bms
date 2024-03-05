#ifndef BATT_MONITOR_H
#define BATT_MONITOR_H

#include <Arduino.h>
#include <cmath>
#include "batt_model.h"
#include "LPF.h"
#include "resistance_estimate.h"

#define A_ms_to_A_h 0.00000027777777777778

class BattMonitor {
    public:
        enum flags {
            EMPTY = 1,
            OVERCHARGED = 2,
            UNDERPERFORMING = 4,
            IMBALANCED = 8,
            HIGH_RESISTANCE = 16,
            LOW_CAPACITY = 32,
        };

        BattMonitor(float voltage, float current, float cell_voltages[4], float mAh_used, float mWh_used, unsigned long time_micros, BattModel* batt_model, BattModel* batt_model_modifiable, int reaction_time_micros, float max_voltage_variance, float max_cell_variance, float capacity_step_percentage);
        void updateConsumption(unsigned long time_micros, float voltage, float current_mA, float cell_voltages[4]);
        void resetFilter(float current) {lpf_->SetInitialParams(current);}
        float getResistanceOhms() {return resistance_estimate_->getResistanceOhms();}
        float getEstimatedCapacity() {return state_->estimated_capacity;}
        float getSimVoltage() {return voltage_diff_ + state_->voltage;}
        float getSimVoltageDiff() {return voltage_diff_;}
        float getFittedSimVoltage() {return fitted_voltage_diff_ + state_->voltage;}
        float getFittedSimVoltageDiff() {return fitted_voltage_diff_;}
        float getmWhUsed() {return state_->mWh_used;}
        float getmAhUsed() {return state_->mAh_used;}
        float getEstmWhUsed() {return est_mWh_used_;}
        float getEstmAhUsed() {return est_mAh_used_;}
        float getFilteredCurrent() {return state_->filtered_current;}
        float getNominalVoltage() {return batt_model_->GetNominalVoltage();}
        int* getCellsStatus() {return state_->cell_status;}
    
    private:
        struct State {
            float voltage;              // Voltage in V
            float current;              // Current in mA
            float filtered_current;     // Low-pass filtered current in mA
            float cell_voltages[4];     // Per-cell voltage in V
            int cell_status[4];         // Per-cell status
            float mAh_used;
            float mWh_used;
            unsigned long last_update;  // Last update of values in microseconds
            int batt_flags;             // Overall battery flags
            float estimated_capacity;
        };

        void updateCellDifferences();
        State *state_;
        BattModel *batt_model_;
        BattModel *modified_batt_model_;
        LPF *lpf_;
        ResistanceEstimate *resistance_estimate_;
        float max_voltage_variance_;
        float max_cell_variance_;
        float voltage_diff_;
        float fitted_voltage_diff_;
        float capacity_step_percentage_;
        float est_initial_mAh_;
        float est_initial_mWh_;
        float est_mAh_used_;
        float est_mWh_used_;
        int voltage_hit_;
};

#endif