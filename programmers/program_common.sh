#!/bin/bash
set -euo pipefail

PART="$1"
EXPECTED_SIG="$2"
LABEL="$3"

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
APP="$ROOT/build/midi_cv_x.hex"
BOOT="$ROOT/bootloader/midicvx_bootloader.hex"

echo "MIDICVX PRODUCTION PROGRAMMER"
echo "MCU: $LABEL"
echo

[[ -f "$APP" ]] || { echo "ERROR: Missing $APP"; exit 1; }
[[ -f "$BOOT" ]] || { echo "ERROR: Missing $BOOT"; exit 1; }

echo "Checking MCU signature..."

SIG=$(avrdude -c usbasp -p "$PART" -B 100 -v 2>&1 |
    sed -n 's/.*Device signature = \([0-9A-Fa-f ]*\).*/\1/p' |
    head -1 |
    tr '[:lower:]' '[:upper:]' |
    sed 's/[[:space:]]*$//')

echo "Detected: $SIG"
echo "Expected: $EXPECTED_SIG"

if [[ "$SIG" != "$EXPECTED_SIG" ]]; then
    echo "ERROR: WRONG MCU. Nothing written."
    exit 1
fi

echo
echo "Programming application + production WAV bootloader..."

avrdude -c usbasp -p "$PART" -B 100 \
  -U flash:w:"$APP":i

avrdude -c usbasp -p "$PART" -B 100 \
  -U flash:w:"$BOOT":i

echo
echo "Programming production fuses FF / D8 / FD..."

avrdude -c usbasp -p "$PART" -B 100 \
  -U lfuse:w:0xff:m \
  -U hfuse:w:0xd8:m \
  -U efuse:w:0xfd:m

echo
echo "SUCCESS: $LABEL programmed."
