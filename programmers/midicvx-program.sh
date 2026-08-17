#!/bin/bash
set -euo pipefail

REPO="Sound-Study-Modular/MIDICVX"
RELEASE_TAG="midicvx-production-2026-08-13"
BASE="https://github.com/${REPO}/releases/download/${RELEASE_TAG}"
APP_NAME="midi_cv_x.hex"
BOOT_NAME="midicvx_bootloader.hex"

TMPDIR_MIDICVX="$(mktemp -d "${TMPDIR:-/tmp}/midicvx-program.XXXXXX")"
trap 'rm -rf "$TMPDIR_MIDICVX"' EXIT

APP="$TMPDIR_MIDICVX/$APP_NAME"
BOOT="$TMPDIR_MIDICVX/$BOOT_NAME"

echo "========================================"
echo " MIDICVX PRODUCTION PROGRAMMER"
echo " Release: $RELEASE_TAG"
echo "========================================"

echo
if ! command -v avrdude >/dev/null 2>&1; then
    echo "ERROR: avrdude is not installed or not in PATH."
    echo "On macOS with Homebrew: brew install avrdude"
    exit 1
fi

if command -v curl >/dev/null 2>&1; then
    FETCH=(curl -fL --retry 3 --connect-timeout 15 -o)
elif command -v wget >/dev/null 2>&1; then
    FETCH=(wget -O)
else
    echo "ERROR: curl or wget is required to download the production files."
    exit 1
fi

echo "Downloading hardware-tested production files from GitHub..."
"${FETCH[@]}" "$APP" "$BASE/$APP_NAME"
"${FETCH[@]}" "$BOOT" "$BASE/$BOOT_NAME"

[[ -s "$APP" ]] || { echo "ERROR: Application download is empty."; exit 1; }
[[ -s "$BOOT" ]] || { echo "ERROR: Bootloader download is empty."; exit 1; }

echo "Downloads complete."
echo
echo "Detecting MCU..."

# ATmega328P and ATmega328 have different avrdude part IDs. Probe without
# writing anything. A successful signature determines the programming path.
if avrdude -c usbasp -p m328p -B 100 -n >/dev/null 2>&1; then
    PART="m328p"
    EXPECTED="1E 95 0F"
    LABEL="ATmega328P (DIP or SMD)"
elif avrdude -c usbasp -p m328 -B 100 -n >/dev/null 2>&1; then
    PART="m328"
    EXPECTED="1E 95 14"
    LABEL="ATmega328 (non-P)"
else
    echo "ERROR: No supported MIDICVX MCU detected."
    echo "Expected ATmega328P signature 1E 95 0F or ATmega328 signature 1E 95 14."
    echo "Nothing has been written."
    exit 1
fi

# Read verbose output as a second explicit signature check before writing.
SIG=$(avrdude -c usbasp -p "$PART" -B 100 -v 2>&1 |
      sed -n 's/.*Device signature = \([0-9A-Fa-f ]*\).*/\1/p' |
      head -1 |
      tr '[:lower:]' '[:upper:]' |
      sed 's/[[:space:]]*$//')

echo "Detected: $LABEL"
echo "Signature: $SIG"

if [[ "$SIG" != "$EXPECTED" ]]; then
    echo "ERROR: Signature check failed. Expected $EXPECTED."
    echo "Nothing has been written."
    exit 1
fi

echo
echo "Programming MIDICVX v1.0.1 production firmware..."
echo "  Application: $APP_NAME"
echo "  Bootloader:  $BOOT_NAME"
echo "  Fuses:       FF / D8 / FD"
echo

# One avrdude invocation performs the erase/program/verify cycle while
# installing both application and bootloader plus the production fuse values.
avrdude -c usbasp -p "$PART" -B 100 \
    -U lfuse:w:0xff:m \
    -U hfuse:w:0xd8:m \
    -U efuse:w:0xfd:m \
    -U flash:w:"$APP":i \
    -U flash:w:"$BOOT":i

echo
echo "========================================"
echo " SUCCESS"
echo " $LABEL programmed and verified."
echo " MIDICVX v1.0.1 application installed."
echo " Production WAV bootloader installed."
echo "========================================"
