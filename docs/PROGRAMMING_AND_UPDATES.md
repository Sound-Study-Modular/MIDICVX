# MIDICVX Programming and Firmware Updates

## Production Firmware

MIDICVX uses an AVR application together with a custom audio/WAV bootloader.

Once the production bootloader has been installed with an AVR ISP programmer, normal application firmware updates can be performed by playing the supplied MIDICVX firmware WAV into the module.

## Normal Startup

Power MIDICVX on normally without holding PROGRAM.

MIDICVX immediately starts the installed application.

## PROGRAM + 1 VOICE: MIDI Calibration

1. Set the VOICE switch to **1 VOICE**.
2. Hold **PROGRAM**.
3. Power on MIDICVX.
4. Release PROGRAM.

MIDICVX enters its MIDI calibration procedure.

This is separate from the WAV firmware updater.

## PROGRAM + 2 VOICE: WAV Firmware Update

1. Set the VOICE switch to **2 VOICE**.
2. Hold **PROGRAM**.
3. Power on MIDICVX.
4. Release PROGRAM.

MIDICVX remains in the audio firmware-update bootloader.

The lower red LED blinks while waiting for valid update audio.

## Installing Firmware by WAV

Use the firmware WAV supplied with the MIDICVX release, for example:

    MIDICVX_v1.0.1.wav

Connect the audio playback source to the MIDICVX **CLOCK input**.

Then:

1. Enter WAV update mode using PROGRAM + 2 VOICE at power-on.
2. Start the WAV from the beginning.
3. Allow the entire WAV to play.
4. Do not seek, pause, disconnect the cable, or power off during programming.

When incoming update audio is detected, the lower red LED changes from its waiting indication.

The yellow LEDs provide packet/update activity feedback while the WAV is being decoded.

After all firmware data has been received, MIDICVX verifies the programmed application.

If verification succeeds, MIDICVX automatically resets and starts the newly programmed application.

## Failed or Interrupted WAV Update

If playback is interrupted or the update does not complete, replay the complete WAV from the beginning.

Power the module off and re-enter WAV update mode:

1. Set **2 VOICE**.
2. Hold **PROGRAM**.
3. Power on.
4. Release PROGRAM.
5. Play the complete WAV again.

The bootloader provides a recovery path if a valid application is not present.

## Initial AVR / ISP Programming

The WAV bootloader itself must initially be installed using an AVR ISP programmer such as USBasp.

Production scripts are located in:

    programmers/

The supported entry points are:

    program_DIP_328P.sh
    program_SMD_328P.sh
    program_DIP_328.sh
    program_SMD_328.sh

Run the script corresponding to the MCU fitted to the MIDICVX.

Example:

    ./programmers/program_DIP_328P.sh

The scripts verify the MCU signature before programming.

## ATmega328P

AVRDUDE device:

    m328p

Expected signature:

    1E 95 0F

Supported scripts:

    program_DIP_328P.sh
    program_SMD_328P.sh

DIP versus SMD packaging does not change the MCU signature.

## ATmega328 (non-P)

AVRDUDE device:

    m328

Expected signature:

    1E 95 14

Supported scripts:

    program_DIP_328.sh
    program_SMD_328.sh

DIP versus SMD packaging does not change the MCU signature.

## Production Fuse Settings

Production MIDICVX programming uses:

    Low fuse:      FF
    High fuse:     D8
    Extended fuse: FD

These are the production settings used with the MIDICVX WAV bootloader.

## Production Firmware Files

Application firmware:

    build/midi_cv_x.hex

Production WAV bootloader:

    bootloader/midicvx_bootloader.hex

End-user audio update:

    MIDICVX_v1.0.1.wav

The WAV updates the MIDICVX application firmware. It does not replace the bootloader itself.

## When an AVR Programmer Is Required

An AVR programmer is required for:

- Initial MCU programming
- Manufacturing
- MCU replacement
- Installing or replacing the bootloader
- Low-level recovery

Once the production WAV bootloader is installed, normal application firmware updates can be performed using the supplied WAV file.

## Production v1.0.1 Tested Behavior

The production v1.0.1 system has been hardware tested for:

- Normal startup
- MIDI/CV operation
- PROGRAM + 1 VOICE calibration startup
- PROGRAM + 2 VOICE WAV-update startup
- Firmware reception through the CLOCK input
- WAV packet reception/activity indication
- Complete application programming
- Flash verification
- Automatic reset after a successful update
- Normal operation after the WAV update
