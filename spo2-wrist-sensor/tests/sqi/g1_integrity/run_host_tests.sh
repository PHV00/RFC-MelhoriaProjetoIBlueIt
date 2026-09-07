#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
OUT="$(mktemp /tmp/g1_integrity_test.XXXXXX)"
trap 'rm -f "$OUT"' EXIT

cc \
  -std=c11 \
  -Wall -Wextra -Werror -pedantic \
  -I "$ROOT_DIR/main" \
  "$ROOT_DIR/main/processing/sqi/gates/g1_integrity/gate_integrity.c" \
  "$ROOT_DIR/tests/sqi/g1_integrity/test_gate_integrity.c" \
  -o "$OUT"

"$OUT"
