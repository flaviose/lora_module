/*
 * lis2dw.h
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

#ifndef _lis2dw_H_
#define _lis2dw_H_

#include <stdint.h>
#include <stdbool.h>
#include "smtc_hal_gpio_pin_names.h"

#define lis2dw_SA0 0b1
#define lis2dw_ADDRESS ( ( 0b001100 << 2 ) | ( lis2dw_SA0 << 1 ) )
#define WHO_AM_I 0x0F
#define WHO_AM_I_ID 0b01000100
#define OUT_T_L 0x0D
#define OUT_T_H 0x0E
#define CTRL1_REG 0x20
#define CTRL2_REG 0x21
#define CTRL3_REG 0x22
#define CTRL4_INT1_PAD_CTRL 0x23
#define CTRL5_INT2_PAD_CTRL 0x24
#define CTRL6_REG 0x25
#define OUT_T 0x26
#define STATUS_REG 0x27
#define OUT_X_L 0x28
#define OUT_X_H 0x29
#define OUT_Y_L 0x2A
#define OUT_Y_H 0x2B
#define OUT_Z_L 0x2C
#define OUT_Z_H 0x2D
#define ACT_THS 0x3E
#define ACT_DUR 0x3F
#define FIFO_CTRL 0x2E
#define FIFO_SAMPLES 0x2F
#define TAP_THS_X 0x30
#define TAP_THS_Y 0x31
#define TAP_THS_Z 0x32
#define INT_DUR 0x33
#define WAKE_UP_THS 0x34
#define WAKE_UP_DUR 0x35
#define FREE_FALL 0x36
#define STATUS_DUP 0x37
#define WAKE_UP_SRC 0x38
#define TAP_SRC 0x39
#define SIXD_SRC 0x3A
#define ALL_INT_SRC 0x3B
#define X_OFS_USR 0x3C
#define Y_OFS_USR 0x3D
#define Z_OFS_USR 0x3E
#define CTRL7_REG 0x3F

#define LIS2DW_FIFO_SIZE 32

/**
 * @brief Initialize the LIS2DW accelerometer
 *
 * @return True if the initialization was successful, false otherwise
 */
bool lis2dw_init( void );

/**
 * @brief Configure the INT2 interrupt for sleep detection
 *        - INT2 is HIGH when LIS2DW12 is in sleep (no activity detected)
 *        - INT2 is LOW when activity is detected
 *
 * @param [in] port_pin
 * @param [in] threshold
 * @return True if the configuration was successful, false otherwise
 */
bool lis2dw_configure_int2_sleep( hal_gpio_pin_names_t port_pin, uint16_t threshold );

/**
 * @brief Configure the INT2 interrupt for data ready
 *        - INT2 pulses once when data is ready
 *
 * @param [in] port_pin
 * @return True if the configuration was successful, false otherwise
 */
bool lis2dw_configure_int2_drdy( hal_gpio_pin_names_t port_pin );

/**
 * @brief Configure the INT2 interrupt for FIFO threshold
 *       - INT2 pulses once when fifo_threshold is reached
 *
 * @param [in] port_pin
 * @param [in] fifo_threshold
 * @return True if the configuration was successful, false otherwise
 */
bool lis2dw_configure_int2_fifo( hal_gpio_pin_names_t port_pin, uint8_t fifo_threshold );

/**
 * @brief Set the threshold of the accelerometer
 *
 * @param [in] threshold
 * @return True if the configuration was successful, false otherwise
 */
bool lis2dw_set_threshold( uint16_t threshold );

/**
 * @brief Return true while activity is being detected, false if LIS2DW is in sleep mode
 */
bool lis2dw_activity_detected( void );

/**
 * @brief Return true if data is ready to be read out
*/
bool lis2dw_data_ready( void );

/**
 * @brief Clear the data ready flag
 */
void lis2dw_clear_data_ready( void );

/**
 * @brief Read out the accelerometer data
 *
 * @param [out] x X-axis acceleration data (unprocessed)
 * @param [out] y Y-axis acceleration data (unprocessed)
 * @param [out] z Z-axis acceleration data (unprocessed)
 */
bool lis2dw_read_accel( int16_t* x, int16_t* y, int16_t* z );

/**
 * @brief Read out the accelerometer data and convert to mg
 *
 * @param [out] x X-axis acceleration data (in mg)
 * @param [out] y Y-axis acceleration data (in mg)
 * @param [out] z Z-axis acceleration data (in mg)
 */
bool lis2dw_read_accel_mg( int16_t* x, int16_t* y, int16_t* z );

/**
 * @brief Reads the status register into the given variable
 *
 * @param [out] status
 */
bool lis2dw_get_status( uint8_t* status );

/**
 * @brief Reads the current FIFO level into the given variable
 *
 * @param [out] level
 */
bool lis2dw_get_fifo_level( uint8_t* level );

/**
 * @brief Print the content of all registers
 */
bool lis2dw_print_regs( void );

#endif

/* --- EOF ------------------------------------------------------------------ */