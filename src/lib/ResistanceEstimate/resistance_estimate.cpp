// Code from Ardupilot (https://github.com/ArduPilot/ardupilot/) used under GPL3 license

#include "resistance_estimate.h"

void ResistanceEstimate::update_resistance_estimate(float current_amps, float voltage, unsigned long now) {
    // return immediately if no current
    if (current_amps <= 0) {
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
    float loop_interval = (now - resistance_timer_ms_) * 0.001f;
    resistance_timer_ms_ = now;

    // estimate short-term resistance
    float filt_alpha = constrain_float(loop_interval/(loop_interval + AP_BATT_MONITOR_RES_EST_TC_1), 0.0f, 0.5f);    // constrain_float(val, min, max) returns val limited to min/ max
    float resistance_alpha = MIN(1, AP_BATT_MONITOR_RES_EST_TC_2*fabsf((_state.current_amps-_current_filt_amps)/_current_max_amps));
    float resistance_estimate = -(_state.voltage-_voltage_filt)/current_delta;
    if (is_positive(resistance_estimate)) {
        _state.resistance = _state.resistance*(1-resistance_alpha) + resistance_estimate*resistance_alpha;
    }

    // calculate maximum resistance
    if ((_resistance_voltage_ref > _state.voltage) && (_state.current_amps > _resistance_current_ref)) {
        float resistance_max = (_resistance_voltage_ref - _state.voltage) / (_state.current_amps - _resistance_current_ref);
        _state.resistance = MIN(_state.resistance, resistance_max);
    }

    // update the filtered voltage and currents
    _voltage_filt = _voltage_filt*(1-filt_alpha) + _state.voltage*filt_alpha;
    _current_filt_amps = _current_filt_amps*(1-filt_alpha) + _state.current_amps*filt_alpha;

    // update estimated voltage without sag
    _state.voltage_resting_estimate = _state.voltage + _state.current_amps * _state.resistance;
}