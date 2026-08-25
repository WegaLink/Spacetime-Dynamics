#!/usr/bin/env python3
"""
Schritt 4 (Vorverarbeitung): liest alle gespeicherten Chart-Snapshots eines
Detektors in einem Zeitfenster [start, start+dauer], dekodiert sie mit den
Routinen aus 2_parse_and_convert.py, entfernt redundante Wiederholungen
und schliesst Luecken (fehlgeschlagene Abrufe) gemaess folgender Logik:

    - "Luecke" = fehlender Rohdaten-Poll zu einem erwarteten Zeitschlitz
      (Standard-Raster: POLL_INTERVAL_S, siehe unten -- Annahme, s. Antworttext)
    - Luecken werden RUECKWIRKEND aus dem naechsten wieder verfuegbaren
      Chart gefuellt (Annahme, s. Antworttext Punkt 3):
        L0  -> Platzhalter-Datensatz aus Nullen, mit Fluecken-Flag
        L1  -> 800 identische Werte = avg-Wert aus L2 des fuellenden Charts
        L2  -> Werte aus dem naechsten verfuegbaren Chart (unveraendert)
        L3  -> Werte aus dem naechsten verfuegbaren Chart (unveraendert)
    - Zeilen, deren dekodierte Werte identisch zur zuletzt geschriebenen
      Zeile sind (natuerliche Redundanz durch langsamen Sensor-Refresh,
      z.B. 8-12s Zyklus bei einzelnen Detektoren), werden NICHT erneut
      geschrieben -- das ist die eigentliche Volumenreduktion.
    - Dateinamen erhalten den Zeitstempel des ERSTEN Datensatzes im Lauf.
    - Fuer L0 wird KEINE Indexdatei erzeugt, fuer L1-L3 schon
      (Timestamp -> laufende Zeilennummer). Das genaue Binaerformat aus
      dem Konzept "SD-Kartenspeicherung..." ist hier NICHT bekannt; die
      Schreibfunktionen unten (write_l0/write_l1/write_l2/write_l3) sind
      bewusst gekapselt und muessen ggf. gegen das reale Format
      ausgetauscht werden.

Aufruf:
    python3 3_preprocess_range.py \
        --detector D1 \
        --start 20260812T175641_564 \
        --duration 3600 \
        --raw-dir data/raw \
        --out-dir data/preprocessed \
        --poll-interval 2.0
"""

import argparse
import csv
import importlib.util
import pathlib
import re
import sys
import datetime as dt


def _load_parse_lib(module_path: pathlib.Path = None):
    """Laedt 2_parse_and_convert.py direkt als Modul, ohne Umbenennen/Kopieren.
    Modulnamen duerfen nicht mit einer Ziffer beginnen, daher ueber
    importlib mit frei waehlbarem internen Namen ('parse_lib') laden."""
    if module_path is None:
        module_path = pathlib.Path(__file__).parent / "2_parse_and_convert.py"
    if not module_path.exists():
        raise FileNotFoundError(
            f"{module_path} nicht gefunden - bitte 2_parse_and_convert.py "
            f"in dasselbe Verzeichnis wie dieses Skript legen, oder Pfad "
            f"ueber --parse-lib angeben.")
    spec = importlib.util.spec_from_file_location("parse_lib", module_path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


lib = _load_parse_lib()  # decode_L0, decode_L1, decode_curve_set, extract_texts, extract_scalars


# ---------------------------------------------------------------- Zeit-Utils
TS_RE = re.compile(r'^(\d{8}T\d{6})_(\d{3})$')


def parse_ts(stem: str) -> dt.datetime:
    m = TS_RE.match(stem)
    if not m:
        raise ValueError(f"Zeitstempel im Dateinamen nicht erkannt: {stem!r}")
    base, ms = m.groups()
    d = dt.datetime.strptime(base, "%Y%m%dT%H%M%S")
    return d.replace(microsecond=int(ms) * 1000)


def fmt_ts(t: dt.datetime) -> str:
    return t.strftime("%Y%m%dT%H%M%S") + f"_{t.microsecond // 1000:03d}"


# ---------------------------------------------------------- Snapshot laden
class Snapshot:
    __slots__ = ("ts", "l0", "l1", "l2", "l3", "scalars", "filled")

    def __init__(self, ts, l0, l1, l2, l3, scalars, filled=False):
        self.ts = ts
        self.l0 = l0
        self.l1 = l1
        self.l2 = l2
        self.l3 = l3
        self.scalars = scalars
        self.filled = filled  # True = synthetischer Luecken-Fuellwert


def decode_file(path: pathlib.Path) -> Snapshot:
    data = path.read_text(encoding="utf-8", errors="replace")
    texts = lib.extract_texts(data)
    scalars = lib.extract_scalars(texts)
    l0 = lib.decode_L0(data, lib.CYCLE_Y)
    l1 = lib.decode_L1(data, texts)
    l2 = lib.decode_curve_set(data, texts, "zoom", lib.ZOOM_Y)
    l3 = lib.decode_curve_set(data, texts, "run", lib.RUN_Y)
    return Snapshot(parse_ts(path.stem), l0, l1, l2, l3, scalars)


def load_range(raw_dir: pathlib.Path, start: dt.datetime, end: dt.datetime):
    files = sorted(raw_dir.glob("*.html"), key=lambda p: p.stem)
    snaps = []
    for f in files:
        try:
            ts = parse_ts(f.stem)
        except ValueError:
            continue
        if start <= ts <= end:
            try:
                snaps.append(decode_file(f))
            except Exception as e:
                print(f"WARNUNG: {f.name} konnte nicht dekodiert werden ({e}) - wird uebersprungen",
                      file=sys.stderr)
    return snaps


# --------------------------------------------------------- Luecken schliessen
def fill_gaps(snaps: list[Snapshot], start: dt.datetime, end: dt.datetime,
              poll_interval_s: float) -> list[Snapshot]:
    """Baut ein lueckenloses nominales Raster [start, end] im Abstand
    poll_interval_s und fuellt fehlende Slots rueckwirkend aus dem naechsten
    tatsaechlich vorhandenen Snapshot. Vorhandene Snapshots bleiben
    unveraendert an ihrem eigenen Zeitstempel stehen (kein Zwang auf das
    exakte Raster), nur echte Fehlstellen werden ergaenzt."""
    if not snaps:
        return []

    by_ts = {s.ts: s for s in snaps}
    grid = []
    t = start
    step = dt.timedelta(seconds=poll_interval_s)
    while t <= end:
        grid.append(t)
        t += step

    result = []
    pending_gap_slots = []  # Slots ohne eigenen Snapshot, warten auf Fuellwert

    def make_filler(slot_ts: dt.datetime, source: Snapshot) -> Snapshot:
        l0_zero = [0] * len(source.l0) if source.l0 else [0] * 1000
        l1_flat = [source.l2.get("avg", 0.0)] * (len(source.l1) if source.l1 else 800)
        return Snapshot(slot_ts, l0_zero, l1_flat, dict(source.l2), dict(source.l3),
                         dict(source.scalars), filled=True)

    for slot in grid:
        # exakter Treffer im Toleranzfenster +/- halbe Poll-Periode
        hit = None
        for cand_ts, cand in by_ts.items():
            if abs((cand_ts - slot).total_seconds()) <= poll_interval_s / 2:
                hit = cand
                break
        if hit is not None:
            # offene Luecken davor jetzt rueckwirkend mit "hit" fuellen
            for gslot in pending_gap_slots:
                result.append(make_filler(gslot, hit))
            pending_gap_slots = []
            result.append(hit)
        else:
            pending_gap_slots.append(slot)

    if pending_gap_slots:
        print(f"WARNUNG: {len(pending_gap_slots)} Luecke(n) am Ende des Zeitfensters "
              f"konnten nicht gefuellt werden (kein nachfolgender Chart vorhanden) "
              f"- werden verworfen.", file=sys.stderr)

    return result


# ------------------------------------------------------------ Deduplizieren
def snapshots_equal(a: Snapshot, b: Snapshot) -> bool:
    return (a.l0 == b.l0 and a.l1 == b.l1 and a.l2 == b.l2 and a.l3 == b.l3)


def dedup(snaps: list[Snapshot]) -> list[Snapshot]:
    """Entfernt aufeinanderfolgende Datensaetze mit identischem Inhalt
    (natuerliche Redundanz durch langsamen Sensor-Refresh). Der erste
    Datensatz einer Wiederholungsserie bleibt stehen, damit der Zeitpunkt
    des ERSTEN Auftretens erhalten bleibt."""
    out = []
    for s in snaps:
        if out and snapshots_equal(out[-1], s) and not s.filled and not out[-1].filled:
            continue
        out.append(s)
    return out


# -------------------------------------------------------------- Storage I/O
# ACHTUNG: Platzhalter-Implementierung (CSV, angelehnt an 2_parse_and_convert.py).
# Muss ggf. gegen das reale Format aus "SD-Kartenspeicherung..." ersetzt werden.

def write_l0(out_dir: pathlib.Path, first_ts: str, snaps: list[Snapshot]):
    path = out_dir / f"L0_cycle_{first_ts}.csv"
    with path.open("w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["timestamp", "bits_1000", "hi_count", "cycle_burst", "cycle_pause", "filled"])
        for s in snaps:
            w.writerow([fmt_ts(s.ts), "".join(map(str, s.l0)), sum(s.l0),
                        s.scalars.get("cycle_burst"), s.scalars.get("cycle_pause"),
                        int(s.filled)])
    return path


def write_l1(out_dir: pathlib.Path, first_ts: str, snaps: list[Snapshot]):
    path = out_dir / f"L1_response_{first_ts}.csv"
    idx_path = out_dir / f"L1_response_{first_ts}.idx.csv"
    with path.open("w", newline="") as f, idx_path.open("w", newline="") as fi:
        w = csv.writer(f)
        wi = csv.writer(fi)
        w.writerow(["timestamp", "filled"] + [f"p{i}" for i in range(len(snaps[0].l1))])
        wi.writerow(["timestamp", "row"])
        for row_no, s in enumerate(snaps):
            w.writerow([fmt_ts(s.ts), int(s.filled)] + [round(v, 1) for v in s.l1])
            wi.writerow([fmt_ts(s.ts), row_no])
    return path


def write_l2(out_dir: pathlib.Path, first_ts: str, snaps: list[Snapshot]):
    path = out_dir / f"L2_zoom_{first_ts}.csv"
    idx_path = out_dir / f"L2_zoom_{first_ts}.idx.csv"
    with path.open("w", newline="") as f, idx_path.open("w", newline="") as fi:
        w = csv.writer(f)
        wi = csv.writer(fi)
        w.writerow(["timestamp", "avg_ps", "min_ps", "max_ps", "burst_ps", "pause_ps", "filled"])
        wi.writerow(["timestamp", "row"])
        for row_no, s in enumerate(snaps):
            l2 = s.l2
            w.writerow([fmt_ts(s.ts), l2.get("avg"), l2.get("min"), l2.get("max"),
                        l2.get("burst"), l2.get("pause"), int(s.filled)])
            wi.writerow([fmt_ts(s.ts), row_no])
    return path


def write_l3(out_dir: pathlib.Path, first_ts: str, snaps: list[Snapshot]):
    path = out_dir / f"L3_run_{first_ts}.csv"
    idx_path = out_dir / f"L3_run_{first_ts}.idx.csv"
    with path.open("w", newline="") as f, idx_path.open("w", newline="") as fi:
        w = csv.writer(f)
        wi = csv.writer(fi)
        w.writerow(["timestamp", "avg_ps", "min_ps", "max_ps", "burst_ps", "pause_ps",
                     "line_delay_ns", "tuning_deg", "tuning_fix_ns",
                     "temperature_c", "voltage_v", "filled"])
        wi.writerow(["timestamp", "row"])
        for row_no, s in enumerate(snaps):
            l3 = s.l3
            sc = s.scalars
            w.writerow([fmt_ts(s.ts), l3.get("avg"), l3.get("min"), l3.get("max"),
                        l3.get("burst"), l3.get("pause"), sc.get("line_delay_ns"),
                        sc.get("tuning_deg"), sc.get("tuning_fix_ns"),
                        sc.get("temperature_c"), sc.get("voltage_v"), int(s.filled)])
            wi.writerow([fmt_ts(s.ts), row_no])
    return path


# ------------------------------------------------------------------- main
def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                  formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--detector", required=True, help="Unterordnername des Detektors unter --raw-dir")
    ap.add_argument("--start", required=True,
                     help="Zeitstempel des ersten zu verarbeitenden Charts, Format wie im Dateinamen "
                          "(z.B. 20260812T175641_564)")
    ap.add_argument("--duration", type=float, required=True, help="Zeitdauer in Sekunden")
    ap.add_argument("--raw-dir", default="data/raw", help="Basisverzeichnis der Rohdaten (enthaelt <detector>/)")
    ap.add_argument("--out-dir", default="data/preprocessed", help="Zielverzeichnis")
    ap.add_argument("--poll-interval", type=float, default=2.0,
                     help="nominales Poll-Raster in Sekunden (Default 2.0, siehe Offene Punkte)")
    ap.add_argument("--parse-lib", default=None,
                     help="Pfad zu 2_parse_and_convert.py, falls nicht im selben Verzeichnis")
    args = ap.parse_args()

    global lib
    if args.parse_lib:
        lib = _load_parse_lib(pathlib.Path(args.parse_lib))

    start = parse_ts(args.start)
    end = start + dt.timedelta(seconds=args.duration)
    raw_dir = pathlib.Path(args.raw_dir) / args.detector
    out_dir = pathlib.Path(args.out_dir) / args.detector
    out_dir.mkdir(parents=True, exist_ok=True)

    print(f"Lade Rohdaten {raw_dir} im Fenster {fmt_ts(start)} .. {fmt_ts(end)} ...")
    snaps = load_range(raw_dir, start, end)
    print(f"  {len(snaps)} Chart(s) gefunden und dekodiert.")
    if not snaps:
        print("Keine Daten im Zeitfenster - Abbruch.", file=sys.stderr)
        sys.exit(1)

    filled = fill_gaps(snaps, start, end, args.poll_interval)
    n_gaps = sum(1 for s in filled if s.filled)
    print(f"  Raster gebaut: {len(filled)} Slot(s), davon {n_gaps} Luecken-Fuellwerte.")

    deduped = dedup(filled)
    print(f"  Nach Deduplizierung: {len(deduped)} Datensaetze "
          f"(Reduktion: {100 * (1 - len(deduped) / len(filled)):.1f}%).")

    first_ts = fmt_ts(deduped[0].ts)
    p0 = write_l0(out_dir, first_ts, deduped)
    p1 = write_l1(out_dir, first_ts, deduped)
    p2 = write_l2(out_dir, first_ts, deduped)
    p3 = write_l3(out_dir, first_ts, deduped)

    print("Geschrieben:")
    for p in (p0, p1, p2, p3):
        print(f"  {p}")


if __name__ == "__main__":
    main()
