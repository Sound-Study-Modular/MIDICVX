# MIDICVX v1.0.1 — User Manual

MIDICVX is an expanded open-source firmware for the Erica Synths DIY MIDI-CV hardware. It keeps the useful core of the original module — MIDI-to-CV conversion, two CV/GATE voices, glide, configurable MIDI operation and clock-related functionality — and adds a performance-oriented arpeggiator, two-voice ARP routing, a dedicated bass voice, external transport clocking, persistent settings, startup split learning, and audio/WAV firmware updates.

> **Quick idea:** MIDI comes in; MIDICVX decides which notes belong to each voice; the module sends pitch CV and gates to your Eurorack voices.

## 1. What the hardware does

The original Erica Synths DIY MIDI-CV is a compact duophonic MIDI interface. Erica Synths describes the original hardware as supporting two CV/GATE outputs, one- or two-channel MIDI configuration, analogue glide on both channels, a configurable modulation output, and MIDI clock functionality.

MIDICVX uses that same hardware as a much more performance-oriented MIDI/CV brain.

### Main connections

```text
MIDI keyboard / DAW / sequencer
              │
              ▼
        ┌─────────────┐
        │   MIDICVX   │
        └─────────────┘
          │         │
       CV1/GATE1  CV2/GATE2
          │         │
          ▼         ▼
       Voice 1    Voice 2
```

Pitch CV is intended for normal 1 V/oct Eurorack oscillators. Gate outputs tell envelopes, synth voices, etc. when a note is active.

## 2. Original Erica Synths functionality retained

The underlying module was designed to emulate traditional monophonic and duophonic synthesizer behaviour. Its hardware provides:

- two pitch CV outputs;
- two GATE outputs;
- one-voice and two-voice operation;
- MIDI input;
- analogue GLIDE for both pitch channels;
- MIDI clock-related I/O on the hardware;
- configurable MIDI operation.

The original Erica specification lists a 0–8 V CV range, 5 V gate/clock level, and approximately 0–2 seconds of analogue glide.

Because glide is analogue, it remains immediately useful with MIDICVX: turn it up for portamento between MIDI notes and down for sharply stepped pitch changes.

## 3. LIVE mode

LIVE mode is the straightforward keyboard-to-CV mode and preserves the familiar MIDI-CV behaviour.

Play your MIDI keyboard and the module immediately translates incoming notes into pitch CV and gates. This is the mode to use when you want to play the modular normally rather than have MIDICVX generate an arpeggio.

### One-voice example

```text
Keyboard:     C3 ───── E3 ───── G3
                 MIDI
                   │
                   ▼
MIDICVX:      CV + GATE
                   │
                   ▼
Oscillator → Filter → VCA
```

### Two-voice example

With two voices available, a chord can be divided across the two CV/GATE pairs so two independent Eurorack voices can be played from the same MIDI performance.

```text
             MIDI notes
           C3  E3  G3  C4
                 │
            ┌────┴────┐
            ▼         ▼
         CV/GATE 1  CV/GATE 2
            │         │
         Synth A    Synth B
```

The physical voice switch selects the appropriate one-/two-voice behaviour. MIDICVX also rebuilds held LIVE notes when the voice configuration changes, so switching modes while playing behaves sensibly.

## 4. ARP mode

MIDICVX adds a full arpeggiator layer while keeping LIVE mode available.

### Entering and leaving ARP

Use a **long press of the PROGRAM button** to toggle between LIVE and ARP operation.

- **LIVE:** normal MIDI-to-CV playing.
- **ARP:** held MIDI notes feed the arpeggiator.

The status LED provides feedback during the hold so you do not have to guess when the long-press threshold has been reached.

### Selecting an arpeggiator pattern

While ARP is active, use a **short PROGRAM press** to move through the available arpeggiator algorithms.

MIDICVX preserves the project's five arpeggiator algorithms. Changing the pattern changes the order in which the held notes are visited; it does not require you to release the chord first.

### Simple ARP example

Hold:

```text
C3 + E3 + G3 + B3
```

An upward pattern produces the musical idea:

```text
C3 → E3 → G3 → B3 → C3 → E3 → ...
```

Another ARP algorithm may traverse those same held notes in a different order.

## 5. External clock / transport

MIDICVX can advance the arpeggiator from the module's external clock/transport input rather than relying only on MIDI note timing.

A qualified LOW-to-HIGH clock edge advances transport. This makes it practical to synchronize MIDICVX to a Eurorack clock source, sequencer or other rhythmic system.

```text
Master Clock ─────► MIDICVX clock input
                       │
MIDI chord ───────────►│
                       ▼
                 clocked arpeggio
```

### Example

Patch a clock divider or sequencer clock to MIDICVX and hold C–E–G on a MIDI keyboard. Each incoming clock pulse advances the ARP by one transport step. Change the clock rate and the arpeggio follows it.

## 6. Split learning at startup

One of MIDICVX's most useful additions is its learned split point.

At normal startup the module enters a short split-learning window. The **first valid MIDI Note On** received during this window becomes the selected lowest ARP note / split reference. The learned value is then retained by the firmware's settings system.

### Visual feedback

The two yellow gate LEDs provide the startup learn indication. Once a valid note is received, the module acknowledges the selection and leaves learn mode.

### Example

Suppose you want notes below C3 to act as your bass region and C3 upward to feed the arpeggiator. During startup learning, play the desired boundary note. MIDICVX uses that learned point for subsequent note routing.

### External clock at startup

MIDICVX v1.0.1 specifically protects the split-learning period from incoming external clock activity. Earlier development builds could allow clock transport to interfere with the yellow learn indication, making the learning window appear extremely short when a clock cable was already connected at power-up. v1.0.1 discards those transport ticks while split learning is active.

You can therefore leave the clock patched when powering the system on.

## 7. One-voice ARP + dedicated bass

This is one of the major MIDICVX performance features.

In one-voice ARP operation, the main output arpeggiates the upper musical material while the second CV/GATE pair acts as a LIVE-style bass voice using the lowest held note.

Conceptually:

```text
Held notes:  C2   G2   C3   E3   G3
             │              │    │
             │              └────┴──► ARP voice
             │
             └───────────────────────► Bass voice
```

The lowest note is reserved for bass rather than also appearing in the main arpeggio.

### Musical example

Hold:

```text
C2 + C3 + E3 + G3
```

Patch the bass CV/GATE pair to a bass oscillator and envelope, and the ARP pair to a brighter synth voice. C2 can sustain/follow your playing as the bass while C3/E3/G3 form the arpeggio.

This lets one MIDI keyboard performance create something much closer to a complete two-part modular arrangement.

## 8. Two-voice ARP

Two-voice ARP mode expands the idea further by distributing incoming musical material between both CV/GATE voices rather than simply duplicating one arpeggio.

MIDICVX can accept multiple held notes — including chords sent from a DAW — and divide them into lower and upper musical groups for the two outputs. This makes dense MIDI material useful with a pair of monophonic Eurorack voices.

```text
Incoming chord
C2 G2 C3 E3 G3
      │
      ▼
  MIDICVX split
   │         │
 lower      upper
 notes      notes
   │         │
CV/GATE    CV/GATE
 voice A    voice B
```

The actual output assignment follows the firmware's current voice-routing rules, while the important musical concept is that the two voices receive separate note pools instead of merely mirroring each other.

## 9. Gate LEDs

The yellow LEDs represent the actual gate activity of the corresponding outputs.

In LIVE mode they behave like normal MIDI gate indicators. In ARP operation they pulse with the generated arpeggiated gates rather than simply staying illuminated for as long as the original MIDI keys remain held.

That makes the LEDs useful diagnostics: if you can see a gate pulse, MIDICVX is actually commanding that output to fire.

## 10. Panic / emergency note reset

If notes or gates ever become stuck because of a disconnected MIDI cable, unusual MIDI source behaviour, or experimentation, MIDICVX includes an emergency reset gesture.

**Hold PROGRAM for approximately eight seconds** during normal operation to invoke the panic/reset behaviour. This clears active note/gate state so you can continue without power-cycling the rack.

## 11. Using glide

The GLIDE control is analogue and works after MIDI has been converted to pitch CV.

At minimum glide, pitches change almost immediately:

```text
C ┌─────┐
  │     └───── G
```

With glide increased, the voltage travels smoothly between pitches:

```text
C ─────╱───── G
```

Try a small amount on the bass voice for classic monosynth phrasing, or a longer setting on an ARP voice for sliding sequences.

## 12. Patch recipes

### Classic monosynth

MIDI keyboard → MIDICVX → CV1 to oscillator pitch, GATE1 to envelope gate. Keep ARP off and use GLIDE to taste.

### Bass + arpeggio

Patch one CV/GATE pair to a bass voice and the ARP output to a second oscillator/voice. Hold a low root plus a chord above it. The lowest note becomes the bass while the upper notes animate the second voice.

### DAW chord animator

Send MIDI chords from your DAW into MIDICVX. Enable ARP and clock the module from your modular master clock. The DAW supplies harmony while Eurorack supplies the rhythm.

### Clock-divider experiment

Send the same master clock through different divisions before MIDICVX. A slower division creates spacious arpeggios; faster divisions create rapid patterns without changing the MIDI chord.

## 13. Firmware updates by WAV audio

Once the MIDICVX WAV bootloader has been installed, normal firmware updates no longer require opening the case and attaching an AVR ISP programmer.

The release firmware includes:

```text
firmware/MIDICVX_v1.0.1.wav
```

Play the update WAV into the bootloader using a clean, unprocessed audio signal. Do **not** normalize, EQ, compress, time-stretch, convert to a lossy format, add fades, or otherwise alter the file. The WAV is encoded data, not music.

Use a reliable playback device and begin with a sensible strong output level. Follow the bootloader's LED feedback through the transfer. Allow the module to complete the update and reboot before interrupting power.

If an audio update fails, simply return to the bootloader and retry with the original WAV file and a clean signal path.

## 14. Initial programming with USBasp

A blank/replacement ATmega requires ISP programming before WAV updates are available.

The repository includes two scripts:

```text
programmers/program_DIP_328P.sh
programmers/program_SMD_328.sh
```

They are intentionally separate because supported boards use two MCU variants:

- DIP **ATmega328P**, signature `1E 95 0F`;
- SMD **ATmega328**, signature `1E 95 14`.

The scripts verify the processor signature before writing. This is an important safeguard against accidentally programming the wrong target.

The known working MIDICVX fuse configuration used for the finished installation is:

```text
LFUSE = FF
HFUSE = D8
EFUSE = FD
```

The programming process installs both the application firmware and the WAV bootloader. Once that has succeeded, future application updates can normally be delivered by WAV.

## 15. What each firmware file is for

```text
firmware/
├── midi_cv_x.hex          AVR application firmware
└── MIDICVX_v1.0.1.wav     audio-encoded firmware update

bootloader/
└── midicvx_bootloader.hex WAV-update bootloader

programmers/
├── program_DIP_328P.sh    USBasp installer for ATmega328P
└── program_SMD_328.sh     USBasp installer for ATmega328
```

If the module already has the MIDICVX bootloader, most users only need the WAV.

If you are building a module from scratch, replacing the MCU, or recovering a processor whose bootloader has been erased, use the appropriate USBasp programming script.

## 16. Troubleshooting

### MIDI notes do nothing

Check MIDI cabling and MIDI source configuration first. Then verify that a gate LED responds. If the gate LED responds but the synth does not, inspect the CV/GATE patch between MIDICVX and the voice.

### ARP is not moving

Confirm that ARP mode is active and that notes are held. If using external transport, confirm that a valid clock is reaching the clock input.

### Bass plays the wrong musical region

Recheck the learned startup split point and the notes you are holding. Remember that the dedicated bass behaviour is based on the lower/lowest-note routing rules.

### Yellow LEDs disappear almost instantly during startup

That was a development regression associated with an already-running external clock. The v1.0.1 startup clock fix prevents transport processing from stealing the split-learning gate/LED indication. Make sure the installed application is the current v1.0.1 build.

### WAV update will not take

Use the original unmodified WAV, disable audio enhancements, and retry with a clean wired audio path. Avoid Bluetooth, streaming services, sample-rate effects, crossfades and other processing that can alter the encoded waveform.

### Completely unresponsive MCU

Use USBasp/ISP and the appropriate DIP or SMD programming script. Confirm the processor signature before allowing any write operation.

## 17. MIDICVX in one picture

```text
                       ┌──────────────────────┐
MIDI keyboard / DAW ──►│       MIDICVX        │◄── External clock
                       │                      │
                       │ LIVE      ARP        │
                       │  │         │         │
                       │  │    ┌────┴────┐    │
                       │  │   bass     arp    │
                       │  │    /     split    │
                       └──┼────┼──────┼───────┘
                          │    │      │
                          ▼    ▼      ▼
                       CV/GATE outputs
                          │    │
                          ▼    ▼
                      Modular voices
```

## 18. A good first session

1. Connect MIDI IN to a keyboard or DAW.
2. Connect CV1/GATE1 to one synth voice.
3. Power on and choose the desired split note during the startup learn indication.
4. Play in LIVE mode and confirm pitch, gate and glide.
5. Long-press PROGRAM to enter ARP.
6. Hold three or four notes.
7. Short-press PROGRAM to hear the different ARP algorithms.
8. Connect an external clock and listen to the arpeggio lock to your modular rhythm.
9. Add the second CV/GATE voice and try bass + ARP or two-voice operation.

Once those steps make sense, the module becomes very immediate: **LIVE for direct playing; ARP for generated movement; split/bass routing for two-part performances; external clock for modular synchronization.**

---

## Project lineage

MIDICVX is based on the open-source Erica Synths DIY MIDI-CV platform. Erica Synths' original design and documentation remain the reference for the physical hardware, assembly and original electrical specifications. MIDICVX adds the firmware behaviour described in this manual.