# MIDICVX audio bootloader RC2

Supports ATmega328P and ATmega328. The same application WAV is used after the correct bootloader has been installed once via ISP.

## Build
From `foundation/bootloader`:

    make clean
    make MCU=atmega328p

For a non-P chip:

    make clean
    make MCU=atmega328

The bootloader must report <= 4096 bytes.

## One-time ISP installation
Build the application first in the parent directory. Then from `bootloader/` run:

    make install MCU=atmega328p
    make verify-install MCU=atmega328p

Only after both verifies succeed:

    make fuses MCU=atmega328p
    make read-fuses MCU=atmega328p

For non-P devices substitute `MCU=atmega328`.

Expected fuse values after installation are LFUSE=0xFF, HFUSE=0xD8, and the extended fuse reads back with BODLEVEL bits equivalent to 0x05 (commonly displayed as 0xFD because unused bits read as 1).

## Generate an update WAV
From `bootloader/`:

    make wav MCU=atmega328p

or directly:

    python3 make_update_wav.py ../build/midi_cv_x.hex --version 1.0.0 -o MIDICVX_v1.0.0.wav

## User update
1. Power off.
2. Hold PROGRAM.
3. Power on.
4. Feed the WAV into the CLOCK jack.
5. Play the WAV without EQ, normalization, fades, or time stretching.
6. Successful CRC verification causes the bootloader to mark the application valid and reboot.
7. If an update is interrupted after the manifest is accepted, the bootloader remains in recovery on future starts until a complete valid WAV is received.
