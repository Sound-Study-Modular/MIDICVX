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

## v1.0.0-beta8.5
- Added three performance modes: LIVE, MONO ARP, and DUAL ARP.
- Long PROGRAM hold cycles the three performance modes.
- Lower red LED: off=LIVE, solid=MONO ARP, slow free-running blink=DUAL ARP.
- MONO ARP 1 VOICE: low live on CV1, high arp on CV2.
- MONO ARP 2 VOICE mirrors the split: low arp on CV1, high live on CV2.
- DUAL ARP 1 VOICE uses the learned keyboard split for two independent arp pools.
- DUAL ARP 2 VOICE preserves beta8.4 dynamic lower/upper assignment.
- Preserved continuous single-note retrigger behavior on every arpeggiated output.
- Preserved sustained held-key gates on the MONO ARP live side.
- Added EEPROM recall for performance mode and arp algorithm.
## v0.8.6 development
- Added versioned CRC-protected MIDICVX EEPROM settings block.
- Restores the last LIVE / MONO ARP / DUAL ARP mode at power-up.
- Restores the last selected arp algorithm and learned split.
- Mode/algorithm changes use a 2-second deferred write to reduce EEPROM wear.
- Migrates beta8.5 settings automatically on first boot.
- Added bootloader project foundation under `bootloader/`; application still reserves the top 4 KiB.

