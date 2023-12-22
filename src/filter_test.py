import math
import random
import matplotlib.pyplot as plt

class LowPassFilter:
    def __init__(self, reaction_time, initial_time=0, initial_current=0):
        self.filtered_current = initial_current
        self.current_time = initial_time
        self.reaction_time = reaction_time

    def update(self, current, time):
        filter_val = math.exp(- (time - self.current_time) / self.reaction_time)
        self.current_time = time
        
        self.filtered_current = self.filtered_current * (1 - filter_val) + current * filter_val
        return self.filtered_current

step_time = 1
response_time = 30
closeness = 0.98
epic_time = - response_time / (math.log(1 - closeness))

current = 0
currents = []
filtered_current = 0.0
filtered_currents = []
class_filtered_currents = []

filt_test = LowPassFilter(epic_time)

for x in range(60):
    filter_val = math.exp(-step_time / epic_time)
    # current = random.randint(1, 200)
    current = 30
    currents.append(current)

    class_filtered_currents.append(filt_test.update(current, x))
    filtered_current = current * (1 - filter_val) + filtered_current * filter_val
    filtered_currents.append(filtered_current)

    if x == 30:
        print(filtered_current / current)

plt.plot(currents)
plt.plot(filtered_currents)
plt.plot(class_filtered_currents)
plt.show()