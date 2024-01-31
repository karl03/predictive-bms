#ifndef BATT_MODEL_H
#define BATT_MODEL_H

#include <cmath>
#include "LPF.h"

class BattModel {
    private:
        float full_voltage_;
        float capacity_;
        float nominal_voltage_;
        float nominal_capacity_;
        float internal_resistance_;
        float exponential_voltage_;
        float exponential_capacity_;
        float curve_current_;
        float min_voltage_;
        float A_;
        float B_;
        float E0_;
        float K_;
        LPF *low_pass_;
        void CalculateParameters();
    
    public:
        // Getters
        float GetFullVoltage() const { return full_voltage_; }
        float GetCapacity() const { return capacity_; }
        float GetNominalVoltage() const { return nominal_voltage_; }
        float GetNominalCapacity() const { return nominal_capacity_; }
        float GetInternalResistance() const { return internal_resistance_; }
        float GetExponentialVoltage() const { return exponential_voltage_; }
        float GetExponentialCapacity() const { return exponential_capacity_; }
        float GetCurveCurrent() const { return curve_current_; }
        float GetMinVoltage() const { return min_voltage_; }
        float GetA() const { return A_; }
        float GetB() const { return B_; }
        float GetE0() const { return E0_; }
        float GetK() const { return K_; }

        // Setters
        void SetFullVoltage(float fullVoltage) { full_voltage_ = fullVoltage; }
        void SetCapacity(float capacity) { capacity_ = capacity; }
        void SetNominalVoltage(float nominalVoltage) { nominal_voltage_ = nominalVoltage; }
        void SetNominalCapacity(float nominalCapacity) { nominal_capacity_ = nominalCapacity; }
        void SetInternalResistance(float internalResistance) { internal_resistance_ = internalResistance; }
        void SetExponentialVoltage(float exponentialVoltage) { exponential_voltage_ = exponentialVoltage; }
        void SetExponentialCapacity(float exponentialCapacity) { exponential_capacity_ = exponentialCapacity; }
        void SetCurveCurrent(float curveCurrent) { curve_current_ = curveCurrent; }
        void SetMinVoltage(float minVoltage) { min_voltage_ = minVoltage; }
        void SetA(float A) { A_ = A; }
        void SetB(float B) { B_ = B; }
        void SetE0(float E0) { E0_ = E0; }
        void SetK(float K) { K_ = K; }
        
        BattModel(float v_full, float cap, float v_nom, float cap_nom, float r_int, float v_exp, float cap_exp, float cur_curve, float v_min, int reaction_time);
        void ResetFilter(unsigned long time, float current);
        float Simulate(float used_capacity, float current, unsigned long time);
};

#endif