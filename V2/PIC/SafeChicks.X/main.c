#include <xc.h>
#include <stdbool.h>

#include "config.h"

#include "motor.h"
#include "fsm.h"

/*******************************************************************************
 *                      Local defines 
 ******************************************************************************/
#define TIMER_MODE_SLEEP 0   /* Sets up the timer in sleep (slow) mode */
#define TIMER_MODE_WORK  1   /* Sets up the timer in work (fast) mode */


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
 * Setup the Timer0 module.
 * @note expects interrupts to be enabled.
 * @param mode
 */
static void setupTimer0(uint8_t mode);

/**
 * Enters the MCU in sleep mode.
 * This will update the Timer0 values to go for a long sleep
 */
static void goToSleep();

/*******************************************************************************
 *                      Variables 
 ******************************************************************************/
bool test = false;

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
    TRISA = 0;
    TRISB = 0;
    TRISC = 0;
    
    /* Interrupt setup */
    RCONbits.IPEN = 1;              /* Enable priority levels on interrupts   */
    INTCONbits.PEIE = 1;            /* Enable all peripheral interrupts       */
    INTCONbits.TMR0IF = 0;          /* Clear the interrupt flag               */
    INTCONbits.TMR0IE = 1;          /* Enable TMR0 interrupt                  */
    INTCON2bits.TMR0IP = 0;         /* Low interrupt priority                 */
    
    /* My own code setups */
    setupTimer0(TIMER_MODE_WORK);
    motor_setup();
//    fsm_setup();
//    
    /* Enable stuff */
    INTCONbits.GIEH = 1;            /* Enables all high-priority interrupts   */
    INTCONbits.GIEL = 1;            /* Enable low interrupts                  */
}

void setupTimer0(uint8_t mode) {
    
    T0CONbits.TMR0ON = 0;           /* Stops Timer0                           */
    T0CONbits.T0CS = 0;             /* Internal instruction cycle clock (CLKO)*/
    TMR0H = 0x00;                   /* Load the compare value to TMR0H        */
    TMR0L = 0x00;                   /* Load the reset value to TMR0L          */
        
    if (mode == TIMER_MODE_WORK) {
        /** 
        * 1MHz clock, 8-bit, 1:1 pre-scale
        * FOSC/4 = 250kHz = 4us
        * 255 * 4E-6 = 1,02 ms period
        */
        
        T0CONbits.T08BIT = 1;       /* Timer0 is configured as an  8-bit timer*/
        T0CONbits.PSA = 1;          /* Timer0 pre-scaler is NOT assigned.     */
    }
    if (mode == TIMER_MODE_SLEEP) {
        /** 
        * 1MHz clock, 16-bit, 1:8 pre-scale
        * FOSC/4 = 250kHz = 4us
        * 1:8 = 32us
        * 65535 * 32E-6 = 2.09712s period
        */
    
        T0CONbits.T08BIT = 0;       /* Timer0 is configured as an 16-bit timer*/
        T0CONbits.PSA = 0;          /* Timer0 pre-scaler is assigned.         */
        T0CONbits.T0PS = 0b010;     /* 1:8   pre-scale value                  */
    }
    
    T0CONbits.TMR0ON = 1;           /* Enable Timer0                          */
}

void goToSleep() {
    
    /**
     * We will need peripheral clock so go to RC_IDLE mode on SLEEP
     */
    
    OSCCONbits.IDLEN = 1;           /* Idle mode on SLEEP instruction         */
    
    setupTimer0(TIMER_MODE_SLEEP);
    
    /* Lets go! */
    SLEEP();

    /* Wake up again */
    setupTimer0(TIMER_MODE_WORK);
}

void main(void)
{    
    initialize();

    while(1)
    {
        __delay_ms(100);
        
//        goToSleep();
        
//        fsm_tick(); // TODO: do this on timer function

    }
}

void __interrupt(low_priority) _LowInterruptManager (void)
{
    /* Check if TMR0 interrupt is enabled and if the interrupt flag is set */
    if(INTCONbits.TMR0IE == 1 && INTCONbits.TMR0IF == 1)
    {
//        if (test) {
//            LATBbits.LATB5 = 1;
//            test = false;
//        } else {
//            LATBbits.LATB5 = 0;
//            test = true;
//        }
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