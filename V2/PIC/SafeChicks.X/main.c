#include <xc.h>
#include <stdbool.h>

#include "config.h"

#include "motor.h"
#include "fsm.h"

/******************* TYPES *******************/
bool test = false;


/******************* MY PRIVATES *******************/

static void initialize() 
{
    /* Oscillators setup */
    OSCCONbits.IRCF = 0b100;        /* 1MHz */
    OSCCONbits.SCS = 0b10;          /* Internal oscillator */
    while(OSCCONbits.IOFS == 0);    /* Wait for OSC to be stable */
    
    /* Port setup */
    TRISB = 0;
    
    /* Timer setup */
    //T0CON = 0x03;                   /* Timer off set the pre-scaler to 1:16 */
    T0CONbits.TMR0ON = 0;           /* Stops Timer0 */
    T0CONbits.T08BIT = 1;           /* Timer0 is configured as an 8-bit timer */
    T0CONbits.T0CS = 0;             /* Internal instruction cycle clock (CLKO)*/
    T0CONbits.T0SE = 0;
    T0CONbits.PSA = 1;              /* Timer0 pre-scaler is NOT assigned.     */
    T0CONbits.T0PS = 0b00;          /* Timer0 pre-scaler Select bits          */
    TMR0H = 0x00;                   /* Load the compare value to TMR0H        */
    TMR0L = 0x05;                   /* Load the reset value to TMR0L          */
    
    /* Interrupt setup */
    RCONbits.IPEN = 1;              /* Enable priority levels on interrupts   */
    INTCONbits.TMR0IF = 0;          /* Clear the interrupt flag               */
    INTCONbits.TMR0IE = 1;          /* Enable TMR0 interrupt                  */
    
    INTCONbits.PEIE = 1;            /* Enable all peripheral interrupts       */
    INTCON2bits.TMR0IP = 0;         /* Low interrupt priority                 */
    
//    motor_setup();
//    fsm_setup();
//    
    /* Enable stuff */
    INTCONbits.GIEH = 1;            /* Enables all high-priority interrupts   */
    INTCONbits.GIEL = 1;            /* Enable low interrupts                  */
    T0CONbits.TMR0ON = 1;           /* Enable timer                           */
}

void main(void)
{    
    initialize();

    while(1)
    {
        LATBbits.LATB5 = 0; 
        __delay_ms(10);
        LATBbits.LATB5 = 1;    // RB-0 to LOW
        __delay_ms(10);
        
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
    /* Check if TMR0 interrupt is enabled and if the interrupt flag is set */
    if(INTCONbits.TMR0IE == 1 && INTCONbits.TMR0IF == 1)
    {
        LATBbits.LATB5 = 0;
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

/* THE END */