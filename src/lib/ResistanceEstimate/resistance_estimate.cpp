// Code from Ardupilot (https://github.com/ArduPilot/ardupilot/) used under GPL3 license
// void update_resistance_estimate() {
//     // return immediately if no current
//     if (!has_current() || !is_positive(_state.current_amps)) {
//         return;
//     }

//     // update maximum current seen since startup and protect against divide by zero
//     _current_max_amps = MAX(_current_max_amps, _state.current_amps);
//     float current_delta = _state.current_amps - _current_filt_amps;
//     if (is_zero(current_delta)) {
//         return;
//     }

//     // update reference voltage and current
//     if (_state.voltage > _resistance_voltage_ref) {
//         _resistance_voltage_ref = _state.voltage;
//         _resistance_current_ref = _state.current_amps;
//     }

//     // calculate time since last update
//     uint32_t now = AP_HAL::millis();
//     float loop_interval = (now - _resistance_timer_ms) * 0.001f;
//     _resistance_timer_ms = now;

//     // estimate short-term resistance
//     float filt_alpha = constrain_float(loop_interval/(loop_interval + AP_BATT_MONITOR_RES_EST_TC_1), 0.0f, 0.5f);    // constrain_float(val, min, max) returns val limited to min/ max
//     float resistance_alpha = MIN(1, AP_BATT_MONITOR_RES_EST_TC_2*fabsf((_state.current_amps-_current_filt_amps)/_current_max_amps));
//     float resistance_estimate = -(_state.voltage-_voltage_filt)/current_delta;
//     if (is_positive(resistance_estimate)) {
//         _state.resistance = _state.resistance*(1-resistance_alpha) + resistance_estimate*resistance_alpha;
//     }

//     // calculate maximum resistance
//     if ((_resistance_voltage_ref > _state.voltage) && (_state.current_amps > _resistance_current_ref)) {
//         float resistance_max = (_resistance_voltage_ref - _state.voltage) / (_state.current_amps - _resistance_current_ref);
//         _state.resistance = MIN(_state.resistance, resistance_max);
//     }

//     // update the filtered voltage and currents
//     _voltage_filt = _voltage_filt*(1-filt_alpha) + _state.voltage*filt_alpha;
//     _current_filt_amps = _current_filt_amps*(1-filt_alpha) + _state.current_amps*filt_alpha;

//     // update estimated voltage without sag
//     _state.voltage_resting_estimate = _state.voltage + _state.current_amps * _state.resistance;
// }