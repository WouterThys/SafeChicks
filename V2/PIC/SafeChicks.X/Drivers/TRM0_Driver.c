#include <xc.h>

#include "../config.h"
#include "TMR0_Driver.h"

/*******************************************************************************
 *          DEFINES
 ******************************************************************************/

/*******************************************************************************
 *          MACRO FUNCTIONS
 ******************************************************************************/

/*******************************************************************************
 *          VARIABLES
 ******************************************************************************/


/*******************************************************************************
 *          BASIC FUNCTIONS
 ******************************************************************************/


/*******************************************************************************
 *          DRIVER FUNCTIONS
 ******************************************************************************/
void D_TMR0_Init(uint8_t mode) {  
    
    D_TMR0_Enable(false);
    
    T0CONbits.T0CS = 0;             /* Internal instruction cycle clock (CLKO)*/
    T0CONbits.PSA = 0;              /* Timer0 pre-scaler is assigned.         */
    TMR0H = 0x00;                   /* Load the compare value to TMR0H        */
    TMR0L = 0x00;                   /* Load the reset value to TMR0L          */
        
    if (mode == TIMER_MODE_WORK) {
        /** 
        * 1MHz clock, 8-bit, 1:8 pre-scale
        * FOSC/4 = 250kHz = 4us
        * 255 * 4E-6 * 8 = 8,16 ms period
        */
        
        T0CONbits.T08BIT = 1;       /* Timer0 is configured as an  8-bit timer*/
        T0CONbits.T0PS = 0b010;     /* 1:8   pre-scale value                  */
    }
    if (mode == TIMER_MODE_SLEEP) {
        /** 
        * 1MHz clock, 16-bit, 1:256 pre-scale
        * FOSC/4 = 250kHz = 4us
        * 65535 * 4E-6 * 256 = 1:10min period
        */
    
        T0CONbits.T08BIT = 0;       /* Timer0 is configured as an 16-bit timer*/
        T0CONbits.T0PS = 0b111;     /* 1:256 pre-scale value                  */
    }
    
    INTCONbits.TMR0IF = 0;          /* Clear the interrupt flag               */
    INTCON2bits.TMR0IP = 0;         /* Low interrupt priority                 */
    
}

void D_TMR0_Enable(bool enable) {
    if(enable) {
        INTCONbits.TMR0IF = 0;      /* Clear the interrupt flag               */
        INTCONbits.TMR0IE = 1;      /* Enable TMR0 interrupt for now          */
        T0CONbits.TMR0ON = 1;       /* Stops Timer0                           */
    } else {
        T0CONbits.TMR0ON = 0;       /* Stops Timer0                           */
        INTCONbits.TMR0IE = 0;      /* Disable TMR0 interrupt for now         */
        INTCONbits.TMR0IF = 0;      /* Clear the interrupt flag               */
    }
}


