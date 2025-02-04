/**
 * \file      region_au_915.c
 *
 * \brief     region_au_915 abstraction layer implementation
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

#include <string.h>  // memcpy
#include "lr1mac_utilities.h"
#include "smtc_modem_hal.h"
#include "region_au_915_defs.h"
#include "region_au_915.h"
#include "smtc_modem_hal_dbg_trace.h"

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS-----------------------------------------------------------
 */

#define real_ctx real->real_ctx
#define real_const real->real_const

#define dr_bitfield_tx_channel real->region.au915.dr_bitfield_tx_channel
#define channel_index_enabled real->region.au915.channel_index_enabled
#define dr_distribution_init real->region.au915.dr_distribution_init
#define dr_distribution real->region.au915.dr_distribution
#define join_dr_distribution real->region.au915.join_dr_distribution
#define custom_dr_distribution_init real->region.au915.custom_dr_distribution_init
#define unwrapped_channel_mask real->region.au915.unwrapped_channel_mask
#define first_ch_mask_received real->region.au915.first_ch_mask_received

#define snapshot_channel_tx_mask real->region.au915.snapshot_channel_tx_mask
#define snapshot_bank_tx_mask real->region.au915.snapshot_bank_tx_mask
#define tx_channel_idx real->region.au915.tx_channel_idx

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE TYPES -----------------------------------------------------------
 */
typedef enum ch_mask_after_join_e
{
    ch_mask_after_join_init = 0,
    ch_mask_after_join_8ch,   // just after join if no ChMask received
    ch_mask_after_join_56ch,  // just after join + first 8ch if no ChMask received
    ch_mask_after_join_full   // nominal way, ChMask received or not
} ch_mask_after_join_t;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

/**
 * @brief init Channel mask after the join accept
 *
 * @param real
 */
static void region_au_915_channel_mask_set_after_join( smtc_real_t* real );

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

void region_au_915_init( smtc_real_t* real )
{
    real_const.const_number_of_tx_channel         = NUMBER_OF_TX_CHANNEL_AU_915;
    real_const.const_number_of_rx_channel         = NUMBER_OF_RX_CHANNEL_AU_915;
    real_const.const_number_of_channel_bank       = BANK_MAX_AU915;
    real_const.const_join_accept_delay1           = JOIN_ACCEPT_DELAY1_AU_915;
    real_const.const_received_delay1              = RECEIVE_DELAY1_AU_915;
    real_const.const_tx_power_dbm                 = TX_POWER_EIRP_AU_915 - 2;  // EIRP to ERP
    real_const.const_max_tx_power_idx             = MAX_TX_POWER_IDX_AU_915;
    real_const.const_adr_ack_limit                = ADR_ACK_LIMIT_AU_915;
    real_const.const_adr_ack_delay                = ADR_ACK_DELAY_AU_915;
    real_const.const_datarate_offsets             = &datarate_offsets_au_915[0][0];
    real_const.const_datarate_backoff             = &datarate_backoff_au_915[0][0];
    real_const.const_ack_timeout                  = ACK_TIMEOUT_AU_915;
    real_const.const_freq_min                     = FREQMIN_AU_915;
    real_const.const_freq_max                     = FREQMAX_AU_915;
    real_const.const_rx2_freq                     = RX2_FREQ_AU_915;
    real_const.const_frequency_factor             = FREQUENCY_FACTOR_AU_915;
    real_const.const_rx2_dr_init                  = RX2DR_INIT_AU_915;
    real_const.const_sync_word_private            = SYNC_WORD_PRIVATE_AU_915;
    real_const.const_sync_word_public             = SYNC_WORD_PUBLIC_AU_915;
    real_const.const_sync_word_lr_fhss            = ( uint8_t* ) SYNC_WORD_LR_FHSS_AU_915;
    real_const.const_min_tx_dr                    = MIN_TX_DR_AU_915;
    real_const.const_max_tx_dr                    = MAX_TX_DR_AU_915;
    real_const.const_min_tx_dr_limit              = MIN_TX_DR_LIMIT_AU_915;
    real_const.const_number_of_tx_dr              = NUMBER_OF_TX_DR_AU_915;
    real_const.const_min_rx_dr                    = MIN_RX_DR_AU_915;
    real_const.const_max_rx_dr                    = MAX_RX_DR_AU_915;
    real_const.const_number_rx1_dr_offset         = NUMBER_RX1_DR_OFFSET_AU_915;
    real_const.const_dr_bitfield                  = DR_BITFIELD_SUPPORTED_AU_915;
    real_const.const_tx_param_setup_req_supported = TX_PARAM_SETUP_REQ_SUPPORTED_AU_915;
    real_const.const_new_channel_req_supported    = NEW_CHANNEL_REQ_SUPPORTED_AU_923;
    real_const.const_dtc_supported                = DTC_SUPPORTED_AU_915;
    real_const.const_lbt_supported                = LBT_SUPPORTED_AU_915;
    real_const.const_max_payload_m                = &M_au_915[0][0];
    real_const.const_coding_rate                  = RAL_LORA_CR_4_5;
    real_const.const_mobile_longrange_dr_distri   = &MOBILE_LONGRANGE_DR_DISTRIBUTION_AU_915[0];
    real_const.const_mobile_lowpower_dr_distri    = &MOBILE_LOWPER_DR_DISTRIBUTION_AU_915[0];
    real_const.const_default_dr_distri            = &DEFAULT_DR_DISTRIBUTION_AU_915[0];
    real_const.const_cf_list_type_supported       = CF_LIST_SUPPORTED_AU_915;
    real_const.const_beacon_dr                    = BEACON_DR_AU_915;
    real_const.const_uplink_dwell_time            = UPLINK_DWELL_TIME_AU_915;

    real_ctx.tx_frequency_channel_ctx        = NULL;
    real_ctx.rx1_frequency_channel_ctx       = NULL;
    real_ctx.channel_index_enabled_ctx       = &channel_index_enabled[0];
    real_ctx.unwrapped_channel_mask_ctx      = &unwrapped_channel_mask[0];
    real_ctx.dr_bitfield_tx_channel_ctx      = &dr_bitfield_tx_channel[0];
    real_ctx.dr_distribution_init_ctx        = &dr_distribution_init[0];
    real_ctx.dr_distribution_ctx             = &dr_distribution[0];
    real_ctx.join_dr_distribution_ctx        = &join_dr_distribution[0];
    real_ctx.custom_dr_distribution_init_ctx = &custom_dr_distribution_init[0];

    memcpy( join_dr_distribution, JOIN_DR_DISTRIBUTION_AU_915, real_const.const_number_of_tx_dr );
    memcpy( custom_dr_distribution_init, real_const.const_default_dr_distri, real_const.const_number_of_tx_dr );
    memset( dr_distribution_init, 0, real_const.const_number_of_tx_dr );
    memset( dr_distribution, 0, real_const.const_number_of_tx_dr );

    // Enable all channels
    memset( &unwrapped_channel_mask[0], 0xFF, BANK_MAX_AU915 );
    memset( &snapshot_channel_tx_mask[0], 0xFF, BANK_MAX_AU915 );

    snapshot_bank_tx_mask = 0;
}

void region_au_915_config( smtc_real_t* real )
{
    // Tx 125 kHz channels
    for( uint8_t i = 0; i < NUMBER_OF_TX_CHANNEL_AU_915 - 8; i++ )
    {
        SMTC_PUT_BIT8( channel_index_enabled, i, CHANNEL_ENABLED );

        // Enable default datarate
        dr_bitfield_tx_channel[i] = DEFAULT_TX_DR_125_BIT_FIELD_AU_915;

        SMTC_MODEM_HAL_TRACE_PRINTF_DEBUG( "TX - idx:%u, freq: %d, dr: 0x%x,\n%s", i,
                                           region_au_915_get_tx_frequency_channel( real, i ), dr_bitfield_tx_channel[i],
                                           ( ( i % 8 ) == 7 ) ? "---\n" : "" );
    }
    // Tx 500 kHz channels
    for( uint8_t i = NUMBER_OF_TX_CHANNEL_AU_915 - 8; i < NUMBER_OF_TX_CHANNEL_AU_915; i++ )
    {
        SMTC_PUT_BIT8( channel_index_enabled, i, CHANNEL_ENABLED );
        // Enable default datarate
        dr_bitfield_tx_channel[i] = DEFAULT_TX_DR_500_BIT_FIELD_AU_915;

        SMTC_MODEM_HAL_TRACE_PRINTF_DEBUG( "TX - idx:%u, freq: %d, dr: 0x%x,\n%s", i,
                                           region_au_915_get_tx_frequency_channel( real, i ), dr_bitfield_tx_channel[i],
                                           ( ( i % 8 ) == 7 ) ? "---\n" : "" );
    }
#if MODEM_HAL_DBG_TRACE == MODEM_HAL_FEATURE_ON
    // Rx 500 kHz channels
    for( uint8_t i = 0; i < NUMBER_OF_RX_CHANNEL_AU_915; i++ )
    {
        SMTC_MODEM_HAL_TRACE_PRINTF_DEBUG( "RX - idx:%u, freq: %d, dr_min: %u, dr_max: %u\n%s", i,
                                           region_au_915_get_rx1_frequency_channel( real, i ), MIN_RX_DR_AU_915,
                                           MAX_RX_DR_AU_915, ( ( i % 8 ) == 7 ) ? "---\n" : "" );
    }
#endif

    first_ch_mask_received = ch_mask_after_join_init;
}

status_lorawan_t region_au_915_is_acceptable_tx_dr( smtc_real_t* real, uint8_t dr, bool is_ch_mask_from_link_adr,
                                                    bool uplink_dwell_time )
{
    status_lorawan_t status                      = ERRORLORAWAN;
    uint8_t          number_channels_125_enabled = 0;

    uint8_t* ch_mask_to_check = ( is_ch_mask_from_link_adr == true ) ? unwrapped_channel_mask : channel_index_enabled;

    // 125 kHz channels
    for( uint8_t i = 0; i < real_const.const_number_of_tx_channel - 8; i++ )
    {
        if( SMTC_GET_BIT8( ch_mask_to_check, i ) == CHANNEL_ENABLED )
        {
            if( SMTC_GET_BIT16( &dr_bitfield_tx_channel[i], dr ) == 1 )
            {
                status = OKLORAWAN;
                number_channels_125_enabled++;
            }
        }
    }
    // 500 kHz channels
    for( uint8_t i = real_const.const_number_of_tx_channel - 8; i < real_const.const_number_of_tx_channel; i++ )
    {
        if( SMTC_GET_BIT8( ch_mask_to_check, i ) == CHANNEL_ENABLED )
        {
            if( SMTC_GET_BIT16( &dr_bitfield_tx_channel[i], dr ) == 1 )
            {
                status = OKLORAWAN;
                break;
            }
        }
    }

    if( dr < MAX_TX_DR_LORA_AU_915 )
    {
        // FCC 15.247 paragraph F mandates to hop on at least 2 125 kHz channels
        if( number_channels_125_enabled < 2 )
        {
            status = ERRORLORAWAN;
        }
    }

    if( uplink_dwell_time == true )
    {
        if( dr < real_const.const_min_tx_dr_limit )
        {
            status = ERRORLORAWAN;
        }
    }

    if( status == ERRORLORAWAN )
    {
        SMTC_MODEM_HAL_TRACE_WARNING( "Not acceptable data rate\n" );
    }
    return ( status );
}

status_lorawan_t region_au_915_get_join_next_channel( smtc_real_t* real, uint8_t* out_tx_data_rate,
                                                      uint32_t* out_tx_frequency, uint32_t* out_rx1_frequency,
                                                      uint8_t* active_channel_nb )
{
    au_915_channels_bank_t bank_tmp_cnt = 0;
    uint8_t                active_channel_index[NUMBER_OF_TX_CHANNEL_AU_915];
    do
    {
        if( snapshot_bank_tx_mask > BANK_8_500_AU915 )
        {
            snapshot_bank_tx_mask = BANK_0_125_AU915;
        }

        // if all channels were used in a block, reset the snapshots block
        if( snapshot_channel_tx_mask[snapshot_bank_tx_mask] == 0 )
        {
            snapshot_channel_tx_mask[snapshot_bank_tx_mask] = channel_index_enabled[snapshot_bank_tx_mask];
        }

        *active_channel_nb = 0;
        for( uint8_t i = snapshot_bank_tx_mask * 8; i < ( ( snapshot_bank_tx_mask * 8 ) + 8 ); i++ )
        {
            if( ( SMTC_GET_BIT8( snapshot_channel_tx_mask, i ) == CHANNEL_ENABLED ) &&
                ( SMTC_GET_BIT8( channel_index_enabled, i ) == CHANNEL_ENABLED ) )
            {
                active_channel_index[*active_channel_nb] = i;
                ( *active_channel_nb )++;
            }
        }
        snapshot_bank_tx_mask++;
        bank_tmp_cnt++;
    } while( ( *active_channel_nb == 0 ) && ( bank_tmp_cnt < BANK_MAX_AU915 ) );

    if( *active_channel_nb == 0 )
    {
        SMTC_MODEM_HAL_TRACE_WARNING( "NO CHANNELS AVAILABLE \n" );
        return ERRORLORAWAN;
    }

    uint8_t temp        = 0xFF;
    uint8_t channel_idx = 0;
    if( snapshot_bank_tx_mask > BANK_8_500_AU915 )
    {
        // active_channel_index[0] contains the first available 500KHz channel
        channel_idx = active_channel_index[0];
    }
    else
    {
        temp        = ( smtc_modem_hal_get_random_nb_in_range( 0, ( *active_channel_nb - 1 ) ) ) % *active_channel_nb;
        channel_idx = active_channel_index[temp];
    }

    if( channel_idx >= NUMBER_OF_TX_CHANNEL_AU_915 )
    {
        SMTC_MODEM_HAL_TRACE_ERROR( "INVALID CHANNEL  active channel = %d and random channel = %d \n",
                                    *active_channel_nb, temp );
        return ERRORLORAWAN;
    }

    tx_channel_idx = channel_idx;

    if( snapshot_bank_tx_mask > BANK_8_500_AU915 )
    {
        *out_tx_data_rate = DR6;
    }
    else
    {
        *out_tx_data_rate = DR2;
    }

    *out_tx_frequency  = region_au_915_get_tx_frequency_channel( real, channel_idx );
    *out_rx1_frequency = region_au_915_get_rx1_frequency_channel( real, channel_idx );

#if( MODEM_HAL_DBG_TRACE == MODEM_HAL_FEATURE_ON ) && ( MODEM_HAL_DEEP_DBG_TRACE == MODEM_HAL_FEATURE_ON )
    SMTC_MODEM_HAL_TRACE_PRINTF_DEBUG( "snapshot channel 125 tx mask\n" );
    for( uint8_t i = 0; i < NUMBER_OF_TX_CHANNEL_AU_915 - 8; i++ )
    {
        uint8_t test = SMTC_GET_BIT8( snapshot_channel_tx_mask, i ) & SMTC_GET_BIT8( channel_index_enabled, i );
        SMTC_MODEM_HAL_TRACE_PRINTF_DEBUG( "%u%s", test, ( ( i % 8 ) == 7 ) ? " \n" : "" );
    }
    SMTC_MODEM_HAL_TRACE_PRINTF_DEBUG( "snapshot channel 500 tx mask\n" );
    for( uint8_t i = 0; i < 8; i++ )
    {
        uint8_t test = SMTC_GET_BIT8( &snapshot_channel_tx_mask[BANK_8_500_AU915], i ) &
                       SMTC_GET_BIT8( &channel_index_enabled[BANK_8_500_AU915], i );
        SMTC_MODEM_HAL_TRACE_PRINTF_DEBUG( "%u%s", test, ( ( i % 8 ) == 7 ) ? " \n" : "" );
    }
#endif

    return OKLORAWAN;
}

status_lorawan_t region_au_915_get_next_channel( smtc_real_t* real, uint8_t tx_data_rate, uint32_t* out_tx_frequency,
                                                 uint32_t* out_rx1_frequency, uint8_t* active_channel_nb )
{
    // if all channels were used, reset the snapshots
    if( ( SMTC_ARE_CLR_BYTE8( snapshot_channel_tx_mask, BANK_8_500_AU915 ) == true ) &&
        ( first_ch_mask_received == ch_mask_after_join_full ) )
    {
        for( au_915_channels_bank_t i = 0; i < BANK_8_500_AU915; i++ )
        {
            snapshot_channel_tx_mask[i] = channel_index_enabled[i];
        }
    }

    if( ( snapshot_channel_tx_mask[BANK_8_500_AU915] == 0 ) && ( first_ch_mask_received == ch_mask_after_join_full ) )
    {
        snapshot_channel_tx_mask[BANK_8_500_AU915] = channel_index_enabled[BANK_8_500_AU915];
    }

    if( ( ( SMTC_ARE_CLR_BYTE8( snapshot_channel_tx_mask, BANK_8_500_AU915 ) == true ) ||
          ( snapshot_channel_tx_mask[BANK_8_500_AU915] == 0 ) ) &&
        ( first_ch_mask_received == ch_mask_after_join_56ch ) )
    {
        memset( unwrapped_channel_mask, 0xFF, real_const.const_number_of_channel_bank );
        region_au_915_set_channel_mask( real );
    }

    if( ( ( SMTC_ARE_CLR_BYTE8( snapshot_channel_tx_mask, BANK_8_500_AU915 ) == true ) ||
          ( snapshot_channel_tx_mask[BANK_8_500_AU915] == 0 ) ) &&
        ( first_ch_mask_received <= ch_mask_after_join_8ch ) )
    {
        region_au_915_init_after_join_snapshot_channel_mask( real, tx_data_rate, *out_tx_frequency );
    }

    *active_channel_nb = 0;
    uint8_t active_channel_index[NUMBER_OF_TX_CHANNEL_AU_915];
    for( uint8_t i = 0; i < NUMBER_OF_TX_CHANNEL_AU_915; i++ )
    {
        if( ( SMTC_GET_BIT8( snapshot_channel_tx_mask, i ) == CHANNEL_ENABLED ) &&
            ( SMTC_GET_BIT8( channel_index_enabled, i ) == CHANNEL_ENABLED ) &&
            ( SMTC_GET_BIT16( &dr_bitfield_tx_channel[i], tx_data_rate ) == 1 ) )
        {
            active_channel_index[*active_channel_nb] = i;
            ( *active_channel_nb )++;
        }
    }
    if( *active_channel_nb == 0 )
    {
        SMTC_MODEM_HAL_PANIC( "NO CHANNELS AVAILABLE\n" );
    }

    uint8_t temp = ( smtc_modem_hal_get_random_nb_in_range( 0, ( *active_channel_nb - 1 ) ) ) % *active_channel_nb;
    uint8_t channel_idx = active_channel_index[temp];
    if( channel_idx >= NUMBER_OF_TX_CHANNEL_AU_915 )
    {
        SMTC_MODEM_HAL_TRACE_ERROR( "INVALID CHANNEL  active channel = %d and random channel = %d \n",
                                    *active_channel_nb, temp );
        return ERRORLORAWAN;
    }

    tx_channel_idx = channel_idx;

    *out_tx_frequency  = region_au_915_get_tx_frequency_channel( real, channel_idx );
    *out_rx1_frequency = region_au_915_get_rx1_frequency_channel( real, channel_idx );

#if( MODEM_HAL_DBG_TRACE == MODEM_HAL_FEATURE_ON ) && ( MODEM_HAL_DEEP_DBG_TRACE == MODEM_HAL_FEATURE_ON )
    SMTC_MODEM_HAL_TRACE_PRINTF_DEBUG( "snapshot channel 125 tx mask\n" );
    for( uint8_t i = 0; i < NUMBER_OF_TX_CHANNEL_AU_915 - 8; i++ )
    {
        uint8_t test = SMTC_GET_BIT8( snapshot_channel_tx_mask, i );
        SMTC_MODEM_HAL_TRACE_PRINTF_DEBUG( "%u%s", test, ( ( i % 8 ) == 7 ) ? " \n" : "" );
    }
    SMTC_MODEM_HAL_TRACE_PRINTF_DEBUG( "snapshot channel 500 tx mask\n" );
    for( uint8_t i = 0; i < 8; i++ )
    {
        uint8_t test = SMTC_GET_BIT8( &snapshot_channel_tx_mask[BANK_8_500_AU915], i );
        SMTC_MODEM_HAL_TRACE_PRINTF_DEBUG( "%u%s", test, ( ( i % 8 ) == 7 ) ? " \n" : "" );
    }
#endif

    return OKLORAWAN;
}

void region_au_915_mask_channel_used_for_tx( smtc_real_t* real )
{
    SMTC_PUT_BIT8( snapshot_channel_tx_mask, tx_channel_idx, CHANNEL_DISABLED );
}

void region_au_915_set_channel_mask( smtc_real_t* real )
{
    region_au_915_channel_mask_set_after_join( real );

    first_ch_mask_received = ch_mask_after_join_full;
}

void region_au_915_init_join_snapshot_channel_mask( smtc_real_t* real )
{
    memset( snapshot_channel_tx_mask, 0xFF, BANK_MAX_AU915 );
    snapshot_bank_tx_mask = 0;
}

void region_au_915_init_after_join_snapshot_channel_mask( smtc_real_t* real, uint8_t tx_data_rate,
                                                          uint32_t tx_frequency )
{
    au_915_channels_bank_t ch_mask_block = 0;

    for( au_915_channels_bank_t i = 0; i < BANK_MAX_AU915; i++ )
    {
        unwrapped_channel_mask[i] = 0x00;
    }

    uint8_t            tx_sf = 7;
    lr1mac_bandwidth_t tx_bw = BW125;
    region_au_915_lora_dr_to_sf_bw( tx_data_rate, &tx_sf, &tx_bw );

    /**
     * Important remark:
     *
     * In case of BW125, search the corresponding "block" of channels used by the last Tx frequency
     * In case of BW500, Search the corresponding "channel" used by the last Tx frequency
     *
     * For each 125KHz block there is a corresponding 500KHs channels.
     * So for example if we are in BW500 and found the channel number 2, the corresponding 125Khz block is also the
     * number 2
     *
     */
    if( tx_bw == BW125 )
    {
        // Search the corresponding block of channels used by the last Tx frequency
        ch_mask_block =
            ( au_915_channels_bank_t ) ( ( tx_frequency - DEFAULT_TX_FREQ_125_START_AU_915 ) /
                                         ( ( DEFAULT_TX_STEP_125_AU_915
                                             << 3 ) ) );  // 1600000 = 8 ch * 200000 MHz, the gap in each block
    }
    else if( tx_bw == BW500 )
    {
        ch_mask_block = ( au_915_channels_bank_t ) ( ( ( tx_frequency - DEFAULT_TX_FREQ_500_START_AU_915 ) /
                                                       DEFAULT_TX_STEP_500_AU_915 ) %
                                                     8 );
    }
    else
    {
        SMTC_MODEM_HAL_PANIC( "invalid BW %d", tx_bw );
    }

    // Block are defined from 0 to 8
    if( ch_mask_block >= BANK_MAX_AU915 )
    {
        SMTC_MODEM_HAL_PANIC( "frequency block out of range %d\n", ch_mask_block );
    }

    if( first_ch_mask_received == ch_mask_after_join_init )
    {
        // 125 kHz channels, init the right block only
        unwrapped_channel_mask[ch_mask_block] = 0xFF;  // In case of BW500, read the remark above

        // 500 kHz channels, init the corresponding 500kHz frequency to this block
        SMTC_PUT_BIT8( &unwrapped_channel_mask[BANK_8_500_AU915], ch_mask_block, CHANNEL_ENABLED );
    }
    else if( first_ch_mask_received == ch_mask_after_join_8ch )
    {
        // 125 kHz channels, init all blocks, except the previously set
        for( au_915_channels_bank_t i = 0; i < BANK_8_500_AU915; i++ )
        {
            unwrapped_channel_mask[i] = 0xFF;
        }
        unwrapped_channel_mask[ch_mask_block] = 0x00;  // In case of BW500, read the remark above

        // 500 kHz channels, init all 500kHz channels, except the previously set
        unwrapped_channel_mask[BANK_8_500_AU915] = ( 0xFF & ~( 1 << ch_mask_block ) );
    }
    else
    {
        SMTC_MODEM_HAL_PANIC( "bad sate\n" );
    }

    // Apply computed channel mask after join
    region_au_915_channel_mask_set_after_join( real );
}

status_channel_t region_au_915_build_channel_mask( smtc_real_t* real, uint8_t channel_mask_cntl, uint16_t channel_mask )
{
    status_channel_t status = OKCHANNEL;
    SMTC_MODEM_HAL_TRACE_PRINTF_DEBUG( "ChCtrl = 0x%u, ChMask = 0x%04x\n", channel_mask_cntl, channel_mask );
    switch( channel_mask_cntl )
    {
    // 125 KHz channels
    case 0:
    case 1:
    case 2:
    case 3:
        memcpy( unwrapped_channel_mask + ( channel_mask_cntl * 2 ), ( uint8_t* ) &channel_mask, 2 );
        break;
    // 500 KHz channels
    case 4:
        memcpy( &unwrapped_channel_mask[BANK_8_500_AU915], ( uint8_t* ) &channel_mask, 1 );
        break;
    // bank of channels
    case 5:
        // Run over each bank
        for( uint8_t i = 0; i < BANK_8_500_AU915; i++ )
        {
            if( ( ( channel_mask >> i ) & 0x01 ) == CHANNEL_ENABLED )
            {
                unwrapped_channel_mask[i] = 0xFF;
                SMTC_PUT_BIT8( &unwrapped_channel_mask[BANK_8_500_AU915], i, CHANNEL_ENABLED );
            }
            else
            {
                unwrapped_channel_mask[i] = 0x00;
                SMTC_PUT_BIT8( &unwrapped_channel_mask[BANK_8_500_AU915], i, CHANNEL_DISABLED );
            }
        }

        break;
    // All 125 kHz ON ChMask applies to channels 64 to 71
    case 6:
        // Enable all 125KHz channels
        memset( unwrapped_channel_mask, 0xFF, BANK_8_500_AU915 );

        // Enable 500KHz channels
        memcpy( &unwrapped_channel_mask[BANK_8_500_AU915], ( uint8_t* ) &channel_mask, 1 );
        break;
    // All 125 kHz OFF ChMask applies to channels 64 to 71
    case 7:
        // Disable all 125KHz channels
        memset( unwrapped_channel_mask, 0x00, BANK_8_500_AU915 );

        // Enable 500KHz channels
        memcpy( &unwrapped_channel_mask[BANK_8_500_AU915], ( uint8_t* ) &channel_mask, 1 );
        break;
    default:
        status = ERROR_CHANNEL_CNTL;
        break;
    }

    // Check if all enabled channels has a valid frequency
    for( uint8_t i = 0; i < real_const.const_number_of_tx_channel; i++ )
    {
        if( ( SMTC_GET_BIT8( unwrapped_channel_mask, i ) == CHANNEL_ENABLED ) &&
            ( region_au_915_get_tx_frequency_channel( real, i ) == 0 ) )
        {
            status = ERROR_CHANNEL_MASK;  // this status is used only for the last multiple link adr req
            break;                        // break for loop
        }
    }

#if( MODEM_HAL_DBG_TRACE == MODEM_HAL_FEATURE_ON )
    SMTC_MODEM_HAL_TRACE_PRINTF_DEBUG( "unwrapped channel 125 tx mask = 0x" );
    for( uint8_t i = BANK_0_125_AU915; i < BANK_8_500_AU915; i++ )
    {
        SMTC_MODEM_HAL_TRACE_PRINTF_DEBUG( "%02x ", unwrapped_channel_mask[i] );
    }
    SMTC_MODEM_HAL_TRACE_PRINTF_DEBUG( " \n" );
    SMTC_MODEM_HAL_TRACE_PRINTF_DEBUG( "unwrapped channel 500 tx mask = 0x%02x\n",
                                       unwrapped_channel_mask[BANK_8_500_AU915] );
#endif

    // check if all channels are disabled, return ERROR_CHANNEL_MASK
    if( SMTC_ARE_CLR_BYTE8( unwrapped_channel_mask, BANK_MAX_AU915 ) == true )
    {
        status = ERROR_CHANNEL_MASK;
    }

    return ( status );
}

void region_au_915_enable_all_channels_with_valid_freq( smtc_real_t* real )
{
    // Tx 125 kHz channels
    for( uint8_t i = 0; i < NUMBER_OF_TX_CHANNEL_AU_915 - 8; i++ )
    {
        SMTC_PUT_BIT8( channel_index_enabled, i, CHANNEL_ENABLED );
        dr_bitfield_tx_channel[i] = DEFAULT_TX_DR_125_BIT_FIELD_AU_915;
    }
    // Tx 500 kHz channels
    for( uint8_t i = NUMBER_OF_TX_CHANNEL_AU_915 - 8; i < NUMBER_OF_TX_CHANNEL_AU_915; i++ )
    {
        SMTC_PUT_BIT8( channel_index_enabled, i, CHANNEL_ENABLED );
        dr_bitfield_tx_channel[i] = DEFAULT_TX_DR_500_BIT_FIELD_AU_915;
    }
}

modulation_type_t region_au_915_get_modulation_type_from_datarate( uint8_t datarate )
{
    if( ( datarate <= DR6 ) || ( ( datarate >= DR8 ) && ( datarate <= DR13 ) ) )
    {
        return LORA;
    }
    else if( datarate == DR7 )
    {
        return LR_FHSS;
    }
    else
    {
        SMTC_MODEM_HAL_PANIC( );
    }
    return LORA;  // never reach
}

void region_au_915_lora_dr_to_sf_bw( uint8_t in_dr, uint8_t* out_sf, lr1mac_bandwidth_t* out_bw )
{
    if( ( in_dr <= DR6 ) || ( ( in_dr >= DR8 ) && ( in_dr <= DR13 ) ) )
    {
        *out_sf = datarates_to_sf_au_915[in_dr];
        *out_bw = datarates_to_bandwidths_au_915[in_dr];
    }
    else
    {
        SMTC_MODEM_HAL_PANIC( );
    }
}

void region_au_915_lr_fhss_dr_to_cr_bw( uint8_t in_dr, lr_fhss_v1_cr_t* out_cr, lr_fhss_v1_bw_t* out_bw )
{
    if( in_dr == DR7 )
    {
        *out_cr = LR_FHSS_V1_CR_1_3;
        *out_bw = LR_FHSS_V1_BW_1523438_HZ;
    }
    else
    {
        SMTC_MODEM_HAL_PANIC( );
    }
}

uint32_t region_au_915_get_tx_frequency_channel( smtc_real_t* real, uint8_t index )
{
    uint32_t freq = 0;
    // 500KHz channels
    if( index >= real_const.const_number_of_tx_channel - 8 )
    {
        freq = DEFAULT_TX_FREQ_500_START_AU_915 + ( ( index % 8 ) * DEFAULT_TX_STEP_500_AU_915 );
    }
    // 125KHz channels
    else
    {
        freq = DEFAULT_TX_FREQ_125_START_AU_915 + ( index * DEFAULT_TX_STEP_125_AU_915 );
    }
    return freq;
}

uint32_t region_au_915_get_rx1_frequency_channel( smtc_real_t* real, uint8_t index )
{
    return ( DEFAULT_RX_FREQ_500_START_AU_915 + ( ( index % 8 ) * DEFAULT_RX_STEP_500_AU_915 ) );
}

uint32_t region_au_915_get_rx_beacon_frequency_channel( smtc_real_t* real, uint32_t gps_time_s )
{
    uint8_t index = ( uint32_t ) ( floorf( gps_time_s / 128 ) ) % 8;
    return ( BEACON_FREQ_START_AU_915 + ( index * BEACON_STEP_AU_915 ) );
}

uint32_t region_au_915_get_rx_ping_slot_frequency_channel( smtc_real_t* real, uint32_t gps_time_s, uint32_t dev_addr )
{
    uint8_t index = ( dev_addr + ( uint32_t ) ( floorf( gps_time_s / 128 ) ) ) % 8;
    return ( PING_SLOT_FREQ_START_AU_915 + ( index * PING_SLOT_STEP_AU_915 ) );
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */
static void region_au_915_channel_mask_set_after_join( smtc_real_t* real )
{
    // Copy all unwrapped channels in channel enable and in snapshot
    memcpy( channel_index_enabled, unwrapped_channel_mask, BANK_MAX_AU915 );
    memcpy( snapshot_channel_tx_mask, unwrapped_channel_mask, BANK_MAX_AU915 );

#if( BSP_DBG_TRACE == BSP_FEATURE_ON )
    SMTC_MODEM_HAL_TRACE_PRINTF_DEBUG( "Ch 125kHz\n" );
    for( uint8_t i = 0; i < NUMBER_OF_TX_CHANNEL_AU_915 - 8; i++ )
    {
        SMTC_MODEM_HAL_TRACE_PRINTF_DEBUG( " %d ", SMTC_GET_BIT8( channel_index_enabled, i ) );
    }
    SMTC_MODEM_HAL_TRACE_PRINTF_DEBUG( " \n" );
    SMTC_MODEM_HAL_TRACE_PRINTF_DEBUG( "Ch 500kHz\n" );
    for( uint8_t i = NUMBER_OF_TX_CHANNEL_AU_915 - 8; i < NUMBER_OF_TX_CHANNEL_AU_915; i++ )
    {
        SMTC_MODEM_HAL_TRACE_PRINTF_DEBUG( " %d ", SMTC_GET_BIT8( channel_index_enabled, i ) );
    }
    SMTC_MODEM_HAL_TRACE_PRINTF_DEBUG( " \n" );
#endif

    first_ch_mask_received++;
}

/* --- EOF ------------------------------------------------------------------ */
