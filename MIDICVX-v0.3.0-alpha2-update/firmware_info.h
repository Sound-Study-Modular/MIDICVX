#ifndef MIDICVX_FIRMWARE_INFO_H_
#define MIDICVX_FIRMWARE_INFO_H_

#include <stdint.h>
#include <avr/pgmspace.h>

#define MIDICVX_VERSION_MAJOR 0
#define MIDICVX_VERSION_MINOR 3
#define MIDICVX_VERSION_PATCH 0
#define MIDICVX_HW_REVISION   1

/* Reserve the top 4 KiB for a future MIDI/audio update bootloader. */
#define MIDICVX_FLASH_BYTES            32768UL
#define MIDICVX_BOOTLOADER_RESERVED    4096UL
#define MIDICVX_APPLICATION_MAX_BYTES  (MIDICVX_FLASH_BYTES - MIDICVX_BOOTLOADER_RESERVED)

typedef struct {
    uint32_t magic;
    uint8_t version_major;
    uint8_t version_minor;
    uint8_t version_patch;
    uint8_t hardware_revision;
    uint16_t application_limit;
    uint16_t feature_flags;
} midicvx_firmware_info_t;

extern const midicvx_firmware_info_t g_midicvx_firmware_info PROGMEM;

#endif
