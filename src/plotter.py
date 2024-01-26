import math
import numpy as np
import matplotlib.pyplot as plt


class LowPassFilter:
    def __init__(self, filter_const, initial_value):
        self.filtered_value = initial_value
        self.filter_const = filter_const

    def update(self, new_value, time_delta):
        filter_factor = min(time_delta * self.filter_const, 1.0)
        
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

def accumulate_value(array, value_index):
    accumulated_array = add_column(array)
    time = 0
    for index in range(len(array)):
        time += int(array[index][value_index])
        accumulated_array[index][-1] = time
    
    return accumulated_array

def ms_to_hours(array, time_index):
    for index in range(len(array)):
        array[index][time_index] = int(array[index][time_index]) / 3600000

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


with open(r"C:\Users\Karl\dev\predictive-bms\data\raw\test-20240123-212326.csv") as F:
    lines = [line.strip().split(',') for line in F.readlines()]

results = np.array(lines)

results_copy = results[1::]
readings_start = find_plugged_in(results_copy, 4, 10)
results_copy = results_copy[readings_start::]
low_pass_filter_array(results_copy, 4, 9, 1/300)
results_copy = average_results(results_copy, 4, 1000)
# results_copy = accumulate_value(results_copy, 9)
ms_to_hours(results_copy, -1)
print(results_copy[-1])
print(len(results_copy))

plt.title("Discharge Test 1\n1500mAh 4s, Low Pass Filter with 1/300 constant, Averaging every 1000 values")
plt.xlabel("Capacity (mAh)")
plt.ylabel("Voltage (V)")
plt.plot(results_copy[::, 7].astype(float), results_copy[::, 4].astype(float))
plt.show()