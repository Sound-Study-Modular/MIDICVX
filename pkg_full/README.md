# Erica Synths - DIY MIDI CV

## Description

The Erica Synths DIY MIDI-CV emulates classical monophonic and duophonic synth behavior, as well as it can be configured for use in two independent MIDI channels. Bonus features – MIDI clock output and analogue glide circuit on both channels.

Target: **ATmega328P @ 16 MHz** (external crystal), MIDI input at 31250 baud on `RXD` (PD0).
Settings (MIDI channels, mod/velocity mode) and DAC calibration are stored in EEPROM.

## Build

Requires the AVR GCC toolchain (`avr-gcc`, `avr-objcopy`, `avr-objdump`, `avr-size`).

```sh
# macOS
brew tap osx-cross/avr && brew install avr-gcc avrdude

# Debian/Ubuntu
sudo apt install gcc-avr avr-libc avrdude
```

Then:

```sh
make               # release build (-Os) into build/
make BUILD=debug   # -O1 -g2 -DDEBUG
make size          # flash/RAM usage
make lss           # disassembly listing
make clean
```

`F_CPU` and `BAUD` are defined in [hardware.h](hardware.h#L14-L15), not passed on the command line. The `DIY_HW` define at the top of the same file selects the DIY pinout (SPI DAC); undefine it for the non-DIY version with I²C-DAC hardware.

## Flash

The board ships with the [Optiboot](https://github.com/optiboot/optiboot) bootloader.

For manufacturing, combine both midi_cv.hex and optiboot.hex in single image.

### Fuses

Fuses are only writable over ISP — the bootloader cannot change them. The Makefile defaults are for a chip running **without** a bootloader; when using Optiboot, only `HFUSE` has to change, so that reset vectors into the boot section:

| Fuse | No bootloader | **With Optiboot** | Meaning |
| --- | --- | --- | --- |
| `LFUSE` | `0xFF` | `0xFF` | External crystal ≥ 8 MHz, slow rising power, CKDIV8 off |
| `HFUSE` | `0xD1` | **`0xDE`** | Both: SPIEN on, watchdog not forced. `0xDE` (stock Arduino Uno) = 512 B boot section at 0x7E00, BOOTRST on, EESAVE off; `0xD1` = no boot section, BOOTRST off, EESAVE on |
| `EFUSE` | `0xFD` | `0xFD` | Brown-out detect at 2.7 V |


Program the fuses and bootloader once, with an ISP programmer:

```sh
make fuses HFUSE=0xDE                     # LFUSE/EFUSE defaults are already correct
avrdude -p m328p -c dragon_isp -P usb -B 8 \
        -U flash:w:optiboot_atmega328.hex:i \
        -U lock:w:0x0F:m                  # lock the boot section against self-overwrite
```

`optiboot_atmega328.hex` is the prebuilt image from the Optiboot repository (`optiboot/bootloaders/optiboot/`), built for 16 MHz / 115200 baud. `make fuses` and the Makefile's ISP settings default to an AVR Dragon; override with `PROGRAMMER=` and `PORT=` for other hardware, e.g. `PROGRAMMER=usbasp` or `PROGRAMMER=avrisp2`.

### Flashing over the bootloader

Connect a 5 V USB-to-serial adapter to the board's programming header — `TXD`→`RXD` (PD0), `RXD`→`TXD` (PD1), `GND`→`GND`, and `DTR` to `RESET` through a 100 nF capacitor so avrdude can auto-reset the chip. Then:

```sh
avrdude -p m328p -c arduino -P /dev/tty.usbserial-XXXX -b 115200 \
        -D -U flash:w:build/midi_cv.hex:i
```

### Flashing over ISP

If you would rather skip the bootloader entirely, program the application directly and use the no-bootloader fuse values from the table above (`HFUSE = 0xD1`, the Makefile default):

```sh
make flash                     # avrdude -c dragon_isp -P usb
make flash PROGRAMMER=usbasp   # or another programmer
make fuses                     # LFUSE=0xFF HFUSE=0xD1 EFUSE=0xFD
```