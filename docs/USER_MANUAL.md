# MIDICVX v1.0.1 — User Manual

MIDICVX is expanded open-source firmware for the Erica Synths DIY MIDI-CV hardware. It keeps the module's core MIDI-to-CV functions and adds three performance modes, five arpeggiator patterns, split learning, bass/ARP routing, external clocked arpeggiation, persistent settings, panic recovery, and WAV firmware updates.

# 1. Performance modes

MIDICVX has three main performance modes:

1. **LIVE** — normal MIDI-to-CV playing.
2. **MONO ARP** — one arpeggiated part plus a dedicated live bass part.
3. **DUAL ARP** — two arpeggiated voices using the two CV/GATE outputs.

Hold **PROGRAM for about 0.7 second** to advance:

```text
LIVE → MONO ARP → DUAL ARP → LIVE
```

Mode indication:

```text
LIVE       = status LED off
MONO ARP   = status LED steady
DUAL ARP   = status LED blinking
```

The DUAL ARP status blink is a mode indication; it is not synchronized to the ARP clock.

# 2. PROGRAM button

| Action | Result |
|---|---|
| Short press in LIVE | LIVE continues; no ARP pattern change. |
| Short press in either ARP mode | Select next ARP pattern. |
| Hold about 0.7 second | Advance to the next performance mode. |
| Hold about 8 seconds | PANIC: clear active notes/gates and reset runtime ARP state. |
| Hold PROGRAM at power-on with switch at **1 VOICE** | Enter CV calibration. |
| Hold PROGRAM at power-on with switch at **2 VOICE** | Enter WAV firmware update mode. |

The original Erica runtime configuration gesture is not exposed unchanged in MIDICVX v1.0.1 because PROGRAM is used for the new performance-mode controls.

# 3. Five ARP patterns

In MONO ARP or DUAL ARP, short-press PROGRAM to cycle:

1. **UP**
2. **DOWN**
3. **PING-PONG**
4. **RANDOM**
5. **ORDER PLAYED**

The status LED acknowledges the selected algorithm with 1–5 quick flashes.

```text
1 = UP
2 = DOWN
3 = PING-PONG
4 = RANDOM
5 = ORDER PLAYED
```

# 4. Startup split learning

On a normal startup, MIDICVX provides a short split-learning window. Both yellow gate LEDs illuminate while the module is waiting.

Play the desired MIDI split note during this window to save it. If no note is played, the stored split remains in use.

External transport ticks are suppressed during this learning period so an already-connected running clock does not prematurely end the indication.

# 5. LIVE mode

LIVE is the conventional MIDI-to-CV mode. MIDI notes directly control the CV/GATE outputs. The physical **1 VOICE / 2 VOICE** switch selects monophonic or duophonic-style operation.

The yellow LEDs follow the actual gate outputs. GLIDE remains an analogue control for pitch portamento.

# 6. MONO ARP

MONO ARP combines an arpeggiated musical part with a dedicated live bass part. This allows one MIDI performance to produce a bass foundation and a moving ARP voice at the same time.

Patch the bass CV/GATE pair to one modular voice and the ARP CV/GATE pair to another.

The split and physical voice switch participate in the routing behavior, allowing the firmware to separate the live and arpeggiated material according to the selected configuration.

# 7. DUAL ARP

DUAL ARP uses both CV/GATE channels for active arpeggiated material.

With a chord or larger held-note group, MIDICVX distributes musical material to two ARP output engines so two modular voices can move independently.

```text
MIDI notes
    │
    ▼
 MIDICVX
  /    \
 ▼      ▼
ARP A  ARP B
 │      │
CV1    CV2
GATE1  GATE2
```

The physical **1 VOICE / 2 VOICE** switch changes the DUAL ARP routing behavior. The status LED blinks whenever DUAL ARP is selected.

The five ARP algorithms remain available in DUAL ARP. RANDOM can produce independently changing output choices, while the ordered algorithms retain their corresponding directional/order behavior.

# 8. External clock

MIDICVX can advance its ARP engine from the external clock/transport input. Qualified clock transitions advance the arpeggio while MIDI supplies the held-note material.

This lets a keyboard or DAW determine harmony while the modular system determines rhythm.

# 9. Panic

If notes or gates become stuck, hold **PROGRAM for about 8 seconds**.

PANIC forces gates low, clears active MIDI/playback note state, and resets ARP runtime state while retaining saved settings. The status LED flashes three times to acknowledge the action.

# 10. Calibration

Calibration and WAV update share the PROGRAM-at-power-on gesture, so the physical voice switch determines which startup service mode is entered.

To enter **CV calibration**:

1. Power MIDICVX off.
2. Set the physical switch to **1 VOICE**.
3. Hold **PROGRAM**.
4. Power MIDICVX on while continuing to hold PROGRAM.
5. Release PROGRAM after startup and perform the normal CV calibration procedure.

Use a well-tracking 1 V/oct oscillator and tuner. Calibration is an installation/service operation and is stored for normal use.

**Do not use 2 VOICE when trying to enter calibration.** PROGRAM + power-on in 2 VOICE is reserved for the WAV updater.

# 11. Firmware updates by WAV

## Important compatibility requirement

The WAV update feature is provided by the **MIDICVX WAV bootloader**. It is not a feature of the original Erica Synths firmware and it cannot be added merely by playing the WAV file.

If the module has never been programmed with the new MIDICVX WAV bootloader, **WAV updating will not work**. The module must first receive the current application and WAV bootloader through an ISP programmer such as USBasp. After that initial installation, later application updates can be delivered by WAV without reconnecting the ISP programmer.

The release update file is:

```text
firmware/MIDICVX_v1.0.1.wav
```

This file is encoded firmware data, not ordinary listening audio.

## Entering WAV update mode

1. Power MIDICVX off.
2. Set the physical switch to **2 VOICE**.
3. Hold **PROGRAM**.
4. Power MIDICVX on while continuing to hold PROGRAM.
5. Release PROGRAM after startup.
6. The lower red/status LED blinks while the bootloader waits for update audio.
7. Play the original `MIDICVX_v1.0.1.wav` through a clean, wired audio path.
8. Allow the complete file to play without interruption.
9. After successful programming and verification, MIDICVX automatically resets and starts the application normally.

Do not normalize, EQ, compress, limit, time-stretch, add fades, convert to MP3, or otherwise process the WAV. Avoid Bluetooth and other paths that can resample or alter the data.

If an update fails, retry with the untouched WAV and a clean wired playback path.

# 12. Initial programming and recovery with USBasp

A blank MCU, replacement MCU, original-firmware module, or processor without the MIDICVX WAV bootloader must first be programmed through ISP.

The repository provides:

```text
programmers/program_DIP_328P.sh
programmers/program_SMD_328P.sh
programmers/program_SMD_328.sh
```

The ATmega328P signature is:

```text
1E 95 0F
```

The non-P ATmega328 signature used by the SMD script is:

```text
1E 95 14
```

The programming scripts check the processor signature before writing.

The MIDICVX WAV-bootloader fuse configuration is:

```text
LFUSE = FF
HFUSE = D8
EFUSE = FD
```

The ISP installation writes both:

```text
firmware/midi_cv_x.hex
bootloader/midicvx_bootloader.hex
```

Once that installation has been completed successfully, normal application updates can be performed with the WAV updater.

# 13. Release files

```text
firmware/
├── midi_cv_x.hex          production application firmware for ISP
└── MIDICVX_v1.0.1.wav     encoded application update

bootloader/
└── midicvx_bootloader.hex WAV-update bootloader

programmers/
├── program_DIP_328P.sh    ATmega328P USBasp installer
├── program_SMD_328P.sh    ATmega328P convenience wrapper
└── program_SMD_328.sh     SMD ATmega328 USBasp installer
```

# 14. Troubleshooting

**I don't know which mode I'm in:** off = LIVE, steady = MONO ARP, blinking = DUAL ARP.

**A short PROGRAM press does nothing in LIVE:** normal. Short presses select ARP algorithms only in an ARP mode.

**ARP is not moving:** confirm an ARP mode is selected, notes are held, and a valid clock is present when external clocking is being used.

**A note or gate is stuck:** hold PROGRAM for about eight seconds for PANIC.

**WAV update mode will not start:** confirm the physical switch is at **2 VOICE** before holding PROGRAM and powering on. Also confirm that the module has already received the MIDICVX WAV bootloader through ISP.

**Calibration will not start:** set the switch to **1 VOICE**, then hold PROGRAM while powering on.

**WAV transfer fails:** use the untouched WAV and a clean wired playback path with no processing.

**MCU is completely unresponsive:** use USBasp/ISP with the appropriate programming script and verify the processor signature before writing.

# 15. Quick first session

1. Connect MIDI and patch CV1/GATE1 to a synth voice.
2. Power up normally and allow the startup split-learning window to finish.
3. Test LIVE operation.
4. Hold PROGRAM about 0.7 second to enter MONO ARP.
5. Hold several MIDI notes and short-press PROGRAM to try the five algorithms.
6. Hold PROGRAM again to enter DUAL ARP and patch the second CV/GATE pair to another voice.
7. Add external clock if desired.
8. Hold PROGRAM again to return to LIVE.

## Project lineage

MIDICVX is based on the open-source Erica Synths DIY MIDI-CV platform. Erica Synths' original documentation remains the reference for the physical hardware, assembly, and electrical specifications. MIDICVX adds and changes the performance firmware behavior documented here.
