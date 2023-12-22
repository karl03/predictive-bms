import math
import random
import matplotlib.pyplot as plt

step_time = 30
response_time = 30

current = 0
currents = []
filtered_current = 0.0
filtered_currents = []

for x in range(30):
    filter_val = math.exp(-step_time / response_time)
    alt = step_time / response_time
    # current = random.randint(1, 200)
    current = 30
    currents.append(current)

    print(f"Alt formula:{current * alt + filtered_current * (1 - alt)}")
    filtered_current = current * (1 - filter_val) + filtered_current * filter_val
    print(filtered_current)
    # current * filter_val + filtered_current * (1 - filter_val)
    # filtered_current * alt + current * (1 - alt)
    filtered_currents.append(filtered_current)

plt.plot(currents)
plt.plot(filtered_currents)
plt.show()