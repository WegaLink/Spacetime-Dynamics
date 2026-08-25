#!/usr/bin/env python3
"""
Plottet eine mit 4_extract_l2_timeseries.py erzeugte L2-CSV zur visuellen
Kontrolle, angelehnt an das "Zoom"-Chart der Rohdaten (avg/min/max/burst/
pause ueber der Zeit, gleiche Y-Achse in ps).

Die CSV enthaelt keine Zeitstempel mehr (siehe 4_extract_l2_timeseries.py);
die Zeitachse wird daher aus dem Zeitstempel im Dateinamen (L2_zoom_<ts>.csv)
plus --poll-interval rekonstruiert. Fehlende Slots (leere Felder = nicht
rekonstruierbare Luecken) werden als Unterbrechung der Linie dargestellt
(matplotlib ueberspringt NaN automatisch) und zusaetzlich rot markiert.

Aufruf:
    python3 plot_l2.py data/preprocessed/D1/L2_zoom_20260812T172641_000.csv \
        --poll-interval 2.0 --out data/preprocessed/D1/L2_zoom_check.png
"""

import argparse
import csv
import pathlib
import re
import sys
import datetime as dt

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.dates as mdates

FILENAME_TS_RE = re.compile(r'L2_zoom_(\d{8}T\d{6})_(\d{3})\.csv$')

COLORS = {"avg": "black", "min": "red", "max": "blue",
          "burst": "darkgoldenrod", "pause": "olive"}


def parse_start_from_filename(path: pathlib.Path) -> dt.datetime:
    m = FILENAME_TS_RE.search(path.name)
    if not m:
        raise ValueError(f"Zeitstempel nicht aus Dateinamen ablesbar: {path.name!r} "
                          f"- bitte --start explizit angeben.")
    base, ms = m.groups()
    d = dt.datetime.strptime(base, "%Y%m%dT%H%M%S")
    return d.replace(microsecond=int(ms) * 1000)


def load_csv(path: pathlib.Path):
    rows = {"avg": [], "min": [], "max": [], "burst": [], "pause": []}
    with path.open(newline="") as f:
        r = csv.DictReader(f)
        for row in r:
            for name in rows:
                v = row.get(f"{name}_ps", "")
                rows[name].append(float(v) if v not in ("", None) else float("nan"))
    return rows


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                  formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("csv_path", type=pathlib.Path)
    ap.add_argument("--start", default=None,
                     help="Nur noetig falls nicht aus Dateinamen ablesbar")
    ap.add_argument("--poll-interval", type=float, default=2.0)
    ap.add_argument("--out", default=None, help="Ziel-PNG (Default: <csv>.png)")
    args = ap.parse_args()

    start = (dt.datetime.fromisoformat(args.start) if args.start
              else parse_start_from_filename(args.csv_path))
    rows = load_csv(args.csv_path)
    n = len(rows["avg"])
    times = [start + dt.timedelta(seconds=i * args.poll_interval) for i in range(n)]

    fig, ax = plt.subplots(figsize=(14, 5))
    for name, color in COLORS.items():
        ax.plot(times, rows[name], color=color, linewidth=0.8, label=name)

    # nicht rekonstruierbare Luecken (avg fehlt) rot markieren
    gap_times = [t for t, v in zip(times, rows["avg"]) if v != v]  # NaN-Check
    if gap_times:
        ax.scatter(gap_times, [0] * len(gap_times), color="red", marker="x",
                   s=15, zorder=5, label=f"Luecke ({len(gap_times)})")

    ax.set_ylabel("ps")
    ax.set_title(f"L2 Zoom-Statistik ab {start.isoformat()} "
                 f"({n} Slots x {args.poll_interval:.0f}s)")
    ax.xaxis.set_major_formatter(mdates.DateFormatter("%H:%M:%S"))
    fig.autofmt_xdate()
    ax.legend(loc="upper right", ncol=6, fontsize=8)
    ax.grid(True, linewidth=0.3, alpha=0.5)
    fig.tight_layout()

    out_path = pathlib.Path(args.out) if args.out else args.csv_path.with_suffix(".png")
    fig.savefig(out_path, dpi=130)
    print(f"Geschrieben: {out_path}")


if __name__ == "__main__":
    main()
