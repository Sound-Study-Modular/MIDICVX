#!/bin/bash
DIR="$(cd "$(dirname "$0")" && pwd)"
exec "$DIR/program_common.sh" m328 "1E 95 14" "SMD ATmega328"
