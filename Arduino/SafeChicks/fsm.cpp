#include "fsm.h"
#include "motor.h"
#include "pinout.h"

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
  bool day;         // True for daytime, false for nighttime
  uint8_t dayCount; // Helper for hysteresis

  // Sleep parameters
  uint16_t sleepCount; // Counter keeping how long we are sleeping

  // Sensor values
  uint16_t lSensorValue; // Value of the light sensor
  bool uSensorValue; // Value of the upper sensor
  bool bSensorValue; // Value of the bottom sensor
};

Fsm fsm;

void print_state(enum State state)
{
  switch (state)
  {
  case State::Sensor:
    Serial.println("Sensor");
    break;
  case State::Sleep:
    Serial.println("Sleep");
    break;
  case State::MotorRun:
    Serial.println("MotorRun");
    break;
  case State::MotorCheck:
    Serial.println("MotorCheck");
    break;
  case State::MotorStop:
    Serial.println("MotorStop");
    break;
  default:
    Serial.println("UNKNOWN STATE");
    break;
  }
}

void debug(Fsm &fsm)
{
  Serial.println("--- FSM STATE ---");
  Serial.print(" - epoch: ");
  Serial.println(fsm.epoch);
  Serial.print(" - state: ");
  print_state(fsm.state);
  Serial.print(" - next: ");
  print_state(fsm.next);
  Serial.print(" - day: ");
  Serial.println(fsm.day);
  Serial.print(" - dayCount: ");
  Serial.println(fsm.dayCount);
  Serial.print(" - sleepCount: ");
  Serial.print(fsm.sleepCount);
  Serial.println("s");
  Serial.print(" - lSensorValue: ");
  Serial.println(fsm.lSensorValue);
  Serial.print(" - uSensorValue: ");
  Serial.println(fsm.uSensorValue);
  Serial.print(" - bSensorValue: ");
  Serial.println(fsm.bSensorValue);

  Serial.println("");
}

void read_input(Fsm &fsm)
{
  fsm.lSensorValue = analogRead(PIN_LSENSOR);
  fsm.uSensorValue = digitalRead(PIN_USENSOR);
  fsm.bSensorValue = digitalRead(PIN_BSENSOR);
}

void state_Sensor(Fsm &fsm)
{
  bool changed = false;

  /* Handle state */
  if (fsm.lSensorValue < THR_DAY_NIGHT)
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
  else
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
  if (fsm.epoch % 100 == 0)
  {
    fsm.sleepCount++;
  }

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
  if (fsm.day)
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
  // Nothing to do, motor should be running

  /* Decide on next state */
  if (fsm.day && fsm.uSensorValue == 0)
  {
    // Upper sensor sees the door!
    fsm.next = State::MotorStop;
  }
  else if (!fsm.day && fsm.bSensorValue == 0)
  {
    // Bottom sensor sees the door!
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
  fsm.state = fsm.next;

  read_input(fsm);
  state_execute(fsm);

  // Debug every 5s or when state changed
  if (/*fsm.epoch % 500 == 0 || */ fsm.state != fsm.next)
  {
    debug(fsm);
  }
}