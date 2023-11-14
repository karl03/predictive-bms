#define end_voltage 3.5
// Measure voltages over a very gradual discharge to build a table of (almost) open circuit voltages.
// From these results, build lookup table, mapping percentage of capacity to voltage.
// Next create function to interpolate if between percentage values.
#define percent_at_voltage [[100, 4.2], [99, 4.1]]