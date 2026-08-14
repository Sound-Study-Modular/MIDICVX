# MIDICVX v1.0.1

MIDICVX is expanded open-source firmware for the Erica Synths DIY MIDI-CV hardware.

It retains the core MIDI-to-CV functionality of the original module and adds three performance modes, five arpeggiator patterns, split learning, bass/ARP routing, external clocked arpeggiation, persistent settings, panic recovery, and firmware updates delivered as an encoded WAV file.

## Documentation

See [`docs/USER_MANUAL.md`](docs/USER_MANUAL.md) for operating instructions, mode descriptions, calibration, WAV updating, troubleshooting, and initial programming.

## Performance modes

- **LIVE** — normal MIDI-to-CV operation.
- **MONO ARP** — arpeggiated material plus a dedicated live bass part.
- **DUAL ARP** — two active arpeggiated CV/GATE voices.

Hold PROGRAM for about 0.7 second to advance through LIVE → MONO ARP → DUAL ARP → LIVE. In either ARP mode, short PROGRAM presses select UP, DOWN, PING-PONG, RANDOM, or ORDER PLAYED.

## Release firmware

The production application image is:

```text
firmware/midi_cv_x.hex
```

The audio update is:

```text
firmware/MIDICVX_v1.0.1.wav
```

The WAV file is encoded firmware data. Do not normalize, EQ, compress, time-stretch, add fades, convert to MP3, or otherwise process it.

**Important:** WAV updating requires the MIDICVX WAV bootloader. Modules still running the original Erica firmware or an earlier MIDICVX installation without this bootloader cannot use the WAV update feature until they are programmed once through ISP.

## Entering the WAV updater

1. Power MIDICVX off.
2. Set the front-panel switch to **2 VOICE**.
3. Hold **PROGRAM** while powering the module on.
4. Release PROGRAM after startup.
5. Play the untouched `MIDICVX_v1.0.1.wav` through a clean wired audio path.
6. Allow programming and verification to finish. MIDICVX resets automatically into the application when the update succeeds.

## Calibration startup

For CV calibration, set the module to **1 VOICE**, hold PROGRAM while powering on, and then follow the calibration procedure in the user manual.

The 1 VOICE / 2 VOICE distinction at startup is intentional: **1 VOICE selects calibration; 2 VOICE selects WAV firmware update.**

## Initial programming / recovery

A blank MCU, replacement MCU, or module that does not yet contain the MIDICVX WAV bootloader must be programmed with USBasp/ISP first.

Programming scripts are provided for the supported processor signatures:

```text
programmers/program_DIP_328P.sh
programmers/program_SMD_328P.sh
programmers/program_SMD_328.sh
```

`program_DIP_328P.sh` is the ATmega328P programmer and can be used for either DIP or SMD packages with signature `1E 95 0F`. `program_SMD_328P.sh` is a convenience wrapper for the same programmer. `program_SMD_328.sh` is for the non-P SMD ATmega328 with signature `1E 95 14`.

The production ISP scripts install both:

```text
firmware/midi_cv_x.hex
bootloader/midicvx_bootloader.hex
```

and use the MIDICVX WAV-bootloader fuse configuration:

```text
LFUSE = FF
HFUSE = D8
EFUSE = FD
```

Do not substitute the old Optiboot instructions or fuse configuration when installing the MIDICVX WAV bootloader.

## Building from source

An AVR GCC toolchain is required (`avr-gcc`, `avr-objcopy`, `avr-objdump`, `avr-size`).

On macOS:

```sh
brew tap osx-cross/avr
brew install avr-gcc avrdude
```

Build with:

```sh
make
```

Build output is placed under `build/`. The checked-in `firmware/midi_cv_x.hex` is the production release image used by the programming scripts; rebuilding source does not automatically make a newly built image the hardware-tested production release.

## Hardware

Target hardware is the Erica Synths DIY MIDI-CV platform using an ATmega328/328P-class MCU at 16 MHz. MIDICVX remains based on the original open-source Erica Synths project; Erica Synths documentation remains the reference for the physical module, assembly, and electrical hardware.

## Release status

MIDICVX v1.0.1 was hardware tested for normal startup, MIDI/CV operation, calibration startup, audio-update startup, complete WAV firmware programming, verification, automatic reset, and normal operation after updating.
