import math
import matplotlib.pyplot as plt

# constant_voltage = 4.2  # maybe???
# internal_resistance = 0.0165
# capacity = 2
# exp_voltage = 3.71
# exp_capacity = 0.6

constant_voltage = 3.7804
internal_resistance = 0.017048
capacity = 2
exp_voltage = 0.45177
exp_capacity = 5


def liion_discharge(used_capacity, low_freq_current, instant_current):
    return constant_voltage - (internal_resistance * (capacity/(capacity - used_capacity)) * low_freq_current) + ((-internal_resistance) * (capacity / (capacity - used_capacity)) * used_capacity) + (exp_voltage * math.exp((-exp_capacity) * used_capacity))

print(liion_discharge(1.6, 1.95, 1.95))

# results = []
# r2 = []
# r3 = []

# for x in range(200):
#     if liion_discharge(x/100, 6.5, 6.5) >= 2.2:
#         results.append(liion_discharge(x/100, 6.5, 6.5))
#     if liion_discharge(x/100, 13, 13) >= 2.2:
#         r2.append(liion_discharge(x/100, 13, 13))
#     if liion_discharge(x/100, 32.5, 32.5) >= 2.2:
#         r3.append(liion_discharge(x/100, 32.5, 32.5))

# plt.plot(results)
# plt.plot(r2)
# plt.plot(r3)
# plt.show()