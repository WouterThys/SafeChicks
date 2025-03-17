#pragma once

#include <Arduino.h>

/* This file contains all pin mapping of the board */

const uint8_t   PIN_MOTOR_PWM = 3; // Pin for motor PWM
const uint8_t   PIN_MOTOR_DIR = 2; // Pin for motor direction

const uint8_t   PIN_LSENSOR = A0; // Pin for reading light sensor
const uint8_t   PIN_USENSOR = 8; // Pin for reading upper door sensor
const uint8_t   PIN_BSENSOR = 7; // Pin for reading bottom door sensor

const uint16_t  THR_DAY_NIGHT = 400; // Threshold used to switch from day/night (sensor is max 1023)
const uint8_t   THR_SLEEP_COUNT = 10; // Time to sleep, in seconds
const uint8_t   THR_DN_COUNT = 3; // Hysteresis counter, depending on time between sleeps this makes how long day/night should be read before changing
                                          


