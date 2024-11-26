#ifndef __EXTENSIONS_DRIVER_SHTC3_H__
#define __EXTENSIONS_DRIVER_SHTC3_H__

#include <stdbool.h>
#include <stdint.h>

bool shtc3_init( uint8_t i2c_address );

bool shtc3_read_temp( int32_t* temp, int32_t* rh );

#endif // __EXTENSIONS_DRIVER_SHTC3_H__
