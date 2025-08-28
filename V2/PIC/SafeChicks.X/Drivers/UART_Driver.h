#ifndef UART_DRIVER_H
#define	UART_DRIVER_H
    
#include <stdbool.h>
#include <stdint.h>
    
/**
* Initializes all the parameters to the default setting, as well as writing the
* tri-state registers. Initializes the UART to the default data rate and settings.
 * Baud rate is configured in the config.h file.
*/
void D_UART_Init();

/**
 * Write data to the TX pin of UART module. 
 * @param data: Date string to write
 */
void D_UART_Write(const char* data);

/**
 * Write integer to the TX pin of UART module.
 * @param command: Command
 * @param d: Integer to write
 */
void D_UART_WriteInt(const char* command, int d);

///**
// * Read data from the RX pin of UART module.
// * @return data: returns the data struct.
// */
//READ_Data D_UART_Read();

/**
 * Enable the UART module
 * @param enable Enable or disable UART.
 */
void D_UART_Enable(bool enable);

#endif	/* UART_DRIVER */