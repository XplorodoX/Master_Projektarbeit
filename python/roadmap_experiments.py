from __future__ import annotations

import argparse
import json
import math
import os
import random
import statistics
from collections import defaultdict, deque
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Iterable

import gymnasium as gym
import numpy as np
import torch
import torch.nn as nn
import torch.nn.functional as F
from stable_baselines3 import PPO
from stable_baselines3.common.callbacks import BaseCallback, CallbackList, EvalCallback
from stable_baselines3.common.env_util import make_vec_env
from stable_baselines3.common.monitor import Monitor
from stable_baselines3.common.torch_layers import BaseFeaturesExtractor

from stoneforge_env import ExitPotentialFieldWrapper, OneHotGridWrapper, StoneforgeConfig, StoneforgeWorldEnv

PROJECT_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_TRAIN_SEEDS = list(range(7000, 7050))
DEFAULT_TRIAL_DIR = PROJECT_ROOT / "logs" / "roadmap_trials"
DEFAULT_RESULTS_FILE = DEFAULT_TRIAL_DIR / "results.md"

_GRID_SIDE = 15
_BASE_GRID_SIZE = _GRID_SIDE * _GRID_SIDE
_BASE_VISITED_START = _BASE_GRID_SIZE
_BASE_SCALAR_START = _BASE_GRID_SIZE * 2
_TAIL_DIM = 14
_ONEHOT_GRID_SIZE = _BASE_GRID_SIZE * 4
_FINAL_OBS_DIM = _ONEHOT_GRID_SIZE + _TAIL_DIM


@dataclass(frozen=True)
class RecipeConfig:
    name: str
    description: str
    n_envs: int = 8
    total_timesteps: int = 1_000_000
    learning_rate: float = 5e-4
    gamma: float = 0.999
    gae_lambda: float = 0.95
    n_steps: int = 256
    batch_size: int = 256
    n_epochs: int = 3
    ent_coef: float = 0.01
    vf_coef: float = 0.5
    clip_range: float = 0.2
    use_plr: bool = False
    use_drac: bool = False
    use_ppg_style: bool = False
    aux_coef: float = 0.25
    aux_updates_per_rollout: int = 4
    aux_batch_size: int = 256
    plr_pool_size: int = 768
    plr_fresh_probability: float = 0.20
    plr_priority_alpha: float = 1.25
    drac_translation: int = 2
    drac_cutout_size: int = 3
    enable_symmetry: bool = False
    disable_curriculum: bool = True
    disable_mobs: bool = True
    disable_energy: bool = True
    target_seed_start: int = 10000


RECIPES: dict[str, RecipeConfig] = {
    "plr": RecipeConfig(
        name="plr",
        description="PPO + Impala-Tiny + LayerNorm + seed-based PLR proxy",
        use_plr=True,
        enable_symmetry=False,
    ),
    "drac": RecipeConfig(
        name="drac",
        description="PPO + Impala-Tiny + LayerNorm + random translation/cutout + BFS auxiliary head",
        use_drac=True,
        aux_coef=0.30,
        aux_updates_per_rollout=6,
        enable_symmetry=False,
    ),
    "ppg": RecipeConfig(
        name="ppg",
        description="PPG-style phased PPO with BFS auxiliary refit",
        use_drac=True,
        use_ppg_style=True,
        aux_coef=0.35,
        aux_updates_per_rollout=8,
        n_steps=2048,
        n_epochs=3,
        enable_symmetry=False,
    ),
}


class ReducedActionEnv(gym.ActionWrapper):
    _ACTION_MAP = [0, 1, 2, 3]

    def __init__(self, env: gym.Env):
        super().__init__(env)
        self.action_space = gym.spaces.Discrete(len(self._ACTION_MAP))

    def action(self, action: int) -> int:
        return self._ACTION_MAP[int(action)]


@dataclass
class SeedStats:
    visits: int = 0
    mean_return: float = 0.0
    mean_length: float = 0.0
    mean_success: float = 0.0
    mean_bfs_distance: float = 0.0
    priority: float = 1.0


class SeedReplayManager:
    def __init__(
        self,
        *,
        pool_size: int,
        fresh_probability: float,
        priority_alpha: float,
        seed_start: int,
    ) -> None:
        self.pool_size = int(pool_size)
        self.fresh_probability = float(fresh_probability)
        self.priority_alpha = float(priority_alpha)
        self._seed_start = int(seed_start)
        self._next_seed = int(seed_start)
        self._stats: dict[int, SeedStats] = {}
        self._known_seeds: list[int] = []
        self._rng = random.Random(seed_start)
        for _ in range(self.pool_size):
            self._register_seed(self._next_seed)
            self._next_seed += 1

    def _register_seed(self, seed: int) -> None:
        if seed not in self._stats:
            self._stats[seed] = SeedStats(priority=1.0)
            self._known_seeds.append(seed)

    def sample_seed(self) -> int:
        if not self._known_seeds or self._rng.random() < self.fresh_probability:
            seed = self._next_seed
            self._next_seed += 1
            self._register_seed(seed)
            return seed

        weights = [max(0.05, self._stats[seed].priority) ** self.priority_alpha for seed in self._known_seeds]
        return int(self._rng.choices(self._known_seeds, weights=weights, k=1)[0])

    def record_episode(self, *, seed: int, episode_return: float, episode_length: int, success: bool, bfs_distance: float) -> None:
        self._register_seed(int(seed))
        stats = self._stats[int(seed)]
        stats.visits += 1
        mix = 1.0 / stats.visits
        stats.mean_return = (1.0 - mix) * stats.mean_return + mix * float(episode_return)
        stats.mean_length = (1.0 - mix) * stats.mean_length + mix * float(episode_length)
        stats.mean_success = (1.0 - mix) * stats.mean_success + mix * (1.0 if success else 0.0)
        stats.mean_bfs_distance = (1.0 - mix) * stats.mean_bfs_distance + mix * float(bfs_distance)

        failure_bonus = 1.0 if not success else 0.2
        distance_bonus = min(1.0, float(bfs_distance) / 128.0)
        return_bonus = max(0.0, 1.0 - float(episode_return) / 50.0)
        length_bonus = min(1.0, float(episode_length) / 4000.0)
        stats.priority = 0.15 + 0.40 * failure_bonus + 0.20 * distance_bonus + 0.15 * return_bonus + 0.10 * length_bonus

    def snapshot(self) -> list[dict[str, float | int]]:
        out: list[dict[str, float | int]] = []
        for seed in sorted(self._stats):
            s = self._stats[seed]
            out.append(
                {
                    "seed": seed,
                    "visits": s.visits,
                    "mean_return": round(s.mean_return, 4),
                    "mean_length": round(s.mean_length, 2),
                    "mean_success": round(s.mean_success, 4),
                    "mean_bfs_distance": round(s.mean_bfs_distance, 2),
                    "priority": round(s.priority, 4),
                }
            )
        return out


# Optional integration with facebookresearch/level-replay if present in
# third_party/level-replay. If available, prefer LevelSampler-based manager.
LevelReplayAvailable = False
try:
    import sys as _sys
    _lr_path = str(PROJECT_ROOT / "third_party" / "level-replay")
    if _lr_path not in _sys.path:
        _sys.path.insert(0, _lr_path)
    from level_replay.level_sampler import LevelSampler as _LevelSampler  # type: ignore
    LevelReplayAvailable = True
except Exception:
    LevelReplayAvailable = False


class LevelReplayManager:
    def __init__(self, *, seeds: list[int], obs_space, action_space, **kwargs) -> None:
        if not LevelReplayAvailable:
            raise RuntimeError("level-replay not available")
        self._sampler = _LevelSampler(seeds, obs_space, action_space, **kwargs)

    def sample_seed(self) -> int:
        return int(self._sampler.sample())

    def record_episode(self, *, seed: int, score: float, num_steps: int) -> None:
        idx = int(seed)
        # level-replay expects seed indices; update by index if present
        try:
            seed_idx = int(self._sampler.seed2index[int(seed)])
        except Exception:
            return
        self._sampler.update_seed_score(0, seed_idx, float(score), int(num_steps))


class SeedReplayWrapper(gym.Wrapper):
    def __init__(self, env: gym.Env, manager: SeedReplayManager) -> None:
        super().__init__(env)
        self._manager = manager

    def reset(self, *, seed: int | None = None, options: dict[str, Any] | None = None):
        if seed is None:
            seed = self._manager.sample_seed()
        return self.env.reset(seed=seed, options=options)


class DrACAugmentationWrapper(gym.Wrapper):
    def __init__(self, env: gym.Env, translation_radius: int = 2, cutout_size: int = 3, augment: bool = True) -> None:
        super().__init__(env)
        self._augment = augment
        self._translation_radius = int(translation_radius)
        self._cutout_size = int(cutout_size)
        self._shift_x = 0
        self._shift_y = 0
        self._cutout_x = 0
        self._cutout_y = 0
        self._rng = np.random.default_rng()

    def reset(self, **kwargs: Any):
        obs, info = self.env.reset(**kwargs)
        if self._augment:
            self._shift_x = int(self._rng.integers(-self._translation_radius, self._translation_radius + 1))
            self._shift_y = int(self._rng.integers(-self._translation_radius, self._translation_radius + 1))
            self._cutout_x = int(self._rng.integers(0, _GRID_SIDE))
            self._cutout_y = int(self._rng.integers(0, _GRID_SIDE))
        else:
            self._shift_x = 0
            self._shift_y = 0
            self._cutout_x = 0
            self._cutout_y = 0
        return self._augment_obs(obs), info

    def step(self, action: int):
        obs, reward, terminated, truncated, info = self.env.step(action)
        return self._augment_obs(obs), reward, terminated, truncated, info

    def _shift_plane(self, plane: np.ndarray) -> np.ndarray:
        if self._shift_x == 0 and self._shift_y == 0:
            return plane
        shifted = np.zeros_like(plane)
        src_x0 = max(0, -self._shift_x)
        src_x1 = min(_GRID_SIDE, _GRID_SIDE - self._shift_x)
        src_y0 = max(0, -self._shift_y)
        src_y1 = min(_GRID_SIDE, _GRID_SIDE - self._shift_y)
        dst_x0 = max(0, self._shift_x)
        dst_y0 = max(0, self._shift_y)
        width = src_x1 - src_x0
        height = src_y1 - src_y0
        if width > 0 and height > 0:
            shifted[dst_y0:dst_y0 + height, dst_x0:dst_x0 + width] = plane[src_y0:src_y1, src_x0:src_x1]
        return shifted

    def _augment_obs(self, obs: np.ndarray) -> np.ndarray:
        if not self._augment:
            return obs
        out = obs.astype(np.float32, copy=True)
        grid = out[:_BASE_GRID_SIZE].reshape(_GRID_SIDE, _GRID_SIDE)
        visited = out[_BASE_VISITED_START:_BASE_VISITED_START + _BASE_GRID_SIZE].reshape(_GRID_SIDE, _GRID_SIDE)

        grid = self._shift_plane(grid)
        visited = self._shift_plane(visited)

        if self._cutout_size > 0:
            half = self._cutout_size // 2
            x0 = max(0, self._cutout_x - half)
            x1 = min(_GRID_SIDE, self._cutout_x + half + 1)
            y0 = max(0, self._cutout_y - half)
            y1 = min(_GRID_SIDE, self._cutout_y + half + 1)
            grid[y0:y1, x0:x1] = 0.0
            visited[y0:y1, x0:x1] = 0.0

        out[:_BASE_GRID_SIZE] = grid.reshape(-1)
        out[_BASE_VISITED_START:_BASE_VISITED_START + _BASE_GRID_SIZE] = visited.reshape(-1)
        return out


class ImpalaBlock(nn.Module):
    def __init__(self, channels: int) -> None:
        super().__init__()
        self.norm1 = nn.GroupNorm(1, channels)
        self.conv1 = nn.Conv2d(channels, channels, kernel_size=3, padding=1)
        self.norm2 = nn.GroupNorm(1, channels)
        self.conv2 = nn.Conv2d(channels, channels, kernel_size=3, padding=1)

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        residual = x
        x = F.relu(self.norm1(x))
        x = self.conv1(x)
        x = F.relu(self.norm2(x))
        x = self.conv2(x)
        return residual + x


class StoneforgeImpalaExtractor(BaseFeaturesExtractor):
    def __init__(self, observation_space: gym.Space, features_dim: int = 256) -> None:
        super().__init__(observation_space, features_dim)
        self.grid_channels = 4
        self.grid_features = _ONEHOT_GRID_SIZE
        self.tail_dim = _TAIL_DIM

        self.conv1 = nn.Conv2d(self.grid_channels, 16, kernel_size=3, padding=1)
        self.block1 = ImpalaBlock(16)
        self.pool1 = nn.MaxPool2d(2, ceil_mode=True)
        self.conv2 = nn.Conv2d(16, 32, kernel_size=3, padding=1)
        self.block2 = ImpalaBlock(32)
        self.pool2 = nn.MaxPool2d(2, ceil_mode=True)
        self.conv3 = nn.Conv2d(32, 32, kernel_size=3, padding=1)
        self.block3 = ImpalaBlock(32)
        self.gap = nn.AdaptiveAvgPool2d(1)

        self.tail_net = nn.Sequential(
            nn.Linear(self.tail_dim, 64),
            nn.LayerNorm(64),
            nn.ReLU(),
            nn.Linear(64, 32),
            nn.LayerNorm(32),
            nn.ReLU(),
        )

        self.trunk = nn.Sequential(
            nn.Linear(32 + 32, 256),
            nn.LayerNorm(256),
            nn.ReLU(),
            nn.Linear(256, features_dim),
            nn.LayerNorm(features_dim),
            nn.ReLU(),
        )

        self.aux_head = nn.Sequential(
            nn.LayerNorm(features_dim),
            nn.Linear(features_dim, 1),
        )

    def forward(self, observations: torch.Tensor) -> torch.Tensor:
        grid = observations[:, : self.grid_features].view(-1, self.grid_channels, _GRID_SIDE, _GRID_SIDE)
        tail = observations[:, self.grid_features : self.grid_features + self.tail_dim]

        x = self.conv1(grid)
        x = self.block1(x)
        x = self.pool1(x)
        x = self.conv2(x)
        x = self.block2(x)
        x = self.pool2(x)
        x = self.conv3(x)
        x = self.block3(x)
        x = self.gap(x).flatten(start_dim=1)

        tail_features = self.tail_net(tail)
        return self.trunk(torch.cat([x, tail_features], dim=1))


@dataclass
class AuxBfsDataset:
    observations: list[np.ndarray] = field(default_factory=list)
    targets: list[float] = field(default_factory=list)

    def add(self, observation: np.ndarray, target: float) -> None:
        self.observations.append(np.asarray(observation, dtype=np.float32).copy())
        self.targets.append(float(target))

    def __len__(self) -> int:
        return len(self.observations)

    def as_arrays(self) -> tuple[np.ndarray, np.ndarray]:
        if not self.observations:
            return np.zeros((0, _FINAL_OBS_DIM), dtype=np.float32), np.zeros((0, 1), dtype=np.float32)
        obs = np.stack(self.observations).astype(np.float32)
        targets = np.asarray(self.targets, dtype=np.float32).reshape(-1, 1)
        return obs, targets


class EpisodeStatsCallback(BaseCallback):
    def __init__(self, manager: SeedReplayManager | None = None, verbose: int = 0) -> None:
        super().__init__(verbose)
        self.manager = manager
        self._dataset = AuxBfsDataset()
        self._episode_counter = 0
        self._recent_returns: deque[float] = deque(maxlen=50)
        self._recent_success: deque[int] = deque(maxlen=50)

    @property
    def aux_dataset(self) -> AuxBfsDataset:
        return self._dataset

    @property
    def recent_mean_return(self) -> float:
        return float(np.mean(self._recent_returns)) if self._recent_returns else 0.0

    @property
    def recent_success_rate(self) -> float:
        return float(np.mean(self._recent_success)) if self._recent_success else 0.0

    def _on_step(self) -> bool:
        infos = self.locals.get("infos", [])
        current_obs = self.locals.get("new_obs", None)
        if current_obs is None:
            current_obs = self.model._last_obs

        for env_index, info in enumerate(infos):
            if "bfs_distance" in info and current_obs is not None:
                self._dataset.add(current_obs[env_index], float(info["bfs_distance"]))

            if "episode" not in info:
                continue

            self._episode_counter += 1
            episode_return = float(info["episode"].get("r", 0.0))
            episode_length = int(info["episode"].get("l", 0))
            success = bool(info.get("reached_exit", False))
            bfs_distance = float(info.get("bfs_distance", 0.0))
            seed = int(info.get("world_seed", -1))

            self._recent_returns.append(episode_return)
            self._recent_success.append(1 if success else 0)

            if self.manager is not None and seed >= 0:
                self.manager.record_episode(
                    seed=seed,
                    episode_return=episode_return,
                    episode_length=episode_length,
                    success=success,
                    bfs_distance=bfs_distance,
                )

        return True


class BfsAuxiliaryRefitCallback(EpisodeStatsCallback):
    def __init__(self, manager: SeedReplayManager | None = None, aux_updates_per_rollout: int = 4, aux_batch_size: int = 256, aux_coef: float = 0.25, verbose: int = 0) -> None:
        super().__init__(manager=manager, verbose=verbose)
        self.aux_updates_per_rollout = int(aux_updates_per_rollout)
        self.aux_batch_size = int(aux_batch_size)
        self.aux_coef = float(aux_coef)

    def _run_aux_update(self) -> None:
        obs, targets = self.aux_dataset.as_arrays()
        if len(obs) == 0:
            return

        device = self.model.device
        obs_tensor = torch.as_tensor(obs, device=device)
        target_tensor = torch.as_tensor(targets / 128.0, device=device)
        policy = self.model.policy
        extractor = policy.features_extractor
        optimizer = policy.optimizer

        permutation = torch.randperm(obs_tensor.shape[0], device=device)
        max_batches = max(1, self.aux_updates_per_rollout)
        batches = 0
        for start in range(0, obs_tensor.shape[0], self.aux_batch_size):
            if batches >= max_batches:
                break
            batch_idx = permutation[start:start + self.aux_batch_size]
            batch_obs = obs_tensor[batch_idx]
            batch_target = target_tensor[batch_idx]
            features = extractor(batch_obs)
            prediction = extractor.aux_head(features)
            aux_loss = F.smooth_l1_loss(prediction, batch_target)
            loss = self.aux_coef * aux_loss
            optimizer.zero_grad()
            loss.backward()
            torch.nn.utils.clip_grad_norm_(policy.parameters(), 0.5)
            optimizer.step()
            batches += 1

    def _on_rollout_end(self) -> None:
        self._run_aux_update()
        self._dataset = AuxBfsDataset()


class PlrLevelReplayCallback(EpisodeStatsCallback):
    pass


@dataclass
class TrialResult:
    recipe: str
    success_count: int
    success_rate: float
    mean_length: float
    mean_return: float
    eval_seeds: list[int]
    model_path: str


def _build_base_env(*, recipe: RecipeConfig, eval_mode: bool, seed_manager: SeedReplayManager | None = None) -> gym.Env:
    config = StoneforgeConfig(
        exit_min_distance=35,
        exit_max_distance=45,
        force_guaranteed_path=True,
        disable_mobs=recipe.disable_mobs,
        disable_energy=recipe.disable_energy,
        disable_potential_field=True,
    )
    env: gym.Env = StoneforgeWorldEnv(config)
    env = ExitPotentialFieldWrapper(env)
    env = ReducedActionEnv(env)

    if recipe.use_drac or recipe.use_ppg_style:
        env = DrACAugmentationWrapper(
            env,
            translation_radius=recipe.drac_translation,
            cutout_size=recipe.drac_cutout_size,
            augment=not eval_mode,
        )
    if seed_manager is not None and not eval_mode:
        env = SeedReplayWrapper(env, seed_manager)

    env = OneHotGridWrapper(env)
    env = Monitor(env)
    return env


def _policy_kwargs() -> dict[str, Any]:
    return {
        "features_extractor_class": StoneforgeImpalaExtractor,
        "features_extractor_kwargs": {},
        "net_arch": dict(pi=[256], vf=[256]),
        "activation_fn": nn.ReLU,
        "ortho_init": False,
        "share_features_extractor": True,
        "normalize_images": False,
    }


def _make_model(recipe: RecipeConfig, env: gym.Env) -> PPO:
    # Prefer Apple MPS on macOS when available (for M1/M2 chips), then CUDA, else CPU.
    device = "cpu"
    try:
        if torch.backends.mps.is_available():
            device = "mps"
        elif torch.cuda.is_available():
            device = "cuda"
    except Exception:
        device = "cpu"

    return PPO(
        "MlpPolicy",
        env,
        verbose=1,
        learning_rate=recipe.learning_rate,
        n_steps=recipe.n_steps,
        batch_size=recipe.batch_size,
        n_epochs=recipe.n_epochs,
        gamma=recipe.gamma,
        gae_lambda=recipe.gae_lambda,
        ent_coef=recipe.ent_coef,
        vf_coef=recipe.vf_coef,
        clip_range=recipe.clip_range,
        policy_kwargs=_policy_kwargs(),
        tensorboard_log=str(PROJECT_ROOT / "tensorboard_logs"),
        device=device,
    )


def evaluate_model(model: PPO, *, recipe: RecipeConfig, seeds: Iterable[int]) -> TrialResult:
    env = _build_base_env(recipe=recipe, eval_mode=True)
    successes = 0
    lengths: list[int] = []
    returns: list[float] = []
    seed_list = list(seeds)

    for seed in seed_list:
        obs, _ = env.reset(seed=int(seed))
        done = False
        total_return = 0.0
        steps = 0
        reached_exit = False
        while not done and steps < 4000:
            action, _ = model.predict(obs, deterministic=True)
            obs, reward, terminated, truncated, info = env.step(int(action))
            total_return += float(reward)
            steps += 1
            reached_exit = reached_exit or bool(info.get("reached_exit", False))
            done = bool(terminated or truncated)
        successes += int(reached_exit)
        lengths.append(steps)
        returns.append(total_return)

    model_path = str(PROJECT_ROOT / "best_models_ppo" / "best_model.zip")
    return TrialResult(
        recipe=recipe.name,
        success_count=successes,
        success_rate=successes / max(1, len(seed_list)),
        mean_length=float(np.mean(lengths)) if lengths else 0.0,
        mean_return=float(np.mean(returns)) if returns else 0.0,
        eval_seeds=seed_list,
        model_path=model_path,
    )


def _write_results_markdown(results: list[TrialResult]) -> None:
    DEFAULT_TRIAL_DIR.mkdir(parents=True, exist_ok=True)
    lines = ["# Roadmap Trials", ""]
    lines.append("| Recipe | Erfolge | Success Rate | Mittl. Episodenlänge | Mittl. Return | Datum |")
    lines.append("|---|---:|---:|---:|---:|---|")
    today = "15.05.2026"
    for result in results:
        lines.append(
            f"| {result.recipe} | {result.success_count} / {len(result.eval_seeds)} | {result.success_rate * 100:.1f} % | {result.mean_length:.1f} | {result.mean_return:.2f} | {today} |"
        )
    lines.append("")
    lines.append("## Notes")
    lines.append("- PLR here is implemented as a seed-replay proxy using episode return / success / BFS distance statistics.")
    lines.append("- DrAC uses translation + cutout augmentation over the Stoneforge grid observation.")
    lines.append("- PPG is implemented as a phasic PPO run with an auxiliary BFS refit phase between policy phases.")
    DEFAULT_RESULTS_FILE.write_text("\n".join(lines), encoding="utf-8")


def run_recipe(recipe_name: str, *, timesteps: int | None = None, eval_seeds: Iterable[int] = DEFAULT_TRAIN_SEEDS, n_envs: int | None = None) -> TrialResult:
    if recipe_name not in RECIPES:
        raise ValueError(f"Unknown recipe: {recipe_name}")

    recipe = RECIPES[recipe_name]
    if timesteps is not None:
        recipe = RecipeConfig(**{**recipe.__dict__, "total_timesteps": int(timesteps)})
    if n_envs is not None:
        recipe = RecipeConfig(**{**recipe.__dict__, "n_envs": int(n_envs)})

    seed_manager = None
    level_callback: EpisodeStatsCallback
    if recipe.use_plr:
        if LevelReplayAvailable:
            # Build a LevelReplayManager mapping to the official implementation
            seed_list = list(range(recipe.target_seed_start, recipe.target_seed_start + recipe.plr_pool_size))
            seed_manager = LevelReplayManager(seeds=seed_list, obs_space=None, action_space=None, num_actors=recipe.n_envs, strategy='random', score_transform='rank', alpha=recipe.plr_priority_alpha, staleness_coef=0.1)
        else:
            seed_manager = SeedReplayManager(
                pool_size=recipe.plr_pool_size,
                fresh_probability=recipe.plr_fresh_probability,
                priority_alpha=recipe.plr_priority_alpha,
                seed_start=recipe.target_seed_start,
            )

    env = make_vec_env(
        lambda: _build_base_env(recipe=recipe, eval_mode=False, seed_manager=seed_manager),
        n_envs=recipe.n_envs,
    )
    eval_env = _build_base_env(recipe=recipe, eval_mode=True)
    model = _make_model(recipe, env)

    eval_callback = EvalCallback(
        eval_env,
        best_model_save_path=str(PROJECT_ROOT / f"best_models_{recipe.name}"),
        log_path=str(PROJECT_ROOT / "logs"),
        eval_freq=max(1, 20_000 // recipe.n_envs),
        n_eval_episodes=20,
        deterministic=True,
        render=False,
        verbose=1,
        warn=False,
    )

    if recipe.use_plr:
        level_callback = PlrLevelReplayCallback(manager=seed_manager, verbose=1)
        callbacks: list[BaseCallback] = [eval_callback, level_callback]
        model.learn(
            total_timesteps=recipe.total_timesteps,
            tb_log_name=f"{recipe.name}_run",
            callback=CallbackList(callbacks),
        )
    elif recipe.use_ppg_style:
        phase_one = int(recipe.total_timesteps * 0.70)
        phase_two = recipe.total_timesteps - phase_one
        aux_callback = BfsAuxiliaryRefitCallback(
            manager=None,
            aux_updates_per_rollout=recipe.aux_updates_per_rollout,
            aux_batch_size=recipe.aux_batch_size,
            aux_coef=recipe.aux_coef,
            verbose=1,
        )
        model.learn(
            total_timesteps=phase_one,
            tb_log_name=f"{recipe.name}_policy",
            callback=CallbackList([eval_callback]),
        )
        model.learn(
            total_timesteps=phase_two,
            tb_log_name=f"{recipe.name}_aux",
            callback=CallbackList([eval_callback, aux_callback]),
        )
    else:
        aux_callback = BfsAuxiliaryRefitCallback(
            manager=None,
            aux_updates_per_rollout=recipe.aux_updates_per_rollout,
            aux_batch_size=recipe.aux_batch_size,
            aux_coef=recipe.aux_coef,
            verbose=1,
        )
        model.learn(
            total_timesteps=recipe.total_timesteps,
            tb_log_name=f"{recipe.name}_run",
            callback=CallbackList([eval_callback, aux_callback]),
        )

    best_model_dir = PROJECT_ROOT / f"best_models_{recipe.name}"
    best_model_dir.mkdir(parents=True, exist_ok=True)
    model_path = best_model_dir / "best_model.zip"
    model.save(model_path)

    result = evaluate_model(model, recipe=recipe, seeds=eval_seeds)
    result.model_path = str(model_path)
    return result


def main() -> None:
    parser = argparse.ArgumentParser(description="Train and compare Stoneforge roadmap recipes")
    parser.add_argument("--recipe", choices=["plr", "drac", "ppg", "all"], default="all")
    parser.add_argument("--timesteps", type=int, default=None)
    parser.add_argument("--n-envs", type=int, default=None)
    parser.add_argument("--eval-only", action="store_true")
    parser.add_argument("--results-file", type=str, default=str(DEFAULT_RESULTS_FILE))
    args = parser.parse_args()

    results: list[TrialResult] = []
    recipes = list(RECIPES) if args.recipe == "all" else [args.recipe]

    if args.eval_only:
        raise SystemExit("--eval-only is not supported yet; run training first.")

    for recipe_name in recipes:
        print(f"\n=== Recipe: {recipe_name.upper()} ===")
        result = run_recipe(recipe_name, timesteps=args.timesteps, n_envs=args.n_envs)
        results.append(result)
        print(
            f"[{recipe_name}] success={result.success_count}/{len(result.eval_seeds)} "
            f"({result.success_rate * 100:.1f}%), mean_len={result.mean_length:.1f}, mean_return={result.mean_return:.2f}"
        )

    _write_results_markdown(results)
    print(f"\nResults written to {DEFAULT_RESULTS_FILE}")


if __name__ == "__main__":
    main()
