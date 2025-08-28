#include <stdbool.h>

#include "FSM_Controller.h"

#include "../Drivers/MOTOR_Driver.h"
#include "../Drivers/UART_Driver.h"
#include "../config.h"


#define LOW_POWER 0 // TODO: (digitalRead(PIN_NOSLEEP) == LOW)

/*******************************************************************************
 *                      Function and type definitions 
 ******************************************************************************/

/* Enum to keep the FSM state */
typedef enum {
    Calculate,    /* Calculate depending on input                             */
    Sleep,        /* Sleep for certain time before starting again             */
    MotorStart,   /* Start running the motor up or down                       */
    MotorRunning, /* Run the motor, read sensors to see if motor should stop  */
    MotorStop     /* Stop running the motor                                   */
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
    uint16_t motorRunningCount; // Counter keeping how long the motor was running

    // Sensor values
    uint16_t lSensorValue; // Value of the light sensor
    uint16_t bSensorValue; // Battery sensor
    bool uSensorClosed; // Value of the upper sensor, True when the sensor is closed
    bool bSensorClosed; // Value of the bottom sensor, True when the sensor is closed

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
 * When the LOW_POWER is set, this will not write to the UART module.
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
 * State function for State::Calculate
 * Takes the input values and calculates if it is currently day or night.
 * Depending on this the next state will be set to Sleep or MotorRun
 * @param fsm
 */
static void state_Calculate(Fsm * fsm);

/**
 * State function for State::Sleep
 * Takes down the whole FSM for certain time.
 * When in LOW_POWER mode the whole MCU will be put into sleep mode.
 * @param fsm
 */
static void state_Sleep(Fsm * fsm);

/**
 * State function for State::MotorStart
 * Run the motor up or down and immediately go to the State::MotorCheck
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
 * Stop the motor and go back to State::Calculate
 * @param fsm
 */
static void state_MotorStop(Fsm * fsm);

/*******************************************************************************
 *                      Variables 
 ******************************************************************************/

Fsm fsm;

/*******************************************************************************
 *                      Public function implementation 
 ******************************************************************************/

void fsm_setup() {
    //  if (!LOW_POWER) 
    //  {
    //    Serial.begin(9600);
    //  }
    //
    //  // Pin control
    //  pinMode(LED_BUILTIN, OUTPUT);
    //  pinMode(PIN_DAY_STATE, OUTPUT);
    //  pinMode(PIN_ERROR_STATE, OUTPUT);
    //  
    //  pinMode(PIN_LSENSOR, INPUT);
    //  pinMode(PIN_BSENSOR, INPUT);
    //  pinMode(PIN_USWITCH, INPUT);
    //  pinMode(PIN_BSWITCH, INPUT);
    //  pinMode(PIN_NOSLEEP, INPUT);

    // Setup the state
    fsm.state = Calculate;
    fsm.next = Calculate;
    fsm.dayCount = THR_DN_COUNT; // Probably install while day? 
    fsm.sleepCount = 0;
    fsm.motorRunningCount = 0;
    fsm.lSensorValue = 0;
    fsm.uSensorClosed = false;
    fsm.bSensorClosed = false;
    fsm.error = 0;
}

/* Run the FSM one time */
void fsm_tick() {
    fsm.epoch++;
    fsm.state = fsm.next;

    read_input(&fsm);
    sanity_check(&fsm);
    state_execute(&fsm);
}

/*******************************************************************************
 *                      Private function implementations 
 ******************************************************************************/



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
    }
}

void debug(Fsm * fsm) {
    if (!LOW_POWER) {
        //    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        //    digitalWrite(PIN_ERROR_STATE, fsm.error > 0 ? HIGH : LOW);
        //    digitalWrite(PIN_DAY_STATE, fsm.isDay() ? HIGH : LOW);
        //
        //    // Allocate the JSON document
        //    JsonDocument doc;
        //
        //    // Add values in the document
        //    doc["epoch"] = fsm.epoch;
        //    doc["state"] = print_state(fsm.state);
        //    doc["next"] = print_state(fsm.next);
        //    doc["day"] = fsm.isDay();
        //    doc["dayCount"] = fsm.dayCount;
        //    doc["sleepCount"] = fsm.sleepCount;
        //    doc["motorRunningCount"] = fsm.motorRunningCount;
        //    doc["lSensorValue"] = fsm.lSensorValue;
        //    doc["bSensorValue"] = fsm.bSensorValue;
        //    doc["uSensorClosed"] = fsm.uSensorClosed;
        //    doc["bSensorClosed"] = fsm.bSensorClosed;
        //    doc["error"] = fsm.error;
        //
        //    // Note: Serial1 is on other connector. 
        //    serializeJson(doc, Serial);
        //    Serial.print("\n");

        // Read from serial here?
        //    if (Serial.available() > 0) 
        //    {
        //        // read the incoming byte:
        //        char incomingByte = Serial.read();
        //
        //        switch(incomingByte) 
        //        {
        //          case 'U': 
        //            fsm.day = true;
        //            fsm.state = State::MotorRun;
        //            Serial.println("Faking day");
        //            break;
        //          case 'D': 
        //            fsm.day = false;
        //            fsm.state = State::MotorRun;
        //            Serial.println("Faking night");
        //            break;
        //          default:
        //            Serial.print("Input unknown: ");
        //            Serial.println(incomingByte);
        //            break;
        //        }

        //        while (Serial.available() > 0) 
        //        {
        //          // Empty the buffer
        //          Serial.read();
        //        }
        //    }

        //    Serial.flush(); // Flush because we are going to sleep soon
    }
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

void read_input(Fsm * fsm) {
    //  fsm.lSensorValue = analogRead(PIN_LSENSOR);
    //  fsm.bSensorValue = analogRead(PIN_BSENSOR);
    //  fsm.uSensorClosed = digitalRead(PIN_USWITCH) == LOW;
    //  fsm.bSensorClosed = digitalRead(PIN_BSWITCH) == LOW;

    debug(fsm);
}

void state_Calculate(Fsm * fsm) {
    bool changed = false;

    /* Handle state */
    if (fsm->lSensorValue < THR_NIGHT) {
        // Reading a nighttime value
        if (fsm->dayCount == 0) {
            // Counted enough nighttime values -> update
            // Note: we get here all the time so only set changed flag when needed
            changed = fsm->day; // When day was true this will set changed
            fsm->day = false;
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
    if (LOW_POWER) {
        //LowPower.deepSleep(SLEEP_TIME_MS);
    } else {
        __delay_ms(LOW_POWER_SLEEP_TIME_MS);
    }
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

    if (isDay(fsm)) {
        if (!isDoorOpen(fsm)) {
            motor_start(Up);
        }
    } else {
        if (!isDoorClosed(fsm)) {
            motor_start(Down);
        }
    }

    /* Decide on next state */
    fsm->next = MotorRunning;
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
        // TODO: sleep before checking again
        __delay_ms(100);
    }
}

void state_MotorStop(Fsm * fsm) {
    /* Handle state */
    motor_stop();

    /* Decide on next state */
    fsm->next = Calculate;
}


