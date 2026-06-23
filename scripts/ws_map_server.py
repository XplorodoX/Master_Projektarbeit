"""Combined HTTP + WebSocket Server für Stoneforge Live Map.

Alles auf einem Port (default 8766):
  GET /          → ws_map.html
  GET /ws        → WebSocket-Verbindung (Browser)
  Intern         → update_agent() / update_meta() direkt im Prozess aufrufen

Kein Cross-Port-Problem mehr — HTTP und WS laufen auf demselben Port.
"""
from __future__ import annotations

import asyncio
import json
import os
import threading
import time

# ──────────────────────────────────────────────────────────────────────────────
# Shared state (in-process, thread-safe)
# ──────────────────────────────────────────────────────────────────────────────
_loop:     asyncio.AbstractEventLoop | None = None
_ws_conns: set   = set()
_lock             = threading.Lock()

_state: dict = {
    "agents": {},
    "meta":   {},
}

# Heatmap: (rx, ry) → Besuchsanzahl, relativ zum Startpunkt jedes Agents
_heatmap: dict[tuple[int, int], int] = {}
HMAP_RANGE = 70   # ±70 Tiles vom Start — reicht für Phase 3 (exit=45)


def update_heatmap(rx: int, ry: int) -> None:
    """Zählt einen Besuch an relativer Position (rx, ry). Thread-safe."""
    if abs(rx) > HMAP_RANGE or abs(ry) > HMAP_RANGE:
        return
    with _lock:
        key = (rx, ry)
        _heatmap[key] = _heatmap.get(key, 0) + 1

_HTML_PATH = os.path.join(os.path.dirname(__file__), "ws_map.html")


# ──────────────────────────────────────────────────────────────────────────────
# Public API — aus beliebigem Thread aufrufbar
# ──────────────────────────────────────────────────────────────────────────────

def update_agent(agent_id: int, data: dict) -> None:
    with _lock:
        _state["agents"][str(agent_id)] = data


def update_meta(meta: dict) -> None:
    with _lock:
        _state["meta"].update(meta)


# ──────────────────────────────────────────────────────────────────────────────
# Async Server (aiohttp)
# ──────────────────────────────────────────────────────────────────────────────

async def _broadcast_loop() -> None:
    global _ws_conns
    tick = 0
    while True:
        await asyncio.sleep(0.1)
        if not _ws_conns:
            tick += 1
            continue
        tick += 1
        with _lock:
            msg: dict = {
                "agents": list(_state["agents"].values()),
                "meta":   _state["meta"],
            }
            # Heatmap nur jede Sekunde senden (Datenmenge sparen)
            if tick % 10 == 0 and _heatmap:
                max_count = max(_heatmap.values())
                msg["heatmap"] = {
                    "cells":     [[k[0], k[1], v] for k, v in _heatmap.items()],
                    "max_count": max_count,
                    "range":     HMAP_RANGE,
                }
        payload = json.dumps(msg)
        dead = set()
        for ws in list(_ws_conns):
            try:
                await ws.send_str(payload)
            except Exception:
                dead.add(ws)
        _ws_conns -= dead


async def _ws_handler(request):
    from aiohttp import web
    ws = web.WebSocketResponse()
    await ws.prepare(request)
    _ws_conns.add(ws)
    try:
        async for _ in ws:
            pass   # Viewer sendet nichts, wir lesen trotzdem um close zu erkennen
    finally:
        _ws_conns.discard(ws)
    return ws


async def _html_handler(request):
    from aiohttp import web
    try:
        with open(_HTML_PATH, "r", encoding="utf-8") as f:
            html = f.read()
        return web.Response(text=html, content_type="text/html")
    except FileNotFoundError:
        return web.Response(text="ws_map.html nicht gefunden", status=404)


async def _run(host: str, port: int) -> None:
    from aiohttp import web
    app = web.Application()
    app.router.add_get("/",    _html_handler)
    app.router.add_get("/ws",  _ws_handler)
    runner = web.AppRunner(app)
    await runner.setup()
    site = web.TCPSite(runner, host, port)
    await site.start()
    asyncio.create_task(_broadcast_loop())
    await asyncio.Future()   # läuft bis Prozessende


def start_server(host: str = "localhost", port: int = 8766) -> bool:
    global _loop
    if _loop is not None:
        return True
    try:
        import aiohttp  # noqa
    except ImportError:
        print("  ⚠️  aiohttp nicht installiert — Live Map deaktiviert")
        print("      pip install aiohttp")
        return False

    _loop = asyncio.new_event_loop()
    t = threading.Thread(
        target=lambda: _loop.run_until_complete(_run(host, port)),
        daemon=True,
        name="ws-map-server",
    )
    t.start()
    time.sleep(0.3)
    return True
