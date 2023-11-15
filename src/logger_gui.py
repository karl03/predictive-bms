import serial
from serial.tools import list_ports
import time
from tkinter import *
from tkinter import ttk

root = Tk()
root.title('BMS Logger')
port = StringVar()
port.set("AUTO")
ports = StringVar()
ports.set(list_ports.comports())

filepath = "./logs"

connected = False
ser = serial.Serial()
ser.baudrate = 115200
ser.timeout = 1

def refresh(menu):
    ports.set([info[0] for info in list_ports.comports()])

def autoConnect():
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

def connect():
    ser.port = port.get()
    ser.open()
    if ser.is_open:
        ser.write(b"hello")
        if ser.readline().decode().strip() == "BMS":
            connected = True
    else:
        ser.close()


def connectedLoop():
    if connected:
        print(f"Connected Successfully to {ser.port}")
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


frame = ttk.Frame(root, padding="3 3 3 3")
frame.grid(column=0, row=0, sticky=(N, W, E, S))
root.columnconfigure(0, weight=1)
root.rowconfigure(0, weight=1)

selector = ttk.OptionMenu(frame, port, *["AUTO", ports.get()]).grid(column=0, row=0, sticky=(N, W))
connectBtn = ttk.Button(frame, text=("Connect" if connected == False else "Disconnect")).grid(column=1, row=0, sticky=(N))
refreshBtn = ttk.Button(frame, text="Refresh", command=refresh).grid(column=2, row=0, sticky=(N, E))



root.mainloop()