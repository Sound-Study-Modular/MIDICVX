# MIDICVX v1.0.1

MIDICVX is an open firmware project for the Erica Synths DIY MIDI-to-CV hardware. It extends the original MIDI/CV firmware with LIVE, MONO ARP, and DUAL ARP performance modes, split-note routing, external-clock arpeggiation, persistent settings, panic recovery, and an audio/WAV bootloader for future firmware updates.

## v1.0.1 status

This repository publishes the known-good v1.0.1 firmware that was hardware-tested on both ATmega328P and ATmega328 variants.

### MCU variants

- DIP ATmega328P — signature `1E 95 0F` — avrdude part `m328p`
- SMD ATmega328 — signature `1E 95 14` — avrdude part `m328`

The included programming scripts verify the device signature before writing.

## Repository layout

```text
firmware/       prebuilt v1.0.1 application HEX and WAV update
bootloader/     WAV bootloader source, encoder, Makefile, and HEX
programmers/    USBasp scripts for DIP ATmega328P and SMD ATmega328
docs/           user, programming, WAV update, and developer guides
*.c / *.h       application source
Makefile        AVR application build
```

## Documentation

- [User manual](docs/USER_MANUAL.md)
- [ISP programming and recovery](docs/PROGRAMMING.md)
- [WAV firmware updates](docs/WAV_UPDATES.md)
- [Developer notes](docs/DEVELOPER.md)

## Quick build

Requires AVR GCC and avr-libc.

```sh
make clean
make
make size
```

The application reserves the top 4 KiB of the 32 KiB AVR flash for the MIDICVX bootloader.

## Initial programming

A new or recovered module must first be programmed over ISP. Use a USBasp or compatible AVR programmer.

For a DIP ATmega328P:

```sh
./programmers/program_DIP_328P.sh
```

For an SMD ATmega328:

```sh
./programmers/program_SMD_328.sh
```

The known-good WAV-bootloader fuse set used by these scripts is:

```text
LFUSE = FF
HFUSE = D8
EFUSE = FD
```

The scripts install both `firmware/midi_cv_x.hex` and `bootloader/midicvx_bootloader.hex`.

## Future firmware updates

Once the MIDICVX WAV bootloader is installed, normal future application updates can be performed by playing a compatible update WAV such as `firmware/MIDICVX_v1.0.1.wav` from the release package. See [docs/WAV_UPDATES.md](docs/WAV_UPDATES.md) for the complete procedure.

## Source heritage and licensing

This project is derived from the Erica Synths DIY MIDI-to-CV firmware. Preserve all upstream copyright and license obligations that apply to that source. A repository license should be added only after the applicable upstream licensing terms are confirmed.
