#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
OUT="$(mktemp /tmp/g1_integrity_test.XXXXXX)"
trap 'rm -f "$OUT"' EXIT

# ESP-IDF may prepend cross-toolchain directories to PATH. A native host GCC can
# then accidentally invoke a RISC-V/Xtensa assembler named `as`, which rejects
# the x86_64 `--64` flag. Force the native host compiler/binutils for this test.
HOST_CC="${HOST_CC:-/usr/bin/cc}"
HOST_BIN_DIR="${HOST_BIN_DIR:-/usr/bin}"

if [[ ! -x "$HOST_CC" ]]; then
  echo "ERROR: host C compiler not found at $HOST_CC" >&2
  echo "Set HOST_CC=/path/to/native/cc and retry." >&2
  exit 2
fi
if [[ ! -x "$HOST_BIN_DIR/as" ]]; then
  echo "ERROR: host assembler not found at $HOST_BIN_DIR/as" >&2
  echo "Set HOST_BIN_DIR=/path/to/native/binutils and retry." >&2
  exit 2
fi

echo "Host compiler: $HOST_CC"
echo "Host assembler: $HOST_BIN_DIR/as"

PATH="$HOST_BIN_DIR:/bin:$PATH" "$HOST_CC" \
  -B"$HOST_BIN_DIR/" \
  -std=c11 \
  -Wall -Wextra -Werror -pedantic \
  -I "$ROOT_DIR/main" \
  "$ROOT_DIR/main/processing/sqi/gates/g1_integrity/gate_integrity.c" \
  "$ROOT_DIR/tests/sqi/g1_integrity/test_gate_integrity.c" \
  -o "$OUT"

"$OUT"
