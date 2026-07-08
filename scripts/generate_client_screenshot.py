import sys
import os
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches

# Make sure we can import from python/
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "python")))
from stoneforge_env import StoneforgeWorldEnv

def main():
    env = StoneforgeWorldEnv(exit_min=35, exit_max=45)
    seed = 7042
    obs, _ = env.reset(seed=seed)
    
    px, py = env.core.player_pos()
    ex, ey = env.core.exit_pos()
    
    print(f"Player at: {px}, {py}")
    print(f"Exit at: {ex}, {ey}")
    
    # Viewport size in C++ client
    rx, ry = 24, 14
    
    # Setup figure
    fig, ax = plt.subplots(figsize=(10.2, 6.2), dpi=150)
    fig.patch.set_facecolor('#14161c')
    ax.set_facecolor('#14161c')
    
    # Define colors matching the C++ client
    color_map = {
        0: "#282f38",  # Empty (Dark blue-grey)
        1: "#555861",  # Wall (Grey)
        2: "#b7862d",  # Resource (Gold-yellow)
        3: "#2ecc71",  # Exit (Green)
        4: "#5b944e",  # Tree (Forest green)
        5: "#9c6f48",  # Workbench (Brown)
        6: "#a4764f",  # WoodWall (Light brown)
        7: "#886142",  # WoodLog (Medium brown)
    }
    # For structures 8 to 14
    for i in range(8, 15):
        color_map[i] = "#5c5043"
        
    # Draw cells
    for vy in range(-ry, ry + 1):
        for vx in range(-rx, rx + 1):
            wx = px + vx
            wy = py + vy
            tile = env.core.tile_at(wx, wy)
            
            # If it's the exit tile, override color
            if wx == ex and wy == ey:
                color = color_map[3]
            else:
                color = color_map.get(tile, "#ffffff")
                
            # Draw cell rectangle
            rect = patches.Rectangle((vx - 0.45, vy - 0.45), 0.9, 0.9, facecolor=color, edgecolor="none")
            ax.add_patch(rect)
            
    # Draw player in center (sky blue)
    player_rect = patches.Rectangle((-0.45, -0.45), 0.9, 0.9, facecolor="#3498db", edgecolor="none")
    ax.add_patch(player_rect)
    
    # Draw Title Bar Mockup
    title_y_start = ry + 0.8
    title_height = 1.8
    title_bar = patches.Rectangle((-rx - 0.8, title_y_start), rx * 2 + 1.6, title_height, facecolor="#2d313d", edgecolor="none", zorder=10)
    ax.add_patch(title_bar)
    
    # Window controls (macOS style dots)
    dot_y = title_y_start + title_height / 2
    ax.add_patch(patches.Circle((-rx - 0.2, dot_y), 0.25, facecolor="#ff5f56", edgecolor="none", zorder=11))
    ax.add_patch(patches.Circle((-rx + 0.4, dot_y), 0.25, facecolor="#ffbd2e", edgecolor="none", zorder=11))
    ax.add_patch(patches.Circle((-rx + 1.0, dot_y), 0.25, facecolor="#27c93f", edgecolor="none", zorder=11))
    
    # Window Title text
    ax.text(-rx + 2.0, dot_y, f"Stoneforge 2D | HP=10 | Energy=100 | Inv=0 | Steps=0 | Seed={seed}",
            color="white", fontsize=9, fontweight="bold", va="center", zorder=11)
            
    # Settings to hide axis and set scale
    ax.set_xlim(-rx - 0.8, rx + 0.8)
    ax.set_ylim(-ry - 0.8, title_y_start + title_height)
    ax.set_aspect("equal")
    plt.axis('off')
    
    plt.tight_layout()
    os.makedirs("docs/figures", exist_ok=True)
    plt.savefig("docs/figures/fig_player_client.png", dpi=150, facecolor=fig.get_facecolor(), edgecolor='none', bbox_inches='tight')
    plt.close()
    
    print("Player client screenshot saved successfully in docs/figures/fig_player_client.png")

if __name__ == "__main__":
    main()
