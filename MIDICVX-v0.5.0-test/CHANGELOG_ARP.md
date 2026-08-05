# MIDICVX v0.3.0

- Adds a real held-note bitmap for all 128 MIDI notes.
- Adds LIVE and ARP UP playback modes.
- Short Program-button press toggles LIVE/ARP UP.
- Preserves original 3-second configuration hold and power-up calibration.
- ARP UP advances once per six MIDI Clock bytes after MIDI START.
- ARP uses existing calibrated CV/gate output routines.
- Supports `make MCU=atmega328` and `make MCU=atmega328p`.
