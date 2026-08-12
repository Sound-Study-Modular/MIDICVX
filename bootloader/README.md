# MIDICVX Audio Bootloader 1.0

The bootloader occupies the top 4 KiB of flash (`0x7000..0x7FFF`) and receives update audio through the existing CLOCK jack on PC1/ADC1.

## Update UX

1. Hold PROGRAM while powering the module.
2. Start the update WAV within about 3 seconds.
3. The lower red LED pulses briefly as pages are accepted.
4. Three slower red flashes mean the full flash CRC32 passed.
5. The MCU reboots automatically into the new application.

If an update is interrupted or fails verification, the EEPROM boot-status byte keeps the module in bootloader recovery mode. Replay a complete valid WAV.

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

## Generate a v1.0.1 update WAV

```sh
python3 bootloader/make_update_wav.py build/midi_cv_x.hex \
  --version 1.0.1 -o MIDICVX_v1.0.1.wav
```

Use an unprocessed output path: no EQ, normalization, fades, crossfades, Bluetooth, or sample-rate conversion if avoidable.

## One-time ISP installation

The bootloader is linked at `0x7000`. The high fuse must select a 4 KiB boot section and BOOTRST (`HFUSE=0xD8`). Use the supplied scripts in `programmers/` for the tested DIP ATmega328P and SMD ATmega328 workflows.
