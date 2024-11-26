#include <string.h>
#include "bmp280.h"
#include "smtc_hal_dbg_trace.h"
#include "smtc_hal_mcu.h"

// Registers
#define BMP280_REG_CHIP_ID              0xD0
#define BMP280_REG_SOFT_RESET           0xE0
#define BMP280_REG_STATUS               0xF3
#define BMP280_REG_CTRL_MEAS            0xF4
#define BMP280_REG_CONFIG               0xF5
#define BMP280_REG_PRES_MSB             0xF7
#define BMP280_REG_PRES_LSB             0xF8
#define BMP280_REG_PRES_XLSB            0xF9
#define BMP280_REG_TEMP_MSB             0xFA
#define BMP280_REG_TEMP_LSB             0xFB
#define BMP280_REG_TEMP_XLSB            0xFC

// Power modes
#define BMP280_MODE_SLEEP               0x00
#define BMP280_MODE_FORCED              0x01
#define BMP280_MODE_NORMAL              0x03

//Filter coefficient macros
#define BMP280_FILTER_OFF               0x00
#define BMP280_FILTER_COEFF_2           0x01
#define BMP280_FILTER_COEFF_4           0x02
#define BMP280_FILTER_COEFF_8           0x03
#define BMP280_FILTER_COEFF_16          0x04

//ODR options
#define BMP280_ODR_0_5_MS               0x00
#define BMP280_ODR_62_5_MS              0x01
#define BMP280_ODR_125_MS               0x02
#define BMP280_ODR_250_MS               0x03
#define BMP280_ODR_500_MS               0x04
#define BMP280_ODR_1000_MS              0x05
#define BMP280_ODR_2000_MS              0x06
#define BMP280_ODR_4000_MS              0x07


#define BMP280_I2C_ADDRESS              (0x76 << 1)
#define BMP280_CHIP_ID                  0x58

#define BMP280_CALIB_START              0x88
#define BMP280_CALIB_BYTE_AMOUNT        24

typedef struct bmp280_calib_param_t {
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;

    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
    int8_t dig_P10;
} bmp280_calib_param_t;

typedef enum {
    eOverSample_None,
    eOverSample_1,
    eOverSample_2,
    eOverSample_4,
    eOverSample_8,
    eOverSample_16
} eOverSample_t;

static struct {
    uint8_t buf[24];
    bmp280_calib_param_t calib_param;
    eOverSample_t p_oversample;
    eOverSample_t t_oversample;
    eDeviceStatus_t* pstatus;
    bmp280_data* pdata;
} bmp280;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

static void compensate_readout( uint8_t* buf, bmp280_data* data );

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */
void bmp280_init( eDeviceStatus_t* pstatus ) {
    bmp280.pstatus = pstatus;

    bmp280.p_oversample = eOverSample_4;
    bmp280.t_oversample = eOverSample_1;

    static int i2c_status;
    hal_i2c_set_addr_size( I2C_ADDR_SIZE_8 );

    // Read chip id
    i2c_status = hal_i2c_read( BMP280_I2C_ADDRESS, BMP280_REG_CHIP_ID, bmp280.buf );

    if( i2c_status != 0 ) { goto error; }

    if( bmp280.buf[0] != BMP280_CHIP_ID ) {
        SMTC_HAL_TRACE_PRINTF( "bmp280: device ID mismatch (0x%02x)\n", bmp280.buf[0] );
        goto error;
    }

    // Read calibration parameters
    i2c_status = hal_i2c_read_buffer( BMP280_I2C_ADDRESS, BMP280_CALIB_START, bmp280.buf, BMP280_CALIB_BYTE_AMOUNT );

    if( i2c_status != 0 ) { goto error; }

    memcpy( ( uint8_t* ) & ( bmp280.calib_param ), bmp280.buf, BMP280_CALIB_BYTE_AMOUNT );

    *bmp280.pstatus = eDevice_Initialized;

    return;
error:
    *bmp280.pstatus = eDevice_Failed;
}

void bmp280_read( eDeviceStatus_t* pstatus, bmp280_data* pdata ) {
    bmp280.pstatus = pstatus;
    bmp280.pdata = pdata;

    static int i2c_status;
    hal_i2c_set_addr_size( I2C_ADDR_SIZE_8 );

    bmp280.buf[0] = ( bmp280.t_oversample << 5 ) | ( bmp280.p_oversample << 2 ) | BMP280_MODE_FORCED;
    i2c_status = hal_i2c_write( BMP280_I2C_ADDRESS, BMP280_REG_CTRL_MEAS, bmp280.buf[0] );

    if( i2c_status != 0 ) { goto error; }

    // Delay
    const uint8_t delay_ms[] = { 8, 10, 15, 24, 45 };
    hal_mcu_delay_ms( delay_ms[bmp280.p_oversample - 1] );

    // Read
    i2c_status = hal_i2c_read_buffer( BMP280_I2C_ADDRESS, BMP280_REG_PRES_MSB, bmp280.buf, 6 );

    if( i2c_status != 0 ) { goto error; }

    compensate_readout( bmp280.buf, bmp280.pdata );
    *bmp280.pstatus = eDevice_Ok;

    return;
error:
    *bmp280.pstatus = eDevice_Failed;
}


static void compensate_readout( uint8_t* buf, bmp280_data* data ) {
    int32_t tvar1, tvar2, t_fine;
    int32_t adc_T, adc_P;

    // convert values from ADC buffer
    adc_P = ( uint32_t )( ( buf[0] << 12 ) | ( buf[1] << 4 ) | ( buf[2] >> 4 ) );
    adc_T = ( uint32_t )( ( buf[3] << 12 ) | ( buf[4] << 4 ) | ( buf[5] >> 4 ) );

    // temperature compensation
    tvar1 = ( ( ( ( adc_T >> 3 )
                  - ( ( int32_t )( bmp280.calib_param.dig_T1 ) << 1 ) ) )
              * ( ( int32_t )( bmp280.calib_param.dig_T2 ) ) ) >> 11;
    tvar2 = ( ( ( ( ( adc_T >> 4 ) - ( ( int32_t )( bmp280.calib_param.dig_T1 ) ) )
                  * ( ( adc_T >> 4 ) - ( ( int32_t )( bmp280.calib_param.dig_T1 ) ) ) )
                >> 12 ) * ( ( int32_t )( bmp280.calib_param.dig_T3 ) ) ) >> 14;
    t_fine = tvar1 + tvar2;
    data->temp = ( t_fine * 5 + 128 ) >> 8;

    // pressure compensation
    int64_t var1, var2, p;
    var1 = ( ( int64_t ) t_fine ) - 128000;
    var2 = var1 * var1 * ( int64_t )( bmp280.calib_param.dig_P6 );
    var2 = var2
           + ( ( var1 * ( int64_t )( bmp280.calib_param.dig_P5 ) ) << 17 );
    var2 = var2 + ( ( ( int64_t )( bmp280.calib_param.dig_P4 ) ) << 35 );
    var1 = ( ( var1 * var1 * ( int64_t )( bmp280.calib_param.dig_P3 ) ) >> 8 )
           + ( ( var1 * ( int64_t )( bmp280.calib_param.dig_P2 ) ) << 12 );
    var1 = ( ( ( ( ( int64_t ) 1 ) << 47 ) + var1 ) )
           * ( ( int64_t )( bmp280.calib_param.dig_P1 ) ) >> 33;

    if( var1 == 0 ) {
        // avoid exception caused by division by zero
        data->p = 0;
        return;
    }

    p = 1048576 - adc_P;
    p = ( ( ( p << 31 ) - var2 ) * 3125 ) / var1;
    var1 = ( ( ( int64_t )( bmp280.calib_param.dig_P9 ) ) * ( p >> 13 )
             * ( p >> 13 ) ) >> 25;
    var2 = ( ( ( int64_t )( bmp280.calib_param.dig_P8 ) ) * p ) >> 19;
    data->p = ( uint32_t )( ( ( p + var1 + var2 ) >> 8 )
                            + ( ( ( int64_t )( bmp280.calib_param.dig_P7 ) ) << 4 ) );
}