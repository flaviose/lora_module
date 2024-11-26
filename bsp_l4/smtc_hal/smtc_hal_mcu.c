/*!
 * \file      smtc_hal_mcu.c
 *
 * \brief     MCU Hardware Abstraction Layer implementation
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

#include "smtc_hal_mcu.h"
#include "modem_pinout.h"

#include "stm32l4xx_hal.h"
#include "stm32l4xx_ll_utils.h"

#include "smtc_hal_uart.h"
#include "smtc_hal_rtc.h"
#include "smtc_hal_spi.h"
#include "smtc_hal_lp_timer.h"
#include "smtc_hal_watchdog.h"

#if( MODEM_HAL_DBG_TRACE == MODEM_HAL_FEATURE_ON )
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#endif

#if defined( SX1272 ) || defined( SX1276 )
#include "sx127x_hal.h"
#endif

#include "timer.h"
#include "smtc_modem_utilities.h"

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS-----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */

// Low power is enabled (0 will disable it)
#define LOW_POWER_MODE 1
// 1 to enable debug with probe (ie do not de init pins)
#ifndef HW_DEBUG_PROBE
#define HW_DEBUG_PROBE 0
#endif

#define WATCHDOG_RELOAD_PERIOD_SECONDS 20
/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE TYPES -----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */

static TimerEvent_t wut;
static volatile bool wut_flag;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */
static void system_clock_config( void );
static void mcu_gpio_init( void );
static void lpm_enter_sleep_mode( void );
static void lpm_exit_sleep_mode( void );
static void sleep_handler( void );

#if( LOW_POWER_MODE == 1 )
static void lpm_mcu_deinit( void );
static void lpm_mcu_reinit( void );
static void wut_cb( void* context );
#endif

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

__attribute__( ( weak ) ) void hal_mcu_critical_section_begin( uint32_t* mask ) {
    *mask = __get_PRIMASK( );
    __disable_irq( );
}

__attribute__( ( weak ) ) void hal_mcu_critical_section_end( uint32_t* mask ) {
    __set_PRIMASK( *mask );
}

__attribute__( ( weak ) ) void hal_mcu_disable_irq( void ) {
    __disable_irq( );
}

__attribute__( ( weak ) ) void hal_mcu_enable_irq( void ) {
    __enable_irq( );
}

__attribute__( ( weak ) ) void hal_mcu_init( void ) {
    // Initialize MCU HAL library
    HAL_Init( );

    // Initialize clocks
    system_clock_config( );

    // Initialize watchdog
    hal_watchdog_init( );

#if( MODEM_HAL_DBG_TRACE == MODEM_HAL_FEATURE_ON )
    // Initialize Uart for debug traces
    trace_uart_init( );
#endif
#if defined( HW_MODEM_ENABLED )
    // Initialize Uart for hw commands
    hw_modem_uart_init( );
#endif

    // Initialize GPIOs
    mcu_gpio_init( );

    // Initialize Low Power Timer
    hal_lp_timer_init( HAL_LP_TIMER_ID_1 );

#if( SX127X )
    hal_lp_timer_init( HAL_LP_TIMER_ID_2 );
#endif

    // Initialize SPI for radio
    hal_spi_init( RADIO_SPI_ID, RADIO_SPI_MOSI, RADIO_SPI_MISO, RADIO_SPI_SCLK );

    // Initialize SPI for application
    hal_spi_init( APPLICATION_SPI_ID, APPLICATION_SPI_MOSI, APPLICATION_SPI_MISO, APPLICATION_SPI_SCLK );

    // Initialize RTC (for real time and wut)
    hal_rtc_init( );

    TimerInit( &wut, &wut_cb );
    wut_flag = false;
}

__attribute__( ( weak ) ) void hal_mcu_reset( void ) {
    __disable_irq( );
    NVIC_SystemReset( );  // Restart system
}

__attribute__( ( weak ) ) void __attribute__( ( optimize( "O0" ) ) ) hal_mcu_wait_us( const int32_t microseconds ) {
    // Work @80MHz
    const uint32_t nb_nop = microseconds * 1000 / 137;

    for( uint32_t i = 0; i < nb_nop; i++ ) {
        __NOP( );
    }
}

__attribute__( ( weak ) ) void hal_mcu_set_sleep_for_ms( const int32_t milliseconds ) {
    bool last_sleep_loop = false;
    int32_t time_counter = milliseconds;

    if( smtc_modem_is_irq_flag_pending( ) == false ) {
        hal_watchdog_reload( );

        if( milliseconds <= 0 ) {
            return;
        }

        do {
            if( ( time_counter > ( WATCHDOG_RELOAD_PERIOD_SECONDS * 1000 ) ) ) {
                time_counter -= WATCHDOG_RELOAD_PERIOD_SECONDS * 1000;
                TimerSetValue( &wut,  WATCHDOG_RELOAD_PERIOD_SECONDS * 1000 );
            } else {
                TimerSetValue( &wut, time_counter );
                // if the sleep time is less than the wdog reload period, this is the last sleep loop
                last_sleep_loop = true;
            }

            wut_flag = false;
            TimerStart( &wut );
            hal_mcu_disable_irq( );
            sleep_handler( );
            hal_mcu_enable_irq( );
            hal_watchdog_reload( );
        } while( ( wut_flag == true ) && ( last_sleep_loop == false ) );

        if( wut_flag == false ) {
            // in case sleep mode is interrupted by an other irq than the wake up timer, stop it and exit
            TimerStop( &wut );
        }
    }

    hal_watchdog_reload( );
}

static void wut_cb( void* context ) {
    wut_flag = true;
}

#ifdef USE_FULL_ASSERT
/*
 * Function Name  : assert_failed
 * Description    : Reports the name of the source file and the source line
 * number where the assert_param error has occurred. Input          : - file:
 * pointer to the source file name
 *                  - line: assert_param error line source number
 * Output         : None
 * Return         : None
 */
void assert_failed( uint8_t* file, uint32_t line ) {
    // User can add his own implementation to report the file name and line
    // number,
    // ex: printf("Wrong parameters value: file %s on line %lu\r\n", file, line)

    SMTC_HAL_TRACE_PRINTF( "Wrong parameters value: file %s on line %lu\r\n", ( const char* ) file, line );

    // Infinite loop
    while( 1 ) {
    }
}
#endif

void SysTick_Handler( void ) {
    HAL_IncTick( );
    HAL_SYSTICK_IRQHandler( );
}

void HAL_MspInit( void ) {
    // System interrupt init
    HAL_NVIC_SetPriority( MemoryManagement_IRQn, 0, 0 );
    HAL_NVIC_SetPriority( BusFault_IRQn, 0, 0 );
    HAL_NVIC_SetPriority( UsageFault_IRQn, 0, 0 );
    HAL_NVIC_SetPriority( SVCall_IRQn, 0, 0 );
    HAL_NVIC_SetPriority( DebugMonitor_IRQn, 0, 0 );
    HAL_NVIC_SetPriority( PendSV_IRQn, 0, 0 );
    HAL_NVIC_SetPriority( SysTick_IRQn, 0, 0 );
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

static void system_clock_config( void ) {
    // Configure the main internal regulator output voltage
    RCC_OscInitTypeDef       rcc_osc_init_struct = { 0 };
    RCC_ClkInitTypeDef       rcc_clk_init_struct = { 0 };
    RCC_PeriphCLKInitTypeDef periph_clk_init     = { 0 };

    // Set low drive on LSE to reduce conso
    HAL_PWR_EnableBkUpAccess( );
    __HAL_RCC_LSEDRIVE_CONFIG( RCC_LSEDRIVE_LOW );

    // HSI is enabled after System reset, activate PLL with HSI as source
    rcc_osc_init_struct.OscillatorType      = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
    rcc_osc_init_struct.LSEState            = RCC_LSE_ON;
    rcc_osc_init_struct.HSIState            = RCC_HSI_ON;
    rcc_osc_init_struct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    rcc_osc_init_struct.LSIState            = RCC_LSI_ON;
    rcc_osc_init_struct.PLL.PLLState        = RCC_PLL_ON;
    rcc_osc_init_struct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
    rcc_osc_init_struct.PLL.PLLM            = 1;
    rcc_osc_init_struct.PLL.PLLN            = 10;
    rcc_osc_init_struct.PLL.PLLP            = RCC_PLLP_DIV7;
    rcc_osc_init_struct.PLL.PLLQ            = RCC_PLLQ_DIV2;
    rcc_osc_init_struct.PLL.PLLR            = RCC_PLLR_DIV2;

    if( HAL_RCC_OscConfig( &rcc_osc_init_struct ) != HAL_OK ) {
        mcu_panic( );  // Initialization Error
    }

    // Initializes the CPU, AHB and APB busses clocks
    rcc_clk_init_struct.ClockType =
        RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    rcc_clk_init_struct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    rcc_clk_init_struct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    rcc_clk_init_struct.APB1CLKDivider = RCC_HCLK_DIV1;
    rcc_clk_init_struct.APB2CLKDivider = RCC_HCLK_DIV1;

    if( HAL_RCC_ClockConfig( &rcc_clk_init_struct, FLASH_LATENCY_4 ) != HAL_OK ) {
        mcu_panic( );  // Initialization Error
    }

    periph_clk_init.PeriphClockSelection =
        RCC_PERIPHCLK_RTC | RCC_PERIPHCLK_LPTIM1 | RCC_PERIPHCLK_LPTIM2 | RCC_PERIPHCLK_RNG | RCC_PERIPHCLK_ADC;
    periph_clk_init.RTCClockSelection       = RCC_RTCCLKSOURCE_LSE;
    periph_clk_init.Lptim1ClockSelection    = RCC_LPTIM1CLKSOURCE_LSE;
    periph_clk_init.Lptim2ClockSelection    = RCC_LPTIM2CLKSOURCE_LSE;
    periph_clk_init.AdcClockSelection       = RCC_ADCCLKSOURCE_SYSCLK;
    periph_clk_init.RngClockSelection       = RCC_RNGCLKSOURCE_PLLSAI1;
    periph_clk_init.PLLSAI1.PLLSAI1Source   = RCC_PLLSOURCE_HSI;
    periph_clk_init.PLLSAI1.PLLSAI1M        = 1;
    periph_clk_init.PLLSAI1.PLLSAI1N        = 12;
    periph_clk_init.PLLSAI1.PLLSAI1P        = RCC_PLLP_DIV7;
    periph_clk_init.PLLSAI1.PLLSAI1Q        = RCC_PLLQ_DIV4;
    periph_clk_init.PLLSAI1.PLLSAI1R        = RCC_PLLR_DIV2;
    periph_clk_init.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_48M2CLK;

#if( MODEM_HAL_DBG_TRACE == MODEM_HAL_FEATURE_ON )
    periph_clk_init.PeriphClockSelection |= RCC_PERIPHCLK_USART1;
    periph_clk_init.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
#endif
#if defined( HW_MODEM_ENABLED )
    periph_clk_init.PeriphClockSelection |= RCC_PERIPHCLK_UART4;
    periph_clk_init.Uart4ClockSelection = RCC_UART4CLKSOURCE_PCLK1;
#endif

    if( HAL_RCCEx_PeriphCLKConfig( &periph_clk_init ) != HAL_OK ) {
        mcu_panic( );  // Initialization Error
    }

    // Enable Power Clock
    __HAL_RCC_PWR_CLK_ENABLE( );

    // Ensure that HSI is wake-up system clock
    __HAL_RCC_WAKEUPSTOP_CLK_CONFIG( RCC_STOP_WAKEUPCLOCK_HSI );
}

static void mcu_gpio_init( void ) {
#if( HW_DEBUG_PROBE == 1 )
    // Enable debug in sleep/stop/standby
    HAL_DBGMCU_EnableDBGSleepMode( );
    HAL_DBGMCU_EnableDBGStopMode( );
    HAL_DBGMCU_EnableDBGStandbyMode( );
// #else
//     HAL_DBGMCU_DisableDBGSleepMode( );
//     HAL_DBGMCU_DisableDBGStopMode( );
//     HAL_DBGMCU_DisableDBGStandbyMode( );
#endif

    hal_gpio_init_out( RADIO_NSS, 1 );
#if defined( SX1272 ) || defined( SX1276 )
    hal_gpio_init_in( RADIO_DIO_0, BSP_GPIO_PULL_MODE_DOWN, BSP_GPIO_IRQ_MODE_RISING, NULL );
    hal_gpio_init_in( RADIO_DIO_1, BSP_GPIO_PULL_MODE_DOWN, BSP_GPIO_IRQ_MODE_RISING_FALLING, NULL );
    hal_gpio_init_in( RADIO_DIO_2, BSP_GPIO_PULL_MODE_DOWN, BSP_GPIO_IRQ_MODE_RISING, NULL );
    hal_gpio_init_in( RADIO_NRST, BSP_GPIO_PULL_MODE_NONE, BSP_GPIO_IRQ_MODE_OFF, NULL );
    hal_gpio_init_out( RADIO_ANTENNA_SWITCH, 0 );
#else
    hal_gpio_init_in( RADIO_BUSY_PIN, BSP_GPIO_PULL_MODE_NONE, BSP_GPIO_IRQ_MODE_OFF, NULL );
    // Here init only the pin as an exti rising and the callback will be attached later
    hal_gpio_init_in( RADIO_DIOX, BSP_GPIO_PULL_MODE_DOWN, BSP_GPIO_IRQ_MODE_RISING, NULL );
    hal_gpio_init_out( RADIO_NRST, 1 );
#if defined( SX128X )
    hal_gpio_init_out( SX128X_RADIO_TCXO_VCC, 0 );
#elif defined( LR11XX_TRANSCEIVER )
    hal_gpio_init_out( LR11XX_RADIO_RF_SWITCH_VCC, 0 );
#elif defined( SX126X )
    hal_gpio_init_out( SX126X_RADIO_RF_SWITCH_VCC, 0 );
#endif
#endif
}

/**
 * @brief Either enters Low Power Stop Mode or calls WFI
 *
 * @note ARM exits the function when waking up
 *
 */
static void lpm_enter_sleep_mode( void ) {
#if( LOW_POWER_MODE == 1 )
    // Deinit periph & enter Stop Mode
    lpm_mcu_deinit( );
#if defined( SX1272 ) || defined( SX1276 )

    if( sx127x_hal_timer_is_started( NULL ) == true ) {
        HAL_PWREx_EnterSTOP1Mode( PWR_STOPENTRY_WFI );
    } else
#endif
    {
        HAL_PWREx_EnterSTOP2Mode( PWR_STOPENTRY_WFI );
    }

#else
    __WFI( );
#endif
}

/**
 * @brief Wakes from Low Power Stop Mode or from WFI
 *
 */
static void lpm_exit_sleep_mode( void ) {
#if( LOW_POWER_MODE == 1 )
    // Initializes the peripherals
    lpm_mcu_reinit( );
#endif
}

/**
 * @brief Sleep handler
 *
 */
static void sleep_handler( void ) {
    // stop systick to avoid getting pending irq while going in stop mode
    // Systick is automatically restart when going out of sleep
    HAL_SuspendTick( );

    // If an interrupt has occurred after entering critical section, it is kept pending
    // and cortex will not enter low power anyway

    lpm_enter_sleep_mode( );
    lpm_exit_sleep_mode( );

    HAL_ResumeTick( );
}

#if( LOW_POWER_MODE == 1 )

/**
 * @brief De-init periph begore going in sleep mode
 *
 */
static void lpm_mcu_deinit( void ) {
    hal_spi_de_init( RADIO_SPI_ID );
    hal_spi_de_init( APPLICATION_SPI_ID );

#if defined( HW_MODEM_ENABLED )
    hw_modem_uart_deinit( );
#endif
#if( MODEM_HAL_DBG_TRACE == MODEM_HAL_FEATURE_ON )
    trace_uart_deinit( );
#endif
}

/**
 * @brief Re-init MCU clock after a wait in stop mode 2
 *
 */
static void lpm_mcu_reinit( void ) {
    RCC_ClkInitTypeDef rcc_clk_init_struct = { 0 };
    RCC_OscInitTypeDef rcc_osc_init_struct = { 0 };
    uint32_t           flash_latency       = 0;

    // Enable Power Control clock
    __HAL_RCC_PWR_CLK_ENABLE( );

    // Get the Oscillators configuration according to the internal RCC registers
    HAL_RCC_GetOscConfig( &rcc_osc_init_struct );

    // Enable PLL
    rcc_osc_init_struct.OscillatorType = RCC_OSCILLATORTYPE_NONE;
    rcc_osc_init_struct.PLL.PLLState   = RCC_PLL_ON;

    if( HAL_RCC_OscConfig( &rcc_osc_init_struct ) != HAL_OK ) {
        mcu_panic( );
    }

    // Get the Clocks configuration according to the internal RCC registers
    HAL_RCC_GetClockConfig( &rcc_clk_init_struct, &flash_latency );

    // Select PLL as system clock source and keep HCLK, PCLK1 and PCLK2 clocks dividers as before
    rcc_clk_init_struct.ClockType    = RCC_CLOCKTYPE_SYSCLK;
    rcc_clk_init_struct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;

    if( HAL_RCC_ClockConfig( &rcc_clk_init_struct, flash_latency ) != HAL_OK ) {
        mcu_panic( );
    }

    // Initialize UART
#if( MODEM_HAL_DBG_TRACE == MODEM_HAL_FEATURE_ON )
    trace_uart_init( );
#endif
#if defined( HW_MODEM_ENABLED )
    hw_modem_uart_init( );
#endif

    // Initialize SPI
    hal_spi_init( RADIO_SPI_ID, RADIO_SPI_MOSI, RADIO_SPI_MISO, RADIO_SPI_SCLK );
    hal_spi_init( APPLICATION_SPI_ID, APPLICATION_SPI_MOSI, APPLICATION_SPI_MISO, APPLICATION_SPI_SCLK );
}

#endif

/**
 * @brief This function handles Hard fault interrupt.
 */
void HardFault_Handler( void ) {
    SMTC_HAL_TRACE_ERROR( "\x1B[0;31m" );  // red color
    SMTC_HAL_TRACE_ERROR( "HARDFAULT_Handler\n" );
    SMTC_HAL_TRACE_ERROR( "\x1B[0m" );     // default color

    while( 1 ) {
    }
}

void hal_mcu_delay_ms( const uint32_t milliseconds ) {
    uint32_t start = hal_rtc_get_time_ms( );

    while( hal_rtc_get_time_ms( ) - start < milliseconds ) {
        /* code */
    }
}

/* --- EOF ------------------------------------------------------------------ */
