class ResistanceEstimate {
    private:
        float filtered_current_;
        unsigned long current_time_;
        int reaction_time_;
        float current_max_amps_;
        float current_filt_amps_;
        float resistance_voltage_ref_;
        float resistance_current_ref_;
        unsigned long resistance_timer_ms_;

    public:
        ResistanceEstimate();
        update_resistance_estimate();
};