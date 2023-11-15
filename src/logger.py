import os
import serial
from serial.tools import list_ports
import time

filepath = "./logs"
port = ""

connected = False
ser = serial.Serial()
ser.baudrate = 115200
ser.timeout = 1

if port == "":
    for port in list_ports.comports():
        print("Trying to connect to " + port[0])
        ser.port = port[0]
        ser.open()
        if ser.is_open:
            ser.write(b"hello")
            if ser.readline().decode().strip() == "BMS":
                connected = True
                break
            else:
                ser.close()

else:
    ser.port = port
    ser.open()
    if ser.is_open:
        ser.write(b"hello")
        if ser.readline().decode().strip() == "BMS":
            connected = True
    else:
        ser.close()


if connected:
    print(f"Connected Successfully to {ser.port}")
    if not os.path.exists(filepath):
        os.makedirs(filepath)
    file = open((filepath + f"/test-{time.strftime('%Y%m%d-%H%M%S')}.csv"), "x")
else:
    print("Failed to Connect")


while connected:
    try:
        if ser.in_waiting > 0:
            line = ser.readline().decode().strip()
            print(line)
            file.write(line + "\n")
    except serial.SerialException:
        print("Serial Connection Lost")
        break
    except KeyboardInterrupt:
        print("Exiting")
        break
    except Exception as exception:
        print(exception)
        break

if connected:
    ser.close()
    file.close()