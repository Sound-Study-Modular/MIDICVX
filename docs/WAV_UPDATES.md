# MIDICVX v1.0.1 WAV Firmware Updates

The MIDICVX audio bootloader allows application firmware updates without an AVR programmer after the bootloader has first been installed over ISP.

## Requirements

Use a valid MIDICVX update WAV such as `MIDICVX_v1.0.1.wav` from the release package and a clean audio playback path. Do not normalize, compress, EQ, time-stretch, resample, fade, or otherwise process the update WAV.

## Enter update mode

1. Power the module off.
2. Connect the audio/update source.
3. Cue the WAV at the exact beginning.
4. Hold PROGRAM.
5. Power the module on while holding PROGRAM.
6. Start the WAV from the beginning during the bootloader listening window.
7. Let the entire file play without interruption.

Do not pause, seek, change gain, unplug the cable, allow notification sounds into the same output, or power-cycle the module during playback.

## If an update fails

Retry from the exact beginning with the known-good file, a clean playback path, and a different level if necessary. If the bootloader can no longer be entered or repeatedly rejects a known-good stream, restore by ISP using [PROGRAMMING.md](PROGRAMMING.md).

## Creating future update WAVs

```sh
python3 bootloader/make_update_wav.py \
  build/midi_cv_x.hex \
  --version X.Y.Z \
  -o MIDICVX_vX.Y.Z.wav
```

Use the supplied MIDICVX encoder; do not substitute a generic HEX-to-audio conversion.
