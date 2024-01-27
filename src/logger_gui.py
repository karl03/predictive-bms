import serial
from serial.tools import list_ports
import time
from tkinter import *
from tkinter import ttk

root = Tk()
root.title('BMS Logger')
port = StringVar()
port.set("")

filepath = "./logs"

connected = False
ser = serial.Serial()
ser.baudrate = 115200
ser.timeout = 1

def refresh():
    selector.set_menu("AUTO", *["AUTO"] + [info[0] for info in list_ports.comports()])
    output.insert(END, "Refreshed Ports\n")

def clear():
    output.delete("1.0", END)

def autoConnect():
    for port in list_ports.comports():
        output.insert(END, "Trying to connect to " + port[0] + "\n")
        ser.port = port[0]
        ser.open()
        if ser.is_open:
            ser.flush()
            try:
                recvd = ser.read_until(b"BMS").decode()
            except:
                output.insert(END, "Failed to connect\n")
                ser.close()
                continue
            if "BMS" in recvd:
                ser.write(b"hello")
                try:
                    if "hello" not in ser.readline().decode():
                        output.insert(END, "Failed to connect\n")
                        ser.close()
                        continue
                    else:
                        return True
                except:
                    output.insert(END, "Failed to connect\n")
                    ser.close()
                    continue
            else:
                output.insert(END, "Failed to connect\n")
                ser.close()
    
    return False

def connect():
    if (port.get() == "AUTO"):
        return autoConnect()
    
    output.insert(END, "Trying to connect to " + port.get() + "\n")
    ser.port = port.get()
    ser.open()
    if ser.is_open:
        ser.flush()
        try:
            recvd = ser.read_until(b"BMS").decode()
        except:
            output.insert(END, "Failed to connect\n")
            ser.close()
        else:
            if "BMS" in recvd:
                ser.write(b"hello")
                try:
                    if "hello" not in ser.readline().decode():
                        output.insert(END, "Failed to connect\n")
                        ser.close()
                    else:
                        return True
                except:
                    output.insert(END, "Failed to connect\n")
                    ser.close()
            else:
                output.insert(END, "Failed to connect\n")
                ser.close()


def connectedLoop():
    if connected:
        print(f"Connected Successfully to {ser.port}")
        file = open((filepath + f"/test-{time.strftime('%Y%m%d-%H%M%S')}.csv"), "x")
    else:
        print("Failed to Connect")


    try:
        while connected:
            try:
                if ser.in_waiting > 0:
                    line = ser.readline().decode().strip()
                    print(line)
                    file.write(line + "\n")
            except serial.SerialException:
                output.insert(END, "Serial Connection Lost\n")
                break
            except KeyboardInterrupt:
                output.insert(END, "Exiting\n")
                break
            except Exception as exception:
                output.insert(END, exception)
                break
    except:
        output.insert(END, "Exiting")


frame = ttk.Frame(root, padding="3 3 3 3")
frame.grid(column=0, row=0, sticky=(N, W, E, S))
root.columnconfigure(0, weight=1)
root.rowconfigure(0, weight=1)

selector = ttk.OptionMenu(frame, port, "AUTO", *["AUTO"] + [info[0] for info in list_ports.comports()])
selector.grid(column=0, row=0, sticky=(N, W))
connectBtn = ttk.Button(frame, text=("Connect" if connected == False else "Disconnect"), command=connect)
connectBtn.grid(column=0, row=0, sticky=(N))
refreshBtn = ttk.Button(frame, text="Refresh", command=refresh)
refreshBtn.grid(column=0, row=0, sticky=(N, E))
clearBtn = ttk.Button(frame, text="Clear Output", command=clear)
clearBtn.grid(column=2, row=0, sticky=(N, E))
output = Text(frame)
output.grid(column=0, row=1, columnspan=3)



root.mainloop()