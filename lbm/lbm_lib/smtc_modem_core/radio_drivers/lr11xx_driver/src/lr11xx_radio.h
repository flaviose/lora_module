/*!
 * @file      lr11xx_radio.h
 *
 * @brief     Radio driver definition for LR11XX
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

#ifndef LR11XX_RADIO_H
#define LR11XX_RADIO_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */

#include "lr11xx_radio_types.h"
#include "lr11xx_types.h"

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC MACROS -----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC CONSTANTS --------------------------------------------------------
 */

/*!
 * @brief Length in byte of the GFSK sync word
 */
#define LR11XX_RADIO_GFSK_SYNC_WORD_LENGTH 8

/*!
 * @brief Default GFSK sync word value
 */
#define LR11XX_RADIO_GFSK_SYNC_WORD_DEFAULT            \
    {                                                  \
        0x97, 0x23, 0x52, 0x25, 0x56, 0x53, 0x65, 0x64 \
    }

/*!
 * @brief Length in byte of the LR-FHSS sync word
 */
#define LR11XX_RADIO_LR_FHSS_SYNC_WORD_LENGTH ( 4 )

/*!
 * @brief Ramp-up delay for the power amplifier in Sigfox context
 *
 * This parameter configures the delay to fine tune the ramp-up time of the power amplifier for BPSK operation.
 */
enum
{
    LR11XX_RADIO_SIGFOX_DBPSK_RAMP_UP_TIME_DEFAULT = 0x0000,  //!< No optimization
    LR11XX_RADIO_SIGFOX_DBPSK_RAMP_UP_TIME_100_BPS = 0x1306,  //!< Ramp-up optimization for 100bps
    LR11XX_RADIO_SIGFOX_DBPSK_RAMP_UP_TIME_600_BPS = 0x0325,  //!< Ramp-up optimization for 600bps
};

/*!
 * @brief Ramp-down delay for the power amplifier in Sigfox context
 *
 * This parameter configures the delay to fine tune the ramp-down time of the power amplifier for BPSK operation.
 */
enum
{
    LR11XX_RADIO_SIGFOX_DBPSK_RAMP_DOWN_TIME_DEFAULT = 0x0000,  //!< No optimization
    LR11XX_RADIO_SIGFOX_DBPSK_RAMP_DOWN_TIME_100_BPS = 0x1D70,  //!< Ramp-down optimization for 100bps
    LR11XX_RADIO_SIGFOX_DBPSK_RAMP_DOWN_TIME_600_BPS = 0x04E1,  //!< Ramp-down optimization for 600bps
};

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC TYPES ------------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS PROTOTYPES ---------------------------------------------
 */

/*!
 * @brief Reset internal statistics of the received packets
 *
 * @param [in] context Chip implementation context
 *
 * @returns Operation status
 *
 * @see lr11xx_radio_get_gfsk_stats, lr11xx_radio_get_lora_stats
 */
lr11xx_status_t lr11xx_radio_reset_stats( const void* context );

/*!
 * @brief Get the internal statistics of the GFSK received packets
 *
 * Internal statistics are reset on Power on Reset, by entering sleep mode without memory retention, or by calling @ref
 * lr11xx_radio_reset_stats.
 *
 * @param [in] context Chip implementation context
 * @param [out] stats The statistics structure of the received packets
 *
 * @returns Operation status
 *
 * @see lr11xx_radio_reset_stats
 */
lr11xx_status_t lr11xx_radio_get_gfsk_stats( const void* context, lr11xx_radio_stats_gfsk_t* stats );

/*!
 * @brief Get the internal statistics of the LoRa received packets
 *
 * Internal statistics are reset on Power on Reset, by entering sleep mode without memory retention, or by calling @ref
 * lr11xx_radio_reset_stats.
 *
 * @param [in] context Chip implementation context
 * @param [out] stats The statistics structure of the received packets
 *
 * @returns Operation status
 *
 * @see lr11xx_radio_reset_stats
 */
lr11xx_status_t lr11xx_radio_get_lora_stats( const void* context, lr11xx_radio_stats_lora_t* stats );

/*!
 * @brief Get the packet type currently configured
 *
 * @param [in] context Chip implementation context
 * @param [out] pkt_type The packet type currently configured
 *
 * @returns Operation status
 *
 * @see lr11xx_radio_set_pkt_type
 */
lr11xx_status_t lr11xx_radio_get_pkt_type( const void* context, lr11xx_radio_pkt_type_t* pkt_type );

/*!
 * @brief Get the length of last received packet, and the offset in the RX internal buffer of the first byte of the
 * received payload
 *
 * @param [in] context Chip implementation context
 * @param [out] rx_buffer_status The structure of RX buffer status
 *
 * @returns Operation status
 */
lr11xx_status_t lr11xx_radio_get_rx_buffer_status( const void*                      context,
                                                   lr11xx_radio_rx_buffer_status_t* rx_buffer_status );

/*!
 * @brief Get the status of last GFSK received packet
 *
 * The value depends on the received packet type
 *
 * @param [in] context Chip implementation context
 * @param [out] pkt_status The last received packet status
 *
 * @returns Operation status
 */
lr11xx_status_t lr11xx_radio_get_gfsk_pkt_status( const void* context, lr11xx_radio_pkt_status_gfsk_t* pkt_status );

/*!
 * @brief Get the status of last LoRa received packet
 *
 * The value depends on the received packet type
 *
 * @param [in] context Chip implementation context
 * @param [out] pkt_status The last received packet status
 *
 * @returns Operation status
 */
lr11xx_status_t lr11xx_radio_get_lora_pkt_status( const void* context, lr11xx_radio_pkt_status_lora_t* pkt_status );

/*!
 * @brief Get the instantaneous RSSI.
 *
 * This command can be used during reception of a packet
 *
 * @param [in] context Chip implementation context
 * @param [out] rssi_in_dbm Instantaneous RSSI.
 *
 * @returns Operation status
 */
lr11xx_status_t lr11xx_radio_get_rssi_inst( const void* context, int8_t* rssi_in_dbm );

/*!
 * @brief Set the GFSK modem sync word
 *
 * This command is used to set the GFSK nodem sync word. This command expects a 8-byte long array to be passed as sync
 * word parameter. By default, the value is 0x9723522556536564.
 *
 * @param [in] context Chip implementation context
 * @param [in] gfsk_sync_word The sync word to be configured
 *
 * @returns Operation status
 */
lr11xx_status_t lr11xx_radio_set_gfsk_sync_word( const void*   context,
                                                 const uint8_t gfsk_sync_word[LR11XX_RADIO_GFSK_SYNC_WORD_LENGTH] );

/*!
 * @brief Set the LoRa modem sync word
 *
 * @param [in] context Chip implementation context
 * @param [in] sync_word The sync word to be configured
 *
 * @returns Operation status
 */
lr11xx_status_t lr11xx_radio_set_lora_sync_word( const void* context, const uint8_t sync_word );

/*!
 * @brief Set the syncword for LR-FHSS
 *
 * Default value: 0x2C0F7995
 *
 * @param [in] context Chip implementation context
 * @param [in] sync_word The syncword to set. It is up to the caller to ensure this array is at least four bytes long
 *
 * @returns Operation status
 */
lr11xx_status_t lr11xx_radio_set_lr_fhss_sync_word( const void*   context,
                                                    const uint8_t sync_word[LR11XX_RADIO_LR_FHSS_SYNC_WORD_LENGTH] );

/*!
 * @brief Set the LoRa modem sync word to private / public
 *
 * This command is used to select which LoRa network is selected
 *
 * @param [in] context Chip implementation context
 * @param [in] network_type The network type to be configured
 *
 * @returns Operation status
 *
 * @warning This function is deprecated. Use lr11xx_radio_set_lora_sync_word for chip firmware equal to or more recent
 * than 0x303.
 */
lr11xx_status_t lr11xx_radio_set_lora_public_network( const void*                            context,
                                                      const lr11xx_radio_lora_network_type_t network_type );

/*!
 * @brief Start RX operations with a timeout in millisecond
 *
 * This command sets the LR11XX to RX mode. The radio must have been configured before using this command with @ref
 * lr11xx_radio_set_pkt_type
 *
 * By default, the timeout parameter allows to return automatically to standby RC mode if no packets have been received
 * after a certain amount of time. This behavior can be altered by @ref lr11xx_radio_set_rx_tx_fallback_mode and @ref
 * lr11xx_radio_auto_tx_rx.
 *
 * @remark To set the radio in Rx continuous mode, the function @ref lr11xx_radio_set_rx_with_timeout_in_rtc_step has to
 * be called with \p timeout_in_rtc_step set to 0xFFFFFF
 *
 * @param [in] context Chip implementation context
 * @param [in] timeout_in_ms The timeout configuration for RX operation
 *
 * @returns Operation status
 *
 * @see lr11xx_radio_set_pkt_type, lr11xx_radio_set_rx_tx_fallback_mode
 */
lr11xx_status_t lr11xx_radio_set_rx( const void* context, const uint32_t timeout_in_ms );

/*!
 * @brief Start RX operations with a timeout in RTC step
 *
 * This command sets the LR11XX to RX mode. The radio must have been configured before using this command with @ref
 * lr11xx_radio_set_pkt_type
 *
 * By default, the timeout parameter allows to return automatically to standby RC mode if no packets have been received
 * after a certain amount of time. This behavior can be altered by @ref lr11xx_radio_set_rx_tx_fallback_mode and @ref
 * lr11xx_radio_auto_tx_rx.
 *
 * The timeout duration is obtained by:
 * \f$ timeout\_duration\_ms = timeout \times \frac{1}{32.768} \f$
 *
 * Maximal timeout value is 0xFFFFFF, which gives a maximal timeout of 511 seconds.
 *
 * The timeout argument can also have the following special values:
 * <table>
 * <tr><th> Special values </th><th> Meaning </th>
 * <tr><td> 0x000000 </td><td> RX single: LR11XX stays in RX mode until a
 * packet is received, then switch to standby RC mode <tr><td> 0xFFFFFF
 * </td><td> RX continuous: LR11XX stays in RX mode even after reception of a
 * packet
 * </table>
 *
 * @param [in] context Chip implementation context
 * @param [in] timeout_in_rtc_step The timeout configuration for RX operation
 *
 * @returns Operation status
 *
 * @see lr11xx_radio_set_pkt_type, lr11xx_radio_set_rx_tx_fallback_mode
 */
lr11xx_status_t lr11xx_radio_set_rx_with_timeout_in_rtc_step( const void* context, const uint32_t timeout_in_rtc_step );

/*!
 * @brief Start TX operations
 *
 * This command sets the LR11XX to TX mode. The radio must have been configured before using this command with @ref
 * lr11xx_radio_set_pkt_type
 *
 * By default, the timeout parameter allows to return automatically to standby RC mode if the packet has not been
 * completely transmitted after a certain amount of time. This behavior can be altered by @ref
 * lr11xx_radio_set_rx_tx_fallback_mode and @ref lr11xx_radio_auto_tx_rx.
 *
 * @param [in] context Chip implementation context
 * @param [in] timeout_in_ms The timeout configuration for TX operation
 *
 * @returns Operation status
 *
 * @see lr11xx_radio_set_pkt_type, lr11xx_radio_set_rx_tx_fallback_mode
 */
lr11xx_status_t lr11xx_radio_set_tx( const void* context, const uint32_t timeout_in_ms );

/*!
 * @brief Start TX operations
 *
 * This command sets the LR11XX to TX mode. The radio must have been configured before using this command with @ref
 * lr11xx_radio_set_pkt_type
 *
 * By default, the timeout parameter allows to return automatically to standby RC mode if the packet has not been
 * completely transmitted after a certain amount of time. This behavior can be altered by @ref
 * lr11xx_radio_set_rx_tx_fallback_mode and @ref lr11xx_radio_auto_tx_rx.
 *
 * The timeout duration is obtained by:
 * \f$ timeout\_duration\_ms = timeout \times \frac{1}{32.768} \f$
 *
 * Maximal value is 0xFFFFFF.
 *
 * If the timeout argument is 0, then no timeout is used.
 *
 * @param [in] context Chip implementation context
 * @param [in] timeout_in_rtc_step The timeout configuration for TX operation
 *
 * @returns Operation status
 *
 * @see lr11xx_radio_set_pkt_type, lr11xx_radio_set_rx_tx_fallback_mode
 */
lr11xx_status_t lr11xx_radio_set_tx_with_timeout_in_rtc_step( const void* context, const uint32_t timeout_in_rtc_step );

/*!
 * @brief Set the frequency for future radio operations.
 *
 * This commands does not set frequency for Wi-Fi and GNSS scan operations.
 *
 * @param [in] context Chip implementation context
 * @param [in] freq_in_hz The frequency in Hz to set for radio operations
 *
 * @returns Operation status
 */
lr11xx_status_t lr11xx_radio_set_rf_freq( const void* context, const uint32_t freq_in_hz );

/*!
 * @brief Configure automatic TX after RX or automatic RX after TX
 *
 * After issuing this command, using the command @ref SetTx will make the LR11XX doing the following:
 *   - Enter TX mode as usual
 *   - Enter configurable Intermediary mode during configurable delay
 *   - Enter RX mode
 *
 * Similarly, after a @ref SetRx command, the LR11XX will do the following:
 *   - Enter RX mode as usual
 *   - Enter configurable Intermediary mode during configurable delay
 *   - Enter TX mode
 *
 * In case delay is 0, the LR11XX does not enter Intermediary mode and directly enter the following mode.
 *
 * To disable this behavior, use this function with delay set to 0xFFFFFFFF.
 *
 * @param [in] context Chip implementation context
 * @param [in] delay Time to spend in Intermediary mode expressed as steps of \f$\frac{1}{32.768 KHz}\f$ steps.
 * @param [in] intermediary_mode The mode the LR11XX enters after first mode completion during delay time
 * @param [in] timeout The timeout duration of the automatic RX or TX, expressed as steps of \f$ \frac{1}{32.768KHz} \f$
 *
 * @returns Operation status
 */
lr11xx_status_t lr11xx_radio_auto_tx_rx( const void* context, const uint32_t delay,
                                         const lr11xx_radio_intermediary_mode_t intermediary_mode,
                                         const uint32_t                         timeout );

/*!
 * @brief Set Channel Activity Detection configuration
 *
 * @param [in] context Chip implementation context
 * @param [in] cad_params The structure defining CAD configuration
 *
 * @returns Operation status
 */
lr11xx_status_t lr11xx_radio_set_cad_params( const void* context, const lr11xx_radio_cad_params_t* cad_params );

/*!
 * @brief Set the packet type
 *
 * @param [in] context Chip implementation context
 * @param [in] pkt_type Packet type to set
 *
 * @returns Operation status
 *
 * @see lr11xx_radio_get_pkt_type
 */
lr11xx_status_t lr11xx_radio_set_pkt_type( const void* context, const lr11xx_radio_pkt_type_t pkt_type );

/*!
 * @brief Set the modulation parameters for GFSK packets
 *
 * The command @ref lr11xx_radio_set_pkt_type must be called prior this one.
 *
 * @param [in] context Chip implementation context
 * @param [in] mod_params The structure of modulation configuration
 *
 * @returns Operation status
 *
 * @see lr11xx_radio_set_pkt_type
 */
lr11xx_status_t lr11xx_radio_set_gfsk_mod_params( const void*                           context,
                                                  const lr11xx_radio_mod_params_gfsk_t* mod_params );

/*!
 * @brief Set the modulation parameters for BPSK packets
 *
 * The command @ref lr11xx_radio_set_pkt_type must be called prior this one.
 *
 * @param [in] context Chip implementation context
 * @param [in] mod_params The structure of modulation configuration
 *
 * @returns Operation status
 *
 * @see lr11xx_radio_set_pkt_type
 */
lr11xx_status_t lr11xx_radio_set_bpsk_mod_params( const void*                           context,
                                                  const lr11xx_radio_mod_params_bpsk_t* mod_params );

/*!
 * @brief Set the modulation parameters for LoRa packets
 *
 * The command @ref lr11xx_radio_set_pkt_type must be called prior this one.
 *
 * @param [in] context Chip implementation context
 * @param [in] mod_params The structure of modulation configuration
 *
 * @returns Operation status
 *
 * @see lr11xx_radio_set_pkt_type
 */
lr11xx_status_t lr11xx_radio_set_lora_mod_params( const void*                           context,
                                                  const lr11xx_radio_mod_params_lora_t* mod_params );

/*!
 * @brief Set the modulation parameters for LR-FHSS
 *
 * The command @ref lr11xx_radio_set_pkt_type must be called prior this one.
 *
 * @param [in] context Chip implementation context
 * @param [in] mod_params The structure of modulation configuration
 *
 * @returns Operation status
 *
 * @see lr11xx_lr_fhss_set_pkt_type
 */
lr11xx_status_t lr11xx_radio_set_lr_fhss_mod_params( const void*                              context,
                                                     const lr11xx_radio_mod_params_lr_fhss_t* mod_params );

/*!
 * @brief Set the packet parameters for GFSK packets
 *
 * The command @ref lr11xx_radio_set_pkt_type must be called prior this one.
 *
 * @param [in] context Chip implementation context
 * @param [in] pkt_params The structure of packet configuration
 *
 * @returns Operation status
 *
 * @see lr11xx_radio_set_pkt_type, lr11xx_radio_set_gfsk_mod_params
 */
lr11xx_status_t lr11xx_radio_set_gfsk_pkt_params( const void*                           context,
                                                  const lr11xx_radio_pkt_params_gfsk_t* pkt_params );

/*!
 * @brief Set the packet parameters for BPSK packets
 *
 * The command @ref lr11xx_radio_set_pkt_type must be called prior this one.
 *
 * @param [in] context Chip implementation context
 * @param [in] pkt_params The structure of packet configuration
 *
 * @returns Operation status
 *
 * @see lr11xx_radio_set_pkt_type, lr11xx_radio_set_bpsk_mod_params
 */
lr11xx_status_t lr11xx_radio_set_bpsk_pkt_params( const void*                           context,
                                                  const lr11xx_radio_pkt_params_bpsk_t* pkt_params );

/*!
 * @brief Set the packet parameters for LoRa packets
 *
 * The command @ref lr11xx_radio_set_pkt_type must be called prior this one.
 *
 * @param [in] context Chip implementation context
 * @param [in] pkt_params The structure of packet configuration
 *
 * @returns Operation status
 *
 * @see lr11xx_radio_set_pkt_type, lr11xx_radio_set_lora_mod_params
 */
lr11xx_status_t lr11xx_radio_set_lora_pkt_params( const void*                           context,
                                                  const lr11xx_radio_pkt_params_lora_t* pkt_params );

/*!
 * @brief Set the parameters for TX power and power amplifier ramp time
 *
 * The command @ref lr11xx_radio_set_pa_cfg must be called prior calling
 * lr11xx_radio_set_tx_params.
 *
 * The range of possible TX output power values depends on PA selected with @ref lr11xx_radio_set_pa_cfg :
 *   - for LPA: power value goes from -17dBm to +14dBm (ie. from 0xEF to 0x0E)
 *   - for HPA: power value goes from -9dBm to +22dBm (ie. from 0xF7 to 0x16)
 *
 * Moreover, to use TX output power value higher than +10dBm, the @ref REGPASUPPLY_VBAT supply must have been selected
 * with @ref lr11xx_radio_set_pa_cfg.
 *
 * @param [in] context Chip implementation context
 * @param [in] pwr_in_dbm The TX output power in dBm
 * @param [in] ramp_time The ramping time configuration for the PA
 *
 * @returns Operation status
 */
lr11xx_status_t lr11xx_radio_set_tx_params( const void* context, const int8_t pwr_in_dbm,
                                            const lr11xx_radio_ramp_time_t ramp_time );

/*!
 * @brief Sets the Node and Broadcast address used for GFSK
 *
 * This setting is used only when filtering is enabled.
 *
 * @param [in] context Chip implementation context
 * @param [in] node_address The node address used as filter
 * @param [in] broadcast_address The broadcast address used as filter
 *
 * @returns Operation status
 */
lr11xx_status_t lr11xx_radio_set_pkt_address( const void* context, const uint8_t node_address,
                                              const uint8_t broadcast_address );

/*!
 * @brief Alter the chip mode after successfull transmission or reception operation
 *
 * This setting is not used during Rx Duty Cycle mode or Auto Tx Rx.
 *
 * @param [in] context Chip implementation context
 * @param [in] fallback_mode The chip mode to enter after successfull transmission or reception.
 *
 * @returns Operation status
 */
lr11xx_status_t lr11xx_radio_set_rx_tx_fallback_mode( const void*                         context,
                                                      const lr11xx_radio_fallback_modes_t fallback_mode );

/*!
 * @brief Configure and start a Rx Duty Cycle operation
 *
 * It executes the following steps:
 *     1. Reception: enters reception state for duration defined by rx_period
 *         - If mode is LR11XX_RADIO_RX_DUTY_CYCLE_MODE_RX: it is standard RX mode
 *         - If mode is LR11XX_RADIO_RX_DUTY_CYCLE_MODE_CAD (only in LoRa) : it is CAD operation
 *     2. Depending on the over-the-air activity detection:
 *         - In case of positive over-the-air detection, the rx_period timeout is recomputed to the value
 *           \f$2 \times rx\_period + sleep\_period\f$
 *         - If no air activity is detected, the LR11XX goes back to sleep mode with retention for a duration defined by
 * sleep_period
 *     3. On wake-up, the LR11XX restarts the process with the reception state.
 *
 * @remark If mode is configured to @ref LR11XX_RADIO_RX_DUTY_CYCLE_MODE_CAD, then the CAD configuration used in step 1.
 * is the one set from the last call to @ref lr11xx_radio_set_cad_params.
 *
 * @param [in] context Chip implementation context
 * @param [in] rx_period_in_ms The length of Rx period
 * @param [in] sleep_period_in_ms The length of sleep period
 * @param [in] mode The operation mode during Rx phase
 *
 * @returns Operation status
 *
 * @see lr11xx_radio_set_cad_params
 */
lr11xx_status_t lr11xx_radio_set_rx_duty_cycle( const void* context, const uint32_t rx_period_in_ms,
                                                const uint32_t                          sleep_period_in_ms,
                                                const lr11xx_radio_rx_duty_cycle_mode_t mode );

/*!
 * @brief Configure and start a Rx Duty Cycle operation
 *
 * It executes the following steps:
 *     1. Reception: enters reception state for duration defined by rx_period
 *         - If mode is LR11XX_RADIO_RX_DUTY_CYCLE_MODE_RX: it is standard RX mode
 *         - If mode is LR11XX_RADIO_RX_DUTY_CYCLE_MODE_CAD (only in LoRa) : it is CAD operation
 *     2. Depending on the over-the-air activity detection:
 *         - In case of positive over-the-air detection, the rx_period timeout is recomputed to the value
 *           \f$2 \times rx\_period + sleep\_period\f$
 *         - If no air activity is detected, the LR11XX goes back to sleep mode with retention for a duration defined by
 * sleep_period
 *     3. On wake-up, the LR11XX restarts the process with the reception state.
 *
 * @remark If mode is configured to @ref LR11XX_RADIO_RX_DUTY_CYCLE_MODE_CAD, then the CAD configuration used in step 1.
 * is the one set from the last call to @ref lr11xx_radio_set_cad_params.
 *
 * @param [in] context Chip implementation context
 * @param [in] rx_period_in_rtc_step The length of Rx period
 * @param [in] sleep_period_in_rtc_step The length of sleep period
 * @param [in] mode The operation mode during Rx phase
 *
 * @returns Operation status
 *
 * @see lr11xx_radio_set_cad_params
 */
lr11xx_status_t lr11xx_radio_set_rx_duty_cycle_with_timings_in_rtc_step( const void*    context,
                                                                         const uint32_t rx_period_in_rtc_step,
                                                                         const uint32_t sleep_period_in_rtc_step,
                                                                         const lr11xx_radio_rx_duty_cycle_mode_t mode );

/*!
 * @brief Set the Power Amplifier configuration
 *
 * It must be called prior using @ref lr11xx_radio_set_tx_params.
 *
 * @param [in] context Chip implementation context
 * @param [in] pa_cfg The structure for PA configuration
 *
 * @returns Operation status
 */
lr11xx_status_t lr11xx_radio_set_pa_cfg( const void* context, const lr11xx_radio_pa_cfg_t* pa_cfg );

/*!
 * @brief Define on which event the Rx timeout shall be stopped
 *
 * The two options are:
 *   - Syncword / Header detection
 *   - Preamble detection
 *
 * @param [in] context Chip implementation context
 * @param [in] stop_timeout_on_preamble The choice of the event to be taken into account
 *
 * @returns Operation status
 */
lr11xx_status_t lr11xx_radio_stop_timeout_on_preamble( const void* context, const bool stop_timeout_on_preamble );

/*!
 * @brief Start the CAD mode
 *
 * The LoRa packet type shall be selected before this function is called. The fallback mode is configured with
 * lr11xx_radio_set_cad_params.
 *
 * @param [in] context Chip implementation context
 *
 * @returns Operation status
 *
 * @see lr11xx_radio_set_cad_params, lr11xx_radio_set_pkt_type
 */
lr11xx_status_t lr11xx_radio_set_cad( const void* context );

/*!
 * @brief Set the device into Tx continuous wave (RF tone).
 *
 * A packet type shall be selected before this function is called.
 *
 * @param [in] context Chip implementation context
 *
 * @returns Operation status
 *
 * @see lr11xx_radio_set_pkt_type
 */
lr11xx_status_t lr11xx_radio_set_tx_cw( const void* context );

/*!
 * @brief Set the device into Tx continuous preamble (modulated signal).
 *
 * @param [in] context Chip implementation context
 *
 * @returns Operation status
 */
lr11xx_status_t lr11xx_radio_set_tx_infinite_preamble( const void* context );

/*!
 * @brief Configure the LoRa modem to issue a RX timeout after an exact number of symbols given in parameter if no LoRa
 * modulation is detected
 *
 * @warning Values of nb_symbol higher than 255 are only valid for chip firmware equal to or more recent than 0x308.
 *
 * @param [in] context Chip implementation context
 * @param [in] nb_symbol number of symbols to compute the timeout
 *
 * @returns Operation status
 */
lr11xx_status_t lr11xx_radio_set_lora_sync_timeout( const void* context, const uint16_t nb_symbol );

/*!
 * @brief Configure the LoRa modem to issue a RX timeout after an exact number of symbols given in parameter if no LoRa
 * modulation is detected
 *
 * @warning This command has been introduced in chip firware 0x0308 and is not available in earlier version
 *
 * @remark The number of symbol is computed as mantissa ^ (2*exponent + 1)
 *
 * @param [in] context Chip implementation context
 * @param [in] mantissa Mantissa - from 0 to 31 - to compute the number of symbols of the timeout
 * @param [in] exponent Exponent - from 0 to 7 - to compute the number of symbols of the timeout
 *
 * @returns Operation status
 */
lr11xx_status_t lr11xx_radio_set_lora_sync_timeout_with_mantissa_exponent( const void* context, const uint8_t mantissa,
                                                                           const uint8_t exponent );

/*!
 * @brief Configure the seed and the polynomial used to compute CRC in GFSK packet
 *
 * @param [in] context Chip implementation context
 * @param [in] seed Seed used to compute the CRC value
 * @param [in] polynomial Polynomial used to compute the CRC value
 *
 * @returns Operation status
 */
lr11xx_status_t lr11xx_radio_set_gfsk_crc_params( const void* context, const uint32_t seed, const uint32_t polynomial );

/*!
 * @brief Configure the whitening seed used in GFSK packet
 *
 * @param [in] context Chip implementation context
 * @param [in] seed Whitening seed value
 *
 * @returns Operation status
 */
lr11xx_status_t lr11xx_radio_set_gfsk_whitening_seed( const void* context, const uint16_t seed );

/*!
 * @brief Configure the boost mode in reception
 *
 * @param [in] context Chip implementation context
 * @param [in] enable_boost_mode Boost mode activation
 *
 * @returns Operation status
 */
lr11xx_status_t lr11xx_radio_cfg_rx_boosted( const void* context, const bool enable_boost_mode );

/*!
 * @brief Set RSSI calibration table
 *
 * @param [in] context Chip implementation context
 * @param [in] rssi_cal_table RSSI calibration table
 *
 * @returns Operation status
 */
lr11xx_status_t lr11xx_radio_set_rssi_calibration( const void*                                  context,
                                                   const lr11xx_radio_rssi_calibration_table_t* rssi_cal_table );

/*!
 * @brief Gets the radio bw parameter for a given bandwidth in Hz
 *
 * @param [in] bw_in_hz Requested GFSK Rx bandwidth
 * @param [out] bw_parameter Radio parameter immediately above requested bw_in_hz
 *
 * @returns Operation status
 */
lr11xx_status_t lr11xx_radio_get_gfsk_rx_bandwidth( uint32_t bw_in_hz, lr11xx_radio_gfsk_bw_t* bw_parameter );

/**
 * @brief Compute the numerator for LoRa time-on-air computation.
 *
 * @remark To get the actual time-on-air in seconds, this value has to be divided by the LoRa bandwidth in Hertz.
 *
 * @param [in] pkt_p Pointer to the structure holding the LoRa packet parameters
 * @param [in] mod_p Pointer to the structure holding the LoRa modulation parameters
 *
 * @returns LoRa time-on-air numerator
 */
uint32_t lr11xx_radio_get_lora_time_on_air_numerator( const lr11xx_radio_pkt_params_lora_t* pkt_p,
                                                      const lr11xx_radio_mod_params_lora_t* mod_p );

/**
 * @brief Get the actual value in Hertz of a given LoRa bandwidth
 *
 * @param [in] bw LoRa bandwidth parameter
 *
 * @returns Actual LoRa bandwidth in Hertz
 */
uint32_t lr11xx_radio_get_lora_bw_in_hz( lr11xx_radio_lora_bw_t bw );

/*!
 * @brief Get the time on air in ms for LoRa transmission
 *
 * @param [in] pkt_p Pointer to a structure holding the LoRa packet parameters
 * @param [in] mod_p Pointer to a structure holding the LoRa modulation parameters
 *
 * @returns Time-on-air value in ms for LoRa transmission
 */
uint32_t lr11xx_radio_get_lora_time_on_air_in_ms( const lr11xx_radio_pkt_params_lora_t* pkt_p,
                                                  const lr11xx_radio_mod_params_lora_t* mod_p );

/**
 * @brief Compute the numerator for GFSK time-on-air computation.
 *
 * @remark To get the actual time-on-air in seconds, this value has to be divided by the GFSK bitrate in bits per
 * second.
 *
 * @param [in] pkt_p Pointer to the structure holding the GFSK packet parameters
 *
 * @returns GFSK time-on-air numerator
 */
uint32_t lr11xx_radio_get_gfsk_time_on_air_numerator( const lr11xx_radio_pkt_params_gfsk_t* pkt_p );

/**
 * @brief Get the time on air in ms for GFSK transmission
 *
 * @param [in] pkt_p Pointer to a structure holding the GFSK packet parameters
 * @param [in] mod_p Pointer to a structure holding the GFSK modulation parameters
 *
 * @returns Time-on-air value in ms for GFSK transmission
 */
uint32_t lr11xx_radio_get_gfsk_time_on_air_in_ms( const lr11xx_radio_pkt_params_gfsk_t* pkt_p,
                                                  const lr11xx_radio_mod_params_gfsk_t* mod_p );

/**
 * @brief Get the number of RTC steps for a given time in millisecond
 *
 * @param [in] time_in_ms Timeout in millisecond
 *
 * @returns Number of RTC steps
 */
uint32_t lr11xx_radio_convert_time_in_ms_to_rtc_step( uint32_t time_in_ms );

/*!
 * @brief Configure the radio for Bluetooth® Low Energy Beaconing Compatibility.
 *
 * The caller shall ensure that the payload, if provided, follows the format of the Advertising physical channel PDU as
 * defined in the Bluetooth® core specification.
 *
 * This automatically configures the syncword to 0x8e89bed6 and the 3-byte CRC (polynomial set to 0x100065b, seed set to
 * 0x555555).
 *
 * @param [in] context Chip implementation context
 * @param [in] channel_id BLE channel - allowed channels are 37, 38, 39
 * @param [in] buffer Array of bytes to be used as beacon payload (optional)
 * @param [in] length Number of bytes in the @ref buffer array
 *
 * @returns Operation status
 *
 * @note As opposed to the function @ref lr11xx_radio_cfg_and_send_bluetooth_low_energy_beaconning_compatibility, this
 * function  only configures the radio interface for Bluetooth® Low Energy Beaconing Compatibility advertising. To
 * actually start the transmission, the function @ref lr11xx_radio_set_tx must be called.
 * @note The previously configured payload with @ref lr11xx_radio_cfg_bluetooth_low_energy_beaconning_compatibility or
 * @ref lr11xx_radio_cfg_and_send_bluetooth_low_energy_beaconning_compatibility is sent if @p length is set to 0, except
 * if a call to @ref lr11xx_regmem_write_buffer8, @ref lr11xx_lr_fhss_build_frame is done in between or @ref
 * lr11xx_system_set_sleep with warm start disabled
 *
 * @sa lr11xx_radio_cfg_and_send_bluetooth_low_energy_beaconning_compatibility
 * @sa lr11xx_radio_set_tx
 */
lr11xx_status_t lr11xx_radio_cfg_bluetooth_low_energy_beaconning_compatibility( const void*    context,
                                                                                const uint8_t  channel_id,
                                                                                const uint8_t* buffer,
                                                                                const uint8_t  length );

/**
 * @brief Get the information from the last received LoRa packet header (if @ref LR11XX_RADIO_LORA_PKT_EXPLICIT) or the
 * locally configured settings (if @ref LR11XX_RADIO_LORA_PKT_IMPLICIT)
 *
 * @remark This function can be called only if @ref LR11XX_RADIO_PKT_TYPE_LORA is selected with @ref
 * lr11xx_radio_set_pkt_type
 *
 * @param [in] context Chip implementation context
 * @param [out] is_crc_present  CRC configuration
 * @param [out] cr              LoRa coding rate
 *
 * @returns Operation status
 */
lr11xx_status_t lr11xx_radio_get_lora_rx_info( const void* context, bool* is_crc_present, lr11xx_radio_lora_cr_t* cr );

/*!
 * @brief Configure the radio for Bluetooth® Low Energy Beaconing Compatibility and send the given beacon on the desired
 * channel.
 *
 * The caller shall ensure that the payload, if provided, follows the format of the Advertising physical channel PDU as
 * defined in the Bluetooth® core specification.
 *
 * This automatically configures the syncword to 0x8e89bed6 and the 3-byte CRC (polynomial set to 0x100065b, seed set to
 * 0x555555).
 *
 * @param [in] context Chip implementation context
 * @param [in] channel_id BLE channel - allowed channels are 37, 38, 39
 * @param [in] buffer Array of bytes to be used as beacon payload (optional)
 * @param [in] length Number of bytes in the @ref buffer array
 *
 * @returns Operation status
 *
 * @note This function combines the configuration for Bluetooth® Low Energy Beaconing Compatibility - done with @ref
 * lr11xx_radio_cfg_bluetooth_low_energy_beaconning_compatibility) - and the actual transmission - done with @ref
 * lr11xx_radio_set_tx.
 * @note The previously configured payload with @ref lr11xx_radio_cfg_bluetooth_low_energy_beaconning_compatibility or
 * @ref lr11xx_radio_cfg_and_send_bluetooth_low_energy_beaconning_compatibility is sent if @p length is set to 0, except
 * if a call to @ref lr11xx_regmem_write_buffer8, @ref lr11xx_lr_fhss_build_frame is done in between or @ref
 * lr11xx_system_set_sleep with warm start disabled
 *
 * @sa lr11xx_radio_cfg_bluetooth_low_energy_beaconning_compatibility
 */
lr11xx_status_t lr11xx_radio_cfg_and_send_bluetooth_low_energy_beaconning_compatibility( const void*    context,
                                                                                         const uint8_t  channel_id,
                                                                                         const uint8_t* buffer,
                                                                                         const uint8_t  length );

/*!
 * @brief Apply the workaround for the high ACP limitation
 *
 * @param [in] context Chip implementation context
 *
 * @returns Operation status
 */
lr11xx_status_t lr11xx_radio_apply_high_acp_workaround( const void* context );

/**
 * @brief Get the mantissa and exponent for a given number of symbol
 *
 * @remark This function computes the [mantissa, exponent] duple which corresponds to nb_of_symb: the smallest
 * value verifying both following conditions:
 * - nb_of_symb >= nb_symbol; and
 * - nb_of_symb = mant * 2 ^ (2 * exp + 1)
 *
 * @param [in] nb_symbol Number of symbols
 * @param [out] mant Mantissa computed from nb_symb
 * @param [out] ext Exponent computed from nb_symb
 *
 * @returns Number of symbols corresponding to the [mantissa, exponent] duple computed with the following formula:
 * nb_of_symb = mant * 2 ^ (2 * exp + 1)
 */
uint16_t lr11xx_radio_convert_nb_symb_to_mant_exp( const uint16_t nb_symbol, uint8_t* mant, uint8_t* exp );

#ifdef __cplusplus
}
#endif

#endif  // LR11XX_RADIO_H

/* --- EOF ------------------------------------------------------------------ */
