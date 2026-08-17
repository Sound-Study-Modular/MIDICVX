#!/bin/bash
set -euo pipefail

REPO="Sound-Study-Modular/MIDICVX"
TAG="midicvx-production-2026-08-13"
SCRIPT_REF="cea5b9103f967d8b71b90ec612ea12a6347c4e26"
OUT="${1:-$HOME/Desktop/MIDICVX_v1.0.1_PROGRAMMING_PACKAGE_CLEAN}"
RELEASE_BASE="https://github.com/${REPO}/releases/download/${TAG}"
RAW_BASE="https://raw.githubusercontent.com/${REPO}/${SCRIPT_REF}"

need() { command -v "$1" >/dev/null 2>&1 || { echo "ERROR: $1 is required"; exit 1; }; }
need curl
need shasum

if [[ -e "$OUT" ]]; then
  echo "ERROR: Destination already exists: $OUT"
  echo "Remove or rename it first; this builder will not overwrite a package."
  exit 1
fi

mkdir -p "$OUT/firmware" "$OUT/bootloader" "$OUT/programmers" "$OUT/docs"

echo "Building clean MIDICVX v1.0.1 programming package"
echo "Production firmware tag: $TAG"
echo "Programming-script ref: $SCRIPT_REF"
echo "Destination: $OUT"
echo

fetch() {
  local url="$1" dest="$2"
  echo "Downloading $(basename "$dest")..."
  curl -fL --retry 3 --connect-timeout 15 "$url" -o "$dest"
  [[ -s "$dest" ]] || { echo "ERROR: Empty download: $dest"; exit 1; }
}

# Hardware-tested release binaries. These are deliberately pinned to the
# production GitHub release rather than main.
fetch "$RELEASE_BASE/midi_cv_x.hex" "$OUT/firmware/midi_cv_x.hex"
fetch "$RELEASE_BASE/midicvx_bootloader.hex" "$OUT/bootloader/midicvx_bootloader.hex"
fetch "$RELEASE_BASE/MIDICVX_v1.0.1.wav" "$OUT/firmware/MIDICVX_v1.0.1.wav"
fetch "$RELEASE_BASE/PROGRAMMING_AND_UPDATES.md" "$OUT/docs/PROGRAMMING_AND_UPDATES.md"

# Pin the manufacturing scripts to the known repository commit used to build
# this package. Do not silently follow future changes on main.
for f in program_DIP_328P.sh program_SMD_328P.sh program_DIP_328.sh program_SMD_328.sh midicvx-program.sh; do
  fetch "$RAW_BASE/programmers/$f" "$OUT/programmers/$f"
done
chmod +x "$OUT/programmers/"*.sh

# Basic integrity/sanity checks before declaring the package complete.
grep -q '^:' "$OUT/firmware/midi_cv_x.hex" || { echo "ERROR: application is not Intel HEX"; exit 1; }
grep -q '^:' "$OUT/bootloader/midicvx_bootloader.hex" || { echo "ERROR: bootloader is not Intel HEX"; exit 1; }
grep -q 'APP=.*firmware/midi_cv_x.hex' "$OUT/programmers/program_DIP_328P.sh" || { echo "ERROR: 328P script path check failed"; exit 1; }
grep -q 'APP=.*firmware/midi_cv_x.hex' "$OUT/programmers/program_SMD_328.sh" || { echo "ERROR: 328 script path check failed"; exit 1; }

cat > "$OUT/README_FIRST.txt" <<'EOF'
MIDICVX v1.0.1 CLEAN PRODUCTION PROGRAMMING PACKAGE

Production firmware source:
  GitHub release midicvx-production-2026-08-13

Use:
  programmers/program_DIP_328P.sh   - ATmega328P, signature 1E 95 0F
  programmers/program_SMD_328.sh    - non-P ATmega328, signature 1E 95 14
  programmers/midicvx-program.sh    - automatic MCU detection; downloads the same pinned production release

Production fuses:
  LFUSE FF
  HFUSE D8
  EFUSE FD

After ISP programming, WAV update mode is entered with PROGRAM held at power-on while the switch is in 2 VOICE.
Calibration is PROGRAM held at power-on while the switch is in 1 VOICE.
EOF

(
  cd "$OUT"
  shasum -a 256 \
    firmware/midi_cv_x.hex \
    bootloader/midicvx_bootloader.hex \
    firmware/MIDICVX_v1.0.1.wav \
    programmers/*.sh \
    docs/PROGRAMMING_AND_UPDATES.md > SHA256SUMS.txt
)

echo
echo "=== PACKAGE VERIFIED ==="
find "$OUT" -type f -maxdepth 2 -print | sort
echo
echo "SHA-256 manifest: $OUT/SHA256SUMS.txt"
echo "Clean package ready: $OUT"
