#!/usr/bin/env python3
import argparse
import json
import sys
from pathlib import Path

EXPECTED = {
    "flatline": {
        "reason": "FLATLINE_RED",
        "required_mask": 8 | 16,
    },
    "clipping-red": {
        "reason": "CLIPPING_RED",
        "required_mask": 32,
    },
    "clipping-ir": {
        "reason": "CLIPPING_IR",
        "required_mask": 64,
    },
    "discontinuity": {
        "reason": "DISCONTINUITY",
        "required_mask": 1,
    },
}


def load_frames(path: Path):
    frames = []
    for raw in path.read_text(errors="replace").splitlines():
        raw = raw.strip()
        if not raw.startswith("{"):
            continue
        try:
            obj = json.loads(raw)
        except json.JSONDecodeError:
            continue
        if obj.get("type") == "oximetry":
            frames.append(obj)
    return frames


def validate_frame(frame, mode):
    exp = EXPECTED[mode]
    errors = []
    g1 = frame.get("g1", {})
    mask = int(g1.get("mask", 0))

    if frame.get("quality_state") != "INVALID":
        errors.append("quality_state != INVALID")
    if frame.get("failed_gate") != "G1_INTEGRITY":
        errors.append("failed_gate != G1_INTEGRITY")
    if frame.get("fail_reason") != exp["reason"]:
        errors.append(f"fail_reason != {exp['reason']}")
    if (mask & exp["required_mask"]) != exp["required_mask"]:
        errors.append(f"mask {mask} missing required bits {exp['required_mask']}")
    if frame.get("hr", {}).get("valid") is not False:
        errors.append("hr.valid != false (fail-fast violation)")
    if frame.get("spo2", {}).get("valid") is not False:
        errors.append("spo2.valid != false (fail-fast violation)")

    if mode == "flatline":
        if int(g1.get("red_range", -1)) != 0 or int(g1.get("ir_range", -1)) != 0:
            errors.append("flatline ranges are not zero")
    elif mode == "clipping-red":
        if float(g1.get("red_clip", 0.0)) <= 0.01:
            errors.append("red_clip <= 0.01")
    elif mode == "clipping-ir":
        if float(g1.get("ir_clip", 0.0)) <= 0.01:
            errors.append("ir_clip <= 0.01")
    elif mode == "discontinuity":
        if float(g1.get("continuity", 1.0)) >= 0.95:
            errors.append("continuity >= 0.95")

    return errors


def main():
    parser = argparse.ArgumentParser(description="Validate G1 fault-injection telemetry")
    parser.add_argument("log", type=Path)
    parser.add_argument("mode", choices=EXPECTED.keys())
    args = parser.parse_args()

    frames = load_frames(args.log)
    if not frames:
        print("FAIL: no oximetry JSON frames found", file=sys.stderr)
        return 2

    failures = []
    for idx, frame in enumerate(frames):
        errors = validate_frame(frame, args.mode)
        if errors:
            failures.append((idx, frame.get("ts_ms"), errors))

    print(f"mode={args.mode} frames={len(frames)} violations={len(failures)}")
    if failures:
        for idx, ts, errors in failures[:10]:
            print(f"  frame#{idx} ts_ms={ts}: {'; '.join(errors)}")
        return 1

    first = frames[0]
    g1 = first["g1"]
    print(
        "PASS: "
        f"reason={first['fail_reason']} mask={g1['mask']} "
        f"continuity={g1['continuity']:.4f} "
        f"red_range={g1['red_range']} ir_range={g1['ir_range']} "
        f"red_clip={g1['red_clip']:.4f} ir_clip={g1['ir_clip']:.4f}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
