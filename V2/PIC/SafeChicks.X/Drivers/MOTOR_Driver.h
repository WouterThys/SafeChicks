#ifndef MOTOR_DRIVER_H
#define	MOTOR_DRIVER_H

#include <stdint.h>

/* This file contains all the motor functions */

typedef enum {
  Up,
  Down,
} Direction;

/**
 * Initialise the motor control
 */
void D_MOTOR_Init(void);

/**
 * Run the motor in a direction and speed.
 * @param d: direction to run
 * @param speed: speed in percent
 */
void D_MOTOR_Run(Direction d, uint8_t speed);


#endif	/* MOTOR_DRIVER_H */

