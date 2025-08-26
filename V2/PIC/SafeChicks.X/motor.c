#include "motor.h"
#include "config.h"

// Private stuff -----------------------------------------------------------------------------

Direction dir;
uint8_t pwm; // Current pwm value, might be in ramp up/down

void setDutyPercent(uint8_t percent) {

    // PWM duty is 10-bit value
    uint32_t pwmValue = ((uint32_t) percent * 0x03FF) / 100;

    // LSB
    CCP1CONbits.DC1B = (pwmValue & 0x03);

    // MSB
    CCPR1L = 0x00FF & (pwmValue >> 2);

}

// Public stuff -----------------------------------------------------------------------------

void motor_setup() {

    /** 
     * Timer2 setup. Max frequency should be 100kHz, aim for 20 kHz = 50us
     * PWM Period = [(PR2) + 1] * 4 * TOSC * (TMR2 pre-scale Value)
     * TOSC = 1/1MHz 
     * TMR2 pre-scale to 1:1
     */

    T2CONbits.TMR2ON = 0; /* Timer2 is off                                    */
    T2CONbits.TOUTPS = 0; /* Post-scale is 1                                  */
    T2CONbits.T2CKPS = 0b00; /* Pre-scaler is 1                               */

    PR2 = 0xFF;
    setDutyPercent(0);

    /* PWM mode */
    CCP1CONbits.CCP1M = 0b1100; /* PWM mode                                   */
    T2CONbits.TMR2ON = 1; /* Enable Timer2                                    */

    //
    dir = Down;
    pwm = 0;
}

void motor_start(Direction d) {
    dir = d;
    if (dir == Up) {
        PIN_MOTOR_DIR = 1;
    } else {
        PIN_MOTOR_DIR = 0;
    }
    while (pwm < MAX_MOTOR_SPEED) {
        pwm += 1;
        setDutyPercent(pwm);
    }
}

void motor_stop() {

    while (pwm > 0) {
        pwm -= 1;
        setDutyPercent(pwm);
    }
}


