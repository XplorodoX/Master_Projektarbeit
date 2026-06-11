#!/usr/bin/env bash
set -euo pipefail

# Quick helper to build bindings, train DQN and/or play back a model with visualization.
# Usage:
#   scripts/start_dqn_play.sh [train|play|both] [timesteps] [model_path] [seed]
# Examples:
#   scripts/start_dqn_play.sh play                # just start playback (uses default model)
#   scripts/start_dqn_play.sh train 200000        # train DQN for 200k steps
#   scripts/start_dqn_play.sh both 100000         # train then play

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
VENV_ACT="$ROOT/.venv/bin/activate"
PYTHON="$ROOT/.venv/bin/python3"
BUILD_DIR="$ROOT/build"
REQ_FILE="$ROOT/python/requirements.txt"
REQ_MARKER="$ROOT/.venv/.requirements_installed"

if [ ! -f "$VENV_ACT" ]; then
  echo "Virtualenv activation not found at $VENV_ACT" >&2
  echo "Please create the virtualenv and install requirements (see README)." >&2
  exit 1
fi

# Activate venv for the remainder of this script
# shellcheck disable=SC1090
source "$VENV_ACT"

action="${1:-both}"
timesteps="${2:-100000}"
model_path="${3:-best_models_dqn/best_model.zip}"
seed="${4:-42}"

build_bindings() {
  if [ -f "$ROOT/python/stoneforge_sim.so" ]; then
    echo "Python bindings already present: python/stoneforge_sim.so"
    return 0
  fi
  # Install requirements once if present
  if [ -f "$REQ_FILE" ] && [ -f "$VENV_ACT" ] && [ ! -f "$REQ_MARKER" ]; then
    echo "Installing Python requirements into venv..."
    "$PYTHON" -m pip install -r "$REQ_FILE"
    touch "$REQ_MARKER" || true
  fi
  echo "Building Python bindings (stoneforge_sim)..."
  cmake -S "$ROOT" -B "$BUILD_DIR"
  cmake --build "$BUILD_DIR" --target stoneforge_sim -j 4
  if [ -f "$BUILD_DIR/stoneforge_sim.so" ]; then
    cp "$BUILD_DIR/stoneforge_sim.so" "$ROOT/python/"
    echo "Copied stoneforge_sim.so to python/"
  else
    echo "Build succeeded but stoneforge_sim.so not found in $BUILD_DIR" >&2
    return 1
  fi
}

case "$action" in
  train)
    build_bindings
    "$PYTHON" "$ROOT/python/train.py" --algo dqn --timesteps "$timesteps"
    ;;
  play)
    build_bindings
    "$PYTHON" "$ROOT/python/ai_play.py" --model "$model_path" --seed "$seed" --speed 1.0
    ;;
  both)
    build_bindings
    "$PYTHON" "$ROOT/python/train.py" --algo dqn --timesteps "$timesteps"
    "$PYTHON" "$ROOT/python/ai_play.py" --model "$model_path" --seed "$seed" --speed 1.0
    ;;
  *)
    echo "Usage: $0 [train|play|both] [timesteps] [model_path] [seed]"
    exit 1
    ;;
esac
