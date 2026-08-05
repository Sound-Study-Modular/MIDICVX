# Changelog

## v0.2.0 — Playback pass-through

- Added a real playback routing layer for Note On and Note Off messages.
- Preserved the original MIDI channel and voice routing exactly.
- Added no new button behavior, EEPROM settings, or playback modes.
- Intended as a hardware-validated baseline before adding the arpeggiator.

## v0.1.0 — Foundation

- Preserved stock runtime behavior.
- Added firmware metadata and versioning.
- Added future EEPROM layout reservation.
- Added 4 KiB bootloader reserve and build-time size guard.
- Updated build/programming defaults for ATmega328 and USBasp `-B 100`.

## v0.5.0 development
- Added ARP DOWN, PING-PONG, and RANDOM modes.
- Short Program presses cycle LIVE/UP/DOWN/PING-PONG/RANDOM.
- Added a nonblocking 2 ms Gate1 retrigger gap.
- Arpeggiator output is explicitly CV1/Gate1 only.
- Random mode avoids immediate repeats when two or more notes are held.
