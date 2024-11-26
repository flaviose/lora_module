#include "smtc_hal_i2c.h"
#include "smtc_hal_dbg_trace.h"
#include "opt3001.h"

#define OPT3001_I2C_ADDR		(0x44<<1)
static uint8_t i2c_addr = 0;

#define OPT3001_REG_RESULT  0x00
#define OPT3001_REG_CONF    0x01
#define OPT3001_REG_LLOW    0x02
#define OPT3001_REG_LHIGH   0x03
#define OPT3001_REG_MFGID   0x7E
#define OPT3001_REG_DEVID   0x7F

#define OPT3001_REG_CONFIG_MASK_RANGE 0xF000
#define OPT3001_REG_CONFIG_POS_RANGE 12
#define OPT3001_REG_CONFIG_VAL_RANGE_FULL_SCALE 0xC

#define OPT3001_REG_CONFIG_MASK_CONVERSION_TIME_LONG 0x0800
#define OPT3001_REG_CONFIG_POS_CONVERSION_TIME_LONG 11

#define OPT3001_REG_CONFIG_MASK_CONVERSION_MODE 0x0600
#define OPT3001_REG_CONFIG_POS_CONVERSION_MODE 9
#define OPT3001_REG_CONFIG_VAL_CONVERSION_MODE_SHUTDOWN 0
#define OPT3001_REG_CONFIG_VAL_CONVERSION_MODE_SINGLE 1
#define OPT3001_REG_CONFIG_VAL_CONVERSION_MODE_CONTINUOUS 2

#define OPT3001_REG_CONFIG_MASK_OVERFLOW 0x0100
#define OPT3001_REG_CONFIG_POS_OVERFLOW 8

#define OPT3001_REG_CONFIG_MASK_CONVERSION_READY 0x0080
#define OPT3001_REG_CONFIG_POS_CONVERSION_READY 7

#define OPT3001_REG_CONFIG_MASK_FLAG_HIGH 0x0040
#define OPT3001_REG_CONFIG_POS_FLAG_HIGH 6

#define OPT3001_REG_CONFIG_MASK_FLAG_LOW 0x0020
#define OPT3001_REG_CONFIG_POS_FLAG_LOW 5

#define OPT3001_REG_CONFIG_MASK_LATCH 0x0010
#define OPT3001_REG_CONFIG_POS_LATCH 4

#define OPT3001_REG_CONFIG_MASK_POLARITY 0x0008
#define OPT3001_REG_CONFIG_POS_POLARITY 3

#define OPT3001_REG_CONFIG_MASK_EXPONENT 0x0004
#define OPT3001_REG_CONFIG_POS_EXPONENT 2

#define OPT3001_REG_CONFIG_MASK_FAULT 0x0003
#define OPT3001_REG_CONFIG_POS_FAULT 0


#define OPT3001_REG_CONFIG_SHUT_DOWN                    (OPT3001_REG_CONFIG_VAL_CONVERSION_MODE_SHUTDOWN << OPT3001_REG_CONFIG_POS_CONVERSION_MODE)
#define OPT3001_REG_CONFIG_SIGNLE_SHOT_FULL_SCALE_100ms (OPT3001_REG_CONFIG_VAL_RANGE_FULL_SCALE         << OPT3001_REG_CONFIG_POS_RANGE | \
                                                         OPT3001_REG_CONFIG_VAL_CONVERSION_MODE_SINGLE   << OPT3001_REG_CONFIG_POS_CONVERSION_MODE)

// uint32_t OPT3001_LSB[12] = {}


// #define OPT3001_REG_CONFIG_SHUTDOWN    0x0400 //  Shutdown Mode enabled
// #define OPT3001_REG_CONFIG_OS          0x0C00 //  One-shot enabled

#define LSR_ROUND(v,a) (((v) + (1 << ((a) - 1))) >> (a))

void opt3001_init( eDeviceStatus_t* pstatus ) {

    int i2c_status;

    if( i2c_addr == 0 ) {
        i2c_addr = OPT3001_I2C_ADDR;
    }

    hal_i2c_set_addr_size( I2C_ADDR_SIZE_8 );

    uint8_t buf[2];

    i2c_status = hal_i2c_read_buffer( i2c_addr, OPT3001_REG_MFGID, buf, 2 );

    if( i2c_status != 0 ) { goto error; }

    // SMTC_HAL_TRACE_PRINTF("opt3001 mfg id: 0x%02X%02X\n", opt3001.buf[0], opt3001.buf[1]);
    i2c_status = hal_i2c_read_buffer( i2c_addr, OPT3001_REG_DEVID, buf, 2 );

    if( i2c_status != 0 ) { goto error; }

    // SMTC_HAL_TRACE_PRINTF("opt3001 dev id: 0x%02X%02X\n", opt3001.buf[0], opt3001.buf[1]);
    *pstatus = eDevice_Initialized;
    return;

error:
    *pstatus = eDevice_Failed;
}

void opt3001_read( eDeviceStatus_t* pstatus, opt3001_data* pdata ) {

    int i2c_status;
    hal_i2c_set_addr_size( I2C_ADDR_SIZE_8 );

    uint8_t buf[2];

    // config
    buf[0] = ( uint8_t )( OPT3001_REG_CONFIG_SIGNLE_SHOT_FULL_SCALE_100ms >> 8 );
    buf[1] = ( uint8_t )( OPT3001_REG_CONFIG_SIGNLE_SHOT_FULL_SCALE_100ms );
    i2c_status = hal_i2c_write_buffer( i2c_addr, OPT3001_REG_CONF, buf, 2 );

    if( i2c_status != 0 ) { goto error; }

    // wait
    hal_mcu_delay_ms( 300 );

    // read if conversion ready
    i2c_status = hal_i2c_read_buffer( i2c_addr, OPT3001_REG_CONF, buf, 2 );
    uint16_t reg = ( ( ( uint16_t )buf[0] ) << 8 ) + buf[1];

    if( i2c_status != 0 || !( reg & OPT3001_REG_CONFIG_MASK_CONVERSION_READY ) ) {
        SMTC_HAL_TRACE_PRINTF( "opt3001: conversion not ready\n" );
        goto error;
    }

    // read
    hal_i2c_read_buffer( i2c_addr, OPT3001_REG_RESULT, buf, 2 );

    if( i2c_status != 0 ) { goto error; }

    // compute light
    uint16_t light_raw = buf[1] | ( ( int )buf[0] << 8 );
    *pstatus = eDevice_Ok;
    uint8_t exponent = 0xF & ( uint32_t )( light_raw >> 12 );
    uint16_t mantisse = light_raw & 0xFFF;
    uint32_t lsb = 1 << exponent;
    uint32_t light = ( uint32_t )( mantisse ) << exponent;
    // SMTC_HAL_TRACE_PRINTF("raw: 0x%x, exponent: 0x%x, mantisse: %d, lsb: %d, light: %d, light: %d.%d\n", light_raw, exponent, mantisse, lsb, light, light/100, light - 100*(light/100));
    pdata->light = light;

    return;
error:
    *pstatus = eDevice_Failed;
}

void opt3001_set_addr( uint8_t addr ) {
    i2c_addr = addr;
}
