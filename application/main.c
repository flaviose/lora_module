/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */
#include <stdint.h>   // C99 types
#include <stdbool.h>  // bool type

#include "smtc_modem_api.h"
#include "smtc_modem_utilities.h"

#include "smtc_modem_hal.h"
#include "smtc_hal_dbg_trace.h"

#include "smtc_hal_mcu.h"

#include "modem_pinout.h"
#include <string.h>
#include "helpers.h"

#include "sht3x.h"
#include "settings.h"
#include "timer.h"
#include "mx25.h"

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS-----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */

/**
 * Stack id value (multistacks modem is not yet available)
 */
#define STACK_ID 0

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE TYPES -----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */
static uint8_t                  rx_payload[SMTC_MODEM_MAX_LORAWAN_PAYLOAD_LENGTH] = { 0 };  // Buffer for rx payload
static uint8_t                  rx_payload_size = 0;      // Size of the payload in the rx_payload buffer
static smtc_modem_dl_metadata_t rx_metadata     = { 0 };  // Metadata of downlink
static uint8_t                  rx_remaining    = 0;      // Remaining downlink payload in modem

static uint32_t                 uplink_counter       = 0;      // uplink raising counter

extern keys_t keys;
extern settings_t settings;

static volatile bool transmission_flag = false;
static TimerEvent_t  transmission_timer;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

/**
 * @brief User callback for modem event
 *
 *  This callback is called every time an event ( see smtc_modem_event_t ) appears in the modem.
 *  Several events may have to be read from the modem when this callback is called.
 */
static void modem_event_callback( void );

/**
 * @brief Send the 32bits uplink counter on chosen port
 */
static void send_uplink_counter_on_port( uint8_t port );

/**
 * @brief Timer callback for transmission
 *
 *  This callback is called every time the transmission timer expires.
 */
static void transmission_timer_cb( void* context );

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

/**
 * @brief Example to send a user payload on an external event
 *
 */
int main( void ) {
    uint32_t sleep_time_ms = 0;

    // Disable IRQ to avoid unwanted behavior during init
    hal_mcu_disable_irq( );

    // Configure all the µC periph (clock, gpio, timer, ...)
    hal_mcu_init( );

    SMTC_HAL_TRACE_INFO( "Periodical uplink example is starting \n" );

    settings_init();
    settings_print();

    // Init the modem and use modem_event_callback as event callback, please note that the callback will be
    // called immediately after the first call to smtc_modem_run_engine because of the reset detection
    smtc_modem_init( &modem_event_callback );

    // Init done: enable interruption
    hal_mcu_enable_irq( );

    bool status = mx25_init( 1, PB_13 );
    SMTC_HAL_TRACE_INFO( "MX25 init status: %d\n", status );


    bool status_2 = sht3x_init(0x45<<1);
    SMTC_HAL_TRACE_INFO( "SHT3X init status: %d\n", status_2 );
    int32_t temp_1 = 0;
    int32_t rh_1 = 0;
    sht3x_read_temp( &temp_1, &rh_1 );
    SMTC_HAL_TRACE_INFO( "temp: %d\n", temp_1 );
    SMTC_HAL_TRACE_INFO( "rh : %d\n", rh_1 );
    while( 1 ) {
        // Modem process launch
        sleep_time_ms = smtc_modem_run_engine( );

        if( transmission_flag ) {
            transmission_flag = false;
            SMTC_HAL_TRACE_INFO( "Transmission triggered \n" );
            send_uplink_counter_on_port( 101 );
        }

        hal_mcu_set_sleep_for_ms( sleep_time_ms );
    }
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

static void modem_event_callback( void ) {
    smtc_modem_event_t current_event;
    uint8_t            event_pending_count;
    uint8_t            stack_id = STACK_ID;

    // Continue to read modem event until all event has been processed
    do {
        // Read modem event
        ASSERT_SMTC_MODEM_RC( smtc_modem_get_event( &current_event, &event_pending_count ) );

        switch( current_event.event_type ) {
        case SMTC_MODEM_EVENT_RESET:
            SMTC_HAL_TRACE_INFO( "Event received: RESET\n" );

            // Set user credentials
            ASSERT_SMTC_MODEM_RC( smtc_modem_set_deveui( stack_id, keys.deveui ) );
            ASSERT_SMTC_MODEM_RC( smtc_modem_set_joineui( stack_id, keys.joineui ) );
            ASSERT_SMTC_MODEM_RC( smtc_modem_set_appkey( stack_id, keys.nwkey ) );
            ASSERT_SMTC_MODEM_RC( smtc_modem_set_nwkkey( stack_id, keys.nwkey ) );

            // Set user region
            ASSERT_SMTC_MODEM_RC( smtc_modem_set_region( stack_id, settings.region ) );

            // Schedule a Join LoRaWAN network
            ASSERT_SMTC_MODEM_RC( smtc_modem_join_network( stack_id ) );
            break;

        case SMTC_MODEM_EVENT_ALARM:
            SMTC_HAL_TRACE_INFO( "Event received: ALARM\n" );
            break;

        case SMTC_MODEM_EVENT_JOINED:
            SMTC_HAL_TRACE_INFO( "Event received: JOINED\n" );
            SMTC_HAL_TRACE_INFO( "Modem is now joined \n" );

            SMTC_HAL_TRACE_INFO( "Transmission triggered \n" );
            send_uplink_counter_on_port( 101 );

            TimerInit( &transmission_timer, transmission_timer_cb );
            TimerSetValue( &transmission_timer, settings.transmission_interval * 1000 );
            TimerStart( &transmission_timer );
            break;

        case SMTC_MODEM_EVENT_TXDONE:
            SMTC_HAL_TRACE_INFO( "Event received: TXDONE\n" );
            SMTC_HAL_TRACE_INFO( "Transmission done \n" );
            break;

        case SMTC_MODEM_EVENT_DOWNDATA:
            SMTC_HAL_TRACE_INFO( "Event received: DOWNDATA\n" );
            // Get downlink data
            ASSERT_SMTC_MODEM_RC( smtc_modem_get_downlink_data( rx_payload, &rx_payload_size, &rx_metadata, &rx_remaining ) );
            SMTC_HAL_TRACE_PRINTF( "Data received on port %u\n", rx_metadata.fport );
            SMTC_HAL_TRACE_ARRAY( "Received payload", rx_payload, rx_payload_size );
            break;

        case SMTC_MODEM_EVENT_JOINFAIL:
            SMTC_HAL_TRACE_INFO( "Event received: JOINFAIL\n" );
            break;

        case SMTC_MODEM_EVENT_ALCSYNC_TIME:
            SMTC_HAL_TRACE_INFO( "Event received: ALCSync service TIME\n" );
            break;

        case SMTC_MODEM_EVENT_LINK_CHECK:
            SMTC_HAL_TRACE_INFO( "Event received: LINK_CHECK\n" );
            break;

        case SMTC_MODEM_EVENT_CLASS_B_PING_SLOT_INFO:
            SMTC_HAL_TRACE_INFO( "Event received: CLASS_B_PING_SLOT_INFO\n" );
            break;

        case SMTC_MODEM_EVENT_CLASS_B_STATUS:
            SMTC_HAL_TRACE_INFO( "Event received: CLASS_B_STATUS\n" );
            break;

        case SMTC_MODEM_EVENT_LORAWAN_MAC_TIME:
            SMTC_HAL_TRACE_WARNING( "Event received: LORAWAN MAC TIME\n" );
            break;

        case SMTC_MODEM_EVENT_LORAWAN_FUOTA_DONE: {
            bool status = current_event.event_data.fuota_status.successful;

            if( status == true ) {
                SMTC_HAL_TRACE_INFO( "Event received: FUOTA SUCCESSFUL\n" );
            } else {
                SMTC_HAL_TRACE_WARNING( "Event received: FUOTA FAIL\n" );
            }

            break;
        }

        case SMTC_MODEM_EVENT_NO_MORE_MULTICAST_SESSION_CLASS_C:
            SMTC_HAL_TRACE_INFO( "Event received: MULTICAST CLASS_C STOP\n" );
            break;

        case SMTC_MODEM_EVENT_NO_MORE_MULTICAST_SESSION_CLASS_B:
            SMTC_HAL_TRACE_INFO( "Event received: MULTICAST CLASS_B STOP\n" );
            break;

        case SMTC_MODEM_EVENT_NEW_MULTICAST_SESSION_CLASS_C:
            SMTC_HAL_TRACE_INFO( "Event received: New MULTICAST CLASS_C \n" );
            break;

        case SMTC_MODEM_EVENT_NEW_MULTICAST_SESSION_CLASS_B:
            SMTC_HAL_TRACE_INFO( "Event received: New MULTICAST CLASS_B\n" );
            break;

        case SMTC_MODEM_EVENT_FIRMWARE_MANAGEMENT:
            SMTC_HAL_TRACE_INFO( "Event received: FIRMWARE_MANAGEMENT\n" );

            if( current_event.event_data.fmp.status == SMTC_MODEM_EVENT_FMP_REBOOT_IMMEDIATELY ) {
                smtc_modem_hal_reset_mcu( );
            }

            break;

        case SMTC_MODEM_EVENT_STREAM_DONE:
            SMTC_HAL_TRACE_INFO( "Event received: STREAM_DONE\n" );
            break;

        case SMTC_MODEM_EVENT_UPLOAD_DONE:
            SMTC_HAL_TRACE_INFO( "Event received: UPLOAD_DONE\n" );
            break;

        case SMTC_MODEM_EVENT_DM_SET_CONF:
            SMTC_HAL_TRACE_INFO( "Event received: DM_SET_CONF\n" );
            break;

        case SMTC_MODEM_EVENT_MUTE:
            SMTC_HAL_TRACE_INFO( "Event received: MUTE\n" );
            break;

        case SMTC_MODEM_EVENT_RELAY_TX_DYNAMIC:  //!< Relay TX dynamic mode has enable or disable the WOR protocol
            SMTC_HAL_TRACE_INFO( "Event received: RELAY_TX_DYNAMIC\n" );
            break;

        case SMTC_MODEM_EVENT_RELAY_TX_MODE:  //!< Relay TX activation has been updated
            SMTC_HAL_TRACE_INFO( "Event received: RELAY_TX_MODE\n" );
            break;

        case SMTC_MODEM_EVENT_RELAY_TX_SYNC:  //!< Relay TX synchronisation has changed
            SMTC_HAL_TRACE_INFO( "Event received: RELAY_TX_SYNC\n" );
            break;

        default:
            SMTC_HAL_TRACE_ERROR( "Unknown event %u\n", current_event.event_type );
            break;
        }
    } while( event_pending_count > 0 );
}

static void send_uplink_counter_on_port( uint8_t port ) {
    // Send uplink counter on port 102
    uint8_t buff[4] = { 0 };
    buff[0]         = ( uplink_counter >> 24 ) & 0xFF;
    buff[1]         = ( uplink_counter >> 16 ) & 0xFF;
    buff[2]         = ( uplink_counter >> 8 ) & 0xFF;
    buff[3]         = ( uplink_counter & 0xFF );
    ASSERT_SMTC_MODEM_RC( smtc_modem_request_uplink( STACK_ID, port, false, buff, 4 ) );
    // Increment uplink counter
    uplink_counter++;
}

static void transmission_timer_cb( void* context ) {
    TimerStop( &transmission_timer );
    transmission_flag = true;
    TimerSetValue( &transmission_timer, settings.transmission_interval * 1000 );
    TimerStart( &transmission_timer );
}
