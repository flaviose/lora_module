#include "lps22hh.h"
#include "smtc_hal_dbg_trace.h"
#include "smtc_hal_mcu.h"

#define FIFO_CTRL 0x13

#define FMODE_BYPASS 0x00 //mask FIFO_CTRL(7:5)
#define FMODE_FIFO 0x01

#define PRESS_OUT_H 0x2A
#define PRESS_OUT_L 0x29
#define PRESS_OUT_XL 0x28

#define TEMP_OUT_H 0x2C
#define TEMP_OUT_L 0x2B

#define LPS22HH_STATUS 0x27
#define FIFO_STATUS1 0x25
#define FIFO_STATUS2 0x26
#define LPS22HH_WHO_AM_I 0x0F //response always 0xB3
#define LPS22HH_DEVICE_ID 0xB3

#define LPS22HH_ODR_50HZ 0x04 //50 Hz (6:4)
#define LPS22HH_ODR_1HZ 0x01 //1 Hz (6:4)
#define LPS22HH_ODR_ONE_SHOT 0x00

#define CTRL_REG1 0x10
#define CTRL_REG2 0x11
#define CTRL_REG3 0x12

#define CTRL_REG2_DEFAULT_MASK 0x10
#define ONE_SHOT_START 0x01
#define MAXIMUM_READOUT_DELAY 11

static struct {
    unsigned char buf[24];
    eDeviceStatus_t* pstatus;
    lps22hh_data* pdata;
} lps22hh;

static const uint8_t LPS22HH_ADDRESS = ( 0x5D << 1 ); //6 bit SAD | 1 bit SA0 | 1 bit R(1)/W(0)

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */
static bool convert_value( uint8_t* buf, lps22hh_data* data );

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

void lps22hh_init( eDeviceStatus_t* pstatus ) {
    lps22hh.pstatus = pstatus;

    static int i2c_status;
    hal_i2c_set_addr_size( I2C_ADDR_SIZE_8 );

    // Read and check who am I
    i2c_status = hal_i2c_read( LPS22HH_ADDRESS, LPS22HH_WHO_AM_I, lps22hh.buf );

    if( i2c_status != 0 ) { goto error; }

    if( lps22hh.buf[0] != LPS22HH_DEVICE_ID ) {
        SMTC_HAL_TRACE_PRINTF( "lps22hh: device ID mismatch (0x%02x)\n", lps22hh.buf[0] );
        goto error;
    }

    // Set output data rate
    lps22hh.buf[0] = ( LPS22HH_ODR_ONE_SHOT << 4 );
    i2c_status = hal_i2c_write_buffer( LPS22HH_ADDRESS, CTRL_REG1, lps22hh.buf, 1 );

    if( ( i2c_status != 0 ) ) { goto error; }

    *lps22hh.pstatus = eDevice_Initialized;

    return;
error:
    *lps22hh.pstatus = eDevice_Failed;
}

void lps22hh_read( eDeviceStatus_t* pstatus, lps22hh_data* pdata ) {
    lps22hh.pstatus = pstatus;
    lps22hh.pdata = pdata;

    static int i2c_status;
    hal_i2c_set_addr_size( I2C_ADDR_SIZE_8 );

    // request
    lps22hh.buf[0] = ( CTRL_REG2_DEFAULT_MASK | ONE_SHOT_START );
    i2c_status = hal_i2c_write_buffer( LPS22HH_ADDRESS, CTRL_REG2, lps22hh.buf, 1 );

    if( i2c_status != 0 ) { goto error; }

    // delay
    hal_mcu_delay_ms( MAXIMUM_READOUT_DELAY );

    // Check if new measurement available
    // hal_i2c_read( LPS22HH_ADDRESS, LPS22HH_STATUS, lps22hh.buf);
    // SMTC_HAL_TRACE_PRINTF("DEBUG lps22hh state = %02x\n", lps22hh.buf[0]);

    // read pressure
    i2c_status = hal_i2c_read_buffer( LPS22HH_ADDRESS, PRESS_OUT_XL, lps22hh.buf, 5 );

    // SMTC_HAL_TRACE_PRINTF("DEBUG lps22hh buf = %02x%02x%02x%02x%02x\n", lps22hh.buf[0], lps22hh.buf[1], lps22hh.buf[2], lps22hh.buf[3], lps22hh.buf[4]);
    if( i2c_status != 0 ) { goto error; }

    if( !convert_value( lps22hh.buf, lps22hh.pdata ) ) {
        SMTC_HAL_TRACE_PRINTF( "lps22hh: pressure value negative!" );
        goto error;
    }

    *lps22hh.pstatus = eDevice_Ok;

    return;
error:
    *lps22hh.pstatus = eDevice_Failed;
}

static bool convert_value( uint8_t* buf, lps22hh_data* data ) {
    uint32_t pr;

    if( buf[2] >> 7 ) { return false; } // if pressure is negative, go to error

    pr = ( uint32_t )( buf[2] << 16 );
    pr |= ( uint16_t )( buf[1] << 8 );
    pr |= ( buf[0] );
    pr *= 100;
    data->p = ( ( pr + 2048 ) >> 12 );
    data->temp = ( uint16_t )( buf[4] << 8 );
    data->temp |= ( buf[3] );
    return true;
}
