#include <builtins.h>
#include <stdbool.h>

#include "FSM_Controller.h"

#include "../Drivers/ADC_Driver.h"
#include "../Drivers/MOTOR_Driver.h"
#include "../config.h"

#include <stdio.h>

/*******************************************************************************
 *                      Function and type definitions
 ******************************************************************************/

/* Enumeration to keep the FSM state */
typedef enum {
  Calculate, /* Calculate depending on input                             */
  Sleep,     /* Sleep for certain time before starting again             */

  MotorStart,   /* Start running the motor up or down                       */
  MotorRunning, /* Run the motor, read sensors to see if motor should stop  */
  MotorSlow,    /* Run the motor, slow, until sensor is out again           */
  MotorStop,    /* Stop running the motor                                   */

  ForceUp,   /* Force the motor to run in Up direction. No checks!       */
  ForceDown, /* Force the motor to run in Down direction. No checks!     */
} State;

/* All FSM variables and data  */
typedef struct {
  uint32_t epoch; // Clock counter, used for down-sampling some stuff
  State state;    // Current state
  State next;     // Next state

  // Day/Night parameters
  bool day;         // Flag to set if day
  uint8_t dayCount; // Helper for hysteresis

  // Sleep parameters
  uint8_t sleepCount; // Counter keeping how long we are sleeping

  // Motor parameters
  Direction motorDir;         // Up or down
  uint8_t motorSpeed;         // The speed of the motor in percentage
  uint32_t motorRunningCount; // Counter keeping how long the motor was running

  // Sensor values
  uint16_t lSensorValue; // Value of the light sensor
  uint16_t bSensorValue; // Battery sensor
  bool uSensorClosed;    // Value of the upper sensor
  bool lSwitchClosed;    // Value of the upper limit switch
  bool uButtonPushed;    // When UP button is pushed
  bool dButtonPushed;    // When DOWN button is pushed
  // TODO: one big 'enabled' flag

  // Error
  uint16_t error; // Last found error value

} Fsm;

#define isDay(fsm) (fsm->day)
#define isNight(fsm) (!(isDay(fsm)))
#define isDoorOpen(fsm) (fsm->uSensorClosed)
#define isDoorClosed(fsm) (fsm->motorRunningCount > MOTOR_DOWN_FULL_CNT)
#define isLimitSwitch(fsm) (fsm->lSwitchClosed)
#define isDirUp(fsm) (fsm->motorDir == Up)
#define isDirDown(fsm) (fsm->motorDir == Down)

/**
 * Execute the current state if the FSM.
 * @param fsm: pointer to the FSM
 */
static void state_execute(Fsm *fsm);

/**
 * Print out debug information for the current FSM variables.
 * This should set all status LEDs and write data to the
 * serial interface if enabled.
 * @param fsm: pointer to the FSM
 */
static void debug(Fsm *fsm);

/**
 * Do a sanity check on the FSM data. This will set error flags depending
 * on what is inside the FSM.
 * @param fsm: pointer to the FSM
 */
static void sanity_check(Fsm *fsm);

/**
 * Read the input sensors like switches, light (or solar panel) voltage,
 * buttons... and store them in the FSM.
 * @param fsm: pointer to the FSM
 */
static void read_input(Fsm *fsm);

/**
 * The buttons can be pressed at any time, check if this is the case and update
 * the states accordingly.
 * When the limit switch is closed it will stop whatever is happening and move
 * slowly down.
 * @param fsm
 */
static void check_force(Fsm *fsm);

/**
 * State function for State::Calculate
 * Takes the input values and calculates if it is currently day or night.
 * Depending on this the next state will be set to Sleep or MotorRun
 * @param fsm
 */
static void state_Calculate(Fsm *fsm);

/**
 * State function for State::Sleep
 * Takes down the whole FSM for certain time.
 * @param fsm
 */
static void state_Sleep(Fsm *fsm);

/**
 * State function for State::MotorStart
 * Ramp the motor up or down and go to State::MotorRunning when the motor is
 * at full speed
 * @param fsm
 */
static void state_MotorStart(Fsm *fsm);

/**
 * State function for State::MotorCheck
 * Check if the motor should stop running depending on input variables.
 * When the sensor is first seen go to State::MotorSlow
 * @param fsm
 */
static void state_MotorRunning(Fsm *fsm);

/**
 * State function for State::MotorSlow
 * Run the motor slowly until the sensor goes out again. This means the door
 * traveled long enough to be fully closed.
 * The next state will be State::MotorStop
 * @param fsm
 */
static void state_MotorSlow(Fsm *fsm);

/**
 * State function for State::MotorStop
 * Stop the motor and go back to State::Calculate when it is ramped to stop
 * @param fsm
 */
static void state_MotorStop(Fsm *fsm);

/**
 * State function for State::ForceUp
 * Force the motor in the up direction. NO sensors are checked and the motor
 * will keep running as long as the button is pressed.
 * @param fsm
 */
static void state_ForceUp(Fsm *fsm);

/**
 * State function for State::ForceDown
 * Force the motor in the up direction. NO sensors are checked and the motor
 * will keep running as long as the button is pressed.
 * @param fsm
 */
static void state_ForceDown(Fsm *fsm);

/*******************************************************************************
 *                      Variables
 ******************************************************************************/

Fsm fsm;
SleepHandler sleepHandler;

/*******************************************************************************
 *                      Public function implementation
 ******************************************************************************/

void C_FSM_Init(SleepHandler handler) {

  sleepHandler = handler;

  //  Pin control1
  U_SENSOR_Dir = 1;
  L_SWITCH_Dir = 1;
  U_BUTTON_Dir = 1;
  D_BUTTON_Dir = 1;

  // Setup the state
  fsm.state = Calculate;
  fsm.next = Calculate;
  fsm.dayCount = DAY_COUNT; // Probably install while day?
  fsm.sleepCount = 0;
  fsm.motorSpeed = 0;
  fsm.motorRunningCount = 0;
  fsm.lSensorValue = 0;
  fsm.uSensorClosed = false;
  fsm.lSwitchClosed = false;
  fsm.uButtonPushed = false;
  fsm.dButtonPushed = false;
  fsm.error = 0;
}

/* Run the FSM one time */
void C_FSM_Tick(void) {

  fsm.state = fsm.next;

  read_input(&fsm);
  sanity_check(&fsm);
  check_force(&fsm);
  state_execute(&fsm);

  fsm.epoch++;
}

void C_FSM_ToString(char *dst, uint8_t size) {

  char state = ((char)fsm.state) + 48;

  snprintf(dst, size,
	//s  is day     dayCount   sleepCount lSensor     bSensor     uSensor    lSwitch	   error
	"%c,%" PRIu8 ",%" PRIu8 ",%" PRIu8 ",%" PRIu16 ",%" PRIu16 ",%" PRIu8 ",%" PRIu8 ",%" PRIu16 "\r\n", 
	state,
	fsm.day,
	fsm.dayCount,
	fsm.sleepCount,
	fsm.lSensorValue,
	fsm.bSensorValue,
	fsm.uSensorClosed,
	fsm.lSwitchClosed,
	fsm.error);
}

/*******************************************************************************
 *                      Private function implementations
 ******************************************************************************/

void debug(Fsm *fsm) {

  if (isDay(fsm)) {
    LED_BLUE_Pin = 1;
  } else {
    LED_BLUE_Pin = 0;
  }

  if (fsm->error != 0) {
    LED_RED_Pin = 1;
  } else {
    LED_RED_Pin = 0;
  }

  // TODO: advanced blinking patterns?
}

void read_input(Fsm *fsm) {

  fsm->lSensorValue = D_ADC_ReadOnce();

  fsm->uSensorClosed = U_SENSOR_Pin == 1;
  fsm->lSwitchClosed = L_SWITCH_Pin == 1;
  fsm->uButtonPushed = U_BUTTON_Pin == 1;
  fsm->dButtonPushed = D_BUTTON_Pin == 1;

  debug(fsm);
}

void sanity_check(Fsm *fsm) {
  fsm->error = 0;

  if (isDoorOpen(fsm) && isNight(fsm)) {
    fsm->error |= ERROR_SENSORS_UP_WHILE_NIGHT;
  }
  if (fsm->motorRunningCount >= MAX_MOTOR_COUNT) {
    fsm->error |= ERROR_MOTOR_RUN_TOO_LONG;
  }
  if (isLimitSwitch(fsm)) {
    // Stop unless moving down
    fsm->error |= ERROR_LIMIT_SWITCH_CLOSED;
  }
}

void check_force(Fsm *fsm) {

  if (isLimitSwitch(fsm)) {
    // Stop unless moving down
    if (fsm->motorDir == Up && fsm->motorSpeed > 0) {
	  // Reverse the direction and move slowly down again	
      fsm->motorSpeed = MOTOR_HALF_SPEED;
	  fsm->motorDir = Down;
      D_MOTOR_Run(fsm->motorDir, fsm->motorSpeed);
	  __delay_ms(1000);
	  // Go to stop state
      fsm->state = MotorStop;
      fsm->next = MotorStop;
    }
  }
  // Buttons can still overwrite?
  if (fsm->uButtonPushed) {
    fsm->state = ForceUp;
  }
  if (fsm->dButtonPushed) {
    fsm->state = ForceDown;
  }
}

void state_execute(Fsm *fsm) {
  switch (fsm->state) {
  case Calculate:
    state_Calculate(fsm);
    break;
  case Sleep:
    state_Sleep(fsm);
    break;
  case MotorStart:
    state_MotorStart(fsm);
    break;
  case MotorRunning:
    state_MotorRunning(fsm);
    break;
  case MotorSlow:
    state_MotorSlow(fsm);
    break;
  case MotorStop:
    state_MotorStop(fsm);
    break;
  case ForceUp:
    state_ForceUp(fsm);
    break;
  case ForceDown:
    state_ForceDown(fsm);
    break;
  }
}



void state_Calculate(Fsm *fsm) {
  bool changed = false;

  /* Handle state */
  if (fsm->lSensorValue < NIGHT_THRESHOLD) {
    // Reading a nighttime value
    if (fsm->dayCount == 0) {
      // Counted enough nighttime values -> update
      // Note: we get here all the time so only set changed flag when needed
      changed = fsm->day; // When day was true this will set changed
      fsm->day = false;
      fsm->motorDir = Down;
      fsm->motorSpeed = 0;
      fsm->motorRunningCount = 0;
    } else {
      changed = false;
      fsm->dayCount--;
    }
  } else if (fsm->lSensorValue > DAY_THRESHOLD) {
    // Reading a daytime values
    if (fsm->dayCount >= DAY_COUNT) {
      // Counted enough daytime values -> update
      // Note: we get here all the time so only set changed flag when needed
      changed = !fsm->day;
      fsm->day = true;
      fsm->motorDir = Up;
      fsm->motorSpeed = 0;
      fsm->motorRunningCount = 0;
    } else {
      changed = false;
      fsm->dayCount++;
    }
  }

  /* Decide on next state */
  if (changed) {
    fsm->next = MotorStart;
  } else {
    fsm->next = Sleep;
  }
}

void state_Sleep(Fsm *fsm) {
  /* Handle state */
  sleepHandler();
  fsm->sleepCount++;

  /* Decide on next state */
  if (fsm->sleepCount >= SLEEP_COUNT) {
    fsm->sleepCount = 0;
    // Wake up
    fsm->next = Calculate;
  } else {
    // Stay asleep
    fsm->next = Sleep;
  }
}

void state_MotorStart(Fsm *fsm) {
  /* Handle state */

  fsm->motorRunningCount++;
  bool stopNow = false;

  if (fsm->motorDir == Up && isDoorOpen(fsm)) {
    stopNow = true;
  } else if (fsm->motorDir == Down && isDoorClosed(fsm)) {
    stopNow = true;
  } else {
    D_MOTOR_Run(fsm->motorDir, fsm->motorSpeed);
    fsm->motorSpeed++;
  }

  /* Decide on next state */
  if (stopNow) {
    /* We have hit a switch, stop immediately */
    fsm->next = MotorStop;
  } else if (fsm->motorSpeed > MOTOR_FULL_SPEED) {
    /* Ramped up, go to running state */
    fsm->motorRunningCount = 0;
    fsm->next = MotorRunning;
  } else {
    /* Still ramping */
    fsm->next = MotorStart;
  }
}

void state_MotorRunning(Fsm *fsm) {
  /* Handle state */
  fsm->motorRunningCount++;

  /* Decide on next state */
  if (fsm->motorRunningCount > MAX_MOTOR_COUNT) {
    // Something went wrong. Stop
    fsm->next = MotorStop;
    return;
  }
  if (isDirUp(fsm)) {
    // Slow down when upper sensor is seen.
    if (isDoorOpen(fsm)) {
      fsm->motorRunningCount = 0;
      fsm->next = MotorSlow;
      return;
    }
  }
  if (isDirDown(fsm)) {
    // Slow down after a certain period of time
    if (isDoorClosed(fsm)) {
      fsm->motorRunningCount = 0;
      fsm->next = MotorSlow;
      return;
    }
  }

  // Keep in the current state
  fsm->next = MotorRunning;
}

void state_MotorSlow(Fsm *fsm) {
  /* Handle state */

  fsm->motorRunningCount++;

  // Slow down to half%
  if (fsm->motorSpeed > MOTOR_HALF_SPEED) {
    fsm->motorSpeed--;
    D_MOTOR_Run(fsm->motorDir, fsm->motorSpeed);
  }

  /* Decide on next state */
  if (isDirUp(fsm)) {
    // Opening door, upper sensor goes out again
    if (!isDoorOpen(fsm)) {
      // Ready to really stop now
      fsm->next = MotorStop;
      return;
    }
  }
  if (isDirDown(fsm)) {
    // Stop down after a certain period of time
    if (fsm->motorRunningCount >= MOTOR_DOWN_SLOW_CNT) {
      fsm->next = MotorStop;
      return;
    }
  }

  // Keep going
  fsm->next = MotorSlow;
}

void state_MotorStop(Fsm *fsm) {
  /* Handle state */

  if (fsm->motorSpeed > 0) {
    fsm->motorSpeed--;
    D_MOTOR_Run(fsm->motorDir, fsm->motorSpeed);
  }

  /* Decide on next state */
  if (fsm->motorSpeed == 0) {
    fsm->next = Calculate;
  } else {
    fsm->next = MotorStop;
  }
}

void state_ForceUp(Fsm *fsm) {

  /* Handle state */
  fsm->motorDir = Up;
  if (isLimitSwitch(fsm)) {
    // We went too far
    fsm->motorSpeed = 0;
  } else {
    // Ramp up
    if (fsm->motorSpeed < 100) {
      fsm->motorSpeed++;
    }
  }
  D_MOTOR_Run(fsm->motorDir, fsm->motorSpeed);

  /* Decide on next state */
  if (fsm->uButtonPushed) {
    fsm->next = ForceUp;
  } else {
    fsm->next = MotorStop;
  }
}

void state_ForceDown(Fsm *fsm) {
  /* Handle state */

  fsm->motorDir = Down;
  if (fsm->motorSpeed < 100) {
    fsm->motorSpeed++;
  }
  D_MOTOR_Run(fsm->motorDir, fsm->motorSpeed);

  /* Decide on next state */
  if (fsm->dButtonPushed) {
    fsm->next = ForceDown;
  } else {
    fsm->next = MotorStop;
  }
}
