#include "timer_rtc.h"

#include "smtc_hal_rtc.h"
#include "timer.h"

static uint32_t context = 0;
static uint32_t wakeup_timer_rounds = 0;

uint32_t RtcGetTimerValue( void ) {
    return ( uint32_t ) rtc_get_timestamp_in_ticks( );
}

uint32_t RtcSetTimerContext( void ) {
    context = RtcGetTimerValue( );
    return context;
}

uint32_t RtcGetTimerContext( void ) {
    return context;
}

uint32_t RtcGetTimerElapsedTime( void ) {
    return RtcGetTimerValue( ) - context;
}

void RtcSetAlarm( uint32_t timeout ) {
    timeout -= RtcGetTimerElapsedTime( );

    if( timeout > 30000 ) {
        wakeup_timer_rounds = timeout / 30000;
        timeout = timeout % 30000;
    } else {
        wakeup_timer_rounds = 0;
    }

    hal_rtc_wakeup_timer_set_ms( RtcTick2Ms( timeout ) );
}

void RtcStopAlarm( void ) {
    hal_rtc_wakeup_timer_stop( );
    wakeup_timer_rounds = 0;
}

uint32_t RtcMs2Tick( TimerTime_t milliseconds ) {
    return rtc_ms_2_tick( milliseconds );
}

TimerTime_t RtcTick2Ms( uint32_t tick ) {
    return rtc_tick_2_ms( tick );
}

uint32_t RtcGetMinimumTimeout( void ) {
    return 3; // in ticks
}

TimerTime_t RtcTempCompensation( TimerTime_t period, float temperature ) {
    return 0;
}

void RtcProcess( void ) {}

void RtcIrq( void ) {
    if( wakeup_timer_rounds == 0 ) {
        TimerIrqHandler();
    } else {
        wakeup_timer_rounds--;
        hal_rtc_wakeup_timer_set_ms( RtcTick2Ms( 30000 ) );
    }
}
