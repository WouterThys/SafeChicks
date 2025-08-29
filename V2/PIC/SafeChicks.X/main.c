#include <xc.h>
#include <stdbool.h>

#include "config.h"

#include "Drivers/TMR0_Driver.h"
#include "Drivers/MOTOR_Driver.h"
#include "Drivers/UART_Driver.h"
#include "Controllers/FSM_Controller.h"

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
static void initialize();

/**
 * Enters the MCU in sleep mode.
 * This will update the Timer0 values to go for a long sleep
 */
static void goToSleep();

/*******************************************************************************
 *                      Variables 
 ******************************************************************************/
bool test = false;
volatile bool runFSM = false;

/*******************************************************************************
 *                      Function implementation 
 ******************************************************************************/

void initialize() 
{
    /* Oscillators setup */
    OSCCONbits.IRCF = 0b100;        /* 1MHz */
    OSCCONbits.SCS = 0b10;          /* Internal oscillator, RC_RUN power mode */
    while(OSCCONbits.IOFS == 0);    /* Wait for OSC to be stable */
    
    /* Port setup */
    TRISA = 0x00;
    TRISB = 0x00;
    TRISC = 0x00;
    
    PORTA = 0x00;
    PORTB = 0x00;
    PORTC = 0x00;
    
    /* Interrupt setup */
    RCONbits.IPEN = 1;              /* Enable priority levels on interrupts   */
    INTCONbits.PEIE = 1;            /* Enable all peripheral interrupts       */

    
    /* My own code setups */
    D_TMR0_Init(TIMER_MODE_WORK);
    D_MOTOR_Init();
    D_UART_Init();
    C_FSM_Init();
    
    /* Enable stuff */
    D_TMR0_Enable(true);
    D_UART_Enable(true);
    INTCONbits.GIEH = 1;            /* Enables all high-priority interrupts   */
    INTCONbits.GIEL = 1;            /* Enable low interrupts                  */
}

void goToSleep() {
    
    /**
     * We will need peripheral clock so go to RC_IDLE mode on SLEEP
     */
    
    OSCCONbits.IDLEN = 1;           /* Idle mode on SLEEP instruction         */
    
    D_TMR0_Init(TIMER_MODE_SLEEP);
    D_TMR0_Enable(true);
    
    /* Lets go! */
    SLEEP();

    /* Wake up again */
    D_TMR0_Init(TIMER_MODE_WORK);
    D_TMR0_Enable(true);
}

void main(void)
{    
    __delay_ms(100);
    initialize();
    __delay_ms(100);

    while(1)
{
        if (runFSM) {
            runFSM = false;
            C_FSM_Tick();
        }
        
        
        
//        if (test) {
//            LATBbits.LATB5 = 1;
//            test = false;
//        } else {
//            LATBbits.LATB5 = 0;
//            test = true;
//        }
//        goToSleep();
        
//        fsm_tick(); // TODO: do this on timer function

    }
}

void __interrupt(low_priority) _LowInterruptManager (void)
{
    /* Check if TMR0 interrupt is enabled and if the interrupt flag is set */
    if(INTCONbits.TMR0IE == 1 && INTCONbits.TMR0IF == 1)
    {
        runFSM = true;
        INTCONbits.TMR0IF = 0; /* clear the TMR0 interrupt flag */
    }
}

void __interrupt(high_priority) _HighInterruptManager (void)
{
    /* Check if TMR1 interrupt is enabled and if the interrupt flag is set */
    if(PIE1bits.TMR1IE== 1 && PIR1bits.TMR1IF == 1)
    {
//        if (test) {
//            LATBbits.LATB5 = 1;
//            test = false;
//        } else {
//            LATBbits.LATB5 = 0;
//            test = true;
//        }
        PIR1bits.TMR1IF = 0; /* clear the interrupt flag */
    }
}

/* THE END */