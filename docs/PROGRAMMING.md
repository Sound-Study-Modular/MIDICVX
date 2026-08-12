# MIDICVX v1.0.1 — Programming and Recovery

This guide covers direct AVR ISP installation, re-flashing, verification, and recovery for MIDICVX v1.0.1.

For ordinary updates on a module that already has the MIDICVX audio bootloader installed, see [WAV_UPDATES.md](WAV_UPDATES.md).

## Release files

The clean v1.0.1 package includes:

- `firmware/midi_cv_x.hex` — MIDICVX application firmware
- `firmware/MIDICVX_v1.0.1.wav` — audio update image
- `bootloader/midicvx_bootloader.hex` — MIDICVX audio bootloader
- `bootloader/make_update_wav.py` — update-WAV generator
- `programmers/program_DIP_328P.sh` — ATmega328P/DIP programming helper
- `programmers/program_SMD_328.sh` — ATmega328/SMD programming helper

## Required tools

A USBasp or another AVR ISP programmer can be used. `avrdude` is required for the supplied command-line workflow.

macOS with Homebrew:

```sh
brew install avrdude
```

Debian/Ubuntu:

```sh
sudo apt install avrdude
```

## Identify the MCU before writing

The Erica Synths hardware has appeared with different 328-family parts. Do not assume the AVR part number from the package style alone.

Typical signatures used by the supplied programming workflows are:

- ATmega328P: `1E 95 0F`
- ATmega328: `1E 95 14`

Read the device successfully before changing flash or fuses. If the signature does not match the command you are using, stop and select the correct AVR device definition.

## ISP wiring

The established SMD-board programming points used during MIDICVX development are:

- MOSI → MCP4822 pin 4
- MISO → TP2 (or MCU MISO pin)
- SCK → MCP4822 pin 3
- RESET → XS1 pin 4
- GND → XS1 pin 1

Power the module normally from the Eurorack supply when using this wiring method. Keep temporary probe/jumper connections stationary while avrdude is communicating.

Always verify the actual board revision before relying on a pin number.

## First communication test

With a USBasp, begin with a deliberately slow ISP clock if the target is uncertain:

```sh
avrdude -c usbasp -p m328p -B 100
```

For an ATmega328 rather than 328P, use the avrdude part definition appropriate to that device/version of avrdude.

A successful device signature read is the prerequisite for every operation below.

## Back up before changing anything

Whenever possible, save the existing flash before programming:

```sh
avrdude -c usbasp -p m328p -B 100 -U flash:r:backup.hex:i
```

You may also read EEPROM before servicing a module whose calibration/settings matter:

```sh
avrdude -c usbasp -p m328p -B 100 -U eeprom:r:eeprom-backup.hex:i
```

Keep these files with the board/service record.

## Flash application only

If the correct bootloader and fuse configuration are already installed, the application can be written over ISP using the v1.0.1 application HEX:

```sh
avrdude -c usbasp -p m328p -B 100 \
  -U flash:w:firmware/midi_cv_x.hex:i
```

Then verify it explicitly:

```sh
avrdude -c usbasp -p m328p -B 100 \
  -U flash:v:firmware/midi_cv_x.hex:i
```

Use the correct `-p` device option for the MCU actually detected on the board.

## Bootloader installation

Installing the MIDICVX audio bootloader is an ISP operation. The bootloader, application layout, and fuse configuration must agree.

Use the supplied programming script for the target hardware rather than improvising fuse values:

```sh
cd programmers
./program_DIP_328P.sh
```

or:

```sh
cd programmers
./program_SMD_328.sh
```

Review the script and confirm programmer, part, paths, and expected fuse values before allowing it to write a board.

## Fuses

Fuse programming is the highest-risk part of AVR installation. Fuses select such things as clock source and boot/reset behavior. A wrong value can make a working board appear dead or require a different recovery method.

The known programming workflow verifies firmware again after fuse programming and reads back the fuse/lock values. For the bootloader configuration used in that workflow, the expected readback is:

```text
LFUSE: 0xFF
HFUSE: 0xDE
EFUSE: 0xFD
LOCK:  0xFF
```

Note: some AVR tools display the extended fuse with unused bits high. Judge the value according to the MCU and avrdude's readback convention rather than changing it merely because its printed representation looks different.

## Verification

Never treat a successful write command alone as proof of a good flash. Perform a verify pass.

At minimum:

```sh
avrdude -c usbasp -p m328p -B 100 \
  -U flash:v:firmware/midi_cv_x.hex:i
```

For a full installation, also read back the fuse and lock state and compare it with the intended configuration.

## If avrdude reports no target

Check these in order:

1. Module power.
2. Programmer USB connection.
3. Common ground between programmer and module.
4. RESET connection.
5. MOSI/MISO/SCK routing.
6. Programmer orientation.
7. Slow the ISP bit clock.
8. Confirm the MCU part selection.

Do not start writing fuses while communication is intermittent.

## Signature 00 00 00

A `00 00 00` signature is not a valid ATmega328-family identification. Treat it as a communication failure. Recheck power, ground, RESET, MISO/MOSI/SCK, contact quality, and ISP clock before attempting writes.

## Recovering a bad application

If the bootloader still runs, reinstall a known-good v1.0.1 WAV as described in `WAV_UPDATES.md`.

If the bootloader cannot accept an update, use ISP:

1. establish a valid device signature;
2. back up anything still readable;
3. flash the known-good bootloader/application using the appropriate supplied script;
4. verify flash;
5. read back and verify fuse/lock state;
6. power-cycle the module;
7. perform a functional MIDI/CV test.

## Building from source

The source tree uses the AVR GCC toolchain. Typical commands are:

```sh
make clean
make
make size
```

The release application is produced under `build/` as `midi_cv_x.hex`. Before distributing a new version, verify that the build fits the application flash region reserved by the bootloader.

## Important distinction

**ISP programming** can install/recover the bootloader and application and can alter fuses.

**WAV updating** only works after the MIDICVX audio bootloader is present and is intended for safe application updates without opening the module or connecting an AVR programmer.
