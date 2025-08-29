#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xc.h>
#include <string.h>

#include "../config.h"
#include "UART_Driver.h"

/*******************************************************************************
 *          DEFINES
 ******************************************************************************/

/*******************************************************************************
 *          MACRO FUNCTIONS
 ******************************************************************************/

/*******************************************************************************
 *          VARIABLES
 ******************************************************************************/
uint8_t baud;

/*******************************************************************************
 *          BASIC FUNCTIONS
 ******************************************************************************/


/*******************************************************************************
 *          DRIVER FUNCTIONS
 ******************************************************************************/
void D_UART_Init(void) {  
    
    // Disable UART while initialising
    D_UART_Enable(false);
    
    // Clear
    RCSTAbits.FERR = 0;
    RCSTAbits.OERR = 0;
    RCSTAbits.CREN = 0;
    RCREG = 0x00;
        
    // TXSTA register settings
    TXSTAbits.TX9 = 0; // Selects 8-bit transmission
    TXSTAbits.SYNC = 0; // Synchronous mode
    TXSTAbits.BRGH = 0; // Low speed
    
    // RCSTA register settings
    RCSTAbits.RX9 = 0; // Selects 8-bit reception

    // BAUDCON register settings
    BAUDCONbits.RXDTP = 0; // RX data is inverted
    BAUDCONbits.TXCKP = 0; // TX data is inverted
    BAUDCONbits.BRG16 = 0; // 8-bit Baud Rate Generator
    
    // Baud
    SPBRGH = 0;
    SPBRG = ((_XTAL_FREQ/1200)/64)-1; // Baud rate selection
    
//    // Interrupts for reading
//    if (interrupts) {
//        RCONbits.IPEN = 1;   // Enable priority levels on interrupts
//        INTCONbits.GIEH = 1; // Enable high interrupt
//        INTCONbits.GIEL = 1; // Enable low interrupt
//        PIR1bits.RCIF = 0; // Clear flag
//        IPR1bits.RCIP = 1; // High priority
//        PIE1bits.RCIE = 1; // Enable UART interrupt
//    }
}

void D_UART_Write(const char* data) {
    printf("%s", data);
    __delay_ms(1);
}

//READ_Data D_UART_Read(){
//    readData.sender = readBuffer.sender;
//    readData.command = readBuffer.command;
//    readData.message = readBuffer.message;
//    readData.ackId = readBuffer.readId;
//    return readData;
//}

void D_UART_Enable(bool enable) {
    if(enable) {
        UART_TX_Dir = 0;
        UART_RX_Dir = 1;
        
        RCSTAbits.SPEN = 1; // Enable UART
        TXSTAbits.TXEN = 1; // Activate TX
        RCSTAbits.CREN = 1; // Activate RX
        
    } else {
        UART_TX_Dir = 0;
        UART_RX_Dir = 0;
        TXSTAbits.TXEN = 0; // Deactivate TX
        RCSTAbits.CREN = 0; // Deactivate RX
        RCSTAbits.SPEN = 0; // Enable UART
    }
}

void putch(char data) {
    uint8_t max = 0;
    // Wait while buffer is still full
    while(TXSTAbits.TRMT == 0 && max < 200) {
        max++;
        __delay_us(5);
    } 
    TXREG = data;
}

//void interrupt HighISR(void) {
//    if (PIR1bits.RC1IF) {
//        PIR1bits.RC1IF = 0;
//        
//        // Framing error (can be updated by reading RCREG register and receiving next valid byte)
//        if(RCSTAbits.FERR == 1) {
//            RCREG = 0x00; 
//            return;
//        }
//        // Overrun error (can be cleared by clearing bit CREN)
//        if(RCSTAbits.OERR == 1) {
//            RCSTAbits.CREN = 0;
//            RCSTAbits.CREN = 1;
//            return;
//        }
//        fillDataBuffer(RCREG);
//    }
//}
