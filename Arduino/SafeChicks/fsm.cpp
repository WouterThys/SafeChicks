#include "fsm.h"
#include "pinout.h"

// Private stuff -----------------------------------------------------------------------------

enum State
{
  Sensor,
  Sleep,
  Door,
  MotorUp,
  MotorDown
};

struct Fsm
{
  uint32_t epoch; // Clock counter, used for down-sampling some stuff
  State state; // Current state
  State next;  // Next state

  // Day/Night parameters
  bool day;         // True for daytime, false for nighttime
  bool changed;     // Indicates value has been changed
  uint8_t dayCount; // Helper for hysteresis

  // Sensor values
  uint16_t lSensorValue; // Value of the light sensor
  uint16_t uSensorValue; // Value of the upper sensor
  uint16_t bSensorValue; // Value of the bottom sensor
};

Fsm fsm;

void debug(Fsm& fsm) 
{
  Serial.println("--- FSM STATE ---");
  Serial.print(" - epoch: "); Serial.println(fsm.epoch);
  Serial.print(" - state: "); Serial.println(fsm.state);
  Serial.print(" - next: "); Serial.println(fsm.next);
  Serial.print(" - day: "); Serial.println(fsm.day);
  Serial.print(" - changed: "); Serial.println(fsm.changed);
  Serial.print(" - dayCount: "); Serial.println(fsm.dayCount);
  Serial.print(" - lSensorValue: "); Serial.println(fsm.lSensorValue);
  Serial.print(" - uSensorValue: "); Serial.println(fsm.uSensorValue);
  Serial.print(" - bSensorValue: "); Serial.println(fsm.bSensorValue);

  Serial.println("");
}

void read_input(Fsm &fsm)
{
  fsm.lSensorValue = analogRead(PIN_LSENSOR);
  fsm.uSensorValue = analogRead(PIN_USENSOR);
  fsm.bSensorValue = analogRead(PIN_BSENSOR);
}

void state_Sensor(Fsm &fsm)
{

  if (fsm.lSensorValue < THRESHOLD_DAY_NIGHT)
  {
    // Reading a nighttime value
    if (fsm.dayCount == 0)
    {
      // Counted enough nighttime values -> update
      // Note: we get here all the time so only set changed flag when needed
      fsm.changed = fsm.day; // When day was true this will set changed
      fsm.day = false;
    }
    else
    {
      fsm.changed = false;
      fsm.dayCount--;
    }
  }
  else
  {
    // Reading a daytime values
    if (fsm.dayCount > THRESHOLD_DAY_NIGHT)
    {
      // Counted enough daytime values -> update
      // Note: we get here all the time so only set changed flag when needed
      fsm.changed = !fsm.day;
      fsm.day = true;
    }
    else
    {
      fsm.changed = false;
      fsm.dayCount++;
    }
  }
}

void state_Sleep(Fsm &fsm)
{
}

void state_Door(Fsm &fsm)
{
}

void state_MotorUp(Fsm &fsm)
{
}

void state_MotorDown(Fsm &fsm)
{
}

void execute_state(Fsm &fsm)
{
  switch (fsm.state)
  {
  case State::Sensor:
    state_Sensor(fsm);
    break;
  case State::Sleep:
    state_Sleep(fsm);
    break;
  case State::Door:
    state_Door(fsm);
    break;
  case State::MotorUp:
    state_MotorUp(fsm);
    break;
  case State::MotorDown:
    state_MotorDown(fsm);
    break;
  }
}

// Public stuff -----------------------------------------------------------------------------

void fsm_setup()
{

  // Pin control
  pinMode(PIN_LSENSOR, INPUT);
  pinMode(PIN_USENSOR, INPUT);
  pinMode(PIN_BSENSOR, INPUT);

  // Setup the state
  fsm.state = State::Sensor;
  fsm.next = State::Sensor;
  fsm.day = false;
  fsm.lSensorValue = 0;
  fsm.uSensorValue = 0;
  fsm.bSensorValue = 0;
}

/* Run the FSM one time */
void fsm_tick()
{
  fsm.epoch++;
  read_input(fsm);
  execute_state(fsm); // Also does transition?

  if (fsm.epoch % 100 == 0) 
  {
    debug(fsm);
  }
}