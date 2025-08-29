#include <xc.h>

#include "../config.h"
#include "ADC_Driver.h"

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

void D_ADC_Init(void) {
    
    ADCON0bits.ADON = 0;        /* A/D converter module is disabled           */
    ADCON0bits.CHS = 0b0000;    /* Channel 0 (AN0)                            */
    
    ADCON1bits.VCFG1 = 0;       /* Voltage Reference VSS                      */
    ADCON1bits.VCFG0 = 0;       /* Voltage Reference VDD                      */
    ADCON1bits.PCFG = 0b1110;   /* AN0 is analog, others are digital          */
    
    ADCON2bits.ADFM = 1;        /* Right justified                            */
    ADCON2bits.ACQT = 0b000;    /* A/D Acquisition Time Select bits, 0        */
    ADCON2bits.ADCS = 0b000;    /* A/D Conversion Clock Select bits: FOSC/2   */
 
    TRISAbits.TRISA0 = 1;       /* A0 should be an input                      */
}

   
uint16_t D_ADC_ReadOnce(void) {
    
    uint16_t result = 0;
    
    // Enable        
    ADCON0bits.ADON = 1;
    
    // Start the conversion
    ADCON0bits.GO = 1;
    while(!ADCON0bits.DONE);
    
    result = ((uint16_t)((ADRESH << 8) + ADRESL));
   
    return result;
}