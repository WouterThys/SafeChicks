#ifndef CONFIG_H
#define	CONFIG_H

#include <xc.h> // include processor files - each processor file is guarded.  

#define _XTAL_FREQ 1000000UL /* 1 MHz clock */
#define DEBUG_MODE 0

#define PRIu8 "hhu"
#define PRId8 "hhd"
#define PRIx8 "hhx"
#define PRIu16 "hu"
#define PRId16 "hd"
#define PRIx16 "hx"
#define PRIu32 "lu"
#define PRId32 "dl"
#define PRIx32 "x"
#define PRIu64 "llu" // or possibly "lu"
#define PRId64 "lld" // or possibly "ld"
#define PRIx64 "llx" // or possibly "lx"


/*******************************************************************************
 *                      PIN MAPPING 
 ******************************************************************************/

/* This file contains all pin mapping of the board */
#define MOTOR_PWM_Pin   PORTCbits.RC2     // Pin for motor PWM
#define MOTOR_DIR_Pin   PORTCbits.RC1     // Pin for motor direction
#define MOTOR_DIR_Dir   TRISCbits.RC1     // Pin for motor direction

// Switched and buttons
/* Button up */
#define U_BUTTON_Pin    PORTBbits.RB0
#define U_BUTTON_Dir    TRISBbits.TRISB0
/* Button down */
#define D_BUTTON_Pin    PORTBbits.RB1
#define D_BUTTON_Dir    TRISBbits.TRISB1
/* Limit switch as a fail safe */
#define L_SWITCH_Pin    PORTBbits.RB3
#define L_SWITCH_Dir    TRISBbits.TRISB3

// LEDs
#define LED_BLUE_Pin    PORTBbits.RB4
#define LED_BLUE_Dir    TRISBbits.TRISB4
#define LED_RED_Pin     PORTBbits.RB5
#define LED_RED_Dir     TRISBbits.TRISB5

// Ports for UART
#define UART_TX         PORTCbits.RC7
#define UART_RX         PORTCbits.RC6
    
#define UART_TX_Dir     TRISCbits.TRISC7
#define UART_RX_Dir     TRISCbits.TRISC6

/*******************************************************************************
 *                      ERROR CODES 
 ******************************************************************************/
const uint8_t   ERROR_LIMIT_SWITCH_CLOSED = 1;
const uint8_t   ERROR_SENSORS_UP_WHILE_NIGHT = 2;
const uint8_t   ERROR_SENSORS_DOWN_WHILE_DAY = 4;
const uint8_t   ERROR_MOTOR_RUN_TOO_LONG = 8;

/*******************************************************************************
 *                      THRESHOLD VALUES 
 ******************************************************************************/
#define SECONDS_IN_MINUTE 60
#define MILLIS_IN_SECOND  1000

/* Threshold used to switch from day->night (sensor is max 1023) */
#define DAY_THRESHOLD     200
#define NIGHT_THRESHOLD   200

#define SLEEP_COUNT   5   /* The total sleep time will be SLEEP_TIME_MS times this value, to allow shorter wakeup intervals for sanity checking */
#define DAY_COUNT     3   /* Hysteresis counter, depending on time between sleeps this makes how long day/night should be read before changing */

/*******************************************************************************
 *                      MOTOR SETTINGS 
 ******************************************************************************/
#define CCW_DIRECTION       0   /* Counter clockwise direction                */
#define CW_DIRECTION        1   /* Clockwise direction                        */

#define MOTOR_FULL_SPEED    70  /* PWM percentage                             */
#define MOTOR_HALF_SPEED    35  /* PWM percentage                             */

#define MOTOR_DOWN_FULL_CNT 1100/* The max count the motor will run fast.     */
#define MOTOR_DOWN_SLOW_CNT 400 /* The max count the motor will run slow.     */
#define MAX_MOTOR_COUNT     (3*(MOTOR_DOWN_FULL_CNT + MOTOR_DOWN_SLOW_CNT))


/*******************************************************************************
 *                      SERIAL SETTINGS 
 ******************************************************************************/

/**
 * Baud rate of serial communication
 */
#define SERIAL_BAUD 1200



#endif	/* CONFIG_H */

