#include "fsm.h"
#include "motor.h"
#include "config.h"

#include <ArduinoLowPower.h>
#include <ArduinoJson.h>
#include <string>

// Private stuff -----------------------------------------------------------------------------

enum State
{
  Sensor,
  Sleep,
  MotorRun,
  MotorCheck,
  MotorStop
};

struct Fsm
{
  uint32_t epoch; // Clock counter, used for down-sampling some stuff
  State state;    // Current state
  State next;     // Next state

  // Day/Night parameters
  bool day; // Flag to set if day
  uint8_t dayCount; // Helper for hysteresis

  // Sleep parameters
  uint16_t sleepCount; // Counter keeping how long we are sleeping

  // Motor parameters
  uint16_t motorRunningCount;// Counter keeping how long the motor was running

  // Sensor values
  uint16_t lSensorValue; // Value of the light sensor
  uint16_t bSensorValue; // Battery sensor
  bool uSensorClosed; // Value of the upper sensor, True when the sensor is closed
  bool bSensorClosed; // Value of the bottom sensor, True when the sensor is closed

  // Error
  uint16_t error; // Last found error value

  // Helpers

  /* It is day */
  bool isDay() 
  {
    return day;
  }

  /* It is night */
  bool isNight() 
  {
    return !day;
  }

  /* The door is open */
  bool isDoorOpen() 
  {
    return uSensorClosed;
  }

  /* The door is closed */
  bool isDoorClosed() 
  {
    return bSensorClosed;
  }

};

Fsm fsm;

const char* print_state(enum State state)
{
  switch (state)
  {
  case State::Sensor:
    return ("Sensor");
    break;
  case State::Sleep:
    return ("Sleep");
    break;
  case State::MotorRun:
    return ("MotorRun");
    break;
  case State::MotorCheck:
    return ("MotorCheck");
    break;
  case State::MotorStop:
    return ("MotorStop");
    break;
  default:
    return ("UNKNOWN STATE");
    break;
  }
}

void debug(Fsm &fsm)
{
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  digitalWrite(PIN_ERROR_STATE, fsm.error > 0 ? HIGH : LOW);
  digitalWrite(PIN_DAY_STATE, fsm.isDay() ? HIGH : LOW);

  if (DEBUG_ENABLED) 
  {
    // Allocate the JSON document
    JsonDocument doc;

    // Add values in the document
    doc["epoch"] = fsm.epoch;
    doc["state"] = print_state(fsm.state);
    doc["next"] = print_state(fsm.next);
    doc["day"] = fsm.isDay();
    doc["dayCount"] = fsm.dayCount;
    doc["sleepCount"] = fsm.sleepCount;
    doc["motorRunningCount"] = fsm.motorRunningCount;
    doc["lSensorValue"] = fsm.lSensorValue;
    doc["bSensorValue"] = fsm.bSensorValue;
    doc["uSensorClosed"] = fsm.uSensorClosed;
    doc["bSensorClosed"] = fsm.bSensorClosed;
    doc["error"] = fsm.error;

    serializeJson(doc, Serial1);
    Serial1.print("\n");
    Serial1.flush(); // Flush because we are going to sleep soon
  }
}

void sanity_check(Fsm &fsm) 
{
  fsm.error = 0;

  // Bot sensors cannot be on
  if (fsm.isDoorOpen() && fsm.isDoorClosed()) 
  {
    fsm.error = ERROR_SENSORS_BOTH_CLOSED;
  }
  if (fsm.isDoorOpen() && fsm.isNight()) 
  {
    fsm.error |= ERROR_SENSORS_UP_WHILE_NIGHT;
  }
  if (fsm.isDoorClosed() && fsm.isDay()) 
  {
    fsm.error |= ERROR_SENSORS_DOWN_WHILE_DAY;
  }
  if (fsm.motorRunningCount >= MAX_MOTOR_COUNT) 
  {
    // It is back ok..
    if ((fsm.isDoorOpen() && fsm.isDay()) ||
        (fsm.isDoorClosed() && fsm.isNight()))
    {
      // Clear
      fsm.motorRunningCount = 0;
    }

    // Set error at least once, if cleared will not show again
    fsm.error |= ERROR_MOTOR_RUN_TOO_LONG;
  }

}

void read_input(Fsm &fsm)
{
  fsm.lSensorValue = analogRead(PIN_LSENSOR);
  fsm.bSensorValue = analogRead(PIN_BSENSOR);
  fsm.uSensorClosed = digitalRead(PIN_USWITCH) == LOW;
  fsm.bSensorClosed = digitalRead(PIN_BSWITCH) == LOW;

  sanity_check(fsm);
  debug(fsm);
}

void state_Sensor(Fsm &fsm)
{
  bool changed = false;

  /* Handle state */
  if (fsm.lSensorValue < THR_NIGHT)
  {
    // Reading a nighttime value
    if (fsm.dayCount == 0)
    {
      // Counted enough nighttime values -> update
      // Note: we get here all the time so only set changed flag when needed
      changed = fsm.day; // When day was true this will set changed
      fsm.day = false;
    }
    else
    {
      changed = false;
      fsm.dayCount--;
    }
  }
  else if (fsm.lSensorValue > THR_DAY)
  {
    // Reading a daytime values
    if (fsm.dayCount >= THR_DN_COUNT)
    {
      // Counted enough daytime values -> update
      // Note: we get here all the time so only set changed flag when needed
      changed = !fsm.day;
      fsm.day = true;
    }
    else
    {
      changed = false;
      fsm.dayCount++;
    }
  }

  /* Decide on next state */
  if (changed)
  {
    fsm.next = State::MotorRun;
  }
  else
  {
    fsm.next = State::Sleep;
  }
}

void state_Sleep(Fsm &fsm)
{
  /* Handle state */
  if (digitalRead(PIN_NOSLEEP) == LOW) 
  {
    LowPower.deepSleep(SLEEP_TIME_MS);
  }
  else 
  {
    Serial.println("ping");
    delay(10000);
  }
  fsm.sleepCount++;

  /* Decide on next state */
  if (fsm.sleepCount >= THR_SLEEP_COUNT)
  {
    fsm.sleepCount = 0;
    // Wake up
    fsm.next = State::Sensor;
  }
  else
  {
    // Stay asleep
    fsm.next = State::Sleep;
  }
}

void state_MotorRun(Fsm &fsm)
{
  /* Handle state */
  
  fsm.motorRunningCount = 0;

  if (fsm.isDay())
  {
    motor_start(Direction::Up);
  }
  else
  {
    motor_start(Direction::Down);
  }

  /* Decide on next state */
  fsm.next = State::MotorCheck;
}

void state_MotorCheck(Fsm &fsm)
{
  /* Handle state */
  fsm.motorRunningCount++;

  /* Decide on next state */
  if ((fsm.isDay() && fsm.isDoorOpen()) ||   // Opening door, upper sensor sees the door!
      (fsm.isNight() && fsm.isDoorClosed()) ||   // Closing door, bottom sensor sees the door!
      (fsm.motorRunningCount > MAX_MOTOR_COUNT)) // This is taking too long
  {
    fsm.next = State::MotorStop;
  }
  else
  {
    fsm.next = State::MotorCheck;
  }
}

void state_MotorStop(Fsm &fsm)
{
  /* Handle state */
  motor_stop();

  /* Decide on next state */
  fsm.next = State::Sensor;
}

void state_execute(Fsm &fsm)
{
  switch (fsm.state)
  {
  case State::Sensor:
    state_Sensor(fsm);
    break;
  case State::Sleep:
    state_Sleep(fsm);
    break;
  case State::MotorRun:
    state_MotorRun(fsm);
    break;
  case State::MotorCheck:
    state_MotorCheck(fsm);
    break;
  case State::MotorStop:
    state_MotorStop(fsm);
    break;
  }
}

// Public stuff -----------------------------------------------------------------------------

void fsm_setup()
{
  if (DEBUG_ENABLED) 
  {
    Serial.begin(9600);
    Serial1.begin(9600);
  }

  // Pin control
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_DAY_STATE, OUTPUT);
  pinMode(PIN_ERROR_STATE, OUTPUT);
  
  pinMode(PIN_LSENSOR, INPUT);
  pinMode(PIN_BSENSOR, INPUT);
  pinMode(PIN_USWITCH, INPUT);
  pinMode(PIN_BSWITCH, INPUT);
  pinMode(PIN_NOSLEEP, INPUT);

  // Setup the state
  fsm.state = State::Sensor;
  fsm.next = State::Sensor;
  fsm.dayCount = THR_DN_COUNT; // Probably install while day? 
  fsm.sleepCount = 0;
  fsm.motorRunningCount = 0;
  fsm.lSensorValue = 0;
  fsm.uSensorClosed = false;
  fsm.bSensorClosed = false;
  fsm.error = 0;
}

/* Run the FSM one time */
void fsm_tick()
{
  fsm.epoch++;
  fsm.state = fsm.next;

  read_input(fsm);
  state_execute(fsm);

  delay(FSM_PERIOD_MS);
}