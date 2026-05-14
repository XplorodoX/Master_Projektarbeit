from __future__ import annotations

import numpy as np
import torch
import torch.nn as nn
import torch.nn.functional as F
import gymnasium as gym


class _RunningMeanStd:
    """Welford online mean/variance estimator."""

    def __init__(self, shape: tuple = (), epsilon: float = 1e-4):
        self.mean = np.zeros(shape, dtype=np.float64)
        self.var = np.ones(shape, dtype=np.float64)
        self.count = float(epsilon)

    def update(self, x: np.ndarray) -> None:
        x = np.asarray(x, dtype=np.float64).reshape(1, -1) if np.asarray(x).ndim <= 1 else np.asarray(x, dtype=np.float64)
        batch_count = x.shape[0]
        batch_mean = x.mean(axis=0)
        batch_var = x.var(axis=0)
        delta = batch_mean - self.mean
        total = self.count + batch_count
        self.mean = self.mean + delta * batch_count / total
        self.var = (
            self.var * self.count
            + batch_var * batch_count
            + delta ** 2 * self.count * batch_count / total
        ) / total
        self.count = total

    @property
    def std(self) -> np.ndarray:
        return np.sqrt(self.var + 1e-8)


class _RNDNet(nn.Module):
    def __init__(self, input_dim: int, output_dim: int):
        super().__init__()
        self.net = nn.Sequential(
            nn.Linear(input_dim, 128),
            nn.ReLU(),
            nn.Linear(128, 128),
            nn.ReLU(),
            nn.Linear(128, output_dim),
        )

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        return self.net(x)


class RNDWrapper(gym.Wrapper):
    """
    Random Network Distillation intrinsic motivation (Burda et al., 2018).

    Berechnet einen Bonus-Reward proportional zum Vorhersagefehler eines
    trainierbaren Predictor-Netzwerks gegenüber einem fixierten Target-Netzwerk.
    Neue, unbesuchte Zustände haben hohen Fehler → hoher Bonus.
    Wiederholt besuchte Zustände haben geringen Fehler → kaum Bonus.

    Ersetzt explizite Multi-Visit-, Stuck- und Pendel-Penalties durch einen
    systemischen Explorationsanreiz.
    """

    def __init__(
        self,
        env: gym.Env,
        embedding_dim: int = 64,
        lr: float = 1e-4,
        intrinsic_scale: float = 0.5,
        update_proportion: float = 0.25,
    ):
        super().__init__(env)
        obs_dim = int(np.prod(env.observation_space.shape))

        self.intrinsic_scale = intrinsic_scale
        self.update_proportion = update_proportion

        # Fixiertes Ziel-Netzwerk — wird nie aktualisiert
        self._target = _RNDNet(obs_dim, embedding_dim)
        for p in self._target.parameters():
            p.requires_grad = False

        # Trainierbares Predictor-Netzwerk
        self._predictor = _RNDNet(obs_dim, embedding_dim)
        self._optimizer = torch.optim.Adam(self._predictor.parameters(), lr=lr)

        # Laufende Statistiken zur Normalisierung
        self._reward_rms = _RunningMeanStd(shape=(1,))
        self._obs_rms = _RunningMeanStd(shape=(obs_dim,))

    def step(self, action):
        obs, reward, terminated, truncated, info = self.env.step(action)
        intrinsic = self._step_rnd(obs)
        info["rnd_intrinsic"] = intrinsic
        return obs, float(reward) + self.intrinsic_scale * intrinsic, terminated, truncated, info

    def reset(self, **kwargs):
        return self.env.reset(**kwargs)

    def _normalize_obs(self, obs: np.ndarray) -> torch.Tensor:
        flat = obs.flatten()
        self._obs_rms.update(flat)
        normed = np.clip((flat - self._obs_rms.mean) / self._obs_rms.std, -5.0, 5.0)
        return torch.as_tensor(normed, dtype=torch.float32)

    def _step_rnd(self, obs: np.ndarray) -> float:
        obs_t = self._normalize_obs(obs).unsqueeze(0)

        with torch.no_grad():
            target_feat = self._target(obs_t)

        pred_feat = self._predictor(obs_t)
        loss = F.mse_loss(pred_feat, target_feat.detach())
        raw_error = loss.item()

        # Predictor nur auf einem Bruchteil der Schritte updaten — verhindert
        # sofortiges Auswendiglernen aller Zustände.
        if np.random.random() < self.update_proportion:
            self._optimizer.zero_grad()
            loss.backward()
            self._optimizer.step()

        # Intrinsischen Reward mit laufender Varianz normalisieren.
        self._reward_rms.update(np.array([[raw_error]]))
        return float(raw_error / (self._reward_rms.std[0] + 1e-8))
