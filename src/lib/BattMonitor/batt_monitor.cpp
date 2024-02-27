#include "batt_monitor.h"

BattMonitor::BattMonitor(float voltage, float current, float cell_voltages[4], float mAh_used, float mWh_used, unsigned long time_micros, BattModel* batt_model, BattModel* batt_model_modifiable, int reaction_time, float max_voltage_variance, float max_cell_variance, float capacity_step_percentage) {
    max_voltage_variance_ = max_voltage_variance;
    max_cell_variance_ = max_cell_variance;
    capacity_step_percentage_ = capacity_step_percentage;

    state_ = new State();

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
    modified_batt_model_ = batt_model_modifiable;
    state_->estimated_capacity = batt_model_->GetCapacity();
    float model_voltage;
    model_voltage = batt_model_->Simulate(state_->mAh_used * 0.001, state_->current * 0.001, state_->filtered_current * 0.001);
    voltage_diff_ = state_->voltage - model_voltage;

    lpf_ = new LPF(reaction_time, state_->current);
    resistance_estimate_ = new ResistanceEstimate();
    updateCellDifferences();
}

void BattMonitor::updateConsumption(unsigned long time_micros, unsigned long max_time, float voltage, float current_mA, float cell_voltages[4]) {
    float model_voltage;

    state_->voltage = voltage;
    state_->current = current_mA;
    for (int i = 0; i < 4; i++) {
        state_->cell_voltages[i] = cell_voltages[i];
    }
    updateCellDifferences();
    float step_mAh_used = (current_mA * ((time_micros - state_->last_update) * 0.001) * A_ms_to_A_h);
    state_->mAh_used += step_mAh_used;
    state_->mWh_used += (step_mAh_used * voltage);
    state_->filtered_current = lpf_->Update(time_micros - state_->last_update, current_mA); // BUG: Filtered current is not updating!
    resistance_estimate_->updateResistanceEstimate((state_->current * 0.001), state_->voltage, (time_micros - state_->last_update));

    model_voltage = batt_model_->Simulate(state_->mAh_used * 0.001, state_->current * 0.001, state_->current * 0.001);
    voltage_diff_ = model_voltage - state_->voltage;

    if (voltage_diff_ > max_voltage_variance_) {
        state_->batt_flags = state_->batt_flags | UNDERPERFORMING;
    }

    modified_batt_model_->SetInternalResistance(resistance_estimate_->getResistanceOhms());
    fitted_voltage_diff_ = modified_batt_model_->Simulate(state_->mAh_used * 0.001, state_->current * 0.001, state_->current * 0.001) - state_->voltage;

    if (fitted_voltage_diff_ > max_voltage_variance_) {
        state_->batt_flags = state_->batt_flags | LOW_CAPACITY;
        if (fitted_voltage_diff_ > max_voltage_variance_) {
            state_->estimated_capacity -= (batt_model_->GetCapacity() * (capacity_step_percentage_ * 0.01));
            modified_batt_model_->SetCapacity(state_->estimated_capacity);
            fitted_voltage_diff_ = modified_batt_model_->Simulate(state_->mAh_used * 0.001, state_->current * 0.001, state_->filtered_current * 0.001) - state_->voltage;
        }
        
    } else {
        state_->batt_flags = state_->batt_flags | HIGH_RESISTANCE;
    }

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
        standard_deviation += pow((state_->cell_voltages[cell] - mean_cell_voltage), 2);
    }
    standard_deviation = pow((standard_deviation * 0.25), 0.5);

    for (int cell = 0; cell < 4; cell++) {
        state_->cell_z_scores[cell] = (state_->cell_voltages[cell] - mean_cell_voltage) / standard_deviation;
    }
}