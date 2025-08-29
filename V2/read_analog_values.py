import serial
import argparse

def main():
    parser = argparse.ArgumentParser(description="Read serial input from a COM port.")
    parser.add_argument("--port", default="COM3", help="COM port to use (default: COM3)")
    parser.add_argument("--baud", type=int, default=9600, help="Baud rate (default: 9600)")
    args = parser.parse_args()

    try:
        ser = serial.Serial(args.port, args.baud, timeout=1)
        print(f"Listening on {args.port} at {args.baud} baud...\nPress Ctrl+C to exit.\n")

        while True:
            if ser.in_waiting > 0:
                analog_value = ser.readline().decode(errors="ignore").strip()
                if analog_value:
                    analog = int(analog_value)
                    voltage = analog * 5 / 1024
                    divided = voltage / 0.344
                    print(f"""{analog_value} => {round(voltage, 2)}V ({round(divided, 2)}V)""")

    except serial.SerialException as e:
        print(f"Error opening serial port: {e}")
    except KeyboardInterrupt:
        print("\nExiting...")
    finally:
        try:
            ser.close()
        except:
            pass

if __name__ == "__main__":
    main()