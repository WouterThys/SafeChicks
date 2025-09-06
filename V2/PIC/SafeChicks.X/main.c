#include <builtins.h>
#include <stdbool.h>
#include <xc.h>

#include "config.h"

#include "Controllers/FSM_Controller.h"
#include "Drivers/ADC_Driver.h"
#include "Drivers/MOTOR_Driver.h"
#include "Drivers/TMR0_Driver.h"
#include "Drivers/UART_Driver.h"

#if DEBUG_MODE
#include <stdio.h>
#endif


/*******************************************************************************
 *                      Local defines
 ******************************************************************************/

/*******************************************************************************
 *                      Function and type definitions
 ******************************************************************************/

/**
 * Main initialisation function.
 * Note that this will only be called only after a reset.
 *
 * Will enable all timers and interrupts for other peripherals to work.
 * Enables the FSM, Motor, ... code
 */
static void initialize(void);

/**
 * Enters the MCU in sleep mode.
 * This will update the Timer0 values to go for a long sleep
 */
static void goToSleep(void);

/*******************************************************************************
 *                      Variables
 ******************************************************************************/
bool test = false;
volatile bool runFSM = false;

#if DEBUG_MODE
#define DEBUG_BUFFER_SIZE 100
char debugBuffer[DEBUG_BUFFER_SIZE];

uint8_t debugCounter = 0;
static void buildConfigString(char *dst, uint8_t size);

#endif

/*******************************************************************************
 *                      Function implementation
 ******************************************************************************/

void initialize(void) {
  /* Oscillators setup */
  OSCCONbits.IRCF = 0b100; /* 1MHz */
  OSCCONbits.SCS = 0b10;   /* Internal oscillator, RC_RUN power mode */
  while (OSCCONbits.IOFS == 0)
    ; /* Wait for OSC to be stable */

  /* Port setup */
  TRISA = 0x00;
  TRISB = 0x00;
  TRISC = 0x00;

  PORTA = 0x00;
  PORTB = 0x00;
  PORTC = 0x00;

  /* Interrupt setup */
  INTCON2bits.INTEDG0 = 1; /* Interrupt on rising edge               */
  //INTCONbits.INT0IP = 0;  /* Low priority                           */
  INTCONbits.INT0IF = 0;  /* Clear flag                             */
  INTCONbits.INT0IE = 1;  /* Enables the INT0 external interrupt    */
  
  INTCON2bits.INTEDG1 = 1; /* Interrupt on rising edge               */
  INTCON3bits.INT1IP = 1;  /* Low priority                           */
  INTCON3bits.INT1IF = 0;  /* Clear flag                             */
  INTCON3bits.INT1IE = 1;  /* Enables the INT0 external interrupt    */

  RCONbits.IPEN = 1;   /* Enable priority levels on interrupts   */
  INTCONbits.PEIE = 1; /* Enable all peripheral interrupts       */

  /* My own code setups */
  D_TMR0_Init(TIMER_MODE_WORK);
  D_MOTOR_Init();
  D_UART_Init();
  D_ADC_Init();
  C_FSM_Init(goToSleep);

  /* Enable stuff */
  D_TMR0_Enable(true);
  D_UART_Enable(true);
  INTCONbits.GIEH = 1; /* Enables all high-priority interrupts   */
  INTCONbits.GIEL = 1; /* Enable low interrupts                  */
}

void goToSleep(void) {

#if DEBUG_MODE

  /* Print the current configuration every 10 sleeps */
  if (debugCounter % 10 == 0) {
    buildConfigString(debugBuffer, DEBUG_BUFFER_SIZE);
    D_UART_Write(debugBuffer);
    debugCounter = 0;
  }

  /* Debug FSM state */
  C_FSM_ToString(debugBuffer, DEBUG_BUFFER_SIZE);
  D_UART_Write(debugBuffer);

  debugCounter++;

#endif

  /**
   * We will need peripheral clock so go to RC_IDLE mode on SLEEP
   */

  OSCCONbits.IDLEN = 1; /* Idle mode on SLEEP instruction         */

  D_TMR0_Init(TIMER_MODE_SLEEP);
  D_TMR0_Enable(true);
  /* Lets go! */
  SLEEP();

  /* Wake up again */
  D_TMR0_Init(TIMER_MODE_WORK);
  D_TMR0_Enable(true);
}

#if DEBUG_MODE
void buildConfigString(char *dst, uint8_t size) {

    snprintf(dst, size,
        "C:%" PRIu8 ",%" PRIu8 ",%" PRIu8 ",%" PRIu8 ",%" PRIu8 ",%" PRIu8 ",%" PRIu16 ",%" PRIu16 ",%" PRIu16 "\r\n", 
        DAY_THRESHOLD,
        NIGHT_THRESHOLD,
        SLEEP_COUNT,
        DAY_COUNT,
        MOTOR_FULL_SPEED,
        MOTOR_HALF_SPEED,
        MAX_MOTOR_COUNT,
        MOTOR_DOWN_FULL_CNT,
        MOTOR_DOWN_SLOW_CNT
);

}
#endif

int main(void) {

  __delay_ms(100);
  initialize();
  __delay_ms(101);

  while (1) {
    if (runFSM) {
      runFSM = false;
      C_FSM_Tick();
    }
  }

  return 0;
}

void __interrupt(low_priority) _LowInterruptManager(void) {
  /* Check if TMR0 interrupt is enabled and if the interrupt flag is set */
  if (INTCONbits.TMR0IE == 1 && INTCONbits.TMR0IF == 1) {
    runFSM = true;
    INTCONbits.TMR0IF = 0; /* clear the TMR0 interrupt flag */
  }
}

void __interrupt(high_priority) _HighInterruptManager(void) {

  /* Check if INT0 interrupt is enabled and if the interrupt flag is set */
  if (INTCONbits.INT0IE == 1 && INTCONbits.INT0IF == 1) {
    runFSM = true;
    INTCONbits.INT0IF = 0; /* clear the INT2 interrupt flag */
  }
  
   /* Check if INT1 interrupt is enabled and if the interrupt flag is set */
  if (INTCON3bits.INT1IE == 1 && INTCON3bits.INT1IF == 1) {
    runFSM = true;
    INTCON3bits.INT1IF = 0; /* clear the INT2 interrupt flag */
  }

}

/* THE END */