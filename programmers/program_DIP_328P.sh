#!/bin/bash
DIR="$(cd "$(dirname "$0")" && pwd)"
exec "$DIR/program_common.sh" m328p "1E 95 0F" "DIP ATmega328P"
