#include "MOTOR_Driver.h"
#include "../config.h"

// Private stuff -----------------------------------------------------------------------------

// Public stuff -----------------------------------------------------------------------------

void D_MOTOR_Init(void) {
    
    MOTOR_DIR_Dir = 0;

    /** 
     * Timer2 setup. Max frequency should be 100kHz, aim for 20 kHz = 50us
     * PWM Period = [(PR2) + 1] * 4 * TOSC * (TMR2 pre-scale Value)
     * TOSC = 1/1MHz 
     * TMR2 pre-scale to 1:1
     */

    T2CONbits.TMR2ON = 0;           /* Timer2 is off                          */
    T2CONbits.TOUTPS = 0;           /* Post-scale is 1                        */
    T2CONbits.T2CKPS = 0b00;        /* Pre-scaler is 1                        */

    PR2 = 0xFF;
    CCP1CONbits.DC1B = 0b00;
    CCPR1L = 0x00;

    /* PWM mode */
    CCP1CONbits.CCP1M = 0b0000;     /* PWM mode disabled                      */
    T2CONbits.TMR2ON = 1;           /* Enable Timer2                          */

}

void D_MOTOR_Run(Direction d, uint8_t speed) {
    
    if (speed > 0) {
    
        if (speed > 100) {
            speed = 100;
        }
        
        if (d == Up) {
            MOTOR_DIR_Pin = CW_DIRECTION;
        } else {
            MOTOR_DIR_Pin = CCW_DIRECTION;
        }

        // PWM duty is 10-bit value
        uint32_t pwmValue = ((uint32_t) speed * 0x03FF) / 100;

        // LSB
        CCP1CONbits.DC1B = (pwmValue & 0x03);

        // MSB
        CCPR1L = 0x00FF & (pwmValue >> 2);
        
        // Enable
        CCP1CONbits.CCP1M = 0b1100;     /* PWM mode enabled                   */
    } else {
        CCP1CONbits.CCP1M = 0b0000;     /* PWM mode off                       */
        CCPR1L = 0x00;
        CCP1CONbits.DC1B = 0b00;
    }
}

