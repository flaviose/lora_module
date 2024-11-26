#ifndef _LPS22HH_H_
#define _LPS22HH_H_

#include "driver_types.h"
#include "smtc_hal_i2c.h"

typedef struct {
    int32_t temp;
    uint32_t p;
} lps22hh_data;

void lps22hh_init( eDeviceStatus_t* pstatus );

void lps22hh_read( eDeviceStatus_t* pstatus, lps22hh_data* pdata );

#endif
