# LeconProjekt

2D top-down, Minecraft-like prototype focused on deterministic procedural generation and RL-first simulation.

## What is included

- C++ core simulation (no engine dependency)
- Deterministic, seed-based chunk world generation
- Headless simulator executable for fast RL rollouts
- Optional raylib playable client
- Optional Python binding via pybind11
- Gymnasium wrapper + PPO training script

## Project layout

- `include/lecon`: public C++ headers
- `src/core`: world + simulation logic
- `src/apps/headless_main.cpp`: fast non-visual runner
- `src/client/raylib_main.cpp`: playable raylib client
- `src/python/py_module.cpp`: pybind11 module
- `python/lecon_env.py`: Gymnasium environment wrapper
- `python/train_ppo.py`: Stable-Baselines3 PPO example

## Build

### 1) Configure

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

### 2) Build

```bash
cmake --build build -j
```

### Optional flags

- `-DBUILD_SDL_CLIENT=ON|OFF`
- `-DBUILD_RAYLIB_CLIENT=ON|OFF`
- `-DBUILD_SDL_CLIENT=ON|OFF` (legacy fallback)
- `-DBUILD_PYTHON_BINDINGS=ON|OFF`
- `-DBUILD_HEADLESS_RUNNER=ON|OFF`

If raylib/SDL2 or Python dev headers are missing, those targets are skipped automatically.

## Run

### Headless random policy smoke test

```bash
./build/lecon_headless --episodes 5 --max-steps 1500 --seed 42
```

### raylib client (if built)

```bash
./build/lecon_client
```

Controls:

- WASD or Arrow keys: move
- `Z`: mine in facing direction
- `X`: place block in facing direction
- `C`: use (placeholder)
- `R`: reset episode
- Mouse wheel: zoom in/out (see more world or more detail)

The raylib client now uses a runtime pixel-art sprite atlas (tile sprites + character sprites), biome color tints, animated portal glow, and a subtle day/night light pass.

## Python RL setup

Use Python 3.12 for best pybind11 compatibility.

On macOS (Homebrew):

```bash
brew install python@3.12
brew install raylib
```

Create environment and install dependencies:

```bash
/opt/homebrew/opt/python@3.12/bin/python3.12 -m venv .venv
source .venv/bin/activate
pip install --upgrade pip
pip install gymnasium stable-baselines3 numpy
```

Build Python module in-place (from repo root):

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_PYTHON_BINDINGS=ON
cmake --build build -j
```

Then expose module path for Python (example):

```bash
export PYTHONPATH="$PWD/build:$PWD/python:$PYTHONPATH"
python python/train_ppo.py
```

## Design notes (V1)

- Goal: reach dungeon exit tile
- Episode horizon: 2500 max steps
- Observation: local 11x11 grid + hp/energy/inventory
- Action space: 9 discrete actions
- Reward shaping:
  - small step penalty
  - progress-to-exit bonus
  - damage penalty
  - large exit bonus

## Next V1 extensions

- Crafting recipes
- Better mob behaviors
- Multiple biome-specific resources
- Save/load chunk cache
- Better reward decomposition logs
