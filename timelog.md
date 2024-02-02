# Timelog

* Battery Management System with Cell Health Estimation and Failure Prediction for Autonomous Drones
* Karl Hartmann
* 2566374h
* Lauritz Thamsen

## Guidance

* This file contains the time log for your project. It will be submitted along with your final dissertation.
* **YOU MUST KEEP THIS UP TO DATE AND UNDER VERSION CONTROL.**
* This timelog should be filled out honestly, regularly (daily) and accurately. It is for *your* benefit.
* Follow the structure provided, grouping time by weeks.  Quantise time to the half hour.

## Week 1

### 18 Sep 2023

* *0.5 hours* Planning meeting time/ communication with supervisor

### 21 Sep 2023

* *2 hours* Attending guidance lecture
* *0.5 hours* Reading through moodle page

## Week 2

### 26 Oct 2023

* *1 hour* Started looking through recommended reading.

### 27 Oct 2023

* *1 hour* Continued reading recommended reading and took notes.
* *0.5 hours* First supervisor meeting

## Week 3

### 02 Oct 2023

* *0.5 hours* Editing and uploading meeting minutes.

### 03 Oct 2023

* *2 hours* Copied folder structure, created GitHub repo, filled out time log and edited templates

### 04 Oct 2023

* *2 hours* Hardware research on voltage sensing and beginning to read relevant research papers

* *0.5 hours* Supervisor meeting

* *0.5 hours* Editing and uploading meeting minutes

### 08 Oct 2023

* *2 hours* Finding and reading relevant papers.

* *2 hours* Researched hardware requirements for voltage sensing.

## Week 4

### 09 Oct 2023

* *1 hour* Continued hardware research and collated findings.

### 10 Oct 2023

* *2 hours* Further hardware research, ordered parts.

* *1 hour* Continued reading research papers.

### 11 Oct 2023

* *0.5 hour* Meeting preparation.

* *0.5 hour* Supervisor meeting

* *0.5 hour* Summarising and uploading meeting minutes.

## Week 5

### 16 Oct 2023

* *2 hours* Attempted to install Batemo/ use Simulink.

### 17 Oct 2023

* *1 hour* Continued Simulink simulation setup.

## Week 6

### 23 Oct 2023

* *1 hour* Set up arduino project using Platform.io.

### 24 Oct 2023

* *2 hours* Finished setup and ran initial simulations in Simulink using Batemo.

### 25 Oct 2023

* *0.5 hours* Running further simulations, meeting prep.
* *0.5 hours* Supervisor meeting.
* *0.5 hours* Summarising and updating meeting minutes.

## Week 7

### 30 Oct 2023

* *1 hours* Research into wiring

### 31 Oct 2023

* *5.5 hours* Planning, soldering, coding and testing voltage measurement, finding and installing libraries and adding OLED display.

### 01 Nov 2023

* *1 hour* Adding current measurement and refresh rate.
* *0.5 hour* Advisor meeting.
* *0.5 hour* Voltage accuracy checking, updated display to show cell voltages.

## Week 7

### 07 Nov 2023

* *1 hour* Researching and beginning SoC implementation

## Week 8

### 13 Nov 2023

* *0.5 hours* Soldering to current shunt resistor

### 15 Nov 2023

* *2.5 hours* Creating data logging application, begin working on communication
* *8 hours* Creating circuit for testing, rewiring shunt resistor, removing INA226 onboard shunt, creating capacity counting code, refactoring, multiple calibration attempts with different techniques.

## Week 9

Note: I was ill and unable to complete work for weeks 9 and 10.

## Week 10

### 29 Nov 2023

* *0.5 hours* Supervisor meeting.
* *0.5 hours* Uploading meeting minutes.

## Week 12

### 12 Dec 2023

* *1 hour* Wiring up SD card module/ formatting for usage
* *4 hours* Learned basics of SD interaction, added ability to run without display. Logs data upon startup if logging is enabled and an SD card is found.

### 13 Dec 2023

* *1 hour* Begun trying to figure out implementation of battery model equations.
* *1 hour* Mounted device on drone, and completed a flight to gather test data.

### 14 Dec 2023

* *6 hours* Continued attempt to figure out battery model equations, created python implementation and compared to results from original model in Matlab, not working correctly yet.
* *2.5 hours* Updated shunt voltage to current conversion (current calibration) method, then calibrated. Calibration seems to be working well now. Implemented Wh counting.

### 15 Dec 2023

* *2 hours* Researched methods for estimating performance, focusing on internal resistance estimation and compensation.
* *2 hours* Wrote status report, submitted.

### 18 Dec 2023

* *1 hour* Attempted to generate code from Matlab model for battery.
* *2 hours* Continued research on available battery models.

### 19 Dec 2023

* *2 hours* Completed further research, found origin of Matlab model, 'Tremblay Model', begun attempting to implement.
* *3 hours* Continued implementation attempt, but not yet fully working.

### 20 Dec 2023

* *2 hours* Attempting to figure out equation for 'polarisation constant' / K.
* *6 hours* After many attempts, finally successfully derived updated function for E0 and K by rearranging given formulas in research paper. Updated graphing code to display new results.

### 21 Dec 2023

* *1.5 hours* Translated Python model to C++, created relevant class.
* *0.5 hours* Supervisor meeting.
* *0.5 hours* Updated/ uploaded meeting minutes.
* *2 hours* Troubleshooting C++ model, added necessary functions, tested to ensure functionality, checked time needed to run on hardware.

### 22 Dec 2023

* *4 hours* Implementing low-pass filter in python for testing.
* *2 hours* Translating filter to C++, testing on hardware.
* *2 hours* Calculating equation needed to bring filtered value within distance of actual value for a certain delay number.

## Week 13

### 08 Jan 2024

* *3.5 hours* Soldered, calibrated and tested other INA226 module, using onboard shunt for low current, high accuracy testing, allowing for discharge tests to take place to generate lookup table. Modified code to print Wh to csv.

### 09 Jan 2024

* *3 hours* Researching further methods for internal resistance estimation, found several papers and looked at implemented method and comments in ardupilot firmware.

## Week 14

### 18 Jan 2024

* *2.5 hours* Tested time for reading voltages and found blocking function to ensure new reading, switched average timing to microseconds for accuracy, added license, added Ardupilot code for resistance estimation, but not yet adapted to my code.

### 19 Jan 2024

* *3 hours* Continued timing testing, learned how to and added waits for conversion on both INA226 and INA3221, found rough time for conversion of 3221 of 64ms, after disabling shunt measurement. As battery model runs within a couple ms can hopefully aim for 64ms total loop time, running operations in time during which the INA's are measuring/ converting.

### 20 Jan 2024

* *3 hours* Tested timing for SD card operations, added check for missed readings, modified timing code to include display operations. Begun conversion/ implementation of resistance estimation code. Found SD code does occasionally cause missed readings due to taking longer. Came up with idea for tracking missed readings via interrupt pin and alert from INA.

### 21 Jan 2024

* *5 hours* Completed initial implementation of resistance estimation. Researched outlier metrics/ detection. Begun implementing battery monitoring as class, added cell Z score for deviation.

## Week 15

### 22 Jan 2024

* *2 hours* Figuring out Platformio environment capabilities, begin work on separate discharge test main file.
* *3.5 hours* Updated serial handshake protocol, now seems to be working reliably. Updated discharge test code to send serial data as required, now planning on storing data through serial for this to ensure no readings are lost, as serial writes are more consistent than SD writes. Fixed bug in timing display code, cleaned up code by removing massive if block.

### 23 Jan 2024

* *2.5 hours* Further serial connection improvements, display and serial code refactoring, now ready to run discharge tests saving to pc directly.
* *1 hour* Setting up/ running charge test gathering data, attempted to plot data.

### 24 Jan 2024

* *0.5 hours* Checking discharge test, ensuring results so far are as expected, restarting discharge cycle when charger timed out.
* *0.5 hours* Advisor meeting.
* *0.5 hours* Editing/ uploading meeting minutes.

### 25 Jan 2024

* *2 hours* Worked on filtering/ cleaning discharge data for visualisation, by adding low-pass filter, averaging, and other helper functions. Produced several visualisations and added to data folder.
* *1.5 hours* Begun added functionality to logger gui, attempted to update simulation based on discharge values, further refinement needed. Tried to get gradients between points to find changes in curve, but not yet working.

## Week 16

### 29 Jan 2024

* *1.5 hours* Cleaned up unused code, updated define for SoC lookup to be valid array, planned how to implement lookup effectively and efficiently, minimising memory usage.

### 30 Jan 2024

* *3 hours* Rewrote gradient function, now working. Further work performed on data analysis functions, and added function to generate lookup table to be used for initial SoC estimation onboard BMS, based on discharge test data.

### 31 Jan 2024

* *1.5 hours* Updated lookup table creation function to ensure functionality in edge/ extreme cases, output copieble c-style array. Begun implementation for lookup.
* *1 hour* Finished lookup function, updated define names/ rearranged defines file. Added and tested platform.io github CI pipeline. Added mWh lookup as addition to mAh lookup in order to achieve full SoC estimation. Created flags enum.
* *1.5 hours* Created code to check/ initialise SoC based on lookup function, including wait to ensure stable voltage. Added average calculation to data interpretation file, updated simulation current parameter to match this. Added BattModel object to monitor class, which will run all monitoring functions as required, keeping main code compact. Further updated defines, added simulation parameters to defines file in preparation for initialisation of this in main, and to allow easy user changing.

### 02 Feb 2024

* *1 hour* Updated class structure to separate battery model from surounding functionality. Begun initialising classes as required in main function, further code cleanup, removed unneeded setters/ getters.