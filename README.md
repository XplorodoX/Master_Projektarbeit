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

## Release CI/CD (GitHub Actions)

This repository contains a multi-platform release workflow in `.github/workflows/release-build.yml`.

- It builds artifacts for macOS, Linux, and Windows.
- It uploads executables as downloadable release assets.
- It runs only for stable tags in the form `vX.Y.Z`.
- Tags with a hyphen (for example `v1.2.0-rc1`, `v1.2.0-beta1`) are treated as pre-release tags and are ignored by the release jobs.

Example stable release tag:

```bash
git tag v1.0.0
git push origin v1.0.0
```

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
- Mouse left or `Z`: mine target tile
- Mouse right on tile: context-sensitive place/use with selected hotbar item
- `1`..`9`: select hotbar slot
- `X`: place using selected hotbar item
- `C`: use selected hotbar item (or quick auto-craft fallback)
- `V`: place workbench kit from inventory (utility shortcut)
- `F1`..`F7`: craft recipes
- `TAB`: toggle expanded inventory + recipe panel
- Inventory panel: drag stack with left mouse, split stack with right mouse
- `R`: reset episode
- `ESC`: back to main menu
- Mouse wheel: zoom in/out (see more world or more detail)

The raylib client now uses a runtime pixel-art sprite atlas (tile sprites + character sprites), biome transition blending with dedicated floor/wall sets per biome, animated portal glow, crack overlays for mining, particle effects for mining and combat hits, and a subtle day/night light pass.

Trees now spawn in biome regions and can be mined for wood.
Mining is progressive (hold mouse or `Z`), and speed depends on your tool levels.

Inventory now tracks multiple material stacks:
- Wood logs
- Planks
- Sticks
- Ore
- Workbench kits (placeable)

Inventory is now a real slot grid (24 slots) with a per-slot stack limit of 64.
You can move/swap stacks with drag-and-drop and split stacks via right mouse drag.
The first 9 slots are mirrored as a bottom hotbar for quick place/use access.
The selected hotbar item is shown as an active hand item near the player sprite.

Crafting supports recipe chains similar to sandbox survival games:
- 1 wood -> 4 planks
- 2 planks -> 4 sticks
- 10 planks -> 1 workbench kit
- Tool recipes (axe/pickaxe tier 1 and tier 2) require a nearby placed workbench

Workbench stations can be placed into the world and mined back into inventory.

It also includes a main menu with seed input, New Run, and Continue.

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

- Better mob behaviors
- Multiple biome-specific resources
- Save/load chunk cache
- Better reward decomposition logs
