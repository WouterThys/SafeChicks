/* 
 * File:   ADC_Driver.h
 * Author: thys_
 *
 * Created on August 29, 2025, 3:06 PM
 */

#ifndef ADC_DRIVER_H
#define	ADC_DRIVER_H

#include <stdint.h>
    
/**
* Initialises all the parameters to the default setting
*/
void D_ADC_Init(void);

    
/**
* Read an ADC value once
*/
uint16_t D_ADC_ReadOnce(void);

#endif	/* ADC_DRIVER_H */

