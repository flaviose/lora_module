/*!
 * \file      smtc_hal_spi.c
 *
 * \brief     SPI Hardware Abstraction Layer implementation
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2021. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Semtech corporation nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SEMTECH CORPORATION BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */

#include <stdint.h>   // C99 types
#include <stdbool.h>  // bool type

#include "smtc_hal_spi.h"
#include "stm32l0xx_ll_spi.h"
#include "stm32l0xx_ll_gpio.h"
#include "stm32l0xx_ll_bus.h"

#include "modem_pinout.h"

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

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

__attribute__( ( weak ) ) void hal_spi_init( const uint32_t id, const hal_gpio_pin_names_t mosi, const hal_gpio_pin_names_t miso,
        const hal_gpio_pin_names_t sclk ) {
    if( id == 1 ) {
        /* Enables GPIO clock and configures the SPI1 pins ********************/
        /* Enable the peripheral clock of GPIOB */
        LL_IOP_GRP1_EnableClock( LL_IOP_GRP1_PERIPH_GPIOB );

        /* Configure SCK Pin PB3 */
        LL_GPIO_SetPinMode( GPIOB, LL_GPIO_PIN_3, LL_GPIO_MODE_ALTERNATE );
        LL_GPIO_SetAFPin_0_7( GPIOB, LL_GPIO_PIN_3, LL_GPIO_AF_0 );
        LL_GPIO_SetPinSpeed( GPIOB, LL_GPIO_PIN_3, LL_GPIO_SPEED_FREQ_VERY_HIGH );
        LL_GPIO_SetPinPull( GPIOB, LL_GPIO_PIN_3, LL_GPIO_PULL_NO );

        /* Configure MISO Pin PB4 */
        LL_GPIO_SetPinMode( GPIOB, LL_GPIO_PIN_4, LL_GPIO_MODE_ALTERNATE );
        LL_GPIO_SetAFPin_0_7( GPIOB, LL_GPIO_PIN_4, LL_GPIO_AF_0 );
        LL_GPIO_SetPinSpeed( GPIOB, LL_GPIO_PIN_4, LL_GPIO_SPEED_FREQ_VERY_HIGH );
        LL_GPIO_SetPinPull( GPIOB, LL_GPIO_PIN_4, LL_GPIO_PULL_NO );

        /* Configure MOSI Pin PB5 */
        LL_GPIO_SetPinMode( GPIOB, LL_GPIO_PIN_5, LL_GPIO_MODE_ALTERNATE );
        LL_GPIO_SetAFPin_0_7( GPIOB, LL_GPIO_PIN_5, LL_GPIO_AF_0 );
        LL_GPIO_SetPinSpeed( GPIOB, LL_GPIO_PIN_5, LL_GPIO_SPEED_FREQ_VERY_HIGH );
        LL_GPIO_SetPinPull( GPIOB, LL_GPIO_PIN_5, LL_GPIO_PULL_NO );

        /* Configure SPI1 functional parameters ********************************/

        /* Enable the peripheral clock of SPI1 */
        LL_APB2_GRP1_EnableClock( LL_APB2_GRP1_PERIPH_SPI1 );

        /* Configure SPI1 communication */
        LL_SPI_SetMode( SPI1, LL_SPI_MODE_MASTER );
        LL_SPI_SetTransferDirection( SPI1, LL_SPI_FULL_DUPLEX );
        LL_SPI_SetDataWidth( SPI1, LL_SPI_DATAWIDTH_8BIT );
        LL_SPI_SetClockPolarity( SPI1, LL_SPI_POLARITY_LOW );
        LL_SPI_SetClockPhase( SPI1, LL_SPI_PHASE_1EDGE );
        LL_SPI_SetNSSMode( SPI1, LL_SPI_NSS_SOFT );
        LL_SPI_SetBaudRatePrescaler( SPI1, LL_SPI_BAUDRATEPRESCALER_DIV8 );
        LL_SPI_SetTransferBitOrder( SPI1, LL_SPI_MSB_FIRST );
        LL_SPI_DisableCRC( SPI1 );

        // Enable SPI
        LL_SPI_Enable( SPI1 );
    } else if( id == 2 ) {
        /* Enables GPIO clock and configures the SPI2 pins ********************/
        /* Enable the peripheral clock of GPIOB */
        LL_IOP_GRP1_EnableClock( LL_IOP_GRP1_PERIPH_GPIOB );

        /* Configure SCK Pin PB10 */
        LL_GPIO_SetPinMode( GPIOB, LL_GPIO_PIN_10, LL_GPIO_MODE_ALTERNATE );
        LL_GPIO_SetAFPin_8_15( GPIOB, LL_GPIO_PIN_10, LL_GPIO_AF_5 );
        LL_GPIO_SetPinSpeed( GPIOB, LL_GPIO_PIN_10, LL_GPIO_SPEED_FREQ_VERY_HIGH );
        LL_GPIO_SetPinPull( GPIOB, LL_GPIO_PIN_10, LL_GPIO_PULL_NO );

        /* Configure MISO Pin PB14 */
        LL_GPIO_SetPinMode( GPIOB, LL_GPIO_PIN_14, LL_GPIO_MODE_ALTERNATE );
        LL_GPIO_SetAFPin_8_15( GPIOB, LL_GPIO_PIN_14, LL_GPIO_AF_0 );
        LL_GPIO_SetPinSpeed( GPIOB, LL_GPIO_PIN_14, LL_GPIO_SPEED_FREQ_VERY_HIGH );
        LL_GPIO_SetPinPull( GPIOB, LL_GPIO_PIN_14, LL_GPIO_PULL_NO );

        /* Configure MOSI Pin PB15 */
        LL_GPIO_SetPinMode( GPIOB, LL_GPIO_PIN_15, LL_GPIO_MODE_ALTERNATE );
        LL_GPIO_SetAFPin_8_15( GPIOB, LL_GPIO_PIN_15, LL_GPIO_AF_0 );
        LL_GPIO_SetPinSpeed( GPIOB, LL_GPIO_PIN_15, LL_GPIO_SPEED_FREQ_VERY_HIGH );
        LL_GPIO_SetPinPull( GPIOB, LL_GPIO_PIN_15, LL_GPIO_PULL_NO );

        /* Configure SPI2 functional parameters ********************************/

        /* Enable the peripheral clock of SPI2 */
        LL_APB1_GRP1_EnableClock( LL_APB1_GRP1_PERIPH_SPI2 );

        /* Configure SPI2 communication */
        LL_SPI_SetMode( SPI2, LL_SPI_MODE_MASTER );
        LL_SPI_SetTransferDirection( SPI2, LL_SPI_FULL_DUPLEX );
        LL_SPI_SetDataWidth( SPI2, LL_SPI_DATAWIDTH_8BIT );
        LL_SPI_SetClockPolarity( SPI2, LL_SPI_POLARITY_LOW );
        LL_SPI_SetClockPhase( SPI2, LL_SPI_PHASE_1EDGE );
        LL_SPI_SetNSSMode( SPI2, LL_SPI_NSS_SOFT );
        LL_SPI_SetBaudRatePrescaler( SPI2, LL_SPI_BAUDRATEPRESCALER_DIV8 );
        LL_SPI_SetTransferBitOrder( SPI2, LL_SPI_MSB_FIRST );
        LL_SPI_DisableCRC( SPI2 );

        // Enable SPI
        LL_SPI_Enable( SPI2 );
    }
}

__attribute__( ( weak ) ) void hal_spi_de_init( const uint32_t id ) {
    if( id == 1 ) {
        // Disable SPI and its corresponding clock
        LL_SPI_Disable( SPI1 );
        LL_APB2_GRP1_DisableClock( LL_APB2_GRP1_PERIPH_SPI1 );

        // set sck, mosi and miso io in sleep state
        // Configure SCK Pin PB3 in input pull down
        LL_GPIO_SetPinMode( GPIOB, LL_GPIO_PIN_3, LL_GPIO_MODE_INPUT );
        LL_GPIO_SetPinSpeed( GPIOB, LL_GPIO_PIN_3, LL_GPIO_SPEED_LOW );
        LL_GPIO_SetPinPull( GPIOB, LL_GPIO_PIN_3, LL_GPIO_PULL_DOWN );

        // Configure MISO Pin PB4 in analog no pull
        LL_GPIO_SetPinMode( GPIOB, LL_GPIO_PIN_4, LL_GPIO_MODE_ANALOG );
        LL_GPIO_SetPinSpeed( GPIOB, LL_GPIO_PIN_4, LL_GPIO_SPEED_LOW );
        LL_GPIO_SetPinPull( GPIOB, LL_GPIO_PIN_4, LL_GPIO_PULL_NO );

        // Configure MOSI Pin PB5 in input pull down
        LL_GPIO_SetPinMode( GPIOB, LL_GPIO_PIN_5, LL_GPIO_MODE_INPUT );
        LL_GPIO_SetPinSpeed( GPIOB, LL_GPIO_PIN_5, LL_GPIO_SPEED_LOW );
        LL_GPIO_SetPinPull( GPIOB, LL_GPIO_PIN_5, LL_GPIO_PULL_DOWN );
    } else if( id == 2 ) {
        // Disable SPI and its corresponding clock
        LL_SPI_Disable( SPI2 );
        LL_APB1_GRP1_DisableClock( LL_APB1_GRP1_PERIPH_SPI2 );

        // set sck, mosi and miso io in sleep state
        // Configure SCK Pin PB10 in input pull down
        LL_GPIO_SetPinMode( GPIOB, LL_GPIO_PIN_10, LL_GPIO_MODE_INPUT );
        LL_GPIO_SetPinSpeed( GPIOB, LL_GPIO_PIN_10, LL_GPIO_SPEED_LOW );
        LL_GPIO_SetPinPull( GPIOB, LL_GPIO_PIN_10, LL_GPIO_PULL_DOWN );

        // Configure MISO Pin PB14 in analog no pull
        LL_GPIO_SetPinMode( GPIOB, LL_GPIO_PIN_14, LL_GPIO_MODE_ANALOG );
        LL_GPIO_SetPinSpeed( GPIOB, LL_GPIO_PIN_14, LL_GPIO_SPEED_LOW );
        LL_GPIO_SetPinPull( GPIOB, LL_GPIO_PIN_14, LL_GPIO_PULL_NO );

        // Configure MOSI Pin PB15 in input pull down
        LL_GPIO_SetPinMode( GPIOB, LL_GPIO_PIN_15, LL_GPIO_MODE_INPUT );
        LL_GPIO_SetPinSpeed( GPIOB, LL_GPIO_PIN_15, LL_GPIO_SPEED_LOW );
        LL_GPIO_SetPinPull( GPIOB, LL_GPIO_PIN_15, LL_GPIO_PULL_DOWN );
    }
}

__attribute__( ( weak ) ) uint16_t hal_spi_in_out( const uint32_t id, const uint16_t out_data ) {
    if( id == 1 ) {
        while( LL_SPI_IsActiveFlag_TXE( SPI1 ) == 0 ) {
        };

        LL_SPI_TransmitData8( SPI1, ( uint8_t )( out_data & 0xFF ) );

        while( LL_SPI_IsActiveFlag_RXNE( SPI1 ) == 0 ) {
        };

        return LL_SPI_ReceiveData8( SPI1 );
    } else if( id == 2 ) {
        while( LL_SPI_IsActiveFlag_TXE( SPI2 ) == 0 ) {
        };

        LL_SPI_TransmitData8( SPI2, ( uint8_t )( out_data & 0xFF ) );

        while( LL_SPI_IsActiveFlag_RXNE( SPI2 ) == 0 ) {
        };

        return LL_SPI_ReceiveData8( SPI2 );
    }

    return 0;
}
/* --- EOF ------------------------------------------------------------------ */
