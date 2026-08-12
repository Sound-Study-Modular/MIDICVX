# MIDICVX v1.0.1 — User Manual

MIDICVX is expanded open-source firmware for the Erica Synths DIY MIDI-CV hardware. It keeps the module's core MIDI-to-CV functions and adds three performance modes, five arpeggiator patterns, split learning, bass/ARP routing, external clocked arpeggiation, persistent settings, panic recovery, and WAV firmware updates.

This manual starts with the things you need while actually playing: **the three modes, how to enter them, every front-panel control and button gesture, and the LED indications.** Detailed explanations and programming instructions follow afterward.

---

# 1. Start here — the three performance modes

MIDICVX has **three main performance modes**:

1. **LIVE** — normal MIDI-to-CV playing.
2. **MONO ARP** — one arpeggiated part plus a dedicated live bass part.
3. **DUAL ARP** — two arpeggiated voices using the two CV/GATE outputs.

You move through them with the **PROGRAM** button.

```text
                 hold PROGRAM ~0.7 s

        ┌──────────┐      ┌──────────┐      ┌──────────┐
        │   LIVE   │ ───► │ MONO ARP │ ───► │ DUAL ARP │
        └──────────┘      └──────────┘      └──────────┘
             ▲                                      │
             └──────────────────────────────────────┘

                  then back to LIVE
```

You do **not** need to power-cycle to change modes. Hold PROGRAM until the mode changes, then release it. Each long hold advances one step through the cycle.

## LIVE

**Status LED: off during normal operation.**

Use LIVE when you want MIDICVX to behave like a conventional MIDI-to-CV interface. Your keyboard, sequencer or DAW directly controls the CV/GATE outputs.

The physical **1 VOICE / 2 VOICE** switch selects the original monophonic or duophonic-style voice routing.

## MONO ARP

**Status LED: steadily on.**

MONO ARP turns the upper musical material into an arpeggio while retaining the lowest-note region as a separate bass part. This is the easiest way to turn one keyboard performance into **bass + arpeggio**.

## DUAL ARP

**Status LED: blinking.**

DUAL ARP uses both CV/GATE channels for arpeggiated material. It is intended for driving two modular voices from the held MIDI notes.

The blinking status LED is deliberately free-running; it identifies DUAL ARP mode and is not meant to flash in time with the external clock.

---

# 2. Quick control reference — learn this first

## PROGRAM button

| Action | What it does |
|---|---|
| **Short press in LIVE** | No ARP-pattern change; LIVE playing continues. |
| **Short press in MONO ARP or DUAL ARP** | Selects the next arpeggiator pattern. |
| **Hold about 0.7 second** | Advances **LIVE → MONO ARP → DUAL ARP → LIVE**. |
| **Hold about 8 seconds** | Emergency **PANIC**: clears active notes/gates and flashes the red/status LED three times. |
| **Hold PROGRAM while powering on** | Enters the original Erica-style **CV calibration mode**. |

### Important note about the original Erica configuration gesture

The original Erica Synths firmware used a roughly **2-second PROGRAM hold** to enter MIDI-channel/MOD configuration. MIDICVX v1.0.1 now uses the long-hold gesture for performance-mode switching. Although the configuration routines remain in the source, the current v1.0.1 button handling does **not expose the old normal-runtime configuration gesture in the same way**. This manual therefore does not tell you to use the original 2-second gesture as though it still behaved exactly like stock firmware.

That distinction matters: the **hardware is based on the original Erica module**, but the MIDICVX performance controls are different.

## 1 VOICE / 2 VOICE switch

In LIVE mode this retains the familiar Erica Synths voice-selection concept:

- **1 VOICE** — monophonic playing, with highest-note priority in the original design.
- **2 VOICE** — two-note/duophonic operation using CV/GATE 1 and CV/GATE 2.

MIDICVX watches the physical switch continuously. If you change it while holding notes in LIVE mode, the firmware rebuilds the held-note routing rather than waiting for you to play another MIDI note.

In the ARP modes the switch participates in the firmware's current routing behavior, while the selected performance mode determines whether you are in MONO ARP or DUAL ARP operation.

## GLIDE knob

GLIDE is an **analogue** control and affects both pitch-CV channels. Turn it down for immediate pitch changes or up for portamento/slides between notes. The original Erica specification gives a glide range of approximately 0–2 seconds.

## MIDI IN

Connect the MIDI output of your keyboard, sequencer, hardware controller or MIDI interface here.

## CV1 / GATE1 and CV2 / GATE2

These are the two pitch and gate pairs that drive your modular voices. Pitch CV is intended for normal 1 V/oct oscillator inputs. The original Erica hardware specifies a 0–8 V CV range and 5 V gate level.

## Yellow gate LEDs

The yellow LEDs indicate the **actual gate outputs**.

- In LIVE they follow live gate activity.
- In ARP they pulse with the generated ARP gates rather than simply remaining on while MIDI keys are held.
- During startup split learning, **both yellow LEDs are held on** to tell you the module is waiting for a split note.

## Status/MIDI LED

In normal performance it also tells you which MIDICVX mode you are in:

```text
LIVE       = OFF
MONO ARP   = ON steady
DUAL ARP   = BLINKING
```

When you change ARP algorithms, this LED temporarily flashes the algorithm number, then returns to the normal mode indication.

---

# 3. The five ARP patterns

When you are in either ARP mode, **short-press PROGRAM** to move to the next pattern.

There are five patterns:

1. **UP**
2. **DOWN**
3. **PING-PONG**
4. **RANDOM**
5. **ORDER PLAYED**

The status LED acknowledges the selected pattern with **1 to 5 quick flashes**.

```text
1 flash   = UP
2 flashes = DOWN
3 flashes = PING-PONG
4 flashes = RANDOM
5 flashes = ORDER PLAYED
```

Example with C3, E3, G3 and B3 held:

```text
UP:          C3 → E3 → G3 → B3 → ...
DOWN:        B3 → G3 → E3 → C3 → ...
PING-PONG:   C3 → E3 → G3 → B3 → G3 → E3 → ...
RANDOM:      notes selected in changing/random order
ORDER PLAYED: follows the order in which you originally pressed the notes
```

---

# 4. Startup split learning

Every normal startup begins with a **2-second split-learning window**.

Both yellow gate LEDs turn on. While they are on, play the MIDI note you want to use as the split/reference note.

```text
POWER ON
   │
   ▼
Both yellow LEDs ON
   │
   ├── Play a MIDI note ──► save that note as the split
   │
   └── Play nothing ──────► keep the previously saved split
```

The **first valid MIDI Note On** during the window becomes the new split and is saved. The status LED gives a brief acknowledgement.

If you do nothing, the learning window simply closes and MIDICVX keeps the previously stored split.

### You can leave external clock connected

MIDICVX v1.0.1 specifically fixes a startup problem found during development. With earlier code, a running external clock could interfere with the yellow split-learning indication and make the window appear almost instantaneous.

In v1.0.1 external transport ticks are discarded while split learning is active. You can therefore power up with the clock cable already patched and still get the full learning window.

---

# 5. Original Erica Synths features at a glance

The MIDICVX hardware began as the Erica Synths DIY MIDI-CV. The original module provides:

- two pitch CV outputs;
- two Gate outputs with gate LEDs;
- MIDI input;
- one-voice and two-voice operation;
- one- or two-MIDI-channel configuration in the original firmware;
- configurable MOD output for modulation wheel or key velocity in the original firmware;
- analogue glide on both pitch channels;
- 4 ppq MIDI clock output in the original design;
- high-accuracy MIDI-to-CV conversion;
- CV calibration.

MIDICVX keeps the same physical platform while repurposing and extending its firmware for live performance and arpeggiation.

The original Erica documentation specifies approximately:

```text
Pitch CV range:       0–8 V
Gate / clock level:   5 V
Maximum CV deviation: 0.001 V / 1 cent
Glide:                0–2 seconds
Width:                 6 HP
Depth:                 35 mm
```

---

# 6. LIVE mode in detail

LIVE is where to begin when checking a new build or learning the module.

Connect:

```text
MIDI keyboard
     │
     ▼
  MIDICVX
     │
     ├── CV1 ───► oscillator 1V/oct
     └── GATE1 ─► envelope gate
```

Play the keyboard. Pitch should track the MIDI notes and the yellow gate LED should follow the gate.

Turn GLIDE up and you will hear the pitch slide between notes. Turn it fully down for normal stepped pitch changes.

### LIVE with two voices

Move the physical switch to 2 VOICE and patch both CV/GATE pairs:

```text
                 ┌── CV1/GATE1 ─► Modular voice A
MIDI ─► MIDICVX ─┤
                 └── CV2/GATE2 ─► Modular voice B
```

This is useful for classic duophonic playing from one MIDI keyboard.

---

# 7. MONO ARP in detail — bass + arpeggio

MONO ARP is one of the major MIDICVX additions.

The idea is simple: **keep the low musical part as bass and arpeggiate the notes above it.**

Suppose you hold:

```text
C2 + C3 + E3 + G3
```

MIDICVX can treat the performance conceptually as:

```text
C2                  C3  E3  G3
│                    │   │   │
▼                    └───┴───┴──► ARP
BASS
```

Patch the bass CV/GATE pair to a bass oscillator/envelope and the ARP pair to a second synth voice. One MIDI hand position can now produce a bass foundation plus a moving arpeggio.

The lowest note is reserved from the main ARP pool in this performance concept, preventing the bass note from constantly reappearing as part of the upper arpeggio.

### Example patch

```text
                         ┌──► Bass oscillator → filter → VCA
MIDI chord ─► MIDICVX ───┤
                         └──► ARP oscillator  → filter → VCA
```

This works particularly well when the bass voice has a short amount of glide and the ARP voice has a brighter envelope.

---

# 8. DUAL ARP in detail

DUAL ARP is for using both CV/GATE channels as active arpeggiated voices.

Feed MIDICVX a larger chord from a keyboard or DAW and the firmware can divide the held musical material between its two output engines.

```text
Incoming MIDI notes
C2 G2 C3 E3 G3
       │
       ▼
    MIDICVX
    /     \
   ▼       ▼
ARP A     ARP B
   │       │
CV/GATE1 CV/GATE2
```

This is especially useful with two contrasting Eurorack voices. For example, patch one output to a low, rounded oscillator voice and the other to a bright pluck.

The status LED blinks whenever DUAL ARP is selected so you can distinguish it immediately from MONO ARP.

---

# 9. External clock and transport

MIDICVX can advance its ARP engine from an external clock signal. Each qualified low-to-high clock transition advances transport.

```text
MIDI chord ───────────────► MIDICVX
                               ▲
                               │
Modular clock / divider ───────┘
```

This allows the harmony to come from MIDI while the rhythm comes from the modular system.

### Example

Hold C–E–G on a MIDI keyboard and patch your master clock to MIDICVX. Each incoming pulse advances the arpeggio. Change the clock division and the same held chord becomes a slower or faster pattern without changing the notes.

---

# 10. Panic — clearing stuck notes

If a MIDI cable is disconnected at the wrong moment or an external device leaves a note hanging, hold **PROGRAM for about 8 seconds**.

MIDICVX performs an emergency runtime reset:

- both gates are forced low;
- active MIDI/playback note state is cleared;
- ARP engine state is reset;
- your selected mode, ARP algorithm, split and saved settings are retained;
- the status LED flashes three times to confirm the panic action.

This lets you recover without cycling power.

---

# 11. Calibration mode

MIDICVX retains the original power-up calibration path.

To enter it:

1. Power the module off.
2. Hold **PROGRAM**.
3. Power the module on while continuing to hold PROGRAM.
4. The status LED indicates calibration mode.

The original Erica calibration procedure uses a well-tracking 1 V/oct oscillator and chromatic tuner. CV1 is connected to the oscillator, and MIDI notes are used to correct scaling across octaves. PROGRAM advances/exits the calibration stages and the calibration is saved.

For a new build, calibration should be treated as an installation/service procedure rather than something you normally change during a performance.

---

# 12. Original Erica MIDI-channel and MOD configuration

For reference, the **stock Erica Synths firmware** allowed the user to hold PROGRAM for about two seconds, then select MIDI channel(s) from the keyboard. C represented channel 1, C# channel 2, D channel 3, and so on through channels 1–12. Two notes selected a two-channel setup. The next configuration stage selected whether MOD represented modulation-wheel CV or key velocity.

That is useful historical information when reading the original Erica manual, but **do not confuse that stock-firmware button map with MIDICVX v1.0.1**. MIDICVX uses the PROGRAM long-hold for its three performance modes, and the current v1.0.1 runtime button logic does not expose the original configuration gesture unchanged.

---

# 13. Gate LEDs and mode LED

The LEDs are useful enough that you can diagnose much of the module without a meter.

### Yellow LEDs

They follow the real gate outputs. If an ARP gate is firing, the corresponding yellow LED pulses. If the yellow LED is pulsing but your envelope is not responding, troubleshoot the patch after MIDICVX rather than the MIDI input.

At startup both yellow LEDs intentionally remain on during split learning.

### Status LED

Remember this simple code:

```text
OFF      LIVE
ON       MONO ARP
BLINK    DUAL ARP
```

A temporary series of 1–5 fast flashes indicates the selected ARP algorithm.

---

# 14. Patch recipes

### Classic monosynth

Use LIVE, 1 VOICE. Patch CV1 to oscillator pitch and GATE1 to an envelope. Add GLIDE to taste.

### Classic duophonic patch

Use LIVE, 2 VOICE. Patch both CV/GATE pairs to two complete synth voices.

### Bass + arpeggio

Select MONO ARP. Hold a low root plus several notes above it. Patch the bass output to a low voice and the ARP output to a second voice.

### Two-voice animated chord

Select DUAL ARP, hold four or five notes, and patch both output pairs to different voices. Try different ARP patterns with short PROGRAM presses.

### DAW harmony + modular rhythm

Send sustained chords from a DAW over MIDI, select an ARP mode, then clock MIDICVX from the modular rack. The DAW controls harmony while the rack controls rhythmic advancement.

### Order-played pattern

Select ORDER PLAYED and deliberately press chord tones in a non-scalar order. MIDICVX uses your performance order as the pattern source.

---

# 15. Firmware updates by WAV

Once the MIDICVX WAV bootloader is installed, application firmware can be updated without reconnecting an ISP programmer.

The release file is:

```text
firmware/MIDICVX_v1.0.1.wav
```

The WAV is **encoded firmware data**, not ordinary audio. Play the original file through a clean, wired audio path. Do not normalize, EQ, compress, time-stretch, add fades, convert it to MP3, or otherwise process it.

Allow the update to finish and the module to reboot before removing power.

If an update fails, retry with the untouched WAV and a clean playback path.

---

# 16. Initial programming and recovery with USBasp

A blank MCU, replacement MCU, or processor with a missing bootloader must first be programmed through ISP.

The repository contains two scripts:

```text
programmers/program_DIP_328P.sh
programmers/program_SMD_328.sh
```

They correspond to the two tested processor variants:

```text
DIP ATmega328P   signature 1E 95 0F
SMD ATmega328    signature 1E 95 14
```

The scripts check the device signature before writing so that the wrong target is not programmed accidentally.

The known working MIDICVX fuse configuration is:

```text
LFUSE = FF
HFUSE = D8
EFUSE = FD
```

The ISP installation writes the application and WAV bootloader. After that, normal application updates can be delivered by WAV.

---

# 17. What the release files are

```text
firmware/
├── midi_cv_x.hex          application firmware for ISP programming
└── MIDICVX_v1.0.1.wav     audio-encoded application update

bootloader/
└── midicvx_bootloader.hex WAV-update bootloader

programmers/
├── program_DIP_328P.sh    USBasp installer for DIP ATmega328P
└── program_SMD_328.sh     USBasp installer for SMD ATmega328
```

If your module already has the MIDICVX WAV bootloader, the WAV is normally all you need for an application update.

---

# 18. Troubleshooting

### I don't know which performance mode I'm in

Look at the status LED: **off = LIVE, steady = MONO ARP, blinking = DUAL ARP**.

### I can't get into the next mode

Hold PROGRAM for roughly 0.7 second. Do not just tap it. One completed long hold advances one mode.

### A short PROGRAM press does nothing

That is normal in LIVE. Short presses select ARP algorithms only while an ARP mode is active.

### The yellow LEDs are both on immediately after startup

That is split learning, not a fault. Play the desired split note, or wait about two seconds to retain the stored split.

### Split learning becomes too short when clock is connected

That was a development regression. v1.0.1 suppresses transport processing during the startup learning window. Confirm that the installed application is the current v1.0.1 build.

### ARP is not moving

Confirm that an ARP mode is selected and notes are held. If you are using external transport, verify that a valid clock reaches the input.

### A note or gate is stuck

Hold PROGRAM for about eight seconds for PANIC.

### Gate LED flashes but my synth voice does not sound

MIDICVX is commanding the gate. Check the patch cable, envelope, VCA and oscillator path after the module.

### WAV update will not take

Use the untouched WAV, a wired playback path and no audio processing. Avoid Bluetooth and any software that may resample, normalize or alter the file.

### MCU is completely unresponsive

Use USBasp/ISP with the correct DIP or SMD programming script and verify the processor signature before writing.

---

# 19. MIDICVX in one picture

```text
                           PROGRAM
                     long hold cycles modes
                              │
                              ▼
MIDI ─────────────────► ┌─────────────┐ ◄──────── External clock
                        │   MIDICVX   │
                        │             │
                        │ LIVE        │
                        │ MONO ARP    │
                        │ DUAL ARP    │
                        └──────┬──────┘
                               │
                     ┌─────────┴─────────┐
                     ▼                   ▼
                 CV1 / GATE1         CV2 / GATE2
                     │                   │
                     ▼                   ▼
                Modular voice A     Modular voice B
```

---

# 20. Five-minute first session

1. Connect MIDI IN to a keyboard.
2. Patch CV1/GATE1 to a synth voice.
3. Power up. While both yellow LEDs are on, either play your desired split note or wait to keep the stored split.
4. You begin in **LIVE**. Confirm normal pitch and gate response.
5. Hold PROGRAM about 0.7 second: **MONO ARP**. The status LED becomes steady.
6. Hold several notes and listen to the ARP/bass behavior.
7. Tap PROGRAM to try the five ARP patterns; count the LED flashes.
8. Hold PROGRAM again: **DUAL ARP**. The status LED now blinks.
9. Patch the second CV/GATE pair to another synth voice.
10. Add external clock if desired.
11. Hold PROGRAM once more to return to **LIVE**.

If you remember only one thing, remember this:

```text
                 PROGRAM ~0.7 s

LIVE  ─────────►  MONO ARP  ─────────►  DUAL ARP  ─────────►  LIVE
OFF LED            STEADY LED             BLINKING LED
```

---

## Project lineage

MIDICVX is based on the open-source Erica Synths DIY MIDI-CV platform. Erica Synths' original documentation remains the reference for the physical hardware, assembly, electrical specifications and original firmware behavior. MIDICVX adds and changes the performance firmware behavior described in this manual.