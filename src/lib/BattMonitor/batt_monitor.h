#include <cmath>

#define A_ms_to_A_h 0.00000027777777777778

class BattMonitor {
    public:
        struct State {
            float voltage;              // Voltage in V
            float current;              // Current in mA
            float cell_voltages[4];     // Per-cell voltage in V
            float cell_z_scores[4];
            float mAh_used;
            float mWh_used;
            unsigned long last_update;  // Last update of values in microseconds
        };

        BattMonitor(State* state) {state_ = state;};
        void update_consumption(float time_delta, float voltage, float current_mA, float cell_voltages[4]);
    
    private:
        State* state_;
};