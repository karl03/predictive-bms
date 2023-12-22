#include "batt_model.h"

BattModel::BattModel(float v_full, float cap_full, float v_nom, float cap_nom, float r_int, float v_exp, float cap_exp, float cur_curve, float v_min, int reaction_time) {
    full_voltage_ = v_full;
    capacity_ = cap_full;
    nominal_voltage_ = v_nom;
    nominal_capacity_ = cap_nom;
    internal_resistance_  = r_int;
    exponential_voltage_ = v_exp;
    exponential_capacity_ = cap_exp;
    curve_current_ = cur_curve;
    min_voltage_ = v_min;
    low_pass_ = new LPF(reaction_time);

    CalculateParameters();
};

void BattModel::CalculateParameters() {
    A_ = full_voltage_ - exponential_voltage_;
    B_ = 3 / exponential_capacity_;
    E0_ = full_voltage_ + internal_resistance_ * curve_current_ - A_;
    K_ = ((capacity_ - exponential_capacity_) * (E0_ - internal_resistance_ * curve_current_ + A_ * exp(-B_ * exponential_capacity_) - exponential_voltage_)) / (capacity_ * (exponential_capacity_ + curve_current_));
};

void BattModel::ResetFilter(unsigned long time, float current) {
    low_pass_->SetInitialParams(time, current);
};

float BattModel::Simulate(float used_capacity, float current, unsigned long time) {
    float filtered_current = low_pass_->Update(time, current);
    return E0_ - (K_ * (capacity_ / (capacity_ - used_capacity)) * used_capacity) - (internal_resistance_ * current) + (A_ * exp(-B_ * used_capacity)) - (K_ * (capacity_ / (capacity_ - used_capacity)) * filtered_current);
};