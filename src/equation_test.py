import math
import matplotlib.pyplot as plt

class LowPassFilter:
    def __init__(self, reaction_time, initial_time=0, initial_current=0):
        self.filtered_current = initial_current
        self.current_time = initial_time
        self.reaction_time = reaction_time
    
    def setInitial(self, time, current):
        self.current_time = time
        self.filtered_current = current

    def update(self, current, time):
        filter_val = math.exp(- (time - self.current_time) / self.reaction_time)
        self.current_time = time
        
        self.filtered_current = self.filtered_current * (1 - filter_val) + current * filter_val
        return self.filtered_current

class LiionDischarger:
    def __init__(self, full_voltage, capacity, nominal_voltage, nominal_capacity, internal_resistance, exponential_voltage, exponential_capacity, curve_current, min_voltage, reaction_time):
        self.full_voltage = full_voltage
        self.capacity = capacity
        self.nominal_voltage = nominal_voltage
        self.nominal_capacity = nominal_capacity
        self.internal_resistance = internal_resistance
        self.exponential_voltage = exponential_voltage
        self.exponential_capacity = exponential_capacity
        self.curve_current = curve_current
        self.min_voltage = min_voltage
        self.reaction_time = reaction_time

        self.A = full_voltage - exponential_voltage
        self.B = 3 / exponential_capacity
        self.E0 = full_voltage + internal_resistance * curve_current - A
        self.K = ((capacity - exponential_capacity) * (E0 - internal_resistance * curve_current + A * math.exp(-B * exponential_capacity) - exponential_voltage)) / (capacity * (exponential_capacity + curve_current))
        self.lpf = LowPassFilter(reaction_time)

    def setInitial(self, time, current):
        self.lpf.setInitial(time, current)
    
    def discharge(self, used_capacity, instant_current, time):
        if (self.capacity - used_capacity == 0):
            return self.min_voltage
        filtered_current = self.lpf.update(instant_current, time)
        return self.E0 - (self.K * (self.capacity/(self.capacity - used_capacity)) * used_capacity) - (self.internal_resistance * instant_current) + (self.A * math.exp(-self.B * used_capacity)) - (self.K * (self.capacity/(self.capacity - used_capacity)) * filtered_current)

# Vals from paper
full_voltage = 1.39
capacity = 7
nominal_voltage = 1.18
nominal_capacity = 6.25
internal_resistance = 0.002
exponential_voltage = 1.28
exponential_capacity = 1.3
curve_current = 1.3
min_voltage = 0.8
reaction_time = 30

# Discharge test 1 values
# Start ['4.20' '4.19' '4.19' '4.22' '16.809999999999874' '1.48' '13.14' '-0.00' '0.00' '67571' '0']
# End ['3.36' '3.34' '3.25' '3.28' '13.354517558166222' '0.82' '6.59' '1541.91' '23790.48' '67584' '0']
# full_voltage = 16.8
# capacity = 1.55
# nominal_voltage = 14.75
# nominal_capacity = 1.42
# internal_resistance = 0.002
# exponential_voltage = 15.26
# exponential_capacity = 0.8
# curve_current = 0.06340690796660907
# min_voltage = 13.3
# reaction_time = 30


A = full_voltage - exponential_voltage
B = 3 / exponential_capacity
E0 = full_voltage + internal_resistance * curve_current - A
# Finally working, derived from functions on page 4 of "Experimental Validation of a Battery Dynamic Model for EV Applications, Olivier Tremblay"
K = ((capacity - exponential_capacity) * (E0 - internal_resistance * curve_current + A * math.exp(-B * exponential_capacity) - exponential_voltage)) / (capacity * (exponential_capacity + curve_current))

print(f"A: {A}\nB: {B}\nK: {K}\nE0: {E0}")

filtered_current = 0
def liionDischarge(used_capacity, instant_current, time_since_last_measurement):
    global filtered_current
    filter_val = math.exp(-time_since_last_measurement / reaction_time)
    filtered_current = filtered_current * (1- filter_val) + instant_current * filter_val
    # filtered_current = instant_current

    # From Tremblay paper 2009:
    return E0 - (K * (capacity/(capacity - used_capacity)) * used_capacity) - (internal_resistance * instant_current) + (A * math.exp(-B * used_capacity)) - (K * (capacity/(capacity - used_capacity)) * filtered_current)

print(f"Test = {liionDischarge(3.5, 15, 30)}")

results = []
r2 = []
r3 = []
high_cap_results = []
low_cap_results = []

test_class_model = LiionDischarger(full_voltage, capacity, nominal_voltage, nominal_capacity, internal_resistance, exponential_voltage, exponential_capacity, curve_current, min_voltage, reaction_time)
high_cap_class_model = LiionDischarger(full_voltage, capacity*1.1, nominal_voltage, nominal_capacity*1.1, internal_resistance, exponential_voltage, exponential_capacity*1.1, curve_current, min_voltage, reaction_time)
low_cap_class_model = LiionDischarger(full_voltage, capacity*0.6, nominal_voltage, nominal_capacity*0.6, internal_resistance, exponential_voltage, exponential_capacity*0.6, curve_current, min_voltage, reaction_time)
# test_class_model.setInitial()

for x in range(int(capacity * 1000)):
    if liionDischarge(x/1000, curve_current, ((1/100) / 32.5 * 3600)) >= min_voltage:
        results.append(liionDischarge(x/1000, 6.5, ((x/1000) / 32.5 * 3600)))
        # high_cap_results.append(high_cap_class_model.discharge(x/1000, curve_current, ((x/1000) / 32.5 * 3600)))
        # r2.append(test_class_model.discharge(x/1000, curve_current, ((x/1000) / 32.5 * 3600)))
    if test_class_model.discharge(x/1000, 13, ((x/1000) / 32.5 * 3600)) >= min_voltage:
        r2.append(test_class_model.discharge(x/1000, 13, ((x/1000) / 32.5 * 3600)))
    if test_class_model.discharge(x/1000, 32.5, ((x/1000) / 32.5 * 3600)) >= min_voltage:
        r3.append(test_class_model.discharge(x/1000, 32.5, ((x/1000) / 32.5 * 3600)))
    # if x/1000 < capacity*0.5:
    low_cap_results.append(high_cap_class_model.discharge(x/1000, 6.5, ((x/1000) / 32.5 * 3600)))
        # low_cap_results.append(low_cap_class_model.discharge(x/1000, 6.5, ((x/1000) / 32.5 * 3600)))

plt.plot(results, label="6.5A")
# plt.plot(high_cap_results, label="Capacity * 1.2")
plt.plot(low_cap_results, label="Capacity * 0.8")
plt.plot(r2, label=f"{curve_current}A")
plt.plot(r3, label="32.5A")
plt.legend()
plt.xlabel("Capacity Used (mAh)")
plt.ylabel("Voltage (V)")
plt.grid(True)
plt.title(f"Simulation of 6.5Ah, 1.2V NiMH Battery\nA={round(A, 4)}, B={round(B, 4)}, R={internal_resistance}, K={round(K, 4)}, E0={round(E0, 4)}")
plt.show()