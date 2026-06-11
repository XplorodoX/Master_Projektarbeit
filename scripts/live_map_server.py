#!/usr/bin/env python3
"""Stoneforge Live Map Server — Echtzeit-Visualisierung des Agenten.

Startet einen lokalen HTTP-Server, der Positions-Updates vom Agenten
empfängt und sie im Browser als animierte Karte darstellt.

Verwendung (zwei Terminals):
    # Terminal 1 — Server starten:
    python scripts/live_map_server.py

    # Terminal 2 — Agent mit Live Map:
    python scripts/watch_agent.py --model models/ppo_lstm_curriculum_v2/best_model.zip --live-map

Dann Browser öffnen: http://localhost:8642
"""
from __future__ import annotations

import json
import os
import threading
import time
from http.server import BaseHTTPRequestHandler, HTTPServer

PORT = 8642
_HTML_PATH = os.path.join(os.path.dirname(__file__), "live_map.html")

_state_lock = threading.Lock()
_state: dict = {
    "episode": 0,
    "step": 0,
    "x": 0,
    "y": 0,
    "seed": 0,
    "bfs": 0,
    "success": None,
    "successes": 0,
    "total_eps": 0,
    "sr": 0.0,
    "exit_dx": 0.0,
    "exit_dy": 0.0,
    "running": False,
}


class _Handler(BaseHTTPRequestHandler):
    def log_message(self, *args) -> None:
        pass  # keine Request-Logs im Terminal

    def do_GET(self) -> None:
        if self.path in ("/", "/index.html"):
            try:
                with open(_HTML_PATH, "rb") as f:
                    body = f.read()
                self._send(200, "text/html; charset=utf-8", body)
            except FileNotFoundError:
                self._send(404, "text/plain", b"live_map.html nicht gefunden")
        elif self.path == "/state":
            with _state_lock:
                body = json.dumps(_state).encode()
            self._send(200, "application/json", body)
        else:
            self._send(404, "text/plain", b"Not found")

    def do_POST(self) -> None:
        if self.path == "/update":
            length = int(self.headers.get("Content-Length", 0))
            body = self.rfile.read(length)
            try:
                data = json.loads(body)
                with _state_lock:
                    _state.update(data)
                self._send(200, "text/plain", b"ok")
            except Exception as exc:
                self._send(400, "text/plain", str(exc).encode())
        else:
            self._send(404, "text/plain", b"Not found")

    def do_OPTIONS(self) -> None:
        self.send_response(200)
        self._cors_headers()
        self.end_headers()

    def _send(self, code: int, ctype: str, body: bytes) -> None:
        self.send_response(code)
        self.send_header("Content-Type", ctype)
        self.send_header("Content-Length", len(body))
        self._cors_headers()
        self.end_headers()
        self.wfile.write(body)

    def _cors_headers(self) -> None:
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")


def start_server(port: int = PORT) -> HTTPServer:
    """Startet den Server in einem Daemon-Thread. Gibt Server-Objekt zurück."""
    server = HTTPServer(("", port), _Handler)
    t = threading.Thread(target=server.serve_forever, daemon=True)
    t.start()
    return server


def send_update(data: dict, port: int = PORT) -> None:
    """Sendet ein State-Update an den lokalen Live Map Server (non-blocking)."""
    import urllib.request
    body = json.dumps(data).encode()
    req = urllib.request.Request(
        f"http://localhost:{port}/update",
        data=body,
        headers={"Content-Type": "application/json"},
        method="POST",
    )
    try:
        urllib.request.urlopen(req, timeout=0.05)
    except Exception:
        pass  # Server nicht erreichbar → still ignorieren


if __name__ == "__main__":
    print(f"╔══════════════════════════════════════╗")
    print(f"║   Stoneforge Live Map Server         ║")
    print(f"║   http://localhost:{PORT}             ║")
    print(f"╚══════════════════════════════════════╝")
    print(f"\nWarte auf Agent (watch_agent.py --live-map)...")
    print(f"Beenden mit Ctrl+C\n")
    server = start_server(PORT)
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        server.shutdown()
        print("\nServer gestoppt.")
