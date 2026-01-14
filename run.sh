#!/usr/bin/env bash
set -euo pipefail

LOG_FILE="build_run.log"

: > "$LOG_FILE"

{
  echo "=== $(date) ==="
  cmake -S . -B build
  cmake --build build
  echo "--- Running game ---"
  ./build/space_game
} 2>&1 | tee -a "$LOG_FILE"
