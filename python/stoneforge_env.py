from __future__ import annotations

import importlib
import random
import threading
from collections import deque
from typing import Any

import gymnasium as gym
import numpy as np
from gymnasium import spaces

stoneforge_sim = importlib.import_module("stoneforge_sim")


class SwarmSeedPool:
    """Thread-sicherer Pool erfolgreicher Seeds für Swarm-Training.

    Jeder erfolgreiche Seed wird nur EINMAL gespeichert (Set-Semantik).
    Beim Reset: mit swarm_prob einen zufälligen Erfolgs-Seed verwenden.
    """

    def __init__(self, maxlen: int = 500, swarm_prob: float = 0.3, plr_mode: bool = False) -> None:
        self._seeds: set[int] = set()
        self._maxlen = maxlen
        self._lock = threading.Lock()
        self.swarm_prob = swarm_prob
        self.plr_mode = plr_mode
        self.total_added = 0
        self.total_sampled = 0

    def add(self, seed: int) -> None:
        with self._lock:
            if seed in self._seeds:
                return   # kein Duplikat
            # Pool-Größe begrenzen: ältesten (zufälligen) Eintrag entfernen
            if len(self._seeds) >= self._maxlen:
                self._seeds.discard(next(iter(self._seeds)))
            self._seeds.add(seed)
            self.total_added += 1

    def remove(self, seed: int) -> None:
        with self._lock:
            self._seeds.discard(seed)

    def clear(self) -> None:
        """Pool leeren (z. B. bei Phasenwechsel — Seeds sind dann nicht mehr aussagekräftig)."""
        with self._lock:
            self._seeds.clear()

    def sample(self) -> int | None:
        with self._lock:
            if self._seeds and random.random() < self.swarm_prob:
                self.total_sampled += 1
                return random.choice(list(self._seeds))
        return None

    def stats(self) -> dict:
        with self._lock:
            return {
                "pool_size":     len(self._seeds),
                "total_added":   self.total_added,
                "total_sampled": self.total_sampled,
            }

# ─── Observation-Layout ───────────────────────────────────────────────────────
#
#  Standard (use_visited_mask=False, include_energy_inventory=False):
#    [0   : gs)    grid — {0,10,15,20,30}/30
#    [gs+0..2]     hp, exitDx, exitDy
#    [gs+3]        step_frac
#    → 225 + 3 + 1 = 229 dims
#
#  Energie und Inventar sind tote Features (disable_energy=True, kein Mining →
#  konstant 100 bzw. 0) und werden standardmäßig entfernt. Für alte Modelle
#  (231-dim, z.B. ppo_lstm_curriculum): include_energy_inventory=True.
#
#  Legacy (include_energy_inventory=True):
#    [gs+0..4]     hp, energy, inventory, exitDx, exitDy
#    [gs+5]        step_frac
#    [gs+6]        visit_count_here (wenn use_visit_count=True, sonst weggelassen)
#    [gs+7..]      action_buffer: action_buffer_len × 4 one-hot (wenn use_last_action_reward)
#    [gs+7+buf]    last_reward geclippt (wenn use_last_action_reward)
#
#  Mit Visited Mask (use_visited_mask=True, CNN-Variante):
#    [0   : gs)    Kanal 0 — Tile-Typen
#    [gs  : 2*gs)  Kanal 1 — Visited Mask
#    [2*gs: 2*gs+6) Extras — hp, energy, inventory, exitDx, exitDy, step_frac
#    Total: 456

_N_ACTIONS = 4
_GRID_SIDE  = 15   # observationRadius=7 → 2*7+1=15

# Bekannte Obs-Layouts: Obs-Dimension eines Modells → Env-Konstruktor-Kwargs.
# Damit funktionieren alte UND neue Modelle in allen Skripten automatisch.
_OBS_DIM_KWARGS: dict[int, dict] = {
    229: {},                                                       # Env v11 (aktuell)
    231: {"include_energy_inventory": True},                       # v2–v10 LSTM-Curriculum
    230: {"include_energy_inventory": True, "use_step_frac": False},   # ppo_no_bfs
    236: {"include_energy_inventory": True, "use_last_action_reward": True},  # ppo_phase4
    249: {"include_energy_inventory": True, "use_visit_count": True,
          "use_last_action_reward": True, "action_buffer_len": 4},          # v7-Fixes
    456: {"include_energy_inventory": True, "use_visited_mask": True},      # CNN
    461: {"include_energy_inventory": True, "use_visited_mask": True,
          "use_last_action_reward": True},                                  # CNN+PLR
}


def env_kwargs_for_model(model) -> dict:
    """Leitet die Env-Kwargs aus der Observation-Dimension eines geladenen
    SB3-Modells ab, damit alte (231-dim) und neue (229-dim) Modelle ohne
    manuelles Flag-Setzen evaluiert/abgespielt werden können.

    >>> env = StoneforgeWorldEnv(exit_min=35, exit_max=45, **env_kwargs_for_model(model))
    """
    n_obs = int(model.observation_space.shape[-1])
    if n_obs not in _OBS_DIM_KWARGS:
        known = ", ".join(str(k) for k in sorted(_OBS_DIM_KWARGS))
        raise ValueError(
            f"Unbekanntes Obs-Layout: Modell erwartet {n_obs} Dimensionen "
            f"(bekannt: {known}). Env-Kwargs bitte manuell setzen."
        )
    return dict(_OBS_DIM_KWARGS[n_obs])


class StoneforgeWorldEnv(gym.Env[np.ndarray, int]):
    """Stoneforge PointGoal environment ohne BFS-Navigation.

    Observation: 15×15 lokales Grid + euklidischer Exit-Kompass.
    Optional: zweiter Grid-Kanal als Visited Mask (für CNN-Policy).
    """

    metadata = {"render_modes": []}

    def __init__(
        self,
        exit_min: int = 35,
        exit_max: int = 45,
        base_seed: int = 42,
        swarm_pool: SwarmSeedPool | None = None,
        use_visited_mask: bool = False,
        use_last_action_reward: bool = False,
        use_visit_count: bool = False,
        action_buffer_len: int = 1,
        use_step_frac: bool = True,
        include_energy_inventory: bool = False,
    ) -> None:
        super().__init__()
        self.core = stoneforge_sim.StoneforgeCoreEnv(base_seed)
        self.core.configure_world_generation(
            exit_min_distance=exit_min,
            exit_max_distance=exit_max,
            force_guaranteed_path=False,
            disable_mobs=True,
            disable_energy=True,
        )
        self._base_seed = base_seed
        self._exit_min = exit_min
        self._exit_max = exit_max
        self.swarm_pool = swarm_pool
        self.use_visited_mask = use_visited_mask
        self.use_last_action_reward = use_last_action_reward
        self.use_visit_count = use_visit_count
        self.action_buffer_len = max(1, action_buffer_len)
        self.use_step_frac = use_step_frac
        self.include_energy_inventory = include_energy_inventory
        self._current_seed: int = 0
        self._visited_tiles: set[tuple[int, int]] = set()
        self._visit_counts: dict[tuple[int, int], int] = {}
        self._steps_no_reward: int = 0
        self._action_buffer: deque[int | None] = deque(
            [None] * self.action_buffer_len, maxlen=self.action_buffer_len
        )

        n_base = self.core.observation_size()   # gs + 5 = 230
        self._gs = n_base - 5                   # 225 für radius=7
        self._radius = (_GRID_SIDE - 1) // 2    # 7
        self._step_count = 0
        self._max_steps = 4000

        # Energie + Inventar sind tote Features (konstant) → standardmäßig entfernt
        n_dead = 0 if include_energy_inventory else 2

        if use_visited_mask:
            self._n_obs = self._gs * 2 + 6 - n_dead               # 456 legacy / 454
            if use_last_action_reward:
                self._n_obs += self.action_buffer_len * _N_ACTIONS # + action buffer
                self._n_obs += 1                                   # + last_reward
        else:
            self._n_obs = n_base - n_dead                          # 230 legacy / 228
            if use_step_frac:
                self._n_obs += 1                                   # + step_frac
            if use_visit_count:
                self._n_obs += 1                                   # + visit_count_here
            if use_last_action_reward:
                self._n_obs += self.action_buffer_len * _N_ACTIONS # + action buffer
                self._n_obs += 1                                   # + last_reward

        self.action_space = spaces.Discrete(_N_ACTIONS)
        self.observation_space = spaces.Box(
            low=-1.0, high=1.0, shape=(self._n_obs,), dtype=np.float32
        )

    # ------------------------------------------------------------------

    def _visited_mask_array(self, px: int, py: int) -> np.ndarray:
        """Baut die 225-dim Visited-Mask für die aktuelle Agenten-Position."""
        mask = np.zeros(self._gs, dtype=np.float32)
        r = self._radius
        side = _GRID_SIDE
        for idx in range(self._gs):
            row = idx // side
            col = idx % side
            wx = px + col - r
            wy = py + row - r
            if (wx, wy) in self._visited_tiles:
                mask[idx] = 1.0
        return mask

    def _normalize(self, raw: list[int]) -> np.ndarray:
        arr = np.asarray(raw, dtype=np.float32)
        gs = self._gs
        arr[:gs]   /= 30.0                                   # grid tiles
        arr[gs]    /= 10.0                                   # hp
        arr[gs+1]  /= 100.0                                  # energy
        arr[gs+2]   = np.clip(arr[gs+2], 0.0, 64.0) / 64.0 # inventory
        arr[gs+3]  /= 64.0                                   # exitDx
        arr[gs+4]  /= 64.0                                   # exitDy
        step_frac   = np.float32(self._step_count / self._max_steps)

        if not self.use_visited_mask:
            extras = []
            if self.use_step_frac:
                extras.append(step_frac)

            # Fix 1: Besuchszähler aktueller Tile → LSTM sieht Loops deterministisch
            if self.use_visit_count:
                px, py = self.core.player_pos()
                vc = self._visit_counts.get((px, py), 0)
                extras.append(np.float32(min(vc, 10) / 10.0))

            # Fix 3: Aktions-Buffer (letzte N Aktionen als One-Hot-Matrix)
            if self.use_last_action_reward:
                for act in self._action_buffer:
                    oh = np.zeros(_N_ACTIONS, dtype=np.float32)
                    if act is not None and 0 <= act < _N_ACTIONS:
                        oh[act] = 1.0
                    extras.append(oh)
                extras.append(np.float32(np.clip(self._last_reward, -1.0, 1.0)))

            if not self.include_energy_inventory:
                # Energie (gs+1) und Inventar (gs+2) entfernen: [grid | hp | exitDx | exitDy]
                arr = np.concatenate([arr[:gs + 1], arr[gs + 3:]])
            return np.concatenate([arr] + [
                np.atleast_1d(np.float32(e)) if np.isscalar(e) else e
                for e in extras
            ])

        # CNN-Variante: [grid(225) | visited(225) | extras]
        px, py   = self.core.player_pos()
        visited  = self._visited_mask_array(px, py)
        if self.include_energy_inventory:
            extras_list = [
                arr[gs], arr[gs+1], arr[gs+2],   # hp, energy, inventory
                arr[gs+3], arr[gs+4],             # exitDx, exitDy
                step_frac,
            ]
        else:
            extras_list = [
                arr[gs],                          # hp
                arr[gs+3], arr[gs+4],             # exitDx, exitDy
                step_frac,
            ]
        if self.use_last_action_reward:
            for act in self._action_buffer:
                oh = np.zeros(_N_ACTIONS, dtype=np.float32)
                if act is not None and 0 <= act < _N_ACTIONS:
                    oh[act] = 1.0
                extras_list.append(oh)
            extras_list.append(np.float32(np.clip(self._last_reward, -1.0, 1.0)))

        extras = np.concatenate([
            np.atleast_1d(np.float32(e)) if np.isscalar(e) else e
            for e in extras_list
        ])
        return np.concatenate([arr[:gs], visited, extras])

    # ------------------------------------------------------------------

    def reset(
        self,
        *,
        seed: int | None = None,
        options: dict[str, Any] | None = None,
    ) -> tuple[np.ndarray, dict]:
        super().reset(seed=seed)
        # Swarm: bei Auto-Reset (seed=None) ggf. erfolgreichen Seed wiederholen
        if seed is None and self.swarm_pool is not None:
            pool_seed = self.swarm_pool.sample()
            if pool_seed is not None:
                seed = pool_seed
        actual_seed = seed if seed is not None else int(
            self.np_random.integers(0, 2**31 - 1)
        )
        self._current_seed = actual_seed
        self._step_count = 0
        self._visited_tiles = {(0, 0)}
        self._visit_counts = {(0, 0): 1}
        self._steps_no_reward = 0
        self._last_action = None
        self._last_reward = 0.0
        self._action_buffer = deque([None] * self.action_buffer_len, maxlen=self.action_buffer_len)
        # Die WorldGen-Config ist im C++-Core PROZESS-GLOBAL (mutableGameConfig()):
        # jede parallel existierende Env-Instanz (z.B. Eval-Env im Callback) überschreibt
        # sie für alle. Daher vor jedem Reset die eigene Config neu stempeln.
        self.core.configure_world_generation(
            exit_min_distance=self._exit_min,
            exit_max_distance=self._exit_max,
            force_guaranteed_path=False,
            disable_mobs=True,
            disable_energy=True,
        )
        raw = self.core.reset(actual_seed)
        return self._normalize(raw), {}

    def step(self, action: int) -> tuple[np.ndarray, float, bool, bool, dict]:
        self._step_count += 1
        raw, reward, terminated, truncated, info = self.core.step(int(action))

        # Position immer tracken (Stuck-Penalty braucht visit_counts)
        px, py = self.core.player_pos()
        self._visit_counts[(px, py)] = self._visit_counts.get((px, py), 0) + 1
        visit_count = self._visit_counts[(px, py)]
        if visit_count > 25:
            reward -= 0.03 * min(visit_count / 25.0, 2.0)

        # Visited Mask nur für CNN-Variante
        if self.use_visited_mask:
            self._visited_tiles.add((px, py))

        # Early Stopping: Truncate wenn 256 Schritte kein positiver Reward
        if reward > 0:
            self._steps_no_reward = 0
        else:
            self._steps_no_reward += 1
        if self._steps_no_reward >= 256 and not terminated:
            truncated = True
            info["early_stop"] = True

        # Aktions-Buffer und letzten Reward aktualisieren
        self._action_buffer.append(action)
        self._last_action = action
        self._last_reward = reward

        # Swarm / PLR
        if self.swarm_pool is not None:
            if self.swarm_pool.plr_mode:
                if info.get("reached_exit"):
                    self.swarm_pool.remove(self._current_seed)
                elif terminated or truncated:
                    self.swarm_pool.add(self._current_seed)
            else:
                if info.get("reached_exit"):
                    self.swarm_pool.add(self._current_seed)

        return self._normalize(raw), float(reward), bool(terminated), bool(truncated), dict(info)
