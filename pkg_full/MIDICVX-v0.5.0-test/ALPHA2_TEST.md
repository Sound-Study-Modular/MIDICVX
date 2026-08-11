# MIDICVX v0.3.0-alpha2 hardware test

This build adds the first functional arpeggiator test.

## Build for the DIP ATmega328P

```sh
make clean
make MCU=atmega328p
```

## Flash the DIP ATmega328P

```sh
avrdude -c usbasp -p m328p -B 100 -U flash:w:build/midi_cv_x.hex:i
```

## Controls

- Short Program-button press: toggle LIVE / ARP UP.
- MIDI LED off: LIVE mode.
- MIDI LED continuously on: ARP UP mode.
- Long Program-button hold: original configuration mode.
- Hold Program during power-up: original calibration mode.

## Arpeggiator test

1. Enter ARP UP mode with a short Program-button press.
2. Hold two or more MIDI notes.
3. Send MIDI START, then MIDI Clock.
4. The arp advances once for every six MIDI clock messages (16th-note rate).
5. CV1 and CV2 output the same arpeggiated pitch; Gate 1 and Gate 2 retrigger together.

This alpha uses MIDI Clock only. External clock input is planned for a later build.
