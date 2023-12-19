import math
import matplotlib.pyplot as plt

# constant_voltage = 4.2  # maybe???
# internal_resistance = 0.0165
# capacity = 2
# exp_voltage = 3.71
# exp_capacity = 0.6

full_voltage = 1.39
capacity = 6.5
nominal_voltage = 1.18
nominal_capacity = 6.25
internal_resistance = 0.002
exponential_voltage = 1.28
exponential_capacity = 1.3
curve_current = 1.3
min_voltage = 0.8

A = full_voltage - exponential_voltage
B = 3 / exponential_capacity
K = ((full_voltage - nominal_voltage + (A * math.exp(-B * nominal_capacity))) * (capacity - nominal_capacity)) / nominal_capacity
E0 = full_voltage + K + (internal_resistance * curve_current) - A

print(f"A: {A}\nB: {B}\nK: {K}\nE0: {E0}")


def liion_discharge(used_capacity, low_freq_current, instant_current):
    return E0 - (K * (capacity/(capacity - used_capacity)) * low_freq_current) + ((-K) * (capacity / (capacity - used_capacity)) * used_capacity) + (A * math.exp((-B) * used_capacity))


# print(liion_discharge(0.6, 1.95, 1.95))

results = []
# r2 = []
# r3 = []

for x in range(int(capacity * 100)):
    if liion_discharge(x/100, curve_current, curve_current) >= min_voltage:
        results.append(liion_discharge(x/100, curve_current, curve_current))
    # if liion_discharge(x/100, 13, 13) >= 2.2:
    #     r2.append(liion_discharge(x/100, 13, 13))
    # if liion_discharge(x/100, 32.5, 32.5) >= 2.2:
    #     r3.append(liion_discharge(x/100, 32.5, 32.5))

plt.plot(results)
# plt.plot(r2)
# plt.plot(r3)
plt.show()