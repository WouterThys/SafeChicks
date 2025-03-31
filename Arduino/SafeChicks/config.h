#ifndef _CONFIG_H
#define _CONFIG_H

#include <Arduino.h>

/* This file contains all pin mapping of the board */

const bool      DEBUG_ENABLED = true;

const uint8_t   PIN_MOTOR_PWM = 3; // Pin for motor PWM
const uint8_t   PIN_MOTOR_DIR = 2; // Pin for motor direction

const uint8_t   PIN_LSENSOR = A0; // Pin for reading light sensor
const uint8_t   PIN_BSENSOR = A1; // Pin for reading battery sensor
const uint8_t   PIN_USWITCH = 8; // Pin for reading upper door sensor // !!!!! switched to 7 in schematic
const uint8_t   PIN_BSWITCH = 7; // Pin for reading bottom door sensor // !!!!! switched to 8 in schematic
const uint8_t   PIN_NOSLEEP = 6; // When this pin is up, don't sleep (and keep COMx open)

const uint8_t   PIN_DAY_STATE = 4; // Pin to connect to LED indicating day / night
const uint8_t   PIN_ERROR_STATE = 5; // Pin to connect to LED indicating error

const uint8_t   ERROR_SENSORS_BOTH_CLOSED = 1;
const uint8_t   ERROR_SENSORS_UP_WHILE_NIGHT = 2;
const uint8_t   ERROR_SENSORS_DOWN_WHILE_DAY = 4;
const uint8_t   ERROR_MOTOR_RUN_TOO_LONG = 8;

/* And a lot more after a while.... */ 

#define SECONDS_IN_MINUTE 60
#define MILLIS_IN_SECOND  1000

const uint32_t  FSM_PERIOD_MS = 100; // The period between FSM state checks
const uint32_t  SLEEP_TIME_MS = (1 * SECONDS_IN_MINUTE * MILLIS_IN_SECOND) - FSM_PERIOD_MS;  // Low power sleep time in sleep state

const uint16_t  THR_DAY_NIGHT = 200; // Threshold used to switch from day/night (sensor is max 1023)

/* Total time (consequent) day/night reading will need to trigger is SLEEP_TIME_MS x THR_SLEEP_COUNT x THR_DN_COUNT */
/* ===> With these values this is 1min x 5 x 3 = 15 min */
const uint8_t   THR_SLEEP_COUNT = 5; // The total sleep time will be SLEEP_TIME_MS times this value, to allow shorter wakeup intervals for sanity checking
const uint8_t   THR_DN_COUNT = 3;    // Hysteresis counter, depending on time between sleeps this makes how long day/night should be read before changing

/* Total time the motor should be running, this is counted every FSM_PERIOD_MS times.*/
/* ===> With these values this is 100ms x 50 = 5s */
const uint8_t   THR_MOTOR_RUN = 50;  // The maximum count the motor should be running. T
                                          

#endif // _CONFIG_H
