#include "batt_monitor.h"

BattMonitor::BattMonitor(float voltage, float current, float cell_voltages[4], float mAh_used, float mWh_used, unsigned long time_micros, BattModel* batt_model, int reaction_time) {
    state_->voltage = voltage;
    state_->current = current;
    state_->filtered_current = current;
    for (int i = 0; i < 4; i++) {
        state_->cell_voltages[i] = cell_voltages[i];
    }
    state_->mAh_used = mAh_used;
    state_->mWh_used = mWh_used;
    state_->last_update = time_micros;
    state_->batt_flags = 0;

    batt_model_ = batt_model;
    lpf_ = new LPF(reaction_time, state_->current);
    resistance_estimate_ = new ResistanceEstimate();
    updateCellDifferences();
}

void BattMonitor::updateConsumption(unsigned long time_micros, float voltage, float current_mA, float cell_voltages[4]) {
    state_->voltage = voltage;
    state_->current = current_mA;
    for (int i = 0; i < 4; i++) {
        state_->cell_voltages[i] = cell_voltages[i];
    }
    updateCellDifferences();
    float mAh_used = (current_mA * ((state_->last_update - time_micros) * 0.001) * A_ms_to_A_h);
    state_->mAh_used += mAh_used;
    state_->mWh_used += (mAh_used * voltage);

    /*
    Check voltage against simulation given initial parameters, then check against simulation given calculated resistance.
    If check against original simulation lines up, performance is as expected.
    If it doesn't, but check against sim with calculated resistance lines up, then likely IR is higher, capacity is similar.
    Otherwise, if worse than both, battery has worse IR and lower capacity. In this case, if possible run sim with decreasing
    capacity until the performance roughly matches the real performance, from this deduce capacity estimate.
    */
   state_->last_update = time_micros;
}

// Run after updating voltages, to update cell difference checks
void BattMonitor::updateCellDifferences() {
    float mean_cell_voltage = state_->voltage * 0.25;
    float standard_deviation;
    for (int cell = 0; cell < 4; cell++) {
        state_->cell_voltages[cell];
        standard_deviation += pow((state_->cell_voltages[cell] - mean_cell_voltage), 2);
    }
    standard_deviation = pow((standard_deviation * 0.25), 0.5);

    for (int cell = 0; cell < 4; cell++) {
        state_->cell_z_scores[cell] = (state_->cell_voltages[cell] - mean_cell_voltage) / standard_deviation;
    }
}