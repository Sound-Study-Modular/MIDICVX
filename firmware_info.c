#include "firmware_info.h"

/* MCVX in ASCII, little-endian in flash. */
const midicvx_firmware_info_t g_midicvx_firmware_info PROGMEM
    __attribute__((used, section(".midicvx_info"))) = {
        .magic = 0x5856434DUL,
        .version_major = MIDICVX_VERSION_MAJOR,
        .version_minor = MIDICVX_VERSION_MINOR,
        .version_patch = MIDICVX_VERSION_PATCH,
        .hardware_revision = MIDICVX_HW_REVISION,
        .application_limit = (uint16_t)MIDICVX_APPLICATION_MAX_BYTES,
        .feature_flags = 0
    };
