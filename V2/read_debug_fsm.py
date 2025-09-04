import serial
import argparse
from datetime import datetime
from rich.console import Console
from rich.table import Table
from rich.panel import Panel

# Mapping of state numbers to names
STATE_MAP = {
    0: "Calculate",
    1: "Sleep",
    2: "MotorStart",
    3: "MotorRunning",
    4: "MotorSlow",
    5: "MotorStop",
    6: "ForceUp",
    7: "ForceDown",
}

# Error bitfield mapping
ERROR_FLAGS = {
    1: "LIMIT_SWITCH_CLOSED",
    2: "SENSORS_UP_WHILE_NIGHT",
    4: "SENSORS_DOWN_WHILE_DAY",
    8: "MOTOR_RUN_TOO_LONG",
}

console = Console()

def decode_errors(error_val: int):
    """Return list of active error names from bitfield"""
    errors = [name for bit, name in ERROR_FLAGS.items() if error_val & bit]
    return errors if errors else ["None"]

def parse_line(line):
    """Parse CSV line into structured dict"""
    try:
        parts = line.strip().split(",")
        if len(parts) != 9:
            print(f"ERROR: Invalid packet length: {len(parts)}")
            return None  # Invalid packet length

        state = STATE_MAP.get(int(parts[0]), f"Unknown({parts[0]})")
        day = bool(int(parts[1]))
        dayCount = int(parts[2])
        sleepCount = int(parts[3])
        lSensor = int(parts[4])
        bSensor = int(parts[5])
        uSensor = bool(int(parts[6]))
        lSwitch = bool(int(parts[7]))
        error_val = int(parts[8])

        return {
            "State": state,
            "IsDay": day,
            "DayCount": f"{dayCount}/3",
            "SleepCount": f"{sleepCount}/5",
            "lSensor": lSensor,
            "bSensor": bSensor,
            "uSensor": uSensor,
            "lSwitch": lSwitch,
            "Error Value": error_val,
            "Error Flags": ", ".join(decode_errors(error_val)),
        }
    except Exception as ex:
        print(f"ERROR: Exception while parsing '{line}': {ex}")
        return None

def main():

    parser = argparse.ArgumentParser(description="Read serial input from a COM port.")
    parser.add_argument("--port", default="COM8", help="COM port to use (default: COM8)")
    parser.add_argument("--baud", type=int, default=1200, help="Baud rate (default: 1200)")
    args = parser.parse_args()

    with serial.Serial(args.port, args.baud, timeout=1) as ser:
        last_line = ""
        last_time = None

        while True:
            line = ser.readline().decode(errors="ignore")
            if not line.strip():
                continue

            data = parse_line(line)
            if not data:
                continue

            last_line = line.strip()
            last_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            title = f"FSM Status ({args.port}:{args.baud})"

            # Build table
            table = Table(title=title, show_header=True, header_style="bold cyan")
            table.add_column("Field", style="dim", width=12)
            table.add_column("Value", style="bold")

            for key, val in data.items():
                table.add_row(key, str(val))

            console.clear()
            if last_time:
                console.print(f"[bold green]Last update:[/bold green] {last_time}")
            console.print(table)
            if last_line:
                console.print(Panel(last_line, title="Raw Line", style="dim"))

if __name__ == "__main__":
    main()
