# MIDICVX v1.0.1 Programming and Recovery

## Supported MCU variants

| Hardware | Signature | avrdude part |
|---|---|---|
| DIP ATmega328P | `1E 95 0F` | `m328p` |
| SMD ATmega328 | `1E 95 14` | `m328` |

Always read the signature before writing.

## Required tools

Use a USBasp or compatible AVR ISP programmer and `avrdude`. On macOS: `brew install avrdude`. On Debian/Ubuntu: `sudo apt install avrdude gcc-avr avr-libc`.

## Established SMD board ISP wiring

- MOSI → MCP4822 pin 4
- MISO → TP2 / ATmega MISO
- SCK → MCP4822 pin 3
- RESET → XS1 pin 4
- GND → XS1 pin 1
- Power the module normally from Eurorack power

Verify board revision and orientation before connecting anything.

## Signature test

DIP: `avrdude -c usbasp -p m328p -B 100 -v`

SMD: `avrdude -c usbasp -p m328 -B 100 -v`

A `00 00 00` signature means communication failed. Do not write until power, ground, RESET, MOSI, MISO, SCK, orientation, and bit clock are corrected.

## One-command installation

From the repository root:

```sh
./programmers/program_DIP_328P.sh
```

or

```sh
./programmers/program_SMD_328.sh
```

The scripts refuse to write if the signature is not the expected one.

## Known-good bootloader fuses

```text
LFUSE = FF
HFUSE = D8
EFUSE = FD
```

Do not substitute older Optiboot fuse values from historical documentation.

## Manual avrdude programming

DIP ATmega328P:

```sh
avrdude -c usbasp -p m328p -B 100 \
  -U lfuse:w:0xff:m -U hfuse:w:0xd8:m -U efuse:w:0xfd:m \
  -U flash:w:firmware/midi_cv_x.hex:i \
  -U flash:w:bootloader/midicvx_bootloader.hex:i
```

SMD ATmega328:

```sh
avrdude -c usbasp -p m328 -B 100 \
  -U lfuse:w:0xff:m -U hfuse:w:0xd8:m -U efuse:w:0xfd:m \
  -U flash:w:firmware/midi_cv_x.hex:i \
  -U flash:w:bootloader/midicvx_bootloader.hex:i
```

The known-good v1.0.1 application image is 9954 bytes and the bootloader build is 2094 bytes.

## Recovery

If a WAV update fails but the bootloader still enters, retry the known-good WAV from the beginning. If the bootloader is missing or unusable, establish a valid AVR signature over ISP, back up anything needed, run the matching DIP/SMD programming script, verify the writes, power-cycle, and test MIDI, both CV/GATE outputs, performance modes, split learning, and persistence.
