# Changelog

## 0.2.0-alpha1

- Added the playback router API (`playback.c` / `playback.h`).
- Routed MIDI Note On and Note Off through transparent LIVE mode.
- Added playback initialization during startup.
- No intended change to stock MIDI-to-CV behavior.
- Retained the 4 KiB future bootloader reservation.

## v0.1.0 — Foundation

- Preserved stock runtime behavior.
- Added firmware metadata and versioning.
- Added future EEPROM layout reservation.
- Added 4 KiB bootloader reserve and build-time size guard.
- Updated build/programming defaults for ATmega328 and USBasp `-B 100`.

## v0.3.0-alpha2
- Added MIDI-clocked UP arpeggiator.
- Short Program-button press toggles LIVE/ARP UP.
- MIDI LED remains lit while ARP UP is active.
- Existing MIDI Clock output remains available.
