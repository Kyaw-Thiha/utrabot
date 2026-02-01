#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC_DIR="${ROOT_DIR}/src"
ARDUINO_DIR="${ROOT_DIR}/arduino/utrabot"
ARDUINO_SRC_DIR="${ARDUINO_DIR}/src"

mkdir -p "${ARDUINO_DIR}"

ROOT_DIR="${ROOT_DIR}" python3 - <<'PY'
import os
import shutil
from pathlib import Path

root = Path(os.environ["ROOT_DIR"])
src_dir = root / "src"
arduino_dir = root / "arduino" / "utrabot"
arduino_src_dir = arduino_dir / "src"

main_cpp = src_dir / "main.cpp"
if not main_cpp.exists():
    raise SystemExit(f"Missing {main_cpp}")

if arduino_src_dir.exists():
    shutil.rmtree(arduino_src_dir)
arduino_src_dir.mkdir(parents=True, exist_ok=True)

ino_path = arduino_dir / "utrabot.ino"
ino_path.write_text(main_cpp.read_text(encoding="utf-8"), encoding="utf-8")

for path in src_dir.rglob("*"):
    if path.is_dir():
        continue
    if path.name == "main.cpp":
        continue
    rel = path.relative_to(src_dir)
    dest = arduino_src_dir / rel
    dest.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(path, dest)
PY
