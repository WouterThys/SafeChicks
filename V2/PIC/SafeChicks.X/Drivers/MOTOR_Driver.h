#ifndef XC_HEADER_TEMPLATE_H
#define	XC_HEADER_TEMPLATE_H

/* This file contains all the motor functions */

typedef enum {
  Up,
  Down,
} Direction;

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


#endif	/* XC_HEADER_TEMPLATE_H */

