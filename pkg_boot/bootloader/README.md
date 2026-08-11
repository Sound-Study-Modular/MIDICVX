# MIDICVX Audio Bootloader 1.0 RC

The bootloader occupies the top 4 KiB of flash (`0x7000..0x7FFF`) and receives
update audio through the existing CLOCK jack on PC1/ADC1.

## Update UX

1. Hold PROGRAM while powering the module.
2. Start the update WAV within about 3 seconds.
3. The lower red LED pulses briefly as pages are accepted.
4. Three slower red flashes mean the full flash CRC32 passed.
5. The MCU reboots automatically into the new application.

If an update is interrupted or fails verification, the EEPROM boot-status byte
keeps the module in bootloader recovery mode. Replay a complete valid WAV.

The gate outputs are forced LOW for the entire bootloader session.

## Build

```sh
cd bootloader
make clean
make MCU=atmega328p
```

For an ATmega328 (non-P):

```sh
make clean
make MCU=atmega328
```

## Generate an update WAV

```sh
python3 bootloader/make_update_wav.py build/midi_cv_x.hex \
  --version 1.0.0 -o MIDICVX_v1.0.0.wav
```

Use an unprocessed output path: no EQ, normalization, fades, crossfades,
Bluetooth, or sample-rate conversion if avoidable. Start with maximum clean
wired output level and reduce only if the CLOCK input clips or misbehaves.

## One-time ISP installation

The bootloader is linked at `0x7000`. The high fuse must select a 4 KiB boot
section and BOOTRST (`HFUSE=0xD8`). Do not change fuses until both the normal
application and bootloader HEX files have been successfully built.

ATmega328P example (single avrdude session):

```sh
avrdude -c usbasp -p m328p -B 100 \
  -U flash:w:build/midi_cv_x.hex:i \
  -U flash:w:bootloader/midicvx_bootloader.hex:i \
  -U lfuse:w:0xFF:m -U hfuse:w:0xD8:m -U efuse:w:0x05:m
```

Use `-p m328` for an ATmega328.
