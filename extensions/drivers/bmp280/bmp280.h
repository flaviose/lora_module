#ifndef _BMP280_H_
#define _BMP280_H_

#include "driver_types.h"
#include "smtc_hal_i2c.h"


typedef struct {
    int32_t temp; // T in 1/100 C
    uint32_t p;   // pressure in Pa Q24.8 format
} bmp280_data;


/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS PROTOTYPES ---------------------------------------------
 */
void bmp280_init( eDeviceStatus_t* pstatus );
void bmp280_read( eDeviceStatus_t* pstatus, bmp280_data* pdata );

#endif