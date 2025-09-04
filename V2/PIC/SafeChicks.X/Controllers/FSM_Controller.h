#ifndef FSM_CONTROLLER_H
#define	FSM_CONTROLLER_H

#include <stdint.h>

/* This file contains all Finite State Machine functions */

typedef void (*SleepHandler)(void);
  
/**
 * Initialise the FSM.
 * @param handler: function pointer that will be called when sleep is requested
 */
void C_FSM_Init(SleepHandler handler);

/* Run the FSM one time */
void C_FSM_Tick(void);

/**
 * Get the FSM as a string.
 * Creates a comma separated string with most usefull values inside it.
 * @param dst: destination buffer to write string to
 * @param size: size of the dst buffer
 */
void C_FSM_ToString(char * dst, uint8_t size);

#endif	/* FSM_CONTROLLER_H */

