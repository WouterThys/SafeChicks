#ifndef _MOTOR_H
#define _MOTOR_H

/* This file contains all the motor functions */

enum Direction {
  Up,
  Down,
};

/**
 * Initialize the motor control
 */
void motor_setup();

/**
 * Start the motor, which will ramp up to 100%
 */
void motor_start(Direction d);

/**
 * Stop the motor, which will ramp down to 0%
 */
void motor_stop();

#endif _MOTOR_H