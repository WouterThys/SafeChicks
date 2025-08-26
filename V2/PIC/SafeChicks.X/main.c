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
    TRISB = 0;
    
    T0CON = 0x03; /* Timer off set the prescaler to 1:16 */
    TMR0H = 0xC1; /* Load the compare value to TMR0H */
    TMR0L = 0x00; /* Load the reset value to TMR0L */
//    INTCONbits.TMR0IF = 0; /* clear the interrupt flag */
//    INTCONbits.TMR0IE = 1; /* enable TMR0 interrupt */
//    INTCONbits.GIEL = 1;    /* enable low interrupts */
//    INTCONbits.PEIE = 1;
//    INTCON2bits.TMR0IP = 0; /* interrupt priority */
    
//    motor_setup();
//    fsm_setup();
//    
//    INTCONbits.GIE = 1; // Enable interrupts
//    T0CONbits.TMR0ON = 1; /* Enable timer */
}

void main(void)
{    
    TRISB = 0;
    
    initialize();

    while(1)
    {
        LATBbits.LATB5 = 0; 

        __delay_ms(1500);

        LATBbits.LATB5 = 1;    // RB-0 to LOW

        __delay_ms(100);
//        
//        fsm_tick(); // TODO: do this on timer function

    }
}

void __interrupt() INTERRUPT_InterruptManager (void)
{
    /* Check if TMR0 interrupt is enabled and if the interrupt flag is set */
//    if(INTCONbits.TMR0IE == 1 && INTCONbits.TMR0IF == 1)
//    {
//        if (test) {
//            LATBbits.LATB5 = 1;
//            test = false;
//        } else {
//            LATBbits.LATB5 = 0;
//            test = true;
//        }
//        INTCONbits.TMR0IF = 0; /* clear the TMR0 interrupt flag */
//    }
}

/* THE END */