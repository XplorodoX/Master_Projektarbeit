from __future__ import annotations

import importlib
from typing import Any

import gymnasium as gym
import numpy as np
from gymnasium import spaces

stoneforge_sim = importlib.import_module("stoneforge_sim")

# Observation layout (after _normalize):
#
#  [0   : gs)    grid channel — {0,10,15,20,30}/30 → {0, 0.33, 0.5, 0.67, 1.0}
#  [gs]          hp           — /10
#  [gs+1]        energy       — /100
#  [gs+2]        inventory    — clip/64
#  [gs+3]        exitDx       — (exit.x - player.x) / 64   Luftlinie Richtung X
#  [gs+4]        exitDy       — (exit.y - player.y) / 64   Luftlinie Richtung Y
#
#  BFS-Kraftfeld (neu): 5 Features, Index gs+5 .. gs+9
#  [gs+5]  bfs_current  — aktueller Pfad-Abstand / 64         (0 = am Exit, 1 = weit)
#  [gs+6]  bfs_up       — Pfad-Abstand nach ↑ Schritt / 64
#  [gs+7]  bfs_down     — Pfad-Abstand nach ↓ Schritt / 64
#  [gs+8]  bfs_left     — Pfad-Abstand nach ← Schritt / 64
#  [gs+9]  bfs_right    — Pfad-Abstand nach → Schritt / 64
#
#  Interpretation Kraftfeld: kleinster Wert = optimale Richtung.
#  Wände → Manhattan-Fallback (großer Wert, nahe 1.0 nach Normalisierung).
#  Mit diesem Signal kann das Netz direkt den BFS-Gradienten folgen,
#  ohne die 225 Grid-Features dekodieren zu müssen.
#
#  Total: gs + 5 + 5 = 235  (für observationRadius=7 → gs=225)

_N_ACTIONS = 4
_BFS_MAX = 64.0   # Normalisierungskonstante (>= maximale Exit-Distanz im Eval-Set)


class StoneforgeWorldEnv(gym.Env[np.ndarray, int]):
    """Stoneforge PointGoal environment mit BFS-Kraftfeld-Observation.

    Observation enthält neben dem 15×15-Grid und exitDx/exitDy auch das
    4-direktionale BFS-Gradientenfeld: Der Agent sieht direkt welche Richtung
    seinen Pfad zum Exit wirklich verkürzt — auch wenn Wände die Luftlinie blockieren.
    """

    metadata = {"render_modes": []}

    def __init__(
        self,
        exit_min: int = 35,
        exit_max: int = 45,
        base_seed: int = 42,
    ) -> None:
        super().__init__()
        self.core = stoneforge_sim.StoneforgeCoreEnv(base_seed)
        self.core.configure_world_generation(
            exit_min_distance=exit_min,
            exit_max_distance=exit_max,
            force_guaranteed_path=True,
            disable_mobs=True,
            disable_energy=True,
        )
        self._base_seed = base_seed

        n_base = self.core.observation_size()   # gs + 5 = 230
        self._gs = n_base - 5                   # 225 für radius=7
        self._n_obs = n_base + 5                # +5 BFS-Kraftfeld-Features = 235

        self.action_space = spaces.Discrete(_N_ACTIONS)
        self.observation_space = spaces.Box(
            low=-1.0, high=1.0, shape=(self._n_obs,), dtype=np.float32
        )

    # ------------------------------------------------------------------

    def _bfs_field(self) -> np.ndarray:
        """5 BFS-Kraftfeld-Features: current + 4 Nachbarn (normalisiert auf [0,1])."""
        cur  = self.core.current_bfs_distance_to_exit()
        up   = self.core.bfs_distance_at_offset( 0, -1)
        down = self.core.bfs_distance_at_offset( 0,  1)
        left = self.core.bfs_distance_at_offset(-1,  0)
        right= self.core.bfs_distance_at_offset( 1,  0)
        return np.clip(
            np.array([cur, up, down, left, right], dtype=np.float32) / _BFS_MAX,
            0.0, 1.0
        )

    def _normalize(self, raw: list[int]) -> np.ndarray:
        arr = np.asarray(raw, dtype=np.float32)
        gs = self._gs
        arr[:gs]   /= 30.0                                            # grid
        arr[gs]    /= 10.0                                            # hp
        arr[gs+1]  /= 100.0                                           # energy
        arr[gs+2]   = np.clip(arr[gs+2], 0.0, 64.0) / 64.0          # inventory
        arr[gs+3]  /= 64.0                                            # exitDx
        arr[gs+4]  /= 64.0                                            # exitDy
        return np.concatenate([arr, self._bfs_field()])

    # ------------------------------------------------------------------

    def reset(
        self,
        *,
        seed: int | None = None,
        options: dict[str, Any] | None = None,
    ) -> tuple[np.ndarray, dict]:
        super().reset(seed=seed)
        actual_seed = seed if seed is not None else (
            self.np_random.integers(0, 2**31 - 1)
        )
        raw = self.core.reset(int(actual_seed))
        return self._normalize(raw), {}

    def step(self, action: int) -> tuple[np.ndarray, float, bool, bool, dict]:
        raw, reward, terminated, truncated, info = self.core.step(int(action))
        return self._normalize(raw), float(reward), bool(terminated), bool(truncated), dict(info)
