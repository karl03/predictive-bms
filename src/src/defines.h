#define serial_timeout 0
#define avg_ms 1000
#define end_voltage 3.5
#define use_display 1
#define sd_logging 0
#define current_offset -982.8158579
#define current_scale 3.7834625
#define A_ms_to_A_h 0.00000027777777777778
// Measure voltages over a very gradual discharge to build a table of (almost) open circuit voltages.
// From these results, build lookup table, mapping percentage of capacity to voltage.
// Next create function to interpolate if between percentage values.
#define percent_at_voltage [[100, 4.2], [99, 4.1]]