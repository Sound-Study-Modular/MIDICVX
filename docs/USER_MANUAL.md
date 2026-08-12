# MIDICVX v1.0.1 User Manual

MIDICVX is firmware for the Erica Synths DIY MIDI-to-CV hardware, extending the original MIDI/CV behavior with performance modes, arpeggiation, split routing, saved settings, and recovery controls.

> This manual describes MIDICVX v1.0.1 behavior. The v1.0.1 release package should be treated as the authoritative firmware source when it differs from older development branches.

## Hardware

Target MCU: ATmega328/ATmega328P family at 16 MHz.

The module provides two CV/GATE output channels, MIDI input, a PROGRAM button, status/gate LEDs, and the hardware voice switch.

## Startup split learning

At startup MIDICVX opens a 2-second split-learning window.

- Both gate/yellow outputs indicate the learning state.
- Play the MIDI note that you want to use as the lowest ARP note.
- The first valid Note On received becomes the split note.
- The selected split is saved immediately to EEPROM and survives power cycling.

If no valid note is received during the learning window, normal operation continues using the stored/default split setting.

## Performance modes

MIDICVX v1.0.1 has three performance modes:

1. **LIVE** — normal MIDI-to-CV performance behavior.
2. **MONO ARP** — arpeggiated performance with the secondary output available for the live/bass routing defined by the firmware.
3. **DUAL ARP** — two-output arpeggiated performance using the split/routing logic.

Changing performance mode is treated as a musical boundary: active gates and pending retriggers are cleared before the new routing is established. Held MIDI-note state is preserved where appropriate.

Performance mode and ARP algorithm settings are stored in EEPROM.

## Voice switch

The physical voice switch remains a routing modifier. MIDICVX detects a switch change during operation and immediately rebuilds the output routing so stale gates are not left active.

## ARP algorithms

MIDICVX retains the firmware's selectable arpeggiator algorithms. The currently selected algorithm is stored with the other playback settings so it can survive a restart.

## LEDs

The yellow output LEDs follow their corresponding gate outputs during normal performance. The red status LED is used for status/confirmation indications, including recovery confirmation.

## Panic / recovery

MIDICVX includes a panic reset for recovering from stuck notes or an invalid runtime performance state.

Panic recovery:

- forces GATE 1 LOW;
- forces GATE 2 LOW;
- resets MIDI/CV runtime state;
- clears held-note state;
- clears pending ARP/retrigger state;
- resets transport timing state.

It intentionally preserves the selected performance mode, ARP algorithm, split point, and saved EEPROM configuration.

The red status LED confirms the panic action. The yellow gate LEDs remain off during the confirmation.

## MIDI clock and transport

The firmware contains transport/clock handling used by the arpeggiator. External MIDI clock can therefore drive ARP timing when that transport mode is active. The playback engine resets stale transport timing when routing or performance state is rebuilt.

## Saved settings

MIDICVX uses EEPROM for persistent configuration. This includes MIDI/CV configuration and calibration inherited from the underlying firmware, plus MIDICVX playback settings such as the split and performance/ARP selections.

## Calibration and configuration

The original firmware calibration/configuration facilities remain part of the codebase. Do not overwrite AVR fuse or EEPROM settings casually when servicing a calibrated module.

## Firmware updates

There are two fundamentally different update paths:

- **Audio/WAV update** — for a module that already has the MIDICVX audio bootloader installed.
- **ISP programming** — direct AVR programming using a USBasp or compatible programmer. This is also the recovery/install method when the bootloader is absent or damaged.

See [PROGRAMMING.md](PROGRAMMING.md) and [WAV_UPDATES.md](WAV_UPDATES.md).

## v1.0.1 release files

The clean v1.0.1 package contains the application HEX (`firmware/midi_cv_x.hex`), audio update (`firmware/MIDICVX_v1.0.1.wav`), MIDICVX bootloader HEX, WAV-generation utility, and separate DIP/SMD programming scripts.

## Safety when servicing

Power the module only from a correctly wired Eurorack supply. Verify programmer orientation and ISP connections before programming. Do not alter fuse values unless you understand the bootloader layout and clock configuration: an incorrect fuse configuration can make normal ISP communication difficult or prevent the application from booting.
