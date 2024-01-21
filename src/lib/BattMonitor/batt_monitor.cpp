#include "batt_monitor.h"

void BattMonitor::update_consumption(float time_micros, float voltage, float current_mA, float cell_voltages[4]) {
    float mAh_used = (current_mA * ((state_->last_update - time_micros) * 0.001) * A_ms_to_A_h);
    state_->mAh_used += mAh_used;
    state_->mWh_used += (mAh_used * voltage);

    float mean_cell_voltage = voltage * 0.25;
    float standard_deviation;
    for (int cell = 0; cell < 4; cell++) {
        state_->cell_voltages[cell] = cell_voltages[cell];
        standard_deviation += pow((cell_voltages[cell] - mean_cell_voltage), 2);
    }
    standard_deviation = pow((standard_deviation / 4), 0.5);

    for (int cell = 0; cell < 4; cell++) {
        state_->cell_z_scores[cell] = (cell_voltages[cell] - mean_cell_voltage) / standard_deviation;
    }

};