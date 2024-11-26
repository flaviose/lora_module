#ifndef __APPLICATION_SETTINGS_APP_H__
#define __APPLICATION_SETTINGS_APP_H__

#include "stdint.h"
#include "stdbool.h"

typedef struct {
    uint8_t     deveui[8];               // 0x00 lorawan deveui
    uint8_t     joineui[8];              // 0x08 lorawan joineui
    uint8_t     nwkey[16];               // 0x10 lorawan nwkey
    uint8_t     hash[32];                // 0x20 keys hash
} keys_t;

typedef struct {
    uint8_t     region;                  // 0x00 lorawan region
    uint8_t     gap1[3];                 // 0x01 gap1
    uint16_t    transmission_interval;   // 0x04 transmission interval in seconds
    uint8_t     gap2[2];                 // 0x06 gap2
    uint8_t     hash[32];                // 0x08 settings hash
} settings_t;

#define SETTINGS_APP_PRINT() \
    SMTC_HAL_TRACE_INFO("Settings:\n"); \
    SMTC_HAL_TRACE_INFO(" - deveui:                   %02X%02X%02X%02X%02X%02X%02X%02X\n", keys.deveui[0], keys.deveui[1], keys.deveui[2], keys.deveui[3], keys.deveui[4], keys.deveui[5], keys.deveui[6], keys.deveui[7]); \
    SMTC_HAL_TRACE_INFO(" - joineui:                  %02X%02X%02X%02X%02X%02X%02X%02X\n", keys.joineui[0], keys.joineui[1], keys.joineui[2], keys.joineui[3], keys.joineui[4], keys.joineui[5], keys.joineui[6], keys.joineui[7]); \
    SMTC_HAL_TRACE_INFO(" - nwkey:                    ????????????????????????????%02X%02X\n", keys.nwkey[14], keys.nwkey[15]); \
    SMTC_HAL_TRACE_INFO(" - region:                   %u\n", settings.region); \
    SMTC_HAL_TRACE_INFO(" - transmission_interval:    %u\n", settings.transmission_interval); \

#endif // __APPLICATION_SETTINGS_APP_H__
