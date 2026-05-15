Roadmap Changes (summary)

Date: 15.05.2026

This file summarizes the code changes and integrations performed as part of the "roadmap" work.

1) PBRS reward shaping
- File: src/core/simulation.cpp
- Change: Replaced the previous delta-BFS + proximity shaping with a formal Potential-Based Reward Shaping (PBRS) term.
  - Φ(s) = -BFS(s) / 128
  - F(s,s') = γ·Φ(s') - Φ(s)
  - r_total += β·F(s,s') with defaults: γ=0.999, β=0.5
  - Minimal step-penalty set to -0.01
- Rationale: PBRS is policy-invariant and matches the experimental design in Section 10.

2) Exposed BFS to Python
- Files: src/python/py_module.cpp, python/stoneforge_env.py
- Change: `current_bfs_distance_to_exit()` exposed via pybind; `StoneforgeWorldEnv` now reports `bfs_distance` and `world_seed` in the `info` dict.

3) Impala-Tiny encoder + BFS auxiliary head
- File: python/roadmap_experiments.py
- Change: `StoneforgeImpalaExtractor` implemented with GroupNorm, GAP and a small tail-MLP; `aux_head` added and trained by `BfsAuxiliaryRefitCallback` using Huber loss.

4) PLR: seed-replay proxy + optional official integration
- File: python/roadmap_experiments.py
- Change: A `SeedReplayManager` (seed-level proxy) exists. Additionally, the official `level-replay` repository was added to `third_party/level-replay` and a `LevelReplayManager` wrapper was added to prefer the official sampler when available.
- Note: the third-party code was patched locally to be importable; see runtime logs for potential numpy compatibility warnings.

5) DrAC augmentations + PPG scaffold
- Files: python/roadmap_experiments.py
- Change: `DrACAugmentationWrapper` (translation + cutout) and a simple PPG phasic scheduling (policy phase then aux refit phase) implemented.

6) Hyperparameter defaults
- File: python/roadmap_experiments.py
- Change: Updated defaults to the recommended table (learning_rate=5e-4, n_steps=256, n_epochs=3, gamma=0.999, gae_lambda=0.95, aux coeffs per recipe).

7) level-replay vendor
- Location: third_party/level-replay
- Note: vendorized clone used since the library is not pip-packaged; it is imported by adding the `third_party/level-replay` folder to `sys.path` at runtime.

How to run on Apple Silicon (M1/M2 / macOS)
- The training code now prefers the `mps` device if available. To run on the M1 Pro GPU, ensure you have a PyTorch build with MPS support installed in the activated environment.

Recommended steps:

```bash
# activate venv
source .venv/bin/activate
# install a compatible PyTorch with MPS support (example for macOS):
pip install --pre torch torchvision --index-url https://download.pytorch.org/whl/nightly/cpu
# then run training (example Phase-1)
python python/train_roadmap.py --recipe plr --timesteps 5000000 --n-envs 64
```

Caveats & next actions
- Running 64 parallel envs on a laptop may be CPU-bound or memory-limited; consider using fewer envs (e.g., 16 or 32) for interactive experiments, or run on a dedicated machine.
- level-replay in `third_party` required a small compatibility patch for modern numpy (if errors appear, replace `np.float` with `float` or `np.float64`).

If you want, I can:
- revert/add more detailed changelog entries to the project's main `CHANGELOG.md`;
- tune PBRS parameters and re-run short tests; or
- fully integrate level-replay API cleanup (patch `third_party` files to fix numpy deprecations). 
