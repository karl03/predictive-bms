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

print(f"A: {A}\nB: {B}\nK: {K}\nE0: {E0}")


def liion_discharge(used_capacity, low_freq_current):
    return E0 - (K * (capacity/(capacity - used_capacity)) * low_freq_current) + ((-K) * (capacity / (capacity - used_capacity)) * used_capacity) + (A * math.exp((-B) * used_capacity))


results = []
r2 = []
r3 = []


for x in range(int(capacity * 100)):
    if liion_discharge(x/100, 6.5) >= min_voltage:
        results.append(((x/100 * capacity) / 6.5, liion_discharge(x/100, 6.5)))
    if liion_discharge(x/100, 13) >= min_voltage:
        r2.append(((x/100 * capacity) / 13, liion_discharge(x/100, 13)))
    if liion_discharge(x/100, 32.5) >= min_voltage:
        r3.append(((x/100 * capacity) / 32.5, liion_discharge(x/100, 32.5)))

plt.plot([x[0] * 10 for x in results], [x[1] for x in results], label="6.5A")
plt.plot([x[0] * 10 for x in r2], [x[1] for x in r2], label="13A")
plt.plot([x[0] * 10 for x in r3], [x[1] for x in r3], label="32.5A")
plt.legend()
plt.xlabel("Time (minutes)")
plt.ylabel("Voltage (V)")
plt.title(f"Simulation of 6.5Ah, 1.2V NiMH Battery\nA={round(A, 4)}, B={round(B, 4)}, R={internal_resistance}, K={round(K, 4)}, E0={round(E0, 4)}")
plt.show()