/* 
 * File:   TMR0_Driver.h
 * Author: thys_
 *
 * Created on August 28, 2025, 10:03 AM
 */

#include <stdbool.h>

#ifndef TMR0_DRIVER_H
#define	TMR0_DRIVER_H

#define TIMER_MODE_SLEEP 0   /* Sets up the timer in sleep (slow) mode */
#define TIMER_MODE_WORK  1   /* Sets up the timer in work (fast) mode */

/**
* Initialises all the parameters to the default setting, as well as writing the
* tri-state registers. 
*/
void D_TMR0_Init(uint8_t mode);

/**
 * Enable the Timer 0 module
 * @param enable Enable or disable.
 */
void D_TMR0_Enable(bool enable);

#endif	/* TMR0_DRIVER_H */

