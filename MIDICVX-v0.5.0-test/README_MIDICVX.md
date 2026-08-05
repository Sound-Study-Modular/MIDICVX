# MIDI-CV X — v0.1 Foundation

This milestone intentionally preserves the stock Erica Synths MIDI-to-CV runtime behavior.
It adds the project foundation needed for the arpeggiator, pattern recorder, external-clock
input, dual patterns, and a future field-update bootloader.

## What changed

- Targets the actual production MCU signature: **ATmega328** (`avrdude -p m328`).
- Keeps the known-good programming speed: **USBasp `-B 100`**.
- Adds firmware identity/version metadata (`MCVX`, v0.1.0, hardware revision 1).
- Reserves the top **4 KiB** of the 32 KiB flash for a future MIDI SysEx/audio bootloader.
- Adds a build-time size check so the application cannot silently consume that reserve.
- Defines a versioned future EEPROM settings structure at EEPROM address 64.
- Does **not** migrate or overwrite stock EEPROM settings or DAC calibration.
- Does **not** alter the module's runtime behavior yet.

## Build

Requires AVR GCC tools:

```sh
make clean
make
```

The build must print an application size below **28,672 bytes**.

## Flash one test module

```sh
make flash PROGRAMMER=usbasp BITCLOCK=100
make verify PROGRAMMER=usbasp BITCLOCK=100
```

Known-good fuse values for the existing modules:

```text
LFUSE = 0xFF
HFUSE = 0xDE
EFUSE = 0x05 (normally reads back as 0xFD)
```

Do not mass-program this milestone until one module has passed the stock-behavior test.

## Hardware test checklist

1. Normal one-voice MIDI-to-CV behavior.
2. Normal two-voice behavior.
3. Gate 1 and Gate 2 behavior.
4. MOD/velocity output behavior.
5. MIDI clock output.
6. Program-button configuration and calibration remain functional.
7. Power-cycle retains settings and calibration.

## Next milestone

v0.2 will add a clock-source abstraction while keeping normal output behavior unchanged.
After that, v0.3 will add the first minimal arpeggiator mode: OFF and UP only.
