#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
BUILD_DIR="${TMPDIR:-/tmp}/sqi_g2_host_tests"
mkdir -p "$BUILD_DIR"

HOST_CC="/usr/bin/cc"
HOST_AS="/usr/bin/as"

if [[ ! -x "$HOST_CC" || ! -x "$HOST_AS" ]]; then
  echo "Host toolchain not found at /usr/bin/cc and /usr/bin/as" >&2
  exit 1
fi

echo "Host compiler: $HOST_CC"
echo "Host assembler: $HOST_AS"

COMMON_FLAGS=(
  -std=c11 -Wall -Wextra -Werror -pedantic
  -I"$ROOT_DIR/main"
)

PATH=/usr/bin:/bin "$HOST_CC" -B/usr/bin/ \
  "${COMMON_FLAGS[@]}" \
  "$ROOT_DIR/tests/sqi/g2_pulsatility/test_g2_pulsatility.c" \
  "$ROOT_DIR/main/processing/sqi/preprocess/ppg_preprocess.c" \
  "$ROOT_DIR/main/processing/sqi/features/threshold_crossing.c" \
  "$ROOT_DIR/main/processing/sqi/features/autocorrelation.c" \
  "$ROOT_DIR/main/processing/sqi/gates/g2_pulsatility/gate_pulsatility.c" \
  -lm \
  -o "$BUILD_DIR/test_g2_pulsatility"

"$BUILD_DIR/test_g2_pulsatility"

PATH=/usr/bin:/bin "$HOST_CC" -B/usr/bin/ \
  "${COMMON_FLAGS[@]}" \
  "$ROOT_DIR/tests/sqi/g2_pulsatility/test_preprocess_frequency_response.c" \
  "$ROOT_DIR/main/processing/sqi/preprocess/ppg_preprocess.c" \
  -lm \
  -o "$BUILD_DIR/test_preprocess_frequency_response"

"$BUILD_DIR/test_preprocess_frequency_response"

PATH=/usr/bin:/bin "$HOST_CC" -B/usr/bin/ \
  "${COMMON_FLAGS[@]}" \
  "$ROOT_DIR/tests/sqi/g2_pulsatility/test_autocorrelation_equivalence.c" \
  "$ROOT_DIR/main/processing/sqi/features/autocorrelation.c" \
  -lm \
  -o "$BUILD_DIR/test_autocorrelation_equivalence"

"$BUILD_DIR/test_autocorrelation_equivalence"

PATH=/usr/bin:/bin "$HOST_CC" -B/usr/bin/ \
  "${COMMON_FLAGS[@]}" \
  "$ROOT_DIR/tests/sqi/g2_pulsatility/test_autocorrelation_vadrevu.c" \
  "$ROOT_DIR/main/processing/sqi/features/autocorrelation.c" \
  -lm \
  -o "$BUILD_DIR/test_autocorrelation_vadrevu"

"$BUILD_DIR/test_autocorrelation_vadrevu"
