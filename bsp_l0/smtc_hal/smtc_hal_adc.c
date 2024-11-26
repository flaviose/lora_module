/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */
#include <stdint.h>   // C99 types
#include <stdbool.h>  // bool type

#include "smtc_hal_adc.h"

#include "stm32l0xx_ll_adc.h"
#include "stm32l0xx_ll_gpio.h"
#include "stm32l0xx_ll_bus.h"

// #include "smtc_hal_rtc.h" TODO add timeouts

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

/*!
 * @brief   Read channel from adc
 *
 * @param   channel channel to read ( see to @defgroup ADC_HAL_EC_CHANNEL)
 * @return  adc raw value
 */
static uint16_t adc_read( uint32_t channel );

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

__attribute__( ( weak ) ) void hal_adc_init( void ) {

    // Enable the peripheral clock for the ADC
    LL_APB2_GRP1_EnableClock( LL_APB2_GRP1_PERIPH_ADC1 );

    // ADC init
    LL_ADC_InitTypeDef adc_init = { 0 };
    LL_ADC_CommonInitTypeDef adc_common_init = { 0 };
    LL_ADC_REG_InitTypeDef adc_reg_init = { 0 };

    adc_init.Clock                  = LL_ADC_CLOCK_ASYNC;
    adc_init.Resolution             = LL_ADC_RESOLUTION_12B;
    adc_init.DataAlignment          = LL_ADC_DATA_ALIGN_RIGHT;
    adc_init.LowPowerMode           = LL_ADC_LP_MODE_NONE;

    adc_common_init.CommonClock     = LL_ADC_CLOCK_ASYNC_DIV12;

    adc_reg_init.TriggerSource      = LL_ADC_REG_TRIG_SOFTWARE;
    adc_reg_init.SequencerDiscont   = LL_ADC_REG_SEQ_DISCONT_DISABLE;
    adc_reg_init.ContinuousMode     = LL_ADC_REG_CONV_SINGLE;
    adc_reg_init.DMATransfer        = LL_ADC_REG_DMA_TRANSFER_NONE;
    adc_reg_init.Overrun            = LL_ADC_REG_OVR_DATA_OVERWRITTEN;

    LL_ADC_Init( ADC1, &adc_init );
    LL_ADC_CommonInit( __LL_ADC_COMMON_INSTANCE( ADC1 ), &adc_common_init );
    LL_ADC_REG_Init( ADC1, &adc_reg_init );

    // Enable internal channels for VREFINT and TEMPSENSOR
    LL_ADC_SetCommonPathInternalCh( __LL_ADC_COMMON_INSTANCE( ADC1 ), ( LL_ADC_PATH_INTERNAL_VREFINT | LL_ADC_PATH_INTERNAL_TEMPSENSOR ) );

    // Perform ADC calibration
    LL_ADC_StartCalibration( ADC1 );

    while( LL_ADC_IsCalibrationOnGoing( ADC1 ) ) {
        // Wait for calibration to complete
    }

    // Enable ADC
    LL_ADC_Enable( ADC1 );

    while( !LL_ADC_IsActiveFlag_ADRDY( ADC1 ) ) {
        // Wait until ADC is ready
    }
}

__attribute__( ( weak ) ) void hal_adc_deinit( void ) {

    // Disable the ADC
    if( LL_ADC_IsEnabled( ADC1 ) ) {
        LL_ADC_Disable( ADC1 );

        while( LL_ADC_IsEnabled( ADC1 ) ) {
            // Wait until ADC is fully disabled
        }
    }

    // Reset ADC configuration to default
    LL_ADC_DeInit( ADC1 );
    LL_ADC_CommonDeInit( __LL_ADC_COMMON_INSTANCE( ADC1 ) );

    // Disable the ADC clock
    LL_APB2_GRP1_DisableClock( LL_APB2_GRP1_PERIPH_ADC1 );
}

__attribute__( ( weak ) ) uint16_t hal_adc_get_vref_int( void ) {
    // Typical ts_vrefint = 10 us (datasheet)
    // ADC clock 1.33 Mhz (HSI16 / 12) --> 10 us = 13.33 cycles --> LL_ADC_SAMPLINGTIME_19CYCLES_5
    LL_ADC_SetSamplingTimeCommonChannels( ADC1, LL_ADC_SAMPLINGTIME_19CYCLES_5 );

    uint16_t adc_value = adc_read( LL_ADC_CHANNEL_VREFINT );

    return ( uint16_t )( __LL_ADC_CALC_VREFANALOG_VOLTAGE( adc_value, LL_ADC_RESOLUTION_12B ) );
}

__attribute__( ( weak ) ) int8_t hal_adc_get_temp( void ) {
    // Minimal ts_temp = 10 us (datasheet)
    // ADC clock 1.33 Mhz (HSI16 / 12) --> 10 us = 13.33 cycles --> LL_ADC_SAMPLINGTIME_19CYCLES_5
    LL_ADC_SetSamplingTimeCommonChannels( ADC1, LL_ADC_SAMPLINGTIME_19CYCLES_5 );

    // Update vref for more precise measure
    uint16_t vref_int_mv = hal_adc_get_vref_int( );

    uint16_t adc_val = adc_read( LL_ADC_CHANNEL_TEMPSENSOR );

    int32_t  temperature = __LL_ADC_CALC_TEMPERATURE( vref_int_mv, adc_val, LL_ADC_RESOLUTION_12B );

    return ( int8_t ) temperature;
}

__attribute__( ( weak ) ) uint16_t hal_adc_get_channel( uint32_t channel ) {
    // Update vref for more precise measure
    uint16_t vref_int_mv = hal_adc_get_vref_int( );

    uint16_t adc_val = adc_read( channel );
    return ( uint16_t )( __LL_ADC_CALC_DATA_TO_VOLTAGE( vref_int_mv, adc_val, LL_ADC_RESOLUTION_12B ) );
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

static uint16_t adc_read( uint32_t channel ) {

    // Ensure the ADC is enabled
    if( !LL_ADC_IsEnabled( ADC1 ) ) {
        LL_ADC_Enable( ADC1 );

        while( !LL_ADC_IsActiveFlag_ADRDY( ADC1 ) ) {
            // Wait until ADC is ready
        }
    }

    // Configure the desired channel
    LL_ADC_REG_SetSequencerChannels( ADC1, channel );

    // Start the ADC conversion
    LL_ADC_REG_StartConversion( ADC1 );

    // Wait for the conversion to complete
    while( !LL_ADC_IsActiveFlag_EOC( ADC1 ) ) {
        // Wait until the end of conversion
    }

    // Read the conversion result
    uint16_t adc_value = LL_ADC_REG_ReadConversionData12( ADC1 );

    // Clear the end of conversion flag (optional, as it may be auto-cleared)
    LL_ADC_ClearFlag_EOC( ADC1 );

    return adc_value;
}