#!/usr/bin/env python3
"""
Schritt 1: Ruft die Messseite im Takt der L1-Statistik (2s) ab und speichert
jede Antwort mit Zeitstempel im Rohdatenverzeichnis.

Hinweis: Das <meta http-equiv="refresh" content="4"> im HTML ist nur ein
Hinweis fuers Browser-Auto-Reload, es hat keinerlei Einfluss auf HTTP-Abrufe
per curl/requests. Wir pollen unabhaengig davon in unserem eigenen Takt.

macOS-Betrieb:
  - Fuer Dauerbetrieb per launchd (empfohlen, ueberlebt Neustarts/Logout),
    NICHT per cron (min. Aufloesung 60s, hier brauchen wir 2s).
  - Alternativ einfach in einem Terminal/tmux laufen lassen.

launchd-Beispiel (~/Library/LaunchAgents/com.spacetime.fetch.plist):
  <key>ProgramArguments</key>
  <array>
    <string>/usr/bin/python3</string>
    <string>/PFAD/ZU/1_fetch.py</string>
  </array>
  <key>KeepAlive</key><true/>
  <key>RunAtLoad</key><true/>
laden mit: launchctl load ~/Library/LaunchAgents/com.spacetime.fetch.plist
"""

import time
import datetime as dt
import pathlib
import urllib.request

URL = "http://<IP-DES-MESSGERAETS>/"          # <-- anpassen
OUT_DIR = pathlib.Path("data/raw")
POLL_SECONDS = 2.0                             # Takt der L1-Statistik

OUT_DIR.mkdir(parents=True, exist_ok=True)


def fetch_once() -> None:
    ts = dt.datetime.now().strftime("%Y%m%dT%H%M%S_%f")[:-3]  # ms-genau
    try:
        with urllib.request.urlopen(URL, timeout=5) as resp:
            body = resp.read()
    except Exception as exc:
        print(f"[{ts}] Fehler beim Abruf: {exc}")
        return
    out_path = OUT_DIR / f"{ts}.html"
    out_path.write_bytes(body)
    print(f"[{ts}] gespeichert -> {out_path} ({len(body)} bytes)")


def main() -> None:
    next_tick = time.monotonic()
    while True:
        fetch_once()
        next_tick += POLL_SECONDS
        sleep_for = next_tick - time.monotonic()
        if sleep_for > 0:
            time.sleep(sleep_for)
        else:
            # Abruf hat laenger als POLL_SECONDS gedauert -> Takt neu ausrichten
            next_tick = time.monotonic()


if __name__ == "__main__":
    main()
