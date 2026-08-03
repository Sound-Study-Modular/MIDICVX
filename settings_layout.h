#ifndef MIDICVX_SETTINGS_LAYOUT_H_
#define MIDICVX_SETTINGS_LAYOUT_H_

#include <stdint.h>

/*
 * Future versioned settings layout. This milestone does not replace the stock
 * EEPROM addresses yet, so existing calibration and user settings remain intact.
 */
#define MIDICVX_SETTINGS_MAGIC   0x58
#define MIDICVX_SETTINGS_VERSION 1

/* Reserved EEPROM region begins after the stock firmware's current data. */
#define MIDICVX_EEPROM_V2_BASE 64

typedef struct {
    uint8_t magic;
    uint8_t version;
    uint8_t length;
    uint8_t crc8;
    uint8_t performance_mode;
    uint8_t arp_mode;
    uint8_t clock_source;
    uint8_t clock_division;
    uint8_t flags;
    uint8_t reserved[7];
} midicvx_settings_v1_t;

#endif
