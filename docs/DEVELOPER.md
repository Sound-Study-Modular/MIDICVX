# MIDICVX v1.0.1 — Developer Notes

This document describes the intended repository/release workflow for MIDICVX development.

## Source of truth

For the v1.0.1 publication pass, the clean `MIDICVX-v1.0.1-GitHub-CLEAN` package is the authoritative release source. Older experimental files, archives, pre-release backups, recovered files, and historical test packages should not be copied into the release source tree.

## Target

MIDICVX targets the ATmega328/ATmega328P family at 16 MHz on the Erica Synths DIY MIDI-to-CV hardware.

The source is C built with the AVR GCC toolchain.

## Main source areas

The release source includes the hardware/MIDI/CV implementation plus MIDICVX playback and transport extensions. Important units include:

- `main.c/.h` — initialization, runtime dispatch, button/status behavior
- `midi.c/.h` — MIDI parsing
- `midi_cv.c/.h` — MIDI-to-CV behavior
- `playback.c/.h` — performance modes, ARP routing, held-note/playback state
- `transport.c/.h` — clock/transport support
- `dac.c/.h` — DAC/calibration support
- `hardware.c/.h` — hardware definitions
- `keyboard.c/.h` — note-state helpers
- `ring_buffer.c/.h` — MIDI receive buffering
- `firmware_info.c/.h` — firmware metadata
- `settings_layout.h` — persistent-settings layout

## Build

Install AVR GCC and avrdude, then from the repository root:

```sh
make clean
make
make size
```

Release artifacts are generated in `build/`. The application image is `build/midi_cv_x.hex`.

The build must remain inside the application region reserved by the MIDICVX bootloader.

## Release artifact layout

Published release packages should use a simple structure:

```text
firmware/
    midi_cv_x.hex
    MIDICVX_vX.Y.Z.wav
bootloader/
    bootloader.c
    Makefile
    make_update_wav.py
    midicvx_bootloader.hex
    README.md
programmers/
    program_DIP_328P.sh
    program_SMD_328.sh
docs/
    USER_MANUAL.md
    PROGRAMMING.md
    WAV_UPDATES.md
    DEVELOPER.md
```

Keep generated development backups, `.DS_Store`, `__MACOSX`, recovery artifacts, historical tarballs, and old test packages out of release archives.

## Persistent settings

MIDICVX extends the original EEPROM-backed configuration with playback settings. Changes to EEPROM layout must be treated as a compatibility concern. A new firmware version should either preserve the existing layout or deliberately migrate/version it.

Do not casually erase EEPROM during application updates because calibration and user settings may be stored there.

## Mode transitions

Performance-mode transitions deliberately clear active output state before rebuilding routing. This prevents stale gates and delayed retriggers from leaking from one mode into another.

When modifying mode logic, test transitions with notes held as well as with no notes held.

## Voice-switch changes

The physical voice switch can change routing while the module is running. Playback code detects this and rebuilds the active routing immediately. Any new routing mode should preserve this no-stale-gate behavior.

## Panic reset

Panic is a runtime recovery operation, not a factory reset. It should clear active MIDI/playback/transport state without erasing saved mode, ARP algorithm, split, calibration, or configuration.

## Release checklist

Before publishing a firmware release:

1. Build from a clean source tree.
2. Confirm application flash usage fits the bootloader-reserved layout.
3. Flash the HEX to representative hardware over ISP.
4. Verify flash readback.
5. Test LIVE, MONO ARP, and DUAL ARP.
6. Test both CV/GATE outputs.
7. Test the physical voice switch while notes are held.
8. Test MIDI clock/transport behavior.
9. Test PROGRAM-button short/long/recovery behavior.
10. Test split learning and persistence.
11. Power-cycle and verify persistent settings.
12. Generate the release WAV with `bootloader/make_update_wav.py`.
13. Install the release WAV through the audio bootloader on real hardware.
14. Power-cycle and repeat the functional smoke test.
15. Package only release files.
16. Inspect the archive for development junk before publication.

## Versioning

Use semantic release names such as `v1.0.1`. Keep source, firmware metadata, HEX, WAV filename, changelog, documentation, and GitHub release tag synchronized.

## Open-source publication

Before calling a repository formally open source, include an explicit license file in the repository root and make sure any inherited/upstream code retains the notices required by its original license.
