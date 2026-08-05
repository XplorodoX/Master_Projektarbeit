"""Nachbearbeitung realer Screenshots fuer die Dokumentation.

fig_game_real.png (C++-Client): Der obere Debug-Overlay-Streifen (Auto-Walk/
Forcefield-Buttons mit ueberlappenden Labels, Goal-Distance/Biome-Debugtext)
wird abgeschnitten -- er zeigt Entwickler-Debugoutput, nicht die reguläre
Spieler-UI -- und die HUD-Leiste (Werkzeuge, Leben, Energie) als vergroesserter
Callout unten angehaengt.

fig_livemap.png (Live-Dashboard): Die Karte des Agenten A0 wird als Callout
vergroessert, da alle vier Agenten-Minimaps im Original sehr klein sind.

Beide Callouts adressieren dieselbe Ursache: Der Text ist bei den in der
Dokumentation verwendeten Bildbreiten (0.72--0.9\\textwidth) sonst kaum lesbar.

Idempotent: liest immer vom Originalscreenshot in docs/figures_src/, nie vom
zuvor bereits nachbearbeiteten Ergebnis.
"""
import os
from PIL import Image, ImageDraw

FIG_DIR = "docs/figures"
SRC_DIR = "docs/figures_src"

HIGHLIGHT = (255, 196, 0)  # Amber-Gelb, stabil sichtbar auf Spiel- und UI-Grafik


def _compose_with_callout(base, box, scale, bg=(10, 12, 16), gap=26, margin=18):
    """Legt unter `base` einen vergroesserten, umrandeten Ausschnitt `box` an
    und verbindet beide mit Linien. Verbreitert die Leinwand, falls der
    Callout breiter als `base` wird."""
    bw, bh = base.size
    crop = base.crop(box)
    inset = crop.resize((int(crop.width * scale), int(crop.height * scale)), Image.LANCZOS)

    canvas_w = max(bw, inset.width + 2 * margin)
    canvas_h = bh + gap + inset.height + 2 * margin
    canvas = Image.new("RGB", (canvas_w, canvas_h), bg)

    base_x = (canvas_w - bw) // 2
    canvas.paste(base, (base_x, 0))

    draw = ImageDraw.Draw(canvas)
    box_abs = (box[0] + base_x, box[1], box[2] + base_x, box[3])
    draw.rectangle(box_abs, outline=HIGHLIGHT, width=5)

    inset_x = (canvas_w - inset.width) // 2
    inset_y = bh + gap
    canvas.paste(inset, (inset_x, inset_y))
    draw.rectangle(
        (inset_x - 3, inset_y - 3, inset_x + inset.width + 3, inset_y + inset.height + 3),
        outline=HIGHLIGHT, width=5,
    )
    draw.line((box_abs[0], box_abs[3], inset_x, inset_y), fill=HIGHLIGHT, width=2)
    draw.line((box_abs[2], box_abs[3], inset_x + inset.width, inset_y), fill=HIGHLIGHT, width=2)
    return canvas


def process_game_real():
    src_path = os.path.join(SRC_DIR, "fig_game_real.png")
    im = Image.open(src_path).convert("RGB")
    w, h = im.size

    # 1) Debug-Overlay oben abschneiden: zeigt Entwickler-Debugoutput
    #    (ueberlappende Button-Labels, Goal-Distance/Biome-Text), nicht die
    #    reguläre Spieler-UI.
    crop_top = int(round(270 / 1328 * h))
    world = im.crop((0, crop_top, w, h))
    ww, wh = world.size

    # 2) HUD-Box (Werkzeuge/Leben/Energie) als Callout, Text ist bei
    #    Dokumentbreite (0.72\textwidth) sonst kaum lesbar.
    hud_box = (
        int(0.069 * ww), int(0.619 * wh),
        int(0.944 * ww), int(0.822 * wh),
    )
    canvas = _compose_with_callout(world, hud_box, scale=1.4)

    out_path = os.path.join(FIG_DIR, "fig_game_real.png")
    canvas.save(out_path)
    print(f"geschrieben: {out_path} ({canvas.size[0]}x{canvas.size[1]})")


def process_livemap():
    src_path = os.path.join(SRC_DIR, "fig_livemap.png")
    im = Image.open(src_path).convert("RGB")

    # Karte des Agenten A0 (Minimap + bfs/episode-Header) als Callout, da die
    # vier Agenten-Minimaps im Original sehr klein sind.
    card_box = (8, 100, 202, 312)
    canvas = _compose_with_callout(im, card_box, scale=1.9, bg=(6, 8, 12))

    out_path = os.path.join(FIG_DIR, "fig_livemap.png")
    canvas.save(out_path)
    print(f"geschrieben: {out_path} ({canvas.size[0]}x{canvas.size[1]})")


if __name__ == "__main__":
    process_game_real()
    process_livemap()
