# G1 threshold calibration plan

## Principle

G1 separates technical signal integrity from pulsatility and morphology. Hardware-defined quantities are derived from the MAX30102 configuration; sensor-dependent thresholds are calibrated from labeled recordings; application latency parameters are justified by sensitivity/latency analysis.

The literature does not provide a universal set of raw MAX30102 count thresholds. Reddy et al. and Vadrevu & Manikandan explicitly adapt amplitude thresholds to the sensing-module dynamic range. Karlen et al. recommend selecting thresholds from feature distributions/histograms and ROC analysis. Public PPG databases are useful for algorithm validation, but absolute DC levels and ADC-rail behavior are not portable across sensor modules, LED currents, ADC ranges, placement and optical mechanics.

## Parameter provenance

| Parameter | Current value | Status | Finalization method |
|---|---:|---|---|
| `window_ms` | 5000 ms | literature-backed baseline | keep 5 s as baseline; optionally compare 3/5/8 s for latency vs errors |
| `step_ms` | 1000 ms | engineering choice | profile processing time, FIFO margin and application latency |
| `adc_min_value` | 0 | hardware-derived | fixed by unsigned ADC output |
| `adc_max_value` | 262143 | hardware-derived | fixed by 18-bit MAX30102 mode |
| `rail_margin_counts` | 1 | provisional | bench saturation sweep; characterize codes observed near both rails |
| `minimum_mean_level` | 5000 | provisional | labeled `NO_CONTACT` vs usable contact distributions, per channel |
| `minimum_raw_range` | 20 | provisional/conservative | characterize stuck/flat faults and lower tail of usable-window range |
| `maximum_clipping_fraction` | 0.01 | provisional | inject/sweep clipping fractions and define tolerated corruption before downstream failure |
| `minimum_continuity_fraction` | 0.95 | provisional | long acquisition runs + injected missing/duplicate intervals; choose tolerated loss budget |
| `maximum_interval_deviation_fraction` | 0.40 | provisional | characterize reconstructed timestamp interval distribution under normal and stressed acquisition |
| `minimum_quality_score` | 0.55 | legacy | do not calibrate as G1; replace/redefine after G2-G4 are complete |

## Local calibration dataset

Store RAW RED, RAW IR, timestamp and configuration metadata. At minimum label windows as `NO_CONTACT`, `STABLE_CONTACT`, `LIGHT_CONTACT`, `PARTIAL_CONTACT`, `PRESSURE`, `AMBIENT_LIGHT`, `MOTION`, `TRANSITION_IN`, `TRANSITION_OUT`, plus synthetic/bench `FLAT_FAULT`, `CLIP_LOW`, `CLIP_HIGH`, and timestamp fault classes.

For each 5 s window compute RED/IR mean, range, clipping fraction and continuity. Do not split overlapping windows from the same recording or subject across calibration and validation sets. Use subject/session-grouped splits when multiple participants are available.

## Threshold selection

1. Freeze sensor configuration for a calibration profile (sample rate, pulse width, ADC range, LED currents and mechanics).
2. Collect labeled development recordings across sessions, contact conditions and lighting.
3. Plot class distributions and percentiles for each G1 feature.
4. Select candidate thresholds using ROC/precision-recall analysis or a constrained rule such as maximizing retained usable windows while limiting acceptance of known-invalid windows.
5. Evaluate candidates on held-out subjects/sessions; report sensitivity, specificity, false-accept and false-reject rates with confidence intervals.
6. Perform a threshold sensitivity analysis around the chosen value; prefer stable plateaus over a narrowly optimal point.
7. Freeze thresholds together with the exact sensor/configuration profile and version them.
8. Recalibrate if LED current, ADC range, optical housing/contact geometry or sampling strategy changes materially.

## Public datasets

Use public databases primarily for G2/G3/G4 and external algorithm validation: PhysioNet BIDMC PPG, MIMIC waveform PPG, Wrist PPG During Exercise, PPG-DaLiA and similar datasets provide varied physiology/motion and references such as ECG. Do not use their absolute raw amplitude to set `minimum_mean_level`, `minimum_raw_range`, or ADC rail margins for the MAX30102 because their acquisition hardware/scaling differs.

## Acquisition caveat

The MAX30102 does not timestamp individual FIFO samples. `ppg_sampler` reconstructs sample timestamps from the configured sample period and FIFO read timing. Therefore `continuity_fraction` measures continuity of the reconstructed acquisition stream, not oscillator-level sample jitter. Calibrate it using long firmware runs, FIFO/sequence diagnostics and controlled fault injection; do not present it as direct measurement of the sensor's instantaneous sampling jitter.
