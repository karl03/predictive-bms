#ifndef BATT_MODEL_H
#define BATT_MODEL_H

#include <cmath>

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
        float capacity_step_percentage_;
        
        float A_;
        float B_;
        float E0_;
        float K_;
        void CalculateParameters();
    
    public:
        // Getters
        float GetA() const { return A_; }
        float GetB() const { return B_; }
        float GetE0() const { return E0_; }
        float GetK() const { return K_; }
        float GetResistanceOhms() const { return internal_resistance_; }
        float GetCapacity() const { return capacity_; }

        // Setters
        void SetCapacity(float new_capacity);
        void SetInternalResistance(float internalResistance) { internal_resistance_ = internalResistance; CalculateParameters();}
        
        BattModel(float v_full, float cap, float v_nom, float cap_nom, float r_int, float v_exp, float cap_exp, float cur_curve, float v_min);
        float Simulate(float used_capacity, float current, float filtered_current);
};

#endif