/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */
#include "smtc_hal_i2c.h"

#include "stm32l0xx_ll_i2c.h"
#include "stm32l0xx_ll_gpio.h"
#include "stm32l0xx_ll_bus.h"

#include "smtc_hal_rtc.h"

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS-----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE TYPES -----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */

static i2c_addr_size i2c_internal_addr_size;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

static uint8_t i2c_write_buffer( const uint8_t device_addr, const uint16_t addr, const uint8_t* buffer, const uint16_t size, const uint32_t timeout_ms );

static uint8_t i2c_read_buffer( const uint8_t device_addr, const uint16_t addr, uint8_t* buffer, const uint16_t size, const uint32_t timeout_ms );

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

__attribute__( ( weak ) ) void hal_i2c_init( void ) {

    // Configure I2C SCL, SDA pins: PB6, PB9
    // enable clock for gpio B port
    LL_IOP_GRP1_EnableClock( LL_IOP_GRP1_PERIPH_GPIOB );

    LL_GPIO_SetPinMode( GPIOB, LL_GPIO_PIN_6, LL_GPIO_MODE_ALTERNATE );
    LL_GPIO_SetPinSpeed( GPIOB, LL_GPIO_PIN_6, LL_GPIO_SPEED_FREQ_VERY_HIGH );
    LL_GPIO_SetAFPin_0_7( GPIOB, LL_GPIO_PIN_6, LL_GPIO_AF_1 );
    LL_GPIO_SetPinOutputType( GPIOB, LL_GPIO_PIN_6, LL_GPIO_OUTPUT_OPENDRAIN );
    LL_GPIO_SetPinPull( GPIOB, LL_GPIO_PIN_6, LL_GPIO_PULL_NO );

    LL_GPIO_SetPinMode( GPIOB, LL_GPIO_PIN_9, LL_GPIO_MODE_ALTERNATE );
    LL_GPIO_SetPinSpeed( GPIOB, LL_GPIO_PIN_9, LL_GPIO_SPEED_FREQ_VERY_HIGH );
    LL_GPIO_SetAFPin_8_15( GPIOB, LL_GPIO_PIN_9, LL_GPIO_AF_4 );
    LL_GPIO_SetPinOutputType( GPIOB, LL_GPIO_PIN_9, LL_GPIO_OUTPUT_OPENDRAIN );
    LL_GPIO_SetPinPull( GPIOB, LL_GPIO_PIN_9, LL_GPIO_PULL_NO );

    // Enable I2C1 clock
    LL_APB1_GRP1_EnableClock( LL_APB1_GRP1_PERIPH_I2C1 );

    // I2C1 init
    LL_I2C_InitTypeDef i2c_init = { 0 };

    i2c_init.PeripheralMode     = LL_I2C_MODE_I2C;
    i2c_init.Timing             = 0x00707CBB;
    i2c_init.AnalogFilter       = LL_I2C_ANALOGFILTER_DISABLE;
    i2c_init.DigitalFilter      = 0x00;
    i2c_init.OwnAddress1        = 0x00;
    i2c_init.TypeAcknowledge    = LL_I2C_ACK;
    i2c_init.OwnAddrSize        = LL_I2C_OWNADDRESS1_7BIT;

    LL_I2C_Init( I2C1, &i2c_init );
    LL_I2C_Enable( I2C1 );

    // Set internal address size
    i2c_internal_addr_size = I2C_ADDR_SIZE_8;
}

__attribute__( ( weak ) ) void hal_i2c_deinit( void ) {

    // Deinit I2C1
    LL_I2C_DeInit( I2C1 );

    // Disable I2C1 clock
    LL_APB1_GRP1_DisableClock( LL_APB1_GRP1_PERIPH_I2C1 );

    // De-init I2C SCL SDA pins: PB6 and PB9
    LL_GPIO_SetPinSpeed( GPIOB, LL_GPIO_PIN_6, LL_GPIO_SPEED_FREQ_LOW );
    LL_GPIO_SetPinMode( GPIOB, LL_GPIO_PIN_6, LL_GPIO_MODE_ANALOG );

    LL_GPIO_SetPinSpeed( GPIOB, LL_GPIO_PIN_9, LL_GPIO_SPEED_FREQ_LOW );
    LL_GPIO_SetPinMode( GPIOB, LL_GPIO_PIN_9, LL_GPIO_MODE_ANALOG );
}

__attribute__( ( weak ) ) void hal_i2c_set_addr_size( i2c_addr_size addr_size ) { i2c_internal_addr_size = addr_size; }


__attribute__( ( weak ) ) uint8_t hal_i2c_write( const uint8_t device_addr, const uint16_t addr, const uint8_t data ) {
    return i2c_write_buffer( device_addr, addr, &data, 1, 2000u );
}

__attribute__( ( weak ) ) uint8_t hal_i2c_read( const uint8_t device_addr, const uint16_t addr, uint8_t* data ) {
    return i2c_read_buffer( device_addr, addr, data, 1, 2000u );
}

__attribute__( ( weak ) ) uint8_t hal_i2c_write_buffer( const uint8_t device_addr, const uint16_t addr, const uint8_t* buffer, const uint16_t size ) {
    return i2c_write_buffer( device_addr, addr, buffer, size, 2000u );
}

__attribute__( ( weak ) ) uint8_t hal_i2c_read_buffer( const uint8_t device_addr, const uint16_t addr, uint8_t* buffer, const uint16_t size ) {
    return i2c_read_buffer( device_addr, addr, buffer, size, 2000u );
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

static uint8_t i2c_write_buffer( const uint8_t device_addr, const uint16_t addr, const uint8_t* buffer, const uint16_t size, const uint32_t timeout_ms ) {
    uint8_t index = 0;
    uint16_t wlen = ( i2c_internal_addr_size == I2C_ADDR_SIZE_8 ) ? size + 1 : size + 2;
    uint32_t start = hal_rtc_get_time_ms( );

    // Ensure the I2C is not busy
    while( LL_I2C_IsActiveFlag_BUSY( I2C1 ) ) {
        if( ( hal_rtc_get_time_ms( ) - start ) > timeout_ms ) { return 1; }
    }

    LL_I2C_HandleTransfer( I2C1, ( uint32_t )device_addr, LL_I2C_ADDRSLAVE_7BIT, ( uint32_t )wlen, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE );

    // Transmit internal register address
    if( i2c_internal_addr_size == I2C_ADDR_SIZE_8 ) {

        // Wait for TXE flag to be raised
        while( !LL_I2C_IsActiveFlag_TXE( I2C1 ) ) {
            if( LL_I2C_IsActiveFlag_NACK( I2C1 ) ) { goto error; }
        }

        LL_I2C_TransmitData8( I2C1, ( uint8_t )addr );

    } else {

        // Wait for TXE flag to be raised
        while( !LL_I2C_IsActiveFlag_TXE( I2C1 ) ) {
            if( LL_I2C_IsActiveFlag_NACK( I2C1 ) ) { goto error; }
        }

        LL_I2C_TransmitData8( I2C1, ( uint8_t )( ( addr >> 8 ) & 0xFF ) );

        // Wait for TXE flag to be raised
        while( !LL_I2C_IsActiveFlag_TXE( I2C1 ) ) {
            if( LL_I2C_IsActiveFlag_NACK( I2C1 ) ) { goto error; }
        }

        LL_I2C_TransmitData8( I2C1, ( uint8_t )( addr & 0xFF ) );

    }

    // Transmit data to write
    while( index < size ) {
        // Wait for TXE flag to be raised
        while( !LL_I2C_IsActiveFlag_TXE( I2C1 ) ) {
            if( LL_I2C_IsActiveFlag_NACK( I2C1 ) ) { goto error; }
        }

        // Write character in Transmit Data register.
        // TXE flag is cleared by writing data in TDR register
        LL_I2C_TransmitData8( I2C1, buffer[index++] );
    }

    // Wait for STOP condition
    while( !LL_I2C_IsActiveFlag_STOP( I2C1 ) ) {
        if( LL_I2C_IsActiveFlag_NACK( I2C1 ) ) { goto error; }

        if( ( hal_rtc_get_time_ms( ) - start ) > timeout_ms ) { goto error; }
    }

    // Clear the stop flag
    LL_I2C_ClearFlag_STOP( I2C1 );

    return 0;

error:
    // Clear NACK flag in case it was raised
    LL_I2C_ClearFlag_NACK( I2C1 );

    // Generate a STOP condition in case of error to release the bus
    LL_I2C_GenerateStopCondition( I2C1 );

    // Reset the I2C peripheral
    LL_I2C_Disable( I2C1 );
    LL_I2C_Enable( I2C1 );

    return 1; // Return nonzero for error
}


static uint8_t i2c_read_buffer( const uint8_t device_addr, const uint16_t addr, uint8_t* buffer, const uint16_t size, const uint32_t timeout_ms ) {
    uint8_t index = 0;
    uint16_t wlen = ( i2c_internal_addr_size == I2C_ADDR_SIZE_8 ) ? 1 : 2;
    uint32_t start = hal_rtc_get_time_ms( );

    // Ensure the I2C is not busy
    while( LL_I2C_IsActiveFlag_BUSY( I2C1 ) ) {
        if( ( hal_rtc_get_time_ms( ) - start ) > timeout_ms ) { return 1; }
    }

    // Transmit internal register address
    LL_I2C_HandleTransfer( I2C1, ( uint32_t )device_addr, LL_I2C_ADDRSLAVE_7BIT, ( uint32_t )wlen, LL_I2C_MODE_SOFTEND, LL_I2C_GENERATE_START_WRITE );

    if( i2c_internal_addr_size == I2C_ADDR_SIZE_8 ) {

        // Wait for TXE flag to be raised
        while( !LL_I2C_IsActiveFlag_TXE( I2C1 ) ) {
            if( LL_I2C_IsActiveFlag_NACK( I2C1 ) ) { goto error; }
        }

        LL_I2C_TransmitData8( I2C1, ( uint8_t )addr );

    } else {

        // Wait for TXE flag to be raised
        while( !LL_I2C_IsActiveFlag_TXE( I2C1 ) ) {
            if( LL_I2C_IsActiveFlag_NACK( I2C1 ) ) { goto error; }
        }

        LL_I2C_TransmitData8( I2C1, ( uint8_t )( ( addr >> 8 ) & 0xFF ) );

        // Wait for TXE flag to be raised
        while( !LL_I2C_IsActiveFlag_TXE( I2C1 ) ) {
            if( LL_I2C_IsActiveFlag_NACK( I2C1 ) ) { goto error; }
        }

        LL_I2C_TransmitData8( I2C1, ( uint8_t )( addr & 0xFF ) );

    }

    // Wait for the transfer to complete
    while( !LL_I2C_IsActiveFlag_TC( I2C1 ) ) {
        if( LL_I2C_IsActiveFlag_NACK( I2C1 ) ) { goto error; }

        if( ( hal_rtc_get_time_ms( ) - start ) > timeout_ms ) { goto error; }
    }

    // Receive data
    LL_I2C_HandleTransfer( I2C1, ( uint32_t )device_addr, LL_I2C_ADDRSLAVE_7BIT, ( uint32_t )size, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_RESTART_7BIT_READ );

    while( index < size ) {

        // Wait for RXNE (Receive Data Register Not Empty) flag to be raised
        while( !LL_I2C_IsActiveFlag_RXNE( I2C1 ) ) {
            if( ( hal_rtc_get_time_ms( ) - start ) > timeout_ms ) { goto error; }

            if( LL_I2C_IsActiveFlag_NACK( I2C1 ) ) { goto error; }
        }

        // Read character from Receive Data register.
        // RXNE flag is cleared by reading data from RXD register
        buffer[index++] = LL_I2C_ReceiveData8( I2C1 );
    }

    // Wait for STOP condition
    while( !LL_I2C_IsActiveFlag_STOP( I2C1 ) ) {
        if( LL_I2C_IsActiveFlag_NACK( I2C1 ) ) { goto error; }

        if( ( hal_rtc_get_time_ms( ) - start ) > timeout_ms ) { goto error; }
    }

    // Clear the stop flag
    LL_I2C_ClearFlag_STOP( I2C1 );

    return 0;

error:
    // Clear NACK flag in case it was raised
    LL_I2C_ClearFlag_NACK( I2C1 );

    // Generate a STOP condition in case of error to release the bus
    LL_I2C_GenerateStopCondition( I2C1 );

    // Reset the I2C peripheral
    LL_I2C_Disable( I2C1 );
    LL_I2C_Enable( I2C1 );

    return 1; // Return nonzero for error
}
