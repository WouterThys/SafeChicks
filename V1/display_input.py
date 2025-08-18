import serial
import json
import threading
import tkinter as tk
from tkinter import ttk

# COM port configuration
COM_PORT = 'COM4'  # Change to your actual COM port
BAUD_RATE = 9600

ERROR_FLAGS = {
    1:  "SENSORS_BOTH_CLOSED",
    2:  "SENSORS_UP_WHILE_NIGHT",
    4:  "SENSORS_DOWN_WHILE_DAY",
    8:  "MOTOR_RUN_TOO_LONG"
}

class SerialJSONReader:
    def __init__(self, port, baudrate, on_data_callback, on_message_callback):
        self.port = port
        self.baudrate = baudrate
        self.on_data_callback = on_data_callback
        self.on_message_callback = on_message_callback
        self.serial = None
        self.running = True
        self.thread = threading.Thread(target=self.read_serial)
        self.thread.daemon = True
        self.thread.start()

    def read_serial(self):
        try:
            self.serial = serial.Serial(self.port, self.baudrate, timeout=1)
            while self.running:
                line = self.serial.readline().decode('utf-8', errors='ignore').strip()
                if line.startswith('{') and line.endswith('}'):
                    print(f"State: '{line}'")
                    try:
                        data = json.loads(line)
                        self.on_data_callback(data)
                    except json.JSONDecodeError:
                        pass  # Ignore malformed JSON
                else:
                    if line:
                        self.on_message_callback(line)
                        print(f"Message: '{line}'")
        except serial.SerialException as e:
            print(f"Serial error: {e}")

    def send_command(self, command):
        if self.serial and self.serial.is_open:
            self.serial.write(command.encode('utf-8'))

    def stop(self):
        self.running = False
        if self.serial and self.serial.is_open:
            self.serial.close()

class JSONDisplayApp:
    def __init__(self, root, serial_reader):
        self.root = root
        self.reader = serial_reader
        self.root.title("Serial JSON Viewer")

        self.data_labels = {}
        self.frame = ttk.Frame(root, padding="10")
        self.frame.grid()

        # Prepare empty layout with known keys
        self.fields = [
            "epoch", "state", "next", "day", "dayCount",
            "sleepCount", "motorRunningCount", "lSensorValue",
            "bSensorValue", "uSensorClosed", "bSensorClosed", "error"
        ]

        for idx, key in enumerate(self.fields):
            ttk.Label(self.frame, text=f"{key}:").grid(row=idx, column=0, sticky=tk.W, padx=5, pady=2)
            label = ttk.Label(self.frame, text="-", width=50)
            label.grid(row=idx, column=1, sticky=tk.W, padx=5, pady=2)
            self.data_labels[key] = label

        # Message label
        self.message_var = tk.StringVar()
        self.message_label = ttk.Label(root, textvariable=self.message_var, foreground="blue", anchor="center")
        self.message_label.grid(row=1, column=0, sticky="ew", padx=10, pady=(5, 5))
        root.grid_rowconfigure(1, weight=0)
        root.grid_columnconfigure(0, weight=1)

        # Buttons for sending commands
        self.button_frame = ttk.Frame(root, padding="10")
        self.button_frame.grid()

        self.up_button = ttk.Button(self.button_frame, text="Send U", command=self.send_u)
        self.up_button.grid(row=0, column=0, padx=5)

        self.down_button = ttk.Button(self.button_frame, text="Send D", command=self.send_d)
        self.down_button.grid(row=0, column=1, padx=5)

    def update_data(self, data):
        def do_update():
            for key, label in self.data_labels.items():
                if key in data:
                    value = data[key]
                    if key == "error":
                        value = self.error_to_str(value)
                    label.config(text=str(value))

        self.root.after(0, do_update)

    def update_message(self, text, duration_ms=3000):
        self.message_var.set(text)
        if duration_ms:
            self.root.after(duration_ms, lambda: self.message_var.set(""))

    def error_to_str(self, error_value):
        flags = []
        for bit, desc in ERROR_FLAGS.items():
            if error_value & bit:
                flags.append(desc)
        return ', '.join(flags) if flags else "None"


    def send_u(self):
        self.reader.send_command('U')

    def send_d(self):
        self.reader.send_command('D')

def main():
    root = tk.Tk()
    reader = SerialJSONReader(COM_PORT, BAUD_RATE, lambda data: app.update_data(data), lambda msg: app.update_message(msg))
    app = JSONDisplayApp(root, reader)

    def on_close():
        reader.stop()
        root.destroy()

    root.protocol("WM_DELETE_WINDOW", on_close)
    root.mainloop()

if __name__ == "__main__":
    main()
