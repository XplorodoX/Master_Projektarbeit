"""Stoneforge CNN Feature Extractor für SB3 RecurrentPPO (Ablation D).

Verarbeitet das 15×15 Tile-Grid als 2D-Bild mit zwei Kanälen:
  Kanal 0: Tile-Typen   (Wände, Boden, Exit)
  Kanal 1: Visited Mask (welche Tiles wurden in dieser Episode betreten)

Die nicht-räumlichen Features (exitDx, exitDy, step_frac, …) werden direkt
mit dem CNN-Output konkateniert und gemeinsam in den LSTM eingespeist.

Obs-Layout (use_visited_mask=True, 456 dims):
  [0   : 225)   Kanal 0 — Tile-Typen   (flach, wird zu 15×15 reshapet)
  [225 : 450)   Kanal 1 — Visited Mask (flach, wird zu 15×15 reshapet)
  [450 : 456)   Extras  — hp, energy, inventory, exitDx, exitDy, step_frac
"""
from __future__ import annotations

import torch
import torch.nn as nn
from gymnasium import spaces
from stable_baselines3.common.torch_layers import BaseFeaturesExtractor

GRID_SIDE    = 15
GRID_FLAT    = GRID_SIDE * GRID_SIDE   # 225
N_CHANNELS   = 2
SPATIAL_FLAT = N_CHANNELS * GRID_FLAT  # 450
N_EXTRAS     = 6                       # hp, energy, inv, exitDx, exitDy, step_frac


class StoneforgeGridCNN(BaseFeaturesExtractor):
    """CNN über das 15×15 Grid (2 Kanäle) + direkte Extras.

    Architektur:
        (2, 15, 15)
          → Conv2d(2→16, 3×3, pad=1)  [→ 16×15×15]
          → ReLU
          → Conv2d(16→32, 3×3, pad=1) [→ 32×15×15]
          → ReLU
          → MaxPool2d(3, stride=3)    [→ 32×5×5 = 800]
          → Conv2d(32→64, 3×3, pad=1) [→ 64×5×5 = 1600]
          → ReLU + Flatten
          → Linear(1600 → cnn_out_features)
          → ReLU
        concat mit Extras (6 dims)
        → features_dim = cnn_out_features + N_EXTRAS
    """

    def __init__(
        self,
        observation_space: spaces.Box,
        cnn_out_features: int = 128,
    ) -> None:
        features_dim = cnn_out_features + N_EXTRAS
        super().__init__(observation_space, features_dim=features_dim)

        self.cnn_out_features = cnn_out_features

        self.cnn = nn.Sequential(
            nn.Conv2d(N_CHANNELS, 16, kernel_size=3, padding=1),
            nn.ReLU(),
            nn.Conv2d(16, 32, kernel_size=3, padding=1),
            nn.ReLU(),
            nn.MaxPool2d(kernel_size=3, stride=3),   # 15→5
            nn.Conv2d(32, 64, kernel_size=3, padding=1),
            nn.ReLU(),
            nn.Flatten(),
        )

        # Tatsächliche CNN-Ausgabegröße berechnen
        with torch.no_grad():
            dummy = torch.zeros(1, N_CHANNELS, GRID_SIDE, GRID_SIDE)
            cnn_flat = self.cnn(dummy).shape[1]

        self.cnn_proj = nn.Sequential(
            nn.Linear(cnn_flat, cnn_out_features),
            nn.ReLU(),
        )

    def forward(self, obs: torch.Tensor) -> torch.Tensor:
        # obs: (batch, 456)
        grid_flat = obs[:, :SPATIAL_FLAT]           # (batch, 450)
        extras    = obs[:, SPATIAL_FLAT:]           # (batch, 6)

        # Reshape → (batch, 2, 15, 15)
        grid_2d   = grid_flat.view(-1, N_CHANNELS, GRID_SIDE, GRID_SIDE)

        cnn_out   = self.cnn_proj(self.cnn(grid_2d))  # (batch, cnn_out_features)
        return torch.cat([cnn_out, extras], dim=1)     # (batch, features_dim)
