# MIDICVX v1.0.1 — Audio/WAV Firmware Updates

MIDICVX can be updated from an audio file after the MIDICVX audio bootloader has been installed on the AVR.

This is the normal end-user update method. It does not require a USBasp or other ISP programmer.

## What you need

- A MIDICVX module with the MIDICVX audio bootloader already installed.
- The release WAV file, for v1.0.1: `firmware/MIDICVX_v1.0.1.wav`.
- A reliable audio playback device/interface and cable connected to the module's MIDI/audio update input as required by the bootloader hardware path.

Use the original WAV file without sample-rate conversion, normalization, EQ, compression, fades, crossfades, or other processing.

## Enter the bootloader

1. Turn the module off.
2. Prepare the v1.0.1 WAV for playback, but do not start it yet.
3. Hold the module's PROGRAM button.
4. Power the module on while continuing to hold PROGRAM.
5. Allow the bootloader to enter its update/listening state.
6. Start playback of the firmware WAV from the beginning.

The bootloader is designed so PROGRAM-at-power-up can still coexist with the application's original startup/calibration behavior: when a valid application exists it provides an update-detection window before handing control to the application if no update stream is detected.

## During playback

Do not:

- touch the audio cable;
- pause playback;
- seek within the WAV;
- change playback level;
- allow notification sounds or other system audio into the update path;
- power-cycle the module.

Play the complete WAV once, continuously, from beginning to end.

## Playback level

Use a clean, strong audio signal without clipping. If an update is not recognized, adjust the playback level and retry from the beginning rather than editing the WAV itself.

Computer/phone sound enhancements should be disabled. The safest source is a DAW or audio player/interface that reproduces the WAV bitstream cleanly without effects.

## After a successful update

After the complete stream has been accepted, power-cycle the module normally and test:

1. MIDI Note On/Off behavior.
2. CV1/GATE1.
3. CV2/GATE2.
4. LIVE mode.
5. MONO ARP.
6. DUAL ARP.
7. ARP clock/transport behavior.
8. PROGRAM-button controls.
9. Saved settings after another power cycle.

## If the update fails

A failed audio update does not automatically mean the AVR is damaged.

Retry with:

- the known-good release WAV;
- playback from the absolute beginning;
- a different playback level;
- all EQ/enhancement/normalization disabled;
- a different audio interface/player if necessary;
- a known-good cable.

If the bootloader no longer enters or repeated clean WAV attempts cannot be accepted, use direct ISP recovery as described in [PROGRAMMING.md](PROGRAMMING.md).

## Creating WAVs for future releases

The v1.0.1 package contains:

```text
bootloader/make_update_wav.py
```

and the application image:

```text
firmware/midi_cv_x.hex
```

The update WAV must be generated using the format expected by the MIDICVX bootloader. Do not substitute a generic "HEX to WAV" conversion.

For a future release, build and verify the application HEX first, generate the WAV with the supplied MIDICVX encoder, and test the resulting update on hardware before publishing it.

## ISP versus WAV update

The WAV method updates firmware through the installed bootloader. It cannot rescue a board whose bootloader/fuse configuration is missing or unusable.

Direct ISP programming is the installation and recovery path for those cases.
