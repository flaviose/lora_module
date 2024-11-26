/*
 * lis2dw.c
 *
 *************************************
 *  Created on: 08.06.2022
 *      Author: Viktor Havar
 * 		miromico AG
 *      Gallusstrasse 4, 8006 Zuerich
 *
 *  Updated to Semtech HAL by Benjamin Boulet (Semtech) in January 2023
 *
 * 	Copyright (c) 2022 Miromico AG
 * 		All rights reserved.
 *************************************
 *
 */

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */
#include "lis2dw.h"
#include "smtc_hal_gpio.h"
#include "smtc_hal_mcu.h"
#include "smtc_hal_i2c.h"

#define LIS2DW_BUFFER_SIZE 6

typedef enum {
    LIS2DW_SENS_2G,
    LIS2DW_SENS_4G,
    LIS2DW_SENS_8G,
    LIS2DW_SENS_16G,
} lis2dw_sens_t;

/*
 * -----------------------------------------------------------------------------
 * --- 	PUBLIC VARIABLES -------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */
static hal_gpio_pin_names_t lis2dw_int2_pin;
static volatile bool lis2dw_sleeping_flag = true;
static volatile bool lis2dw_drdy_flag = false;
static lis2dw_sens_t lis2dw_sens = LIS2DW_SENS_16G;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */
static bool lis2dw_write_byte( uint8_t register_addr, uint8_t value );
static bool lis2dw_read_byte( uint8_t register_addr, uint8_t* value );
static bool lis2dw_read_multiple( uint8_t register_addr, uint8_t* value, uint8_t size );
static void lis2dw_set_int2_pin( hal_gpio_pin_names_t port_pin );
static void lis2dw_enable_int2_gpio( hal_gpio_irq_t* callback, bool risingEdge, bool fallingEdge );
static void lis2dw_int2_sleep_callback( void* context );
static void lis2dw_int2_drdy_callback( void* context );
static uint16_t lis2dw_sens_to_mg( lis2dw_sens_t sensitivity );
static int32_t sign( int32_t value );

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

bool lis2dw_init( void ) {
    bool i2c_status = true;

    // check who am i
    uint8_t who_am_i;
    i2c_status = lis2dw_read_byte( WHO_AM_I, &who_am_i );

    if( !i2c_status || who_am_i != WHO_AM_I_ID ) {
        return false;
    }

    // soft reset to defaults
    i2c_status = lis2dw_write_byte( CTRL2_REG, 0b01000000 );

    if( !i2c_status ) {
        return false;
    }

    uint8_t answer;

    do {
        i2c_status = lis2dw_read_byte( CTRL2_REG, &answer );
        hal_mcu_wait_us( 5000 );
    } while( answer & 0x40 );  // check if the reset is finished

    // ODR = 12.5 Hz | low-power mode | low-power mode 1 (12-bit)
    i2c_status = lis2dw_write_byte( CTRL1_REG | 0x80, 0b0010 << 4 | 0b00 << 2 | 0b00 );

    if( !i2c_status ) {
        return false;
    }

    // Bandwidth ODR/2 | full-scale 16g | low-pass filter | low-noise disabled
    lis2dw_sens = LIS2DW_SENS_16G;
    i2c_status = lis2dw_write_byte( CTRL6_REG, 0b00 << 6 | 0b11 << 4 | 0b0 << 3 | 0b0 << 2 );

    if( !i2c_status ) {
        return false;
    }

    // Increment register address automatically during multiple byte access (IF_ADD_INC)
    i2c_status = lis2dw_write_byte( CTRL2_REG, 0b00000100 );

    if( !i2c_status ) {
        return false;
    }

    return true;
}

bool lis2dw_set_threshold( uint16_t threshold ) {
    bool i2c_status = true;

    // Find matching sensitivity for given threshold
    // -> The largest threshold that is configurable for 16g is 15.75g since the threshold is a 6 bit value.
    //    If all 6 bits are 1, the threshold is equal to 63 * 1 LSB = 63 * FS/64 = 63 * 16g/64 = 15.75g
    if( threshold <= 63 * lis2dw_sens_to_mg( LIS2DW_SENS_2G ) / 64 ) {
        lis2dw_sens = LIS2DW_SENS_2G;
    } else if( threshold <= 63 *  lis2dw_sens_to_mg( LIS2DW_SENS_4G ) / 64 ) {
        lis2dw_sens = LIS2DW_SENS_4G;
    } else if( threshold <= 63 * lis2dw_sens_to_mg( LIS2DW_SENS_8G ) / 64 ) {
        lis2dw_sens = LIS2DW_SENS_8G;
    } else {
        lis2dw_sens = LIS2DW_SENS_16G;
    }

    // Ensure threshold is configurable
    if( threshold > 63 * lis2dw_sens_to_mg( lis2dw_sens ) / 64 ) {
        threshold = 63 * lis2dw_sens_to_mg( lis2dw_sens ) / 64;
    }

    // Calculate the 6 bit threshold value
    // -> "+ lis2dw_sens_to_mg(lis2dw_sens)/2" is used to round the value to the nearest integer (integer division)
    uint8_t value = ( threshold * 64 + lis2dw_sens_to_mg( lis2dw_sens ) / 2 ) / lis2dw_sens_to_mg( lis2dw_sens );

    if( value == 0 ) {
        value = 1;
    }

    // SMTC_HAL_TRACE_INFO( "lis2dw_set_threshold: threshold %u mg, sensitivity %u mg, value %u\n", threshold, lis2dw_sens_to_mg( lis2dw_sens ), value );

    // Disable interrupts while configuring the threshold
    uint32_t mask = 0;
    hal_mcu_critical_section_begin( &mask );

    // Bandwidth ODR/2 | full-scale 16g | low-pass filter | low-noise disabled
    i2c_status = lis2dw_write_byte( CTRL6_REG, 0b00 << 6 | lis2dw_sens << 4 | 0b0 << 3 | 0b0 << 2 );

    if( !i2c_status ) {
        hal_mcu_critical_section_end( &mask );
        return false;
    }

    // Single tap event enabled, sleep enabled, wake-up threshold 1 LSB = FS/64
    i2c_status = lis2dw_write_byte( WAKE_UP_THS, 0 << 7 | 1 << 6 | ( value & 0b111111 ) );

    if( !i2c_status ) {
        hal_mcu_critical_section_end( &mask );
        return false;
    }

    hal_mcu_critical_section_end( &mask );
    return true;
}

bool lis2dw_configure_int2_sleep( hal_gpio_pin_names_t pin, uint16_t threshold ) {
    // set INT2 port and pin
    lis2dw_set_int2_pin( pin );

    bool i2c_status = true;

    // wake-up duration 2/ODR (= 2/12.5 = 0.16 sec), stationary detection off
    // sleep duration 16/ODR (= 16/12.5 = 1.28 sec)
    i2c_status = lis2dw_write_byte( WAKE_UP_DUR, 0b01000000 );

    if( !i2c_status ) {
        return false;
    }

    i2c_status = lis2dw_set_threshold( threshold );

    if( !i2c_status ) {
        return false;
    }

    // enabling both bits, SLEEP_STATE and SLEEP_CHG, effectively enable SLEEP_STATE on INT2
    // -> see LIS2DW12 AN5038, section 5.7  Activity/inactivity recognition
    // -> INT2 is HIGH when LIS2DW12 is in sleep, low when activity is detected
    i2c_status = lis2dw_write_byte( CTRL5_INT2_PAD_CTRL, 1 << 7 | 1 << 6 );

    if( !i2c_status ) {
        return false;
    }

    // enable interrupts, everything else on default
    i2c_status = lis2dw_write_byte( CTRL7_REG, 1 << 5 );

    if( !i2c_status ) {
        return false;
    }

    // enable INT2 on rising edge and falling edge
    static hal_gpio_irq_t lis2dw_int2 = {
        // .pin      = lis2dw_int2_pin,
        .context  = NULL,
        .callback = lis2dw_int2_sleep_callback
    };
    lis2dw_enable_int2_gpio( &lis2dw_int2, true, true );
    lis2dw_sleeping_flag = true;

    return true;
}

bool lis2dw_configure_int2_drdy( hal_gpio_pin_names_t port_pin ) {
    // set INT2 port and pin
    lis2dw_set_int2_pin( port_pin );

    bool i2c_status = true;

    // Route data-ready to INT2
    i2c_status = lis2dw_write_byte( CTRL5_INT2_PAD_CTRL, 1 << 0 );

    if( !i2c_status ) {
        return false;
    }

    // Use plused mode for data-ready signal, enable interrupts
    i2c_status = lis2dw_write_byte( CTRL7_REG, 1 << 7 | 1 << 5 );

    if( !i2c_status ) {
        return false;
    }

    // Enable INT2 only for rising edge
    static hal_gpio_irq_t lis2dw_int2 = {
        // .pin      = lis2dw_int2_pin,
        .context  = NULL,
        .callback = lis2dw_int2_drdy_callback
    };
    lis2dw_enable_int2_gpio( &lis2dw_int2, true, false );
    lis2dw_drdy_flag = false;

    return true;
}

bool lis2dw_configure_int2_fifo( hal_gpio_pin_names_t port_pin, uint8_t fifo_threshold ) {
    // set INT2 port and pin
    lis2dw_set_int2_pin( port_pin );

    bool i2c_status = true;

    if( fifo_threshold > 31 ) {
        // Route FIFO FULL signal to INT2
        i2c_status = lis2dw_write_byte( CTRL5_INT2_PAD_CTRL, 1 << 2 );

        if( !i2c_status ) {
            return false;
        }

        // Enable FIFO in continuous mode
        i2c_status = lis2dw_write_byte( FIFO_CTRL, 0b110 << 5 );

        if( !i2c_status ) {
            return false;
        }
    } else {
        // Route FIFO threshold signal to INT2
        i2c_status = lis2dw_write_byte( CTRL5_INT2_PAD_CTRL, 1 << 1 );

        if( !i2c_status ) {
            return false;
        }

        // Enable FIFO in continuous mode, set FIFO threshold to fifo_threshold
        i2c_status = lis2dw_write_byte( FIFO_CTRL, 0b110 << 5 | fifo_threshold );

        if( !i2c_status ) {
            return false;
        }
    }

    // Enable interrupts, everything else on default
    i2c_status = lis2dw_write_byte( CTRL7_REG, 1 << 5 );

    if( !i2c_status ) {
        return false;
    }

    // Enable INT2 only for rising edge
    static hal_gpio_irq_t lis2dw_int2 = {
        // .pin      = lis2dw_int2_pin,
        .context  = NULL,
        .callback = ( ( void* )lis2dw_int2_drdy_callback )
    };
    lis2dw_enable_int2_gpio( &lis2dw_int2, true, false );
    lis2dw_drdy_flag = false;

    return true;
}

bool lis2dw_activity_detected( void ) {
    return !lis2dw_sleeping_flag;
}

bool lis2dw_data_ready( void ) {
    return lis2dw_drdy_flag;
}

void lis2dw_clear_data_ready( void ) {
    lis2dw_drdy_flag = false;
}

bool lis2dw_read_accel( int16_t* x, int16_t* y, int16_t* z ) {
    uint8_t buf[LIS2DW_BUFFER_SIZE];
    bool status = lis2dw_read_multiple( OUT_X_L, buf, sizeof( buf ) );

    if( status ) {
        *x = buf[0] | ( buf[1] << 8 );
        *y = buf[2] | ( buf[3] << 8 );
        *z = buf[4] | ( buf[5] << 8 );
    }

    return status;
}

bool lis2dw_read_accel_mg( int16_t* x_out, int16_t* y_out, int16_t* z_out ) {
    // Read raw acceleration values
    bool status = lis2dw_read_accel( x_out, y_out, z_out );

    if( status ) {
        // Since low-power mode 1 is enabled, only 12 bits are used for the acceleration values
        // -> The values need to be shifted right by 4 bits to get the correct values
        int32_t x_tmp = *x_out >> 4;
        int32_t y_tmp = *y_out >> 4;
        int32_t z_tmp = *z_out >> 4;

        // Check sensitivity to set factor according to datasheet (table 3, mechanical characteristics)
        // -> The values below are only valid for low-power mode 1
        int32_t factor = 0;

        switch( lis2dw_sens ) {
        case LIS2DW_SENS_2G:
            factor = 976;
            break;

        case LIS2DW_SENS_4G:
            factor = 1952;
            break;

        case LIS2DW_SENS_8G:
            factor = 3904;
            break;

        case LIS2DW_SENS_16G:
            factor = 7808;
            break;

        default:
            SMTC_HAL_TRACE_ERROR( "Invalid sensitivity value '%u'!\n", lis2dw_sens );
            factor = 7808;  // Return factor for 16g as default
            break;
        }

        // Convert to mg ("+ (sign(x_tmp) * 500)" to ensure mathematical rounding after integer division)
        x_tmp = ( x_tmp * factor + ( sign( x_tmp ) * 500 ) ) / 1000;
        y_tmp = ( y_tmp * factor + ( sign( y_tmp ) * 500 ) ) / 1000;
        z_tmp = ( z_tmp * factor + ( sign( z_tmp ) * 500 ) ) / 1000;

        // Convert to int16_t
        *x_out = ( int16_t ) x_tmp;
        *y_out = ( int16_t ) y_tmp;
        *z_out = ( int16_t ) z_tmp;
    }

    return status;
}

bool lis2dw_get_status( uint8_t* status ) {
    return lis2dw_read_byte( STATUS_REG, status );
}

bool lis2dw_get_fifo_level( uint8_t* level ) {
    uint8_t fifo_samples = 0;
    bool status = lis2dw_read_byte( FIFO_SAMPLES, &fifo_samples );

    if( !status ) {
        SMTC_HAL_TRACE_ERROR( "lis2dw_get_fifo_level: Could not read FIFO_SAMPLES register!\n" );
        return false;
    }

    if( fifo_samples & 0x40 ) {
        SMTC_HAL_TRACE_WARNING( "lis2dw_get_fifo_level: FIFO overrun, some samples were lost!\n" );
    }

    // Bits 0 - 5 of FIFO_SAMPLES register contain the number of samples in the FIFO
    *level = fifo_samples & 0x3F;

    if( *level > LIS2DW_FIFO_SIZE ) {
        SMTC_HAL_TRACE_ERROR( "lis2dw_get_fifo_level: FIFO level is larger than LIS2DW_FIFO_SIZE (%u > %u)!\n", *level, LIS2DW_FIFO_SIZE );
        *level = LIS2DW_FIFO_SIZE;
    }

    return true;
}

bool lis2dw_print_regs( void ) {
    bool ok = true;
    uint8_t reg_val = 0;
    ok &= lis2dw_read_byte( STATUS_REG, &reg_val );
    SMTC_HAL_TRACE_INFO( "Reading %02X from STATUS_REG\n", reg_val );
    ok &= lis2dw_read_byte( CTRL1_REG, &reg_val );
    SMTC_HAL_TRACE_INFO( "Reading %02X from CTRL1_REG\n", reg_val );
    ok &= lis2dw_read_byte( CTRL2_REG, &reg_val );
    SMTC_HAL_TRACE_INFO( "Reading %02X from CTRL2_REG\n", reg_val );
    ok &= lis2dw_read_byte( CTRL3_REG, &reg_val );
    SMTC_HAL_TRACE_INFO( "Reading %02X from CTRL3_REG\n", reg_val );
    ok &= lis2dw_read_byte( CTRL4_INT1_PAD_CTRL, &reg_val );
    SMTC_HAL_TRACE_INFO( "Reading %02X from CTRL4_INT1_PAD_CTRL\n", reg_val );
    ok &= lis2dw_read_byte( CTRL5_INT2_PAD_CTRL, &reg_val );
    SMTC_HAL_TRACE_INFO( "Reading %02X from CTRL5_INT2_PAD_CTRL\n", reg_val );
    ok &= lis2dw_read_byte( CTRL6_REG, &reg_val );
    SMTC_HAL_TRACE_INFO( "Reading %02X from CTRL6_REG\n", reg_val );
    ok &= lis2dw_read_byte( CTRL7_REG, &reg_val );
    SMTC_HAL_TRACE_INFO( "Reading %02X from CTRL7_REG\n", reg_val );
    ok &= lis2dw_read_byte( FIFO_CTRL, &reg_val );
    SMTC_HAL_TRACE_INFO( "Reading %02X from FIFO_CTRL\n", reg_val );
    ok &= lis2dw_read_byte( FIFO_SAMPLES, &reg_val );
    SMTC_HAL_TRACE_INFO( "Reading %02X from FIFO_SAMPLES\n", reg_val );
    ok &= lis2dw_read_byte( TAP_THS_X, &reg_val );
    SMTC_HAL_TRACE_INFO( "Reading %02X from TAP_THS_X\n", reg_val );
    ok &= lis2dw_read_byte( TAP_THS_Y, &reg_val );
    SMTC_HAL_TRACE_INFO( "Reading %02X from TAP_THS_Y\n", reg_val );
    ok &= lis2dw_read_byte( TAP_THS_Z, &reg_val );
    SMTC_HAL_TRACE_INFO( "Reading %02X from TAP_THS_Z\n", reg_val );
    ok &= lis2dw_read_byte( INT_DUR, &reg_val );
    SMTC_HAL_TRACE_INFO( "Reading %02X from INT_DUR\n", reg_val );
    ok &= lis2dw_read_byte( WAKE_UP_THS, &reg_val );
    SMTC_HAL_TRACE_INFO( "Reading %02X from WAKE_UP_THS\n", reg_val );
    ok &= lis2dw_read_byte( WAKE_UP_DUR, &reg_val );
    SMTC_HAL_TRACE_INFO( "Reading %02X from WAKE_UP_DUR\n", reg_val );
    ok &= lis2dw_read_byte( FREE_FALL, &reg_val );
    SMTC_HAL_TRACE_INFO( "Reading %02X from FREE_FALL\n", reg_val );
    ok &= lis2dw_read_byte( STATUS_DUP, &reg_val );
    SMTC_HAL_TRACE_INFO( "Reading %02X from STATUS_DUP\n", reg_val );
    ok &= lis2dw_read_byte( WAKE_UP_SRC, &reg_val );
    SMTC_HAL_TRACE_INFO( "Reading %02X from WAKE_UP_SRC\n", reg_val );
    ok &= lis2dw_read_byte( TAP_SRC, &reg_val );
    SMTC_HAL_TRACE_INFO( "Reading %02X from TAP_SRC\n", reg_val );
    ok &= lis2dw_read_byte( SIXD_SRC, &reg_val );
    SMTC_HAL_TRACE_INFO( "Reading %02X from SIXD_SRC\n", reg_val );
    ok &= lis2dw_read_byte( ALL_INT_SRC, &reg_val );
    SMTC_HAL_TRACE_INFO( "Reading %02X from ALL_INT_SRC\n", reg_val );
    ok &= lis2dw_read_byte( X_OFS_USR, &reg_val );
    SMTC_HAL_TRACE_INFO( "Reading %02X from X_OFS_USR\n", reg_val );
    ok &= lis2dw_read_byte( Y_OFS_USR, &reg_val );
    SMTC_HAL_TRACE_INFO( "Reading %02X from Y_OFS_USR\n", reg_val );
    ok &= lis2dw_read_byte( Z_OFS_USR, &reg_val );
    SMTC_HAL_TRACE_INFO( "Reading %02X from Z_OFS_USR\n", reg_val );
    ok &= lis2dw_read_byte( WHO_AM_I, &reg_val );
    SMTC_HAL_TRACE_INFO( "Reading %02X from WHO_AM_I\n", reg_val );
    return ok;
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

static bool lis2dw_write_byte( uint8_t register_addr, uint8_t value ) {
    hal_i2c_set_addr_size( I2C_ADDR_SIZE_8 );
    return !hal_i2c_write( lis2dw_ADDRESS, register_addr, value );
}

static bool lis2dw_read_byte( uint8_t register_addr, uint8_t* value ) {
    hal_i2c_set_addr_size( I2C_ADDR_SIZE_8 );
    return !hal_i2c_read( lis2dw_ADDRESS, register_addr, value );
}

static bool lis2dw_read_multiple( uint8_t register_addr, uint8_t* value, uint8_t size ) {
    hal_i2c_set_addr_size( I2C_ADDR_SIZE_8 );
    return !hal_i2c_read_buffer( lis2dw_ADDRESS, register_addr, value, size );
}

static void lis2dw_set_int2_pin( hal_gpio_pin_names_t pin ) {
    lis2dw_int2_pin = pin;
}

static void lis2dw_int2_sleep_callback( void* context ) {
    bool value = hal_gpio_get_value( lis2dw_int2_pin );

    if( value ) {
        lis2dw_sleeping_flag = true;
    } else {
        lis2dw_sleeping_flag = false;
    }
}

static void lis2dw_int2_drdy_callback( void* context ) {
    bool value = hal_gpio_get_value( lis2dw_int2_pin );

    if( value ) {
        lis2dw_drdy_flag = true;
    }
}

static void lis2dw_enable_int2_gpio( hal_gpio_irq_t* callback, bool risingEdge, bool fallingEdge ) {
    uint8_t edge = ( risingEdge && fallingEdge ) ? BSP_GPIO_IRQ_MODE_RISING_FALLING :
                   ( risingEdge ? BSP_GPIO_IRQ_MODE_RISING :
                     ( fallingEdge ? BSP_GPIO_IRQ_MODE_FALLING : BSP_GPIO_IRQ_MODE_OFF ) );
    hal_gpio_init_in( lis2dw_int2_pin, BSP_GPIO_PULL_MODE_NONE, edge, callback );
    hal_gpio_irq_attach( callback );
}

static uint16_t lis2dw_sens_to_mg( lis2dw_sens_t sensitivity ) {
    switch( sensitivity ) {
    case LIS2DW_SENS_2G:
        return 2000;

    case LIS2DW_SENS_4G:
        return 4000;

    case LIS2DW_SENS_8G:
        return 8000;

    case LIS2DW_SENS_16G:
        return 16000;

    default:
        SMTC_HAL_TRACE_ERROR( "Invalid sensitivity value '%u'!\n", sensitivity );
        // Return 16g as default
        return 16000;
    }
}

static int32_t sign( int32_t value ) {
    if( value > 0 ) {
        return 1;
    } else if( value < 0 ) {
        return -1;
    } else {
        return 0;
    }
}

/* --- EOF ------------------------------------------------------------------ */
