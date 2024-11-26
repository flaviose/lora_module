#include "settings.h"

#include "string.h"

#if defined( STM32L071xx )
#include "smtc_hal_eeprom.h"
#elif defined( STM32L451xx )
#include "smtc_hal_flash.h"
#endif

#include "smtc_hal_dbg_trace.h"

#include "sha256.h"

#if defined( STM32L451xx )
#define KEYS_FLASH_ADDRESS ADDR_FLASH_PAGE_148
#define SETTINGS_FLASH_ADDRESS ADDR_FLASH_PAGE_149
#endif

keys_t keys;
settings_t settings;

void settings_init( void ) {
#if defined( STM32L071xx )
    hal_eeprom_read_buffer( 0, ( uint8_t* )&keys, sizeof( keys_t ) );
    hal_eeprom_read_buffer( 64, ( uint8_t* )&settings, sizeof( settings_t ) );
#elif defined( STM32L451xx )
    hal_flash_read_buffer( KEYS_FLASH_ADDRESS, ( uint8_t* )&keys, sizeof( keys_t ) );
    hal_flash_read_buffer( SETTINGS_FLASH_ADDRESS, ( uint8_t* )&settings, sizeof( settings_t ) );
#endif
    uint8_t hash[32];

    sha256( hash, ( uint8_t* )&keys, sizeof( keys_t ) - 32 );
    uint8_t result_keys = memcmp( hash, keys.hash, 32 );

    sha256( hash, ( uint8_t* )&settings, sizeof( settings_t ) - 32 );
    uint8_t result_settings = memcmp( hash, settings.hash, 32 );

    if( result_keys != 0 || result_settings != 0 ) {
        SMTC_HAL_TRACE_ERROR( "Failed to load settings\n" );
        SMTC_HAL_TRACE_INFO( "Abort startup\n" );

        while( 1 );
    }
}

void settings_save( void ) {
    sha256( keys.hash, ( uint8_t* )&keys, sizeof( keys_t ) - 32 );
    sha256( settings.hash, ( uint8_t* )&settings, sizeof( settings_t ) - 32 );
#if defined( STM32L071xx )
    hal_eeprom_write_buffer( 0, ( uint8_t* )&keys, sizeof( keys_t ) );
    hal_eeprom_write_buffer( 64, ( uint8_t* )&settings, sizeof( settings_t ) );
#elif defined( STM32L451xx )
    hal_flash_erase_page( KEYS_FLASH_ADDRESS, 1 );
    hal_flash_write_buffer( KEYS_FLASH_ADDRESS, ( uint8_t* )&keys, sizeof( keys_t ) );
    hal_flash_erase_page( SETTINGS_FLASH_ADDRESS, 1 );
    hal_flash_write_buffer( SETTINGS_FLASH_ADDRESS, ( uint8_t* )&settings, sizeof( settings_t ) );
#endif
}

void settings_print( void ) {
    SETTINGS_APP_PRINT();
}
