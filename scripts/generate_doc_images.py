import sys
import os
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib.lines import Line2D

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
    
    # Query a 31x31 area centered on the player
    radius = 15
    grid_size = radius * 2 + 1
    
    tile_grid = np.zeros((grid_size, grid_size), dtype=int)
    for r in range(grid_size):
        for c in range(grid_size):
            wx = px + (c - radius)
            wy = py + (r - radius)
            tile_grid[r, c] = env.core.tile_at(wx, wy)
            
    # Mappings from TileType to colors
    # Empty = 0, Wall = 1, Resource = 2, Exit = 3, Tree = 4, Workbench = 5
    color_map = {
        0: "#e6ecd8",  # Empty (Grassland)
        1: "#70766d",  # Wall (Stone)
        2: "#cca33d",  # Resource (Ore)
        3: "#ff4d4d",  # Exit (Goal)
        4: "#2e5c1e",  # Tree (Wood)
        5: "#8B5A2B",  # Workbench
        6: "#a0522d",  # WoodWall
        7: "#cd853f",  # WoodLog
    }
    # For structures 8 to 14
    for i in range(8, 15):
        color_map[i] = "#574e44"
        
    # --- PLOT 1: GAME WORLD VIEW ---
    fig, ax = plt.subplots(figsize=(6, 6), dpi=150)
    
    for r in range(grid_size):
        for c in range(grid_size):
            tile = tile_grid[r, c]
            color = color_map.get(tile, "#ffffff")
            # coordinates centered around 0 for player
            dx = c - radius
            dy = r - radius
            ax.add_patch(patches.Rectangle((dx, dy), 1, 1, facecolor=color, edgecolor="#cccccc", linewidth=0.2))
            
            if tile == 4: # Tree
                ax.plot(dx + 0.5, dy + 0.5, marker='^', color='#143d0e', markersize=5, zorder=3)
            elif tile == 3: # Exit
                ax.plot(dx + 0.5, dy + 0.5, marker='*', color='#ffff00', markersize=10, markeredgecolor='black', zorder=4)
                
    # Draw player in center
    ax.add_patch(patches.Circle((0.5, 0.5), 0.4, facecolor="#4d79ff", edgecolor="black", linewidth=1, zorder=5))
    
    # Title & limits
    ax.set_xlim(-radius, radius + 1)
    ax.set_ylim(-radius, radius + 1)
    ax.set_aspect("equal")
    ax.set_xticks(range(-radius, radius + 2, 5))
    ax.set_yticks(range(-radius, radius + 2, 5))
    ax.grid(True, which='both', color='#cccccc', linestyle='--', linewidth=0.5)
    ax.set_title("Stoneforge: Originales Spiel (31x31)", fontsize=14, fontweight='bold')
    ax.tick_params(axis='both', labelsize=11)

    legend_elements = [
        patches.Patch(facecolor="#e6ecd8", label="Boden (Empty)"),
        patches.Patch(facecolor="#70766d", label="Wand (Wall)"),
        patches.Patch(facecolor="#2e5c1e", label="Baum (Tree)"),
        patches.Patch(facecolor="#cca33d", label="Erz (Resource)"),
        patches.Patch(facecolor="#ff4d4d", label="Ausgang (Exit)"),
        patches.Patch(facecolor="#4d79ff", label="Agent"),
    ]
    ax.legend(handles=legend_elements, loc="upper right", fontsize=10.5)
    
    plt.tight_layout()
    os.makedirs("docs/figures", exist_ok=True)
    plt.savefig("docs/figures/fig_game_world.png", dpi=150)
    plt.close()
    
    # --- PLOT 2: AI OBSERVATION VIEW ---
    fig, ax = plt.subplots(figsize=(6, 6), dpi=150)
    ai_radius = 7  # 15x15 grid has radius 7 around player
    
    for r in range(grid_size):
        for c in range(grid_size):
            dx = c - radius
            dy = r - radius
            if -ai_radius <= dx <= ai_radius and -ai_radius <= dy <= ai_radius:
                # Inside observation window
                tile = tile_grid[r, c]
                color = color_map.get(tile, "#ffffff")
                ax.add_patch(patches.Rectangle((dx, dy), 1, 1, facecolor=color, edgecolor="#cccccc", linewidth=0.2))
                if tile == 4:
                    ax.plot(dx + 0.5, dy + 0.5, marker='^', color='#143d0e', markersize=5, zorder=3)
                elif tile == 3:
                    ax.plot(dx + 0.5, dy + 0.5, marker='*', color='#ffff00', markersize=10, markeredgecolor='black', zorder=4)
            else:
                # Blacked out (AI cannot see)
                ax.add_patch(patches.Rectangle((dx, dy), 1, 1, facecolor="#1c1c1c", edgecolor="#262626", linewidth=0.2))
                
    # Draw player
    ax.add_patch(patches.Circle((0.5, 0.5), 0.4, facecolor="#4d79ff", edgecolor="black", linewidth=1, zorder=5))
    
    # Draw boundary of AI field of view (15x15 grid)
    rect = patches.Rectangle((-ai_radius, -ai_radius), ai_radius * 2 + 1, ai_radius * 2 + 1, linewidth=2, edgecolor='#ff3333', facecolor='none', linestyle='--', zorder=10)
    ax.add_patch(rect)
    
    # Draw Exit Direction Compass arrow
    ex_dx = ex - px
    ex_dy = ey - py
    length = np.sqrt(ex_dx**2 + ex_dy**2)
    if length > 0:
        # Draw compass arrow from agent center (0.5, 0.5)
        arrow_length = 3.0
        ux = (ex_dx / length) * arrow_length
        uy = (ex_dy / length) * arrow_length
        ax.annotate('', xy=(0.5 + ux, 0.5 + uy), xytext=(0.5, 0.5),
                    arrowprops=dict(facecolor='#ffff00', edgecolor='black', width=3, headwidth=8, zorder=15),
                    zorder=15)
        
    ax.set_xlim(-radius, radius + 1)
    ax.set_ylim(-radius, radius + 1)
    ax.set_aspect("equal")
    ax.set_xticks(range(-radius, radius + 2, 5))
    ax.set_yticks(range(-radius, radius + 2, 5))
    ax.grid(True, which='both', color='#333333', linestyle='--', linewidth=0.5)
    ax.set_title("Sichtweise der KI (15x15 Obs + Kompass)", fontsize=14, fontweight='bold')
    ax.tick_params(axis='both', labelsize=11)

    legend_elements_ai = [
        patches.Patch(facecolor="#e6ecd8", label="Sichtbarer Boden"),
        patches.Patch(facecolor="#70766d", label="Sichtbare Wand"),
        patches.Patch(facecolor="#1c1c1c", label="Unbekannt (Nebel)"),
        patches.Patch(facecolor="none", edgecolor="#ff3333", linestyle='--', label="Sichtgrenze (15x15)"),
        Line2D([0], [0], color='#ffff00', marker='>', markersize=8, markeredgecolor='black', label="Exit-Kompassrichtung", linestyle=''),
        patches.Patch(facecolor="#4d79ff", label="Agent"),
    ]
    ax.legend(handles=legend_elements_ai, loc="upper right", fontsize=10.5)
    
    plt.tight_layout()
    plt.savefig("docs/figures/fig_ai_observation.png", dpi=150)
    plt.close()
    
    print("Images saved successfully in docs/figures/")

if __name__ == "__main__":
    main()
