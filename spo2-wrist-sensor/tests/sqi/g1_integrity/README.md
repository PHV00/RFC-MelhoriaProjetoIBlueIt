# G1 final validation

This directory closes the functional validation of `G1_INTEGRITY` without changing the production branch.

## 1. Host unit tests

Run from `spo2-wrist-sensor/`:

```bash
./tests/sqi/g1_integrity/run_host_tests.sh
```

Expected output:

```text
G1 integrity tests: PASS (18 cases)
```

The suite covers clean input, no-signal, channel isolation, flatline, clipping on RED/IR, exact threshold boundaries, continuity boundaries, duplicated timestamps, invalid configuration and input immutability.

## 2. Integration fault injection on ESP32-C3

The validation branch adds `SQI_G1_FAULT_MODE` as a CMake cache variable. Mode `0` leaves real MAX30102 samples untouched. Modes `1-4` inject deterministic faults in `ppg_sampler.c` before samples enter `SampleBuffer`, exercising the complete path `acquisition -> G1 -> fail-fast -> telemetry`.

| Value | Mode | Expected G1 result |
|---:|---|---|
| 0 | OFF | real sensor |
| 1 | FLATLINE_BOTH | `FLATLINE_RED`, mask contains 24 |
| 2 | CLIPPING_RED | `CLIPPING_RED`, mask contains 32 |
| 3 | CLIPPING_IR | `CLIPPING_IR`, mask contains 64 |
| 4 | DISCONTINUITY | `DISCONTINUITY`, mask contains 1 |

For every injected failure, `hr.valid` and `spo2.valid` must remain `false`.

### Flatline

```bash
idf.py fullclean
idf.py -DSQI_G1_FAULT_MODE=1 build flash
idf.py monitor 2>&1 | tee g1_flatline.log
```

Wait at least 10 seconds after acquisition starts, then stop the monitor and validate:

```bash
python3 tests/sqi/g1_integrity/validate_fault_log.py g1_flatline.log flatline
```

### RED clipping

```bash
idf.py fullclean
idf.py -DSQI_G1_FAULT_MODE=2 build flash
idf.py monitor 2>&1 | tee g1_clipping_red.log
python3 tests/sqi/g1_integrity/validate_fault_log.py g1_clipping_red.log clipping-red
```

### IR clipping

```bash
idf.py fullclean
idf.py -DSQI_G1_FAULT_MODE=3 build flash
idf.py monitor 2>&1 | tee g1_clipping_ir.log
python3 tests/sqi/g1_integrity/validate_fault_log.py g1_clipping_ir.log clipping-ir
```

### Discontinuity

```bash
idf.py fullclean
idf.py -DSQI_G1_FAULT_MODE=4 build flash
idf.py monitor 2>&1 | tee g1_discontinuity.log
python3 tests/sqi/g1_integrity/validate_fault_log.py g1_discontinuity.log discontinuity
```

The monitor will also show an ESP-IDF warning such as `G1 VALIDATION: fault injection active: CLIPPING_RED`, making accidental test-mode use visible.

## 3. Return to real acquisition

Before any physiological or normal hardware test:

```bash
idf.py fullclean
idf.py -DSQI_G1_FAULT_MODE=0 build flash
```

Confirm the build output contains:

```text
SQI G1 fault injection mode: 0
```

Do not merge the fault-injection changes into the production G1 branch. The expanded host tests may later be cherry-picked independently.
