# MIDICVX WAV Bootloader and Programming

The MIDICVX bootloader occupies the top 4 KiB of flash (`0x7000..0x7FFF`) and receives firmware-update audio through the existing CLOCK jack on PC1/ADC1.

## End-user WAV update

1. Power the module off.
2. Hold PROGRAM and power on.
3. Begin playing the unmodified MIDICVX update WAV into CLOCK within about 3 seconds.
4. The lower red/status LED pulses as valid pages are accepted.
5. Three slower red flashes indicate that the full-image CRC32 passed.
6. The MCU automatically resets and starts the new application.

If playback is interrupted or verification fails, the EEPROM boot-status byte keeps the module in recovery. Power-cycle if necessary and replay a complete valid WAV. Gates are forced LOW throughout the bootloader session.

Use a clean wired audio path. Do not EQ, normalize, compress, time-stretch, fade, crossfade, convert to MP3, use Bluetooth, or otherwise process the WAV. Avoid sample-rate conversion when possible.

## Calibration versus WAV-update startup

PROGRAM is intentionally used during startup for both service paths. The bootloader gets first chance to hear a WAV. If no valid update begins during its normal startup window, it hands control to the application. Continuing to hold PROGRAM into the application enters the Erica-style CV calibration path. For a WAV update, start the WAV promptly; for calibration, keep holding PROGRAM through startup until calibration begins.

## Build the bootloader

ATmega328P:

```sh
cd bootloader
make clean
make MCU=atmega328p
```

ATmega328 (non-P):

```sh
cd bootloader
make clean
make MCU=atmega328
```

## Generate a firmware WAV

From the repository root:

```sh
python3 bootloader/make_update_wav.py firmware/midi_cv_x.hex \
  --version 1.0.1 -o firmware/MIDICVX_v1.0.1.wav
```

The WAV contains application firmware only. Once the audio bootloader is installed, routine releases normally require only the WAV.

## One-time ISP installation

A blank/replacement MCU or a processor whose bootloader is missing must be programmed over ISP. The bootloader is linked at `0x7000`; MIDICVX uses a 4 KiB boot section with BOOTRST.

Known working fuses:

```text
LFUSE = FF
HFUSE = D8
EFUSE = FD
```

The repository's `programmers/` scripts are the preferred installation method. They identify the expected MCU, write the application and WAV bootloader, set the fuses, and verify the result.

Supported AVR signatures:

```text
ATmega328P  = 1E 95 0F
ATmega328   = 1E 95 14
```

The package (DIP versus SMD) does not change the AVRDUDE part ID; the P/non-P processor variant does. Use `m328p` for ATmega328P and `m328` for ATmega328.

Do not substitute ATmega328PB: it has a different device signature and is not one of the MIDICVX production targets documented here.
