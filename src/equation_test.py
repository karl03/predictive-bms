import math
import matplotlib.pyplot as plt

full_voltage = 1.39
capacity = 7
nominal_voltage = 1.18
nominal_capacity = 6.25
internal_resistance = 0.002
exponential_voltage = 1.28
exponential_capacity = 1.3
curve_current = 1.3
min_voltage = 0.8


A = full_voltage - exponential_voltage
B = 3 / exponential_capacity
E0 = full_voltage + internal_resistance * curve_current - A
# Finally working, derived from functions on page 4 of "Experimental Validation of a Battery Dynamic Model for EV Applications, Olivier Tremblay"
K = ((capacity - exponential_capacity) * (E0 - internal_resistance * curve_current + A * math.exp(-B * exponential_capacity) - exponential_voltage)) / (capacity * (exponential_capacity + curve_current))
# Working, but was generated via WolframAlpha
# K = (math.exp(-B * exponential_capacity) * (exponential_capacity - capacity) * (math.exp(B * exponential_capacity) * (curve_current * internal_resistance + exponential_voltage - E0) - A)) / (capacity * (curve_current + exponential_capacity))

# K = ((full_voltage - nominal_voltage + (A * (math.exp(-B * nominal_capacity) - 1))) * (capacity - nominal_capacity)) / nominal_capacity # wrong??
# E0 = full_voltage + K + (internal_resistance * curve_current) - A

print(f"A: {A}\nB: {B}\nK: {K}\nE0: {E0}")


def liion_discharge(used_capacity, low_freq_current):
    return E0 - (K * (capacity/(capacity - used_capacity)) * low_freq_current) + ((-K) * (capacity / (capacity - used_capacity)) * used_capacity) + (A * math.exp((-B) * used_capacity))


print(liion_discharge(0, 0))

results = []
# r2 = []
# r3 = []

for x in range(int(capacity * 100)):
    if liion_discharge(x/100, curve_current) >= min_voltage:
        results.append(liion_discharge(x/100, curve_current))
    # if liion_discharge(x/100, 13, 13) >= 2.2:
    #     r2.append(liion_discharge(x/100, 13, 13))
    # if liion_discharge(x/100, 32.5, 32.5) >= 2.2:
    #     r3.append(liion_discharge(x/100, 32.5, 32.5))

plt.plot(results)
# plt.plot(r2)
# plt.plot(r3)
plt.show()