#ifndef CONFIG_H
#define	CONFIG_H

#include <xc.h> // include processor files - each processor file is guarded.  

#define _XTAL_FREQ 1000000UL /* 1 MHz clock */
#define DEBUG_MODE 0

/*******************************************************************************
 *                      PIN MAPPING 
 ******************************************************************************/

/* This file contains all pin mapping of the board */
#define MOTOR_PWM       PORTCbits.RC2     // Pin for motor PWM
#define MOTOR_DIR_Pin   PORTCbits.RC1     // Pin for motor direction
#define MOTOR_DIR_Dir   TRISCbits.RC1     // Pin for motor direction

// Switched and buttons
#define U_SENSOR_Pin    PORTBbits.RB0
#define U_SENSOR_Dir    TRISBbits.TRISB0
#define B_SENSOR_Pin    PORTBbits.RB1
#define B_SENSOR_Dir    TRISBbits.TRISB1
#define U_BUTTON_Pin    PORTBbits.RB2
#define U_BUTTON_Dir    TRISBbits.TRISB2
#define D_BUTTON_Pin    PORTBbits.RB3
#define D_BUTTON_Dir    TRISBbits.TRISB3

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
const uint8_t   ERROR_SENSORS_BOTH_CLOSED = 1;
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

/* When not in LOW_POWER (see LowPower switch) we don't go to sleep, 
 * but use this delay to fake a sleep time */
#define  LOW_POWER_SLEEP_TIME_MS 10000

#define   THR_SLEEP_COUNT   5   /* The total sleep time will be SLEEP_TIME_MS times this value, to allow shorter wakeup intervals for sanity checking */
#define   THR_DN_COUNT      3   /* Hysteresis counter, depending on time between sleeps this makes how long day/night should be read before changing */

#define   MAX_MOTOR_COUNT   1000/* The max count the motor should be running. */
#define   MAX_MOTOR_SPEED   100 /* PWM percentage                             */
#define   CCW_DIRECTION     1   /* Counter clockwise direction                */
#define   CW_DIRECTION      0   /* Clockwise direction                */



/*******************************************************************************
 *                      SERIAL SETTINGS 
 ******************************************************************************/

/**
 * Baud rate of serial communication
 */
#define SERIAL_BAUD 1200



#endif	/* CONFIG_H */

