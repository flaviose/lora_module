#ifndef __SMTC_HAL_I2C_H__
#define __SMTC_HAL_I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */

#include <stdint.h>   // C99 types
#include "smtc_hal_gpio_pin_names.h"

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC MACROS -----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC CONSTANTS --------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC TYPES ------------------------------------------------------------
 */

/*!
 * @brief I2C device's internal register address size
 */
typedef enum {
    I2C_ADDR_SIZE_8 = 0,
    I2C_ADDR_SIZE_16,
} i2c_addr_size;

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS PROTOTYPES ---------------------------------------------
 */

/*!
 * @brief  Initializes I2C1 peripheral
 */
void hal_i2c_init( void );

/*!
 * @brief  Deinitializes I2C1 peripheral
 */
void hal_i2c_deinit( void );

/*!
 * @brief  Set size to be used for internal registers of a i2c device
 * @param  addr_size This parameter can be one of the following values:
 *         @arg @ref I2C_ADDR_SIZE_8
 *         @arg @ref I2C_ADDR_SIZE_16
 */
void hal_i2c_set_addr_size( i2c_addr_size addr_size );

/*!
 * @brief  Write single byte
 * @param  device_addr  I2C address of the device
 * @param  addr         Register to write to
 * @param  data         Data to write
*/
uint8_t hal_i2c_write( const uint8_t device_addr, const uint16_t addr, const uint8_t data );

/*!
 * @brief  Read single byte
 * @param  device_addr  I2C address of the device
 * @param  addr         Register to write to
 * @param  data         Pointer to store the read data
*/
uint8_t hal_i2c_read( const uint8_t device_addr, const uint16_t addr, uint8_t* data );

/*!
 * @brief  Write multiple bytes
 * @param  device_addr  I2C address of the device
 * @param  addr         Register to write to
 * @param  buffer       Pointer to the data to write
 * @param  size         Number of bytes to write
*/
uint8_t hal_i2c_write_buffer( const uint8_t device_addr, const uint16_t addr, const uint8_t* buffer, const uint16_t size );

/*!
 * @brief  Read multiple bytes
 * @param  device_addr  I2C address of the device
 * @param  addr         Register to write to
 * @param  buffer       Pointer to store the read data
 * @param  size         Number of bytes to read
*/
uint8_t hal_i2c_read_buffer( const uint8_t device_addr, const uint16_t addr, uint8_t* buffer, const uint16_t size );

#ifdef __cplusplus
}
#endif

#endif  // __SMTC_HAL_I2C_H__

/* --- EOF ------------------------------------------------------------------ */