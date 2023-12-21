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
        float A_;
        float B_;
        float E0_;
        float K_;
        void CalculateParameters();
    
    public:
        BattModel(float v_full, float cap, float v_nom, float cap_nom, float r_int, float v_exp, float cap_exp, float cur_curve, float v_min);
        float Simulate(float used_capacity, float current);
};