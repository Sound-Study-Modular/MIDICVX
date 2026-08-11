#ifndef MIDICVX_SETTINGS_LAYOUT_H_
#define MIDICVX_SETTINGS_LAYOUT_H_

#include <stdint.h>

/* Versioned MIDICVX settings block. Stock Erica calibration/config EEPROM
 * remains at its original addresses; MIDICVX owns bytes 64..79. */
#define MIDICVX_SETTINGS_MAGIC        0x58U
#define MIDICVX_SETTINGS_VERSION      1U
#define MIDICVX_EEPROM_SETTINGS_BASE 64U

/* Legacy beta8.x locations retained only for one-time migration. */
#define MIDICVX_EEPROM_SPLIT_MAGIC 80U
#define MIDICVX_EEPROM_SPLIT_NOTE  81U
#define MIDICVX_EEPROM_SPLIT_CHECK 82U
#define MIDICVX_EEPROM_MODE_MAGIC  83U
#define MIDICVX_EEPROM_MODE        84U
#define MIDICVX_EEPROM_ARP_MODE    85U
#define MIDICVX_EEPROM_MODE_CHECK  86U
#define MIDICVX_MODE_EEPROM_MAGIC  0x85U

typedef struct {
    uint8_t magic;
    uint8_t version;
    uint8_t length;
    uint8_t crc8;
    uint8_t performance_mode;
    uint8_t arp_mode;
    uint8_t split_note;
    uint8_t flags;
    uint8_t reserved[8];
} midicvx_settings_v1_t;

#endif
