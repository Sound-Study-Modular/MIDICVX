#!/bin/bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
APP="$ROOT/firmware/midi_cv_x.hex"
BOOT="$ROOT/bootloader/midicvx_bootloader.hex"

echo "========================================"
echo " MIDICVX SMD ATmega328 PROGRAMMER"
echo "========================================"

[[ -f "$APP" ]] || { echo "ERROR: Missing $APP"; exit 1; }
[[ -f "$BOOT" ]] || { echo "ERROR: Missing $BOOT"; exit 1; }

echo
echo "Checking MCU signature..."

SIG=$(avrdude -c usbasp -p m328 -B 100 -v 2>&1 |
      sed -n 's/.*Device signature = \([0-9A-Fa-f ]*\).*/\1/p' |
      head -1 |
      tr '[:lower:]' '[:upper:]' |
      sed 's/[[:space:]]*$//')

echo "Detected: $SIG"

if [[ "$SIG" != "1E 95 14" ]]; then
    echo
    echo "ERROR: Expected SMD ATmega328 signature 1E 95 14."
    echo "Nothing has been written."
    exit 1
fi

echo
echo "Correct ATmega328 detected."
echo "Programming fuses, application, and WAV bootloader..."

avrdude -c usbasp -p m328 -B 100 \
    -U lfuse:w:0xff:m \
    -U hfuse:w:0xd8:m \
    -U efuse:w:0xfd:m \
    -U flash:w:"$APP":i \
    -U flash:w:"$BOOT":i

echo
echo "========================================"
echo " SUCCESS - SMD ATmega328 programmed"
echo " Fuses: FF / D8 / FD"
echo " WAV bootloader installed"
echo "========================================"
