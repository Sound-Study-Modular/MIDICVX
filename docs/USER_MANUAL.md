# MIDICVX v1.0.1 User Manual

## Overview

MIDICVX converts incoming MIDI notes to two CV/GATE channels and adds three performance modes: LIVE, MONO ARP, and DUAL ARP. It also supports a learned keyboard split, external transport clocking, persistent settings, and panic recovery.

## Startup split learning

Every normal startup opens a 2-second split-learning window. Both yellow gate LEDs are held on during this window.

- Play one MIDI note during the window to make that note the split point / lowest note of the upper ARP region.
- The first valid Note On is saved immediately to EEPROM.
- If you do not play a note during the window, MIDICVX keeps the previously saved split.
- The external-clock startup regression is fixed in v1.0.1: incoming clock ticks are discarded during split learning so the yellow indication remains usable.

## Performance modes

A long PROGRAM-button press cycles:

`LIVE → MONO ARP → DUAL ARP → LIVE`

The selected performance mode is stored so it can survive a power cycle.

### LIVE

LIVE preserves the normal MIDI-to-CV playing behavior of the original firmware. The physical voice switch controls the live routing/voice behavior.

### MONO ARP

MONO ARP provides one arpeggiated voice and one live split/bass voice according to the current split and physical voice-switch routing.

### DUAL ARP

DUAL ARP uses both CV/GATE outputs for arpeggiated performance. The learned split divides the keyboard into lower and upper regions and each region owns its output routing.

## ARP algorithm selection

While in an ARP performance mode, a short PROGRAM press advances the arpeggiator algorithm. MIDICVX retains five algorithms: UP, DOWN, PING-PONG, RANDOM, and ORDER PLAYED. The algorithm selection is saved with the other playback settings.

## Physical voice switch

The hardware voice switch remains active in every performance mode. MIDICVX detects switch changes while running and rebuilds routing immediately so stale gates are not left behind.

## Yellow LEDs

The two yellow LEDs follow the actual GATE1 and GATE2 output state during normal use. In ARP modes they pulse with generated gates rather than merely following held MIDI notes. At startup both yellow LEDs are also used for the split-learning indication.

## Red status LED

The red status LED indicates mode/confirmation/status behavior. Panic confirmation uses only the red LED so the gate outputs remain safely low.

## External clock / transport

MIDICVX accepts transport clock events for ARP stepping. External clock is intentionally ignored during the short startup split-learning window, then normal transport processing resumes.

## Panic reset

Hold PROGRAM for about 8 seconds during normal operation to perform a panic reset. Panic forces both gates low, clears held MIDI/playback state, resets generated ARP state and transport timing, and preserves the selected mode, ARP algorithm, learned split, calibration, and EEPROM settings.

## Persistent settings

MIDICVX stores its playback settings in EEPROM. Split learning is committed immediately. Mode and ARP settings are also persisted by the firmware settings system.

## Calibration and configuration

The original Erica Synths MIDI-to-CV calibration/configuration behavior remains part of the firmware. Avoid erasing EEPROM casually because it can contain calibration and MIDI configuration information.

## Updating firmware

If the MIDICVX WAV bootloader is already installed, use [WAV_UPDATES.md](WAV_UPDATES.md). For first-time installation, fuse/bootloader replacement, or recovery, use [PROGRAMMING.md](PROGRAMMING.md).
