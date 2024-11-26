#include "bme680.h"
#include "smtc_hal_i2c.h"

typedef enum {
    eModeSleep,
    eModeForced
} eMode_t;

static struct {
    unsigned char buf[24];
    eMode_t Operation_mode;
    eDeviceStatus_t* pstatus;
} bme680;

#define BME680_CTRL_MEAS                0x74

static const uint8_t BME680_ADDRESS = ( 0x77 << 1 );

void bme680_init( eDeviceStatus_t* pstatus ) {
    /* Driver still to be developed. It just set the bme680 in sleep mode*/

    bme680.pstatus = pstatus;
    bme680.Operation_mode = eModeSleep;

    hal_i2c_set_addr_size( I2C_ADDR_SIZE_8 );

    bme680.buf[0] = bme680.Operation_mode;

    if( !hal_i2c_write( BME680_ADDRESS, BME680_CTRL_MEAS, *bme680.buf ) ) {
        *bme680.pstatus = eDevice_Initialized;
    } else {
        *bme680.pstatus = eDevice_Failed;
    }

    return;
}