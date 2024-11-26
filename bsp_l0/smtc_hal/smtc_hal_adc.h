#ifndef __SMTC_HAL_ADC_H__
#define __SMTC_HAL_ADC_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */

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


/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS PROTOTYPES ---------------------------------------------
 */

/*!
 * @brief   Initializes the MCU ADC peripheral
 */
void hal_adc_init( void );

/*!
 * @brief   Deinitialize the MCU ADC peripheral
 */
void hal_adc_deinit( void );

/*!
 * @brief   Measure the vrefint value
 *
 * @param   none
 * @return  the vrefint value in mv
 */
uint16_t hal_adc_get_vref_int( void );

/*!
 * @brief   Measure the internal temperature sensor
 *
 * @param   none
 * @return  the internal temperature in °C
 */
int8_t hal_adc_get_temp( void );

/**
 * @brief   Measure channel
 *
 * @return  uint16_t channel in mv
 */
uint16_t hal_adc_get_channel( uint32_t channel );


#ifdef __cplusplus
}
#endif

#endif  // __SMTC_HAL_ADC_H__

/* --- EOF ------------------------------------------------------------------ */