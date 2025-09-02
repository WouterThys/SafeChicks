#ifndef FSM_CONTROLLER_H
#define	FSM_CONTROLLER_H

/* This file contains all Finite State Machine functions */

typedef void (*SleepHandler)(void);
  
/**
 * Initialise the FSM.
 * @param handler: function pointer that will be called when sleep is requested
 */
void C_FSM_Init(SleepHandler handler);

/* Run the FSM one time */
void C_FSM_Tick(void);

#endif	/* FSM_CONTROLLER_H */

