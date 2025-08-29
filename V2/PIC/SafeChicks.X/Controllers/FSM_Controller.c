#include <stdbool.h>

#include "FSM_Controller.h"

#include "../Drivers/MOTOR_Driver.h"
#include "../Drivers/UART_Driver.h"
#include "../config.h"

#if DEBUG_MODE
#include <stdio.h>
#include <string.h>
#endif

/*******************************************************************************
 *                      Function and type definitions 
 ******************************************************************************/

/* Enumeration to keep the FSM state */
typedef enum {
    Calculate,    /* Calculate depending on input                             */
    Sleep,        /* Sleep for certain time before starting again             */

    MotorStart,   /* Start running the motor up or down                       */
    MotorRunning, /* Run the motor, read sensors to see if motor should stop  */
    MotorStop,    /* Stop running the motor                                   */

    ForceUp,      /* Force the motor to run in Up direction. No checks!       */
    ForceDown,    /* Force the motor to run in Down direction. No checks!     */
} State;

/* All FSM variables and data  */
typedef struct {
    uint32_t epoch; // Clock counter, used for down-sampling some stuff
    State state; // Current state
    State next; // Next state

    // Day/Night parameters
    bool day; // Flag to set if day
    uint8_t dayCount; // Helper for hysteresis

    // Sleep parameters
    uint16_t sleepCount; // Counter keeping how long we are sleeping

    // Motor parameters
    Direction motorDir; // Up or down
    uint8_t motorSpeed; // The speed of the motor in percentage
    uint16_t motorRunningCount; // Counter keeping how long the motor was running

    // Sensor values
    uint16_t lSensorValue; // Value of the light sensor
    uint16_t bSensorValue; // Battery sensor
    bool uSensorClosed; // Value of the upper sensor
    bool bSensorClosed; // Value of the bottom sensor
    bool uButtonPushed; // When UP button is pushed
    bool dButtonPushed; // When DOWN button is pushed
    // TODO: one big 'enabled' flag

    // Error
    uint16_t error; // Last found error value

} Fsm;

#define isDay(fsm)          (fsm->day)
#define isNight(fsm)        (!(isDay(fsm)))
#define isDoorOpen(fsm)     (fsm->uSensorClosed)
#define isDoorClosed(fsm)   (fsm->bSensorClosed)


/**
 * Execute the current state if the FSM.
 * @param fsm: pointer to the FSM
 */
static void state_execute(Fsm * fsm);

/**
 * Print out debug information for the current FSM variables.
 * This should set all status LEDs and write data to the
 * serial interface if enabled. 
 * @param fsm: pointer to the FSM
 */
static void debug(Fsm * fsm);

/**
 * Do a sanity check on the FSM data. This will set error flags depending
 * on what is inside the FSM.
 * @param fsm: pointer to the FSM
 */
static void sanity_check(Fsm * fsm);

/**
 * Read the input sensors like switches, light (or solar panel) voltage,
 * buttons... and store them in the FSM.
 * @param fsm: pointer to the FSM
 */
static void read_input(Fsm * fsm);

/**
 * The buttons can be pressed at any time, check if this is the case and update
 * the states accordingly.
 * @param fsm
 */
static void check_force(Fsm * fsm);

/**
 * State function for State::Calculate
 * Takes the input values and calculates if it is currently day or night.
 * Depending on this the next state will be set to Sleep or MotorRun
 * @param fsm
 */
static void state_Calculate(Fsm * fsm);

/**
 * State function for State::Sleep
 * Takes down the whole FSM for certain time.
 * @param fsm
 */
static void state_Sleep(Fsm * fsm);

/**
 * State function for State::MotorStart
 * Ramp the motor up or down and go to State::MotorRunning when the motor is
 * at full speed
 * @param fsm
 */
static void state_MotorStart(Fsm * fsm);

/**
 * State function for State::MotorCheck
 * Check if the motor should stop running depending on input variables.
 * When the motor should stop go to State::MotorStop
 * @param fsm
 */
static void state_MotorRunning(Fsm * fsm);

/**
 * State function for State::MotorStop
 * Stop the motor and go back to State::Calculate when it is ramped to stop
 * @param fsm
 */
static void state_MotorStop(Fsm * fsm);

/**
 * State function for State::ForceUp
 * Force the motor in the up direction. NO sensors are checked and the motor
 * will keep running as long as the button is pressed.
 * @param fsm
 */
static void state_ForceUp(Fsm * fsm);

/**
 * State function for State::ForceDown
 * Force the motor in the up direction. NO sensors are checked and the motor
 * will keep running as long as the button is pressed.
 * @param fsm
 */
static void state_ForceDown(Fsm * fsm);



/*******************************************************************************
 *                      Variables 
 ******************************************************************************/

Fsm fsm;

#if DEBUG_MODE
#define DEBUG_BUFFER_SIZE 100
char debugBuffer[DEBUG_BUFFER_SIZE];
#endif

/*******************************************************************************
 *                      Public function implementation 
 ******************************************************************************/

void C_FSM_Init() {

    //  Pin control
    U_SENSOR_Dir = 1;
    B_SENSOR_Dir = 1;
    U_BUTTON_Dir = 1;
    D_BUTTON_Dir = 1;

    // Setup the state
    fsm.state = Calculate;
    fsm.next = Calculate;
    fsm.dayCount = THR_DN_COUNT; // Probably install while day? 
    fsm.sleepCount = 0;
    fsm.motorSpeed = 0;
    fsm.motorRunningCount = 0;
    fsm.lSensorValue = 0;
    fsm.uSensorClosed = false;
    fsm.bSensorClosed = false;
    fsm.uButtonPushed = false;
    fsm.dButtonPushed = false;
    fsm.error = 0;
}

/* Run the FSM one time */
void C_FSM_Tick() {

    fsm.state = fsm.next;

    read_input(&fsm);
    sanity_check(&fsm);
    check_force(&fsm);
    state_execute(&fsm);

    fsm.epoch++;
}

/*******************************************************************************
 *                      Private function implementations 
 ******************************************************************************/

void debug(Fsm * fsm) {
    
//    if (isDay(fsm)) {
//        LED_BLUE_Pin = 1;
//    } else {
//        LED_BLUE_Pin = 0;
//    }
//    
//    if (fsm->error != 0) {
//        LED_RED_Pin = 1;
//    } else {
//        LED_RED_Pin = 0;
//    }
    
    if (fsm->uButtonPushed || fsm->dButtonPushed) {
        LED_RED_Pin = 1;
    } else {
        LED_RED_Pin = 0;
    }

#if DEBUG_MODE
    // Only check on multiples of 100 => every second
    if (fsm->epoch % 100 == 0) {

        char state = ((char) fsm->state) + 48;

        snprintf(debugBuffer, DEBUG_BUFFER_SIZE,
                "%lu,%c.\r\n",
                fsm->epoch,
                state);

        D_UART_Write(debugBuffer);

    }
#endif // DEBUG_MODE
}

void read_input(Fsm * fsm) {
    //  fsm.lSensorValue = analogRead(PIN_LSENSOR);
    //  fsm.bSensorValue = analogRead(PIN_BSENSOR);

    fsm->uSensorClosed = U_SENSOR_Pin == 1;
    fsm->bSensorClosed = B_SENSOR_Pin == 1;
    fsm->uButtonPushed = U_BUTTON_Pin == 1;
    fsm->dButtonPushed = D_BUTTON_Pin == 1;

    debug(fsm);
}

void sanity_check(Fsm * fsm) {
    fsm->error = 0;

    // Bot sensors cannot be on
    if (isDoorOpen(fsm) && isDoorClosed(fsm)) {
        fsm->error = ERROR_SENSORS_BOTH_CLOSED;
    }
    if (isDoorOpen(fsm) && isNight(fsm)) {
        fsm->error |= ERROR_SENSORS_UP_WHILE_NIGHT;
    }
    if (isDoorClosed(fsm) && isDay(fsm)) {
        fsm->error |= ERROR_SENSORS_DOWN_WHILE_DAY;
    }
    if (fsm->motorRunningCount >= MAX_MOTOR_COUNT) {
        // It is back OK..
        if ((isDoorOpen(fsm) && isDay(fsm)) ||
                (isDoorClosed(fsm) && isNight(fsm))) {
            // Clear
            fsm->motorRunningCount = 0;
        }

        // Set error at least once, if cleared will not show again
        fsm->error |= ERROR_MOTOR_RUN_TOO_LONG;
    }

}

void check_force(Fsm * fsm) {
    
    if (fsm->uButtonPushed) {
        fsm->state = ForceUp;
    }
    if (fsm->dButtonPushed) {
        fsm->state = ForceDown;
    }
    
}

void state_execute(Fsm * fsm) {
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





void state_Calculate(Fsm * fsm) {
    bool changed = false;
    LED_BLUE_Pin = 0;
    /* Handle state */

    /* Analog sensor value */
    if (fsm->lSensorValue < THR_NIGHT) {
        // Reading a nighttime value
        if (fsm->dayCount == 0) {
            // Counted enough nighttime values -> update
            // Note: we get here all the time so only set changed flag when needed
            changed = fsm->day; // When day was true this will set changed
            fsm->day = false;
            fsm->motorDir = Down;
            fsm->motorSpeed = 0;
        } else {
            changed = false;
            fsm->dayCount--;
        }
    } else if (fsm->lSensorValue > THR_DAY) {
        // Reading a daytime values
        if (fsm->dayCount >= THR_DN_COUNT) {
            // Counted enough daytime values -> update
            // Note: we get here all the time so only set changed flag when needed
            changed = !fsm->day;
            fsm->day = true;
            fsm->motorDir = Up;
            fsm->motorSpeed = 0;
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

void state_Sleep(Fsm * fsm) {
    /* Handle state */
    //    if (LOW_POWER) {
    //        //LowPower.deepSleep(SLEEP_TIME_MS);
    //    } else {
    //        //__delay_ms(LOW_POWER_SLEEP_TIME_MS);
    //    }
    fsm->sleepCount++;

    /* Decide on next state */
    if (fsm->sleepCount >= THR_SLEEP_COUNT) {
        fsm->sleepCount = 0;
        // Wake up
        fsm->next = Calculate;
    } else {
        // Stay asleep
        fsm->next = Sleep;
    }
}

void state_MotorStart(Fsm * fsm) {
    /* Handle state */

    fsm->motorRunningCount = 0;
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
    } else if (fsm->motorSpeed > 100) {
        /* Ramped up, go to running state */
        fsm->next = MotorRunning;
    } else {
        /* Still ramping */
        fsm->next = MotorStart;
    }
}

void state_MotorRunning(Fsm * fsm) {
    /* Handle state */
    fsm->motorRunningCount++;

    /* Decide on next state */
    if ((isDay(fsm) && isDoorOpen(fsm)) || // Opening door, upper sensor sees the door!
            (isNight(fsm) && isDoorClosed(fsm)) || // Closing door, bottom sensor sees the door!
            (fsm->motorRunningCount > MAX_MOTOR_COUNT)) // This is taking too long
    {
        fsm->next = MotorStop;
    } else {
        // Keep in the current state
        fsm->next = MotorRunning;
    }
}

void state_MotorStop(Fsm * fsm) {
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

void state_ForceUp(Fsm * fsm) {
    
    /* Handle state */
    fsm->motorDir = Up;
    if (fsm->motorSpeed < 100) {
        fsm->motorSpeed++;
    }
    D_MOTOR_Run(fsm->motorDir, fsm->motorSpeed);
    
    /* Decide on next state */
    if (fsm->uButtonPushed) {
        fsm->next = ForceUp;
    } else {
        fsm->next = MotorStop;
    }
    
}

void state_ForceDown(Fsm * fsm) {
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



