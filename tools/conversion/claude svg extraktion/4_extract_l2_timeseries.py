#!/usr/bin/env python3
"""
Schritt 4 (L2-Fokus): erzeugt eine luecklose 2-sec-Zeitreihe der L2-Statistik
(avg/min/max/burst/pause) fuer einen Detektor ueber [--start, --start+--duration].

Kernidee (siehe Ruecksprache): jede einzelne Chart-Momentaufnahme enthaelt in
der "Zoom"-Kurve (SVG-Bereich ZOOM_Y) nicht nur den neuesten Messwert, sondern
eine vollstaendige Historie von >30 Minuten im 2-sec-Raster (x-Achse
"time [min]", 0 .. -30). Damit laesst sich JEDER 2-sec-Slot im Zielfenster
aus dem zeitlich naechstgelegenen erfolgreich abgerufenen Chart rekonstru-
ieren, auch wenn der Abruf selbst zum eigentlichen Zeitpunkt fehlgeschlagen
ist -- solange irgendein Chart bis zu ~30 Minuten spaeter wieder verfuegbar
war.

Ablauf:
    1. Alle Rohdaten-Charts im erweiterten Suchfenster
       [start - lookback, start + duration + lookback] laden (lookback =
       Reichweite der Zoom-Historie, s. ZOOM_LOOKBACK_MIN).
    2. Je Chart: Zeitachsen-Kalibrierung ueber die "time [min]"-Tickbe-
       schriftung (linear), y-Achsen-Kalibrierung wie bisher ueber die
       "...ps"-Vollausschlag-Beschriftung.
    3. Alle Punkte jeder der 5 Zoom-Kurven in eine absolute Zeit umrechnen
       und dem naechstgelegenen 2-sec-Slot zuordnen (Toleranz +/- 1s).
    4. Chronologisch verarbeiten: neuere Charts ueberschreiben aeltere
       Rekonstruktionen desselben Slots (aktuellster verfuegbarer Stand
       gewinnt).
    5. Slots ohne jegliche Deckung bleiben leer (= Feld bleibt in der CSV
       leer/NaN) -- das ist dann eine tatsaechlich nicht rekonstruierbare
       Luecke (kein Chart-Abruf innerhalb der Lookback-Reichweite nach der
       Unterbrechung).

Ausgabe: EINE CSV-Datei je Lauf, KEIN Zeitstempel je Zeile (Zeilenindex *
--poll-interval ab --start ergibt die Zeit), KEINE Indexdatei -- wie
gewuenscht platzsparend/lesbar zugleich.

Aufruf:
    python3 4_extract_l2_timeseries.py \
        --detector D1 \
        --start "2026-08-12 17:56:41" \
        --duration 3600 \
        --raw-dir data/raw \
        --out-dir data/preprocessed
"""

import argparse
import csv
import importlib.util
import pathlib
import re
import sys
import datetime as dt

# ---- Kalibrierungs-Konstanten der Zoom-Kurve (Firmware v1.0.1, per Hand
#      aus der SVG-Struktur ermittelt -- bei Firmware-Aenderung pruefen) ----
ZOOM_Y = (190, 340)          # y-Pixelgrenzen des Zoom-Chart-Bereichs
ZOOM_TIME_LABEL_Y = 320      # y-Position der "time [min]"-Zahlenbeschriftung
ZOOM_LOOKBACK_MIN = 32       # Sicherheitsmarge oberhalb der 30 min (Chart geht laut
                              # Pixel-Messung leicht darueber hinaus, siehe Analyse)
ZOOM_COLORS = {"avg": "black", "min": "red", "max": "blue",
               "burst": "darkgoldenrod", "pause": "olive"}


def _load_parse_lib(module_path: pathlib.Path = None):
    if module_path is None:
        module_path = pathlib.Path(__file__).parent / "2_parse_and_convert.py"
    if not module_path.exists():
        raise FileNotFoundError(
            f"{module_path} nicht gefunden - bitte 2_parse_and_convert.py "
            f"in dasselbe Verzeichnis legen, oder Pfad ueber --parse-lib angeben.")
    spec = importlib.util.spec_from_file_location("parse_lib", module_path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


lib = _load_parse_lib()


# ---------------------------------------------------------------- Zeit-Utils
FILENAME_TS_RE = re.compile(r'^(\d{8}T\d{6})(?:_(\d{1,3}))?$')


def parse_start(s: str) -> dt.datetime:
    """Flexibler Parser fuer --start: akzeptiert das interne Dateinamens-
    Format (mit/ohne Millisekunden) sowie gaengige ISO-/lesbare Formate."""
    s = s.strip()
    m = FILENAME_TS_RE.match(s)
    if m:
        base, ms = m.groups()
        d = dt.datetime.strptime(base, "%Y%m%dT%H%M%S")
        if ms:
            d = d.replace(microsecond=int(ms.ljust(3, "0")) * 1000)
        return d
    try:
        return dt.datetime.fromisoformat(s.replace(" ", "T"))
    except ValueError:
        pass
    for fmt in ("%Y-%m-%d %H:%M:%S", "%Y-%m-%d %H:%M", "%Y%m%d %H%M%S",
                "%d.%m.%Y %H:%M:%S", "%d.%m.%Y %H:%M"):
        try:
            return dt.datetime.strptime(s, fmt)
        except ValueError:
            continue
    raise ValueError(f"Zeitstempel-Format fuer --start nicht erkannt: {s!r}")


def parse_raw_filename_ts(stem: str) -> dt.datetime:
    m = FILENAME_TS_RE.match(stem)
    if not m:
        raise ValueError(f"Zeitstempel im Dateinamen nicht erkannt: {stem!r}")
    base, ms = m.groups()
    d = dt.datetime.strptime(base, "%Y%m%dT%H%M%S")
    if ms:
        d = d.replace(microsecond=int(ms.ljust(3, "0")) * 1000)
    return d


def fmt_ts(t: dt.datetime) -> str:
    return t.strftime("%Y%m%dT%H%M%S") + f"_{t.microsecond // 1000:03d}"


# --------------------------------------------------------- Zoom-Dekodierung
def zoom_time_calibration(texts):
    """Liefert (slope, intercept) fuer minute_offset = slope*x_px + intercept,
    basierend auf den Zahlen-Ticks der 'time [min]'-Achse."""
    ticks = []
    for x, y, t in texts:
        if abs(y - ZOOM_TIME_LABEL_Y) <= 4:
            try:
                v = float(t)
            except ValueError:
                continue
            ticks.append((x, v))
    if len(ticks) < 2:
        raise ValueError("Zeitachsen-Ticks der Zoom-Kurve nicht gefunden")
    return lib.linreg(ticks)


def decode_zoom_history(data: str, texts, poll_ts: dt.datetime):
    """Liefert dict: name -> Liste von (abs_time, value) fuer alle 5 Kurven,
    ueber die komplette eingebettete Historie (nicht nur juengster Punkt)."""
    scale = lib.full_scale_ps(texts, ZOOM_Y[0] - 10, ZOOM_Y[0] + 10)
    y_mid = (ZOOM_Y[0] + ZOOM_Y[1]) / 2
    height = ZOOM_Y[1] - ZOOM_Y[0]
    ps_per_px = scale / height
    t_slope, t_intercept = zoom_time_calibration(texts)

    out = {}
    for name, color in ZOOM_COLORS.items():
        marker = f"<!-- zoom {name} curve -->"
        frag = lib.polyline_after(data, marker)
        pts = lib.read_points(frag)
        pts.sort(key=lambda p: p[0])
        series = []
        for x, y in pts:
            minute_offset = t_slope * x + t_intercept
            abs_time = poll_ts + dt.timedelta(minutes=minute_offset)
            value = (y_mid - y) * ps_per_px
            series.append((abs_time, value))
        out[name] = series
    return out


# --------------------------------------------------------------- Hauptlogik
def build_timeseries(raw_dir: pathlib.Path, start: dt.datetime, duration_s: float,
                      poll_interval_s: float):
    n_slots = int(round(duration_s / poll_interval_s))
    grid_end = start + dt.timedelta(seconds=duration_s)
    lookback = dt.timedelta(minutes=ZOOM_LOOKBACK_MIN)
    scan_from = start - lookback
    scan_to = grid_end + lookback

    files = []
    for f in sorted(raw_dir.glob("*.html"), key=lambda p: p.stem):
        try:
            ts = parse_raw_filename_ts(f.stem)
        except ValueError:
            continue
        if scan_from <= ts <= scan_to:
            files.append((ts, f))

    print(f"  {len(files)} Chart(s) im erweiterten Suchfenster "
          f"({fmt_ts(scan_from)} .. {fmt_ts(scan_to)}) gefunden.")

    slots = [dict() for _ in range(n_slots)]  # slot_idx -> {name: value}
    n_used = 0
    for poll_ts, f in files:
        try:
            data = f.read_text(encoding="utf-8", errors="replace")
            texts = lib.extract_texts(data)
            history = decode_zoom_history(data, texts, poll_ts)
        except Exception as e:
            print(f"WARNUNG: {f.name} konnte nicht dekodiert werden ({e}) - uebersprungen",
                  file=sys.stderr)
            continue
        n_used += 1
        for name, series in history.items():
            for abs_time, value in series:
                offset_s = (abs_time - start).total_seconds()
                slot_idx = round(offset_s / poll_interval_s)
                if not (0 <= slot_idx < n_slots):
                    continue
                nominal_time = start + dt.timedelta(seconds=slot_idx * poll_interval_s)
                if abs((abs_time - nominal_time).total_seconds()) > poll_interval_s / 2:
                    continue
                slots[slot_idx][name] = value  # chronologisch: neuere Charts gewinnen

    print(f"  {n_used} Chart(s) erfolgreich zur Rekonstruktion herangezogen.")
    return slots


def write_csv(out_dir: pathlib.Path, start: dt.datetime, slots: list[dict]) -> pathlib.Path:
    out_dir.mkdir(parents=True, exist_ok=True)
    path = out_dir / f"L2_zoom_{fmt_ts(start)}.csv"
    names = ["avg", "min", "max", "burst", "pause"]
    with path.open("w", newline="") as f:
        w = csv.writer(f)
        w.writerow([f"{n}_ps" for n in names])
        for slot in slots:
            w.writerow([round(slot[n], 1) if n in slot else "" for n in names])
    return path


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                  formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--detector", required=True)
    ap.add_argument("--start", required=True,
                     help="Beliebiges Format, z.B. '2026-08-12 17:56:41', "
                          "'2026-08-12T17:56:41', oder 20260812T175641[_564]")
    ap.add_argument("--duration", type=float, required=True, help="Sekunden")
    ap.add_argument("--raw-dir", default="data/raw")
    ap.add_argument("--out-dir", default="data/preprocessed")
    ap.add_argument("--poll-interval", type=float, default=2.0)
    ap.add_argument("--parse-lib", default=None)
    args = ap.parse_args()

    global lib
    if args.parse_lib:
        lib = _load_parse_lib(pathlib.Path(args.parse_lib))

    start = parse_start(args.start)
    raw_dir = pathlib.Path(args.raw_dir) / args.detector
    out_dir = pathlib.Path(args.out_dir) / args.detector

    print(f"Rekonstruiere L2-Zeitreihe fuer {args.detector}: "
          f"{fmt_ts(start)} + {args.duration:.0f}s im {args.poll_interval:.1f}s-Raster ...")
    slots = build_timeseries(raw_dir, start, args.duration, args.poll_interval)

    n_missing = sum(1 for s in slots if "avg" not in s)
    n_total = len(slots)
    print(f"  {n_total - n_missing}/{n_total} Slots rekonstruiert "
          f"({n_missing} nicht rekonstruierbare Luecke(n)).")

    path = write_csv(out_dir, start, slots)
    print(f"Geschrieben: {path}")


if __name__ == "__main__":
    main()
