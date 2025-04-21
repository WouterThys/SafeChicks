import json
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from matplotlib.patches import Rectangle
from datetime import datetime
import argparse

class StateEntry:
    def __init__(self, fromTime, tillTime, state):
        self.fromTime = fromTime
        self.tillTime = tillTime
        self.state = state
        if state == 'Sleep':
            self.color = 'midnightblue'
        elif state == 'Sensor':
            self.color = 'lime'
        elif state == 'MotorRun':
            self.color = 'orange'
        elif state == 'MotorCheck':
            self.color = 'orange'
        elif state == 'MotorStop':
            self.color = 'orange'
        else:
            self.color = 'white'

class LogEntry:
    def __init__(self, timestamp, data):
        self.timestamp = datetime.strptime(timestamp, "%H:%M:%S")
        self.epoch = data["epoch"]
        self.state = data["state"]
        self.next_state = data["next"]
        self.day = data["day"]
        self.lSensorValue = data["lSensorValue"]
        self.bSensorValue = data["bSensorValue"]
        self.error = data["error"]

    @staticmethod
    def from_log_line(log_line):
        parts = log_line.split(" ", 1)
        if len(parts) != 2:
            return None
        timestamp, json_part = parts
        data = json.loads(json_part)
        return LogEntry(timestamp, data)


def read_logs(filename):
    log_entries = []
    with open(filename, "r") as file:
        for line in file:
            entry = LogEntry.from_log_line(line.strip())
            if entry:
                log_entries.append(entry)
    return log_entries

def decode_states(log_entries):
    last_state = log_entries[0].state
    last_ts = log_entries[0].timestamp

    result = []
    for entry in log_entries:
        if entry.state != last_state:
            # add to list
            result.append(StateEntry(last_ts, entry.timestamp, last_state))
            last_ts = entry.timestamp

        last_state = entry.state

    return result

def low_pass_filter(data, alpha=0.1):
    filtered_data = [data[0]]  # Initialize with first value
    for i in range(1, len(data)):
        filtered_data.append(alpha * data[i] + (1 - alpha) * filtered_data[i - 1])
    return filtered_data

def fsm_error_as_string(error_str):

    # const uint8_t   ERROR_SENSORS_BOTH_CLOSED = 1;
    # const uint8_t   ERROR_SENSORS_UP_WHILE_NIGHT = 2;
    # const uint8_t   ERROR_SENSORS_DOWN_WHILE_DAY = 4;
    # const uint8_t   ERROR_MOTOR_RUN_TOO_LONG = 8;

    error_int = int(error_str)
    error_res = ""
    if error_int & 0x01:
        error_res += " Both sensors closed"
    if error_int & 0x02:
        error_res += " Sensor up while night"
    if error_int & 0x04:
        error_res += " Sensor down while day"
    if error_int & 0x08:
        error_res += " Motor turned too long"

    return error_res


def plot_sensor_values(log_entries, title):
    timestamps = [entry.timestamp for entry in log_entries]
    lSensorValues = [entry.lSensorValue for entry in log_entries]
    bSensorValues = [entry.bSensorValue for entry in log_entries]
    lSensorFiltered = low_pass_filter(lSensorValues, alpha=0.05)
    bSensorFiltered = low_pass_filter(bSensorValues, alpha=0.05)
    dayValues = [entry.day for entry in log_entries]

    # Create the figure
    fig, ax = plt.subplots(figsize=(10, 5))

    # sensor value lines
    line1, = ax.plot(timestamps, lSensorValues, label="Light Sensor", marker="o", linestyle="-", color='darkblue')
    ax.plot(timestamps, lSensorFiltered, label="LPF", linestyle="--", color='skyblue')
    line2, = ax.plot(timestamps, bSensorValues, label="Battery Sensor", marker="s", linestyle="-", color='orange')
    ax.plot(timestamps, bSensorFiltered, label="LPF", linestyle="--", color='gold')

    # draw the state
    states = decode_states(log_entries)
    
    for state in states:
        #print(f"-{state.fromTime} - {state.tillTime}: {state.state}")
        ax.add_patch(Rectangle((state.fromTime, -100), state.tillTime - state.fromTime, 80, color=state.color))

    # constant line indicators
    ax.axhline(y=200, color='y', linestyle='--', label='Day/night')
    ax.axhline(y=868, color='g', linestyle='--', label='Battery full')
    ax.axhline(y=640, color='r', linestyle='--', label='Battery dead')

    # Detect transition from night (False) to day (True) or other way around
    for i in range(len(timestamps)):
        if i > 0 and not dayValues[i-1] and dayValues[i]:
            ax.axvline(x=timestamps[i], color='blue', alpha=0.8, linestyle='-')  # Mark transition with a blue line

        if i > 0 and dayValues[i-1] and not dayValues[i]:
            ax.axvline(x=timestamps[i], color='blue', alpha=0.8, linestyle='-')  # Mark transition with a blue line
    
    # titles and meta
    ax.set_xlabel("Time")
    ax.set_ylabel("Sensor Values")
    ax.set_title(title)
    ax.legend()
    ax.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M'))
    ax.xaxis.set_major_locator(mdates.HourLocator(interval=1))  # Show every hour
    ax.grid()
    plt.xticks(rotation=45)
    
    # Add interactive tooltip
    annot = ax.annotate("", xy=(0, 0), xytext=(10, 10), textcoords="offset points",
                         bbox=dict(boxstyle="round", fc="w"), arrowprops=dict(arrowstyle="->"))
    annot.set_visible(False)
    
    def update_annot(line, ind):
        x, y = line.get_xdata()[ind[0]], line.get_ydata()[ind[0]]
        entry = next((e for e in log_entries if e.timestamp == x), None)
        if entry:

            # ADC has 1023 values, rule-of-3 back to actual value when REF is 3.3V
            light_in_v = round((entry.lSensorValue / 1023) * 3.3, 2)

            # battery can be at 4.2V, which is too much for the ADC. Attenuated
            # with resistor voltage divider by 2/3. Multiply back to get actual value
            battery_in_v = round(((entry.bSensorValue * 3 / 2) / 1023) * 3.3, 2)


            annot.xy = (x, y)
            text = f"""Values
 Time: {x.strftime('%H:%M:%S')}
 State: {entry.state}
 Next: {entry.next_state}
 Day: {entry.day}
 Light: {light_in_v}V (ADC: {entry.lSensorValue})
 Battery: {battery_in_v}V (ADC: {entry.bSensorValue})
 Error: {entry.error} {fsm_error_as_string(entry.error)}
"""
            annot.set_text(text)
            annot.get_bbox_patch().set_alpha(0.8)

    def on_hover(event):
        vis = annot.get_visible()
        if event.inaxes == ax:
            for line in [line1, line2]:
                cont, ind = line.contains(event)
                if cont:
                    update_annot(line, ind["ind"])
                    annot.set_visible(True)
                    fig.canvas.draw_idle()
                    return
        if vis:
            annot.set_visible(False)
            fig.canvas.draw_idle()
    
    fig.canvas.mpl_connect("motion_notify_event", on_hover)
    
    plt.show()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Plot sensor data from chicken door logs.")
    parser.add_argument("filename", type=str, help="Path to the log file")
    args = parser.parse_args()

    logs = read_logs(args.filename)
    plot_sensor_values(logs, args.filename)