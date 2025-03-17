#pragma once

#include <Arduino.h>

/* This file contains all pin mapping of the board */

const uint8_t PIN_MOTOR = 9;     // Pin for motor PWM
const uint8_t PIN_MOTOR_DIR = 8; // Pin for motor direction

const uint8_t PIN_LSENSOR = A0; // Pin for reading light sensor
const uint8_t PIN_USENSOR = A1; // Pin for reading upper door sensor
const uint8_t PIN_BSENSOR = A2; // Pin for reading bottom door sensor


const uint16_t THRESHOLD_DAY_NIGHT = 200; // Threshold used to switch from day/night
const uint8_t THRESHOLD_DN_COUNT = 3; // Hysteresis counter, depending on time between sleeps this makes how long day/night should be read before changing
                                          


