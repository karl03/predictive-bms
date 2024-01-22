#define serial_timeout 5
#define avg_ms 1000
#define end_voltage 3.5
#define use_display 0
#define sd_logging 1
// Values for INA226 onboard shunt
#define current_offset -1.63992595  // mA
#define current_scale 999.5734377   // mV/10A
// Values for 200A shunt:
// #define current_offset -982.8158579
// #define current_scale 3.7834625
#define A_ms_to_A_h 0.00000027777777777778
// Measure voltages over a very gradual discharge to build a table of (almost) open circuit voltages.
// From these results, build lookup table, mapping percentage of capacity to voltage.
// Next create function to interpolate if between percentage values.
#define percent_at_voltage [[100, 4.2], [99, 4.1]]