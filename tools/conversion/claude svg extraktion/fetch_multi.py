#!/usr/bin/env python3
"""
Schritt 1 (erweitert): Ruft die Messseiten mehrerer Rechner gleichzeitig,
unabhaengig voneinander, im Takt der L1-Statistik (2s) ab und speichert
jede Antwort mit Zeitstempel im jeweiligen Rohdatenverzeichnis.

Kernidee gegenueber der Single-Host-Version:
  - Jeder Rechner bekommt einen eigenen Thread mit eigener Taktschleife.
  - urlopen() blockiert nur den jeweiligen Thread, nicht die anderen.
  - Ein haengender/langsamer Rechner (z.B. durch Netzwerkproblem am
    Port-Forwarding) verzoegert damit NICHT die Abrufe der anderen drei.
  - Jeder Host schreibt in sein eigenes Unterverzeichnis.

Hinweis: Das <meta http-equiv="refresh" content="4"> im HTML ist nur ein
Hinweis fuers Browser-Auto-Reload, es hat keinerlei Einfluss auf HTTP-Abrufe
per curl/requests. Wir pollen unabhaengig davon in unserem eigenen Takt.

macOS-Betrieb:
  - Fuer Dauerbetrieb per launchd (empfohlen, ueberlebt Neustarts/Logout),
    NICHT per cron (min. Aufloesung 60s, hier brauchen wir 2s).
  - Alternativ einfach in einem Terminal/tmux laufen lassen.
"""

import time
import datetime as dt
import pathlib
import threading
import urllib.request

# -----------------------------------------------------------------------
# Konfiguration: hier die 4 Messwertrechner eintragen.
# "name" wird als Unterverzeichnisname und im Log verwendet.
# "url" ist der ueber Port-Forwarding erreichbare lokale Endpunkt.
# -----------------------------------------------------------------------
HOSTS = [
    {"name": "rechner1", "url": "http://localhost:8051/"},
    {"name": "rechner2", "url": "http://localhost:8052/"},
    {"name": "rechner3", "url": "http://localhost:8053/"},
    {"name": "rechner4", "url": "http://localhost:8054/"},
]

BASE_OUT_DIR = pathlib.Path("data/raw")
POLL_SECONDS = 2.0          # Takt der L1-Statistik
REQUEST_TIMEOUT = 3.0       # muss < POLL_SECONDS sein, sonst Taktdrift

_log_lock = threading.Lock()
_stop_event = threading.Event()


def log(msg: str) -> None:
    """Thread-sicheres Logging, damit sich Ausgaben nicht vermischen."""
    with _log_lock:
        print(msg, flush=True)


def fetch_once(name: str, url: str, out_dir: pathlib.Path) -> None:
    ts = dt.datetime.now().strftime("%Y%m%dT%H%M%S_%f")[:-3]  # ms-genau
    try:
        with urllib.request.urlopen(url, timeout=REQUEST_TIMEOUT) as resp:
            body = resp.read()
    except Exception as exc:
        log(f"[{name}] [{ts}] Fehler beim Abruf: {exc}")
        return
    out_path = out_dir / f"{ts}.html"
    out_path.write_bytes(body)
    log(f"[{name}] [{ts}] gespeichert -> {out_path} ({len(body)} bytes)")


def worker(name: str, url: str) -> None:
    """Eigene, unabhaengige Taktschleife pro Rechner (eigener Thread)."""
    out_dir = BASE_OUT_DIR / name
    out_dir.mkdir(parents=True, exist_ok=True)

    next_tick = time.monotonic()
    while not _stop_event.is_set():
        fetch_once(name, url, out_dir)
        next_tick += POLL_SECONDS
        sleep_for = next_tick - time.monotonic()
        if sleep_for > 0:
            # Kurzes Warten in kleinen Schritten, damit ein Ctrl-C
            # zuegig reagiert statt bis zu 2s zu warten.
            _stop_event.wait(timeout=sleep_for)
        else:
            # Abruf hat laenger als POLL_SECONDS gedauert -> Takt neu ausrichten
            next_tick = time.monotonic()


def main() -> None:
    threads = []
    for host in HOSTS:
        t = threading.Thread(
            target=worker,
            args=(host["name"], host["url"]),
            name=f"fetch-{host['name']}",
            daemon=True,
        )
        threads.append(t)
        t.start()

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        log("Beende alle Abruf-Threads...")
        _stop_event.set()
        for t in threads:
            t.join(timeout=REQUEST_TIMEOUT + 1)
        log("Alle Threads beendet.")


if __name__ == "__main__":
    main()