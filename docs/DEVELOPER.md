# MIDICVX Developer Notes

## Build

```sh
make clean
make
make size
```

The ATmega328-family device has 32 KiB flash. MIDICVX reserves the top 4 KiB for the audio bootloader, leaving 28672 bytes for the application.

## Key source files

- `main.c/.h` — initialization, button behavior, split-learning startup, LEDs, runtime dispatch
- `playback.c/.h` — performance modes, split routing, ARP algorithms, persistent settings, panic state
- `transport.c/.h` — external clock transport input
- `midi.c/.h` and `midi_cv.c/.h` — MIDI parsing and CV behavior
- `settings_layout.h` — EEPROM layout
- `firmware_info.c/.h` — embedded version and application-limit metadata

## Bootloader

`bootloader/bootloader.c` is linked into the top 4 KiB boot region. `bootloader/make_update_wav.py` generates compatible audio updates from an Intel HEX application image.

## Release checklist

Build cleanly; confirm the app fits below `0x7000`; update firmware metadata; ISP-program and verify; test LIVE/MONO ARP/DUAL ARP, all ARP algorithms, both outputs, voice-switch changes, external clock, split learning with clock connected and disconnected, persistence, and panic; generate and hardware-test the release WAV; then package only release files.

## EEPROM compatibility

Do not casually move or erase persistent settings. Calibration, MIDI configuration, split note, performance mode, and ARP settings rely on EEPROM.
