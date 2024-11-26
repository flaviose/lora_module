#ifndef _OPT3001_H_
#define _OPT3001_H_

#include "driver_types.h"

#define OPT3001_INIT_STEPS   OPT3001_INIT, OPT3001_DONE

typedef struct {
    uint32_t light; // unit 0.01 lx
} opt3001_data;

void opt3001_init( eDeviceStatus_t* pstatus );

void opt3001_read( eDeviceStatus_t* pstatus, opt3001_data* pdata );

void opt3001_set_addr( uint8_t addr );

#endif
