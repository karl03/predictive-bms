import math
import numpy as np
import matplotlib.pyplot as plt

file_path = r"..\data\raw\test-20240123-212326.csv"  # Path where data file is stored
maximum_voltage = 16.8              # Fully charged voltage
minimum_voltage = 13.3              # Voltage at end of discharge test

reaction_time = 200                 # Low pass filter reaction time in seconds
values_to_average = 1000            # Number of values to use for each average


class LowPassFilter:
    def __init__(self, reaction_time, initial_value):
        self.filtered_value = initial_value
        self.reaction_time = reaction_time

    def update(self, new_value, time_delta):
        filter_factor = min(time_delta / self.reaction_time, 1.0)
        
        self.filtered_value = float(self.filtered_value) * (1 - filter_factor) + float(new_value) * filter_factor
        return self.filtered_value


def add_column(array):
    new_arr = np.zeros((array.shape[0], array.shape[1] + 1))
    new_arr[:, :-1] = array
    return new_arr

def find_plugged_in(array, sub_index, min_voltage):
    for index, val in enumerate(array):
        if float(val[sub_index]) > min_voltage:
            return index

def average_results(array, value_index, avg_num):
    if avg_num <= 1 or avg_num > len(array):
        return 0

    averaged = array[::avg_num]
    sum = 0
    counter = 0

    for index, value in enumerate(array):
        counter += 1
        sum += float(value[value_index])
        if counter == avg_num:
            averaged[index // avg_num][value_index] = sum / avg_num
            sum = 0
            counter = 0
    
    if (counter > 0):
        averaged[-1][value_index] = sum / counter

    return averaged

def low_pass_filter_array(array, value_index, time_index, filter_const):
    lpf = LowPassFilter(filter_const, array[0][value_index])

    for index in range(len(array) - 1):
        array[index + 1][value_index] = lpf.update(float(array[index + 1][value_index]), int(array[index + 1][time_index]) / 1000000)

def find_empty_and_fill(array, end=True):
    if end:
        start_counter = len(array) - 1
        end_counter = len(array)

        value = array[start_counter]

        while value == 0:
            start_counter -= 1
            value = array[start_counter]
        
        array[start_counter + 1: end_counter] = value
    
    else:
        start_counter = 0
        end_counter = 0

        value = array[0]

        while value == 0:
            end_counter += 1
            value = array[end_counter]
        
        array[start_counter: end_counter + 1] = value


def get_lookup_vals(array, values_array, max_voltage, min_voltage):
    output = np.zeros(int(max_voltage * 100) - int(min_voltage * 100))
    prev_val = array[0]
    min_reached = False
    max_reached = False

    for index, value in enumerate(array):
        if value > max_voltage or value < min_voltage:
            continue
        if value == max_voltage:
            max_reached = True
        if value == min_voltage:
            min_reached = True

        if output[int(value * 100) - int(min_voltage * 100)] == 0:
            output[int(value * 100) - int(min_voltage * 100)] = values_array[index]

            if int(prev_val * 100) - int(value * 100) > 1:
                output[int(value * 100) - int(min_voltage * 100) + 1: int(prev_val * 100) - int(min_voltage * 100)] = values_array[index]
                
            prev_val = value

        else:
            output[int(value * 100) - int(min_voltage * 100)] += values_array[index]
            output[int(value * 100) - int(min_voltage * 100)] /= 2

    if not max_reached:
        find_empty_and_fill(output)
        
    if not min_reached:
        find_empty_and_fill(output, False)
        
    return output

def array_to_c_array(array):
    output = "{"

    for index, element in enumerate(array):
        output += str(element)
        if index < len(array) - 1:
            output += ", "

    return output + "}"


with open(file_path) as F:
    lines = [line.strip().split(',') for line in F.readlines()]

results = np.array(lines)

results_copy = results[1::]
readings_start = find_plugged_in(results_copy, 4, minimum_voltage)
results_copy = results_copy[readings_start::]
low_pass_filter_array(results_copy, 4, 9, reaction_time)
results_copy = average_results(results_copy, 4, values_to_average)

print("Capacity lookup:", array_to_c_array(get_lookup_vals(results_copy[::, 4].astype(float), results_copy[::, 7].astype(float), maximum_voltage, minimum_voltage)))
print("Energy lookup:", array_to_c_array(get_lookup_vals(results[1::, 4].astype(float), results[1::, 8].astype(float), maximum_voltage, minimum_voltage)))
print(f"Average current: {np.average(results[1::, 6].astype(float))}mA")

gradients = [0]
for index in range(len(results_copy) - 1):
    gradients.append(float(results_copy[index + 1][4]) - float(results_copy[index][4]))

fig, ax1 = plt.subplots()
ax1.set_xlabel("Capacity (mAh)")
ax1.set_ylabel("Voltage (V)", color='tab:red')
ax1.plot(results_copy[::, 7].astype(float), results_copy[::, 4].astype(float), color='tab:red')
ax1.tick_params(axis='y', labelcolor='tab:red')

ax2 = ax1.twinx()
ax2.set_ylabel("Voltage Gradient", color='tab:blue')
ax2.plot(results_copy[::, 7].astype(float), gradients, color='tab:blue')
ax2.tick_params(axis='y', labelcolor='tab:blue')

plt.title("Discharge Test")
plt.grid(True)
plt.show()