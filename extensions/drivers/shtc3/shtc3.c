#include "shtc3.h"
#include "smtc_hal_i2c.h"
#include "smtc_hal_mcu.h"

static const uint8_t CRC_POLYNOMIAL    = 0x31;
static const uint8_t CRC_INIT          = 0xff;
static uint8_t address = 0;

static uint8_t shtc3_generate_crc( uint8_t* data, uint16_t count );
static inline bool shtc3_check_crc( uint8_t* data, uint16_t count );

bool shtc3_init( uint8_t i2c_address ) {
    address = i2c_address;
    hal_i2c_set_addr_size( I2C_ADDR_SIZE_16 );
    uint8_t dummy = 0;

    // Wakeup in case sht3c was put to sleep and board wasn't powered off
    hal_i2c_write_buffer( address, 0x3517, &dummy, 0 );
    hal_mcu_wait_us( 240 );

    uint8_t buffer[3] = {0};
    hal_i2c_read_buffer( address, 0xEFC8, buffer, 3 );
    bool status = shtc3_check_crc( buffer, 2 );

    if( status ) {
        // Sleep command
        uint8_t dummy = 0;
        hal_i2c_write_buffer( address, 0xB098, &dummy, 0 );
    }

    return status;
}

bool shtc3_read_temp( int32_t* temp, int32_t* rh ) {
    hal_i2c_set_addr_size( I2C_ADDR_SIZE_16 );
    uint8_t dummy = 0;

    // Wake up
    hal_i2c_write_buffer( address, 0x3517, &dummy, 0 );
    hal_mcu_wait_us( 240 );

    // Measurement command + readout. Normal mode (not lowpower mode), temp first
    uint8_t buffer[6] = { 0 };
    hal_i2c_read_buffer( address, 0x7CA2, &buffer, 6 );

    // Sleep again
    hal_i2c_write_buffer( address, 0xB098, &dummy, 0 );

    if( !shtc3_check_crc( buffer, 2 ) || !shtc3_check_crc( &buffer[3], 2 ) ) {
        return false;
    }

    int32_t temp_ticks = ( buffer[1] & 0xff ) | ( ( int32_t )buffer[0] << 8 );
    int32_t rh_ticks = ( buffer[4] & 0xff ) | ( ( int32_t )buffer[3] << 8 );

    *temp = ( ( 21875 * temp_ticks ) >> 13 ) - 45000;
    *rh = ( ( 12500 * rh_ticks ) >> 13 );

    return true;
}

static uint8_t shtc3_generate_crc( uint8_t* data, uint16_t count ) {
    uint8_t crc = CRC_INIT;
    uint8_t current_byte;
    uint8_t crc_bit;

    /* calculates 8-Bit checksum with given polynomial */
    for( current_byte = 0; current_byte < count; ++current_byte ) {
        crc ^= ( data[current_byte] );

        for( crc_bit = 8; crc_bit > 0; --crc_bit ) {
            if( crc & 0x80 ) {
                crc = ( crc << 1 ) ^ CRC_POLYNOMIAL;
            } else {
                crc = ( crc << 1 );
            }
        }
    }

    return crc;
}

static inline bool shtc3_check_crc( uint8_t* data, uint16_t count ) {
    return shtc3_generate_crc( data, count ) == data[count];
}
