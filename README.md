# StoneforgeFrontier

2D top-down, Minecraft-like prototype focused on deterministic procedural generation and RL-first simulation.

## What is included

- C++ core simulation (no engine dependency)
- Deterministic, seed-based chunk world generation
- Headless simulator executable for fast RL rollouts
- Optional raylib playable client
- Optional Python binding via pybind11
- Gymnasium wrapper + PPO training script

## Project layout

- `include/stoneforge`: public C++ headers
- `include/stoneforge/client/render_engine.hpp`: raylib render engine entry interface
- `include/stoneforge/client/render_ui.hpp`: HUD/menu/inventory rendering API
- `include/stoneforge/client/render_fx.hpp`: particles/cracks/visual effects API
- `include/stoneforge/mod/*`: content registry, asset loader, mod loader, script runtime
- `src/core`: world + simulation logic
- `src/apps/headless_main.cpp`: fast non-visual runner
- `src/client/raylib_main.cpp`: minimal executable entrypoint
- `src/client/render_engine.cpp`: gameplay loop, rendering, input and UI engine implementation
- `src/client/render_ui.cpp`: UI rendering module (menu, HUD, hotbar, inventory)
- `src/client/render_fx.cpp`: effects module (particles + crack overlays)
- `src/mod/*`: modular content + scripting foundation
- `src/python/py_module.cpp`: pybind11 module
- `python/stoneforge_env.py`: Gymnasium environment wrapper
- `python/train_ppo.py`: Stable-Baselines3 PPO example
- `assets/base/*`: base content manifests for texture overrides
- `mods/*`: user mods (data + optional lua scripts)

## Build

### 1) Configure

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

### 2) Build

```bash
cmake --build build -j
```

The raylib client now follows a simple engine-oriented structure:
- `raylib_main.cpp` starts the process
- `RenderEngine::run()` owns update/render loop state
- `render_ui.cpp` owns HUD/menu/inventory drawing
- `render_fx.cpp` owns particle/crack effect drawing
- content/mod loading is handled by dedicated mod modules before render boot
- simulation remains isolated in `src/core`

### Optional flags

- `-DBUILD_SDL_CLIENT=ON|OFF`
- `-DBUILD_RAYLIB_CLIENT=ON|OFF`
- `-DBUILD_SDL_CLIENT=ON|OFF` (legacy fallback)
- `-DBUILD_PYTHON_BINDINGS=ON|OFF`
- `-DBUILD_HEADLESS_RUNNER=ON|OFF`
- `-DSTONEFORGE_ENABLE_LUA=ON|OFF`

If raylib/SDL2 or Python dev headers are missing, those targets are skipped automatically.
If Lua dev headers are missing, scripting is disabled automatically while data mods still load.

## Modding and Custom Textures

Minecraft-style data-first pipeline is prepared:

- blocks/items/sprites are data-driven via JSON files
- textures are loaded from files in `assets/base` and `mods/<modname>/textures`
- fallback rendering remains active when a texture file is missing
- script hooks are prepared through sandboxed Lua callbacks

Supported Lua callbacks in V1 foundation:

- `onTick(payload)`
- `onBlockPlaced(payload)`
- `onBlockBroken(payload)`
- `onItemUsed(payload)`
- `onCraft(payload)`

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
./build/stoneforge_headless --episodes 5 --max-steps 1500 --seed 42
```

### raylib client (if built)

```bash
./build/stoneforge_client
```

Controls:

- WASD or Arrow keys: move
- Mouse left or `Z`: mine target tile
- Mouse right on tile: context-sensitive place/use with selected hotbar item
- Ghost preview on hover: green = placeable, red = blocked
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

Vitals are also shown Minecraft-style above the hotbar:
- Hearts for HP
- Energy bar

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

Energy drain has been tuned down:
- No hard loss every simulation tick
- Slow passive drain over time
- Small idle regeneration
- Starvation damage applies less frequently

Building material visuals:
- Wood places log-like wooden blocks
- Planks place dedicated smooth wooden walls (not stone-looking walls)

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
