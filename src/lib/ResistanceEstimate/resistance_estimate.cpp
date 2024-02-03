// Code from Ardupilot (https://github.com/ArduPilot/ardupilot/) used under GPL3 license

#include "resistance_estimate.h"

void ResistanceEstimate::updateResistanceEstimate(float current_amps, float voltage, unsigned long time_delta_micros) {
    // return immediately if no current
    if (!isPositive(current_amps)) {
        return;
    }

    // update maximum current seen since startup and protect against divide by zero
    current_max_amps_ = current_max_amps_ > current_amps ? current_max_amps_ : current_amps;
    float current_delta = current_amps - current_filt_amps_;
    if (current_delta == 0) {
        return;
    }

    // update reference voltage and current
    if (voltage > resistance_voltage_ref_) {
        resistance_voltage_ref_ = voltage;
        resistance_current_ref_ = current_amps;
    }

    // calculate time since last update
    float loop_interval_s = time_delta_micros * 0.000001f;

    // estimate short-term resistance
    float filt_alpha = constrainFloat(loop_interval_s/(loop_interval_s + AP_BATT_MONITOR_RES_EST_TC_1), 0.0f, 0.5f);
    float resistance_alpha = 1 < AP_BATT_MONITOR_RES_EST_TC_2*fabsf((current_amps-current_filt_amps_)/current_max_amps_) ? 1 : AP_BATT_MONITOR_RES_EST_TC_2*fabsf((current_amps-current_filt_amps_)/current_max_amps_);
    float resistance_estimate = -(voltage-voltage_filt_)/current_delta;
    if (isPositive(resistance_estimate)) {
        resistance_ = resistance_*(1-resistance_alpha) + resistance_estimate*resistance_alpha;
    }

    // calculate maximum resistance
    if ((resistance_voltage_ref_ > voltage) && (current_amps > resistance_current_ref_)) {
        float resistance_max = (resistance_voltage_ref_ - voltage) / (current_amps - resistance_current_ref_);
        resistance_ = resistance_ < resistance_max ? resistance_ : resistance_max;
    }

    // update the filtered voltage and currents
    voltage_filt_ = voltage_filt_*(1-filt_alpha) + voltage*filt_alpha;
    current_filt_amps_ = current_filt_amps_*(1-filt_alpha) + current_amps*filt_alpha;

    // update estimated voltage without sag
    voltage_resting_estimate_ = voltage + current_amps * resistance_;
}

float ResistanceEstimate::constrainFloat(float val, float min, float max) {
    if (val > max) {
        return max;
    } else if (val < min) {
        return min;
    } else {
        return val;
    }
}

bool ResistanceEstimate::isPositive(float val) {
    return (val >= __FLT_EPSILON__);
}