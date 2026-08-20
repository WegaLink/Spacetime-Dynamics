#!/usr/bin/env python3
"""
Schritt 2 + 3: liest eine gespeicherte Snapshot-Datei (Ergebnis von 1_fetch.py),

  Schritt 2 - legt die vier Chart-Bereiche (response/cycle/zoom/run) als
              eigene *.svg-Rohausschnitte in data/svg/<ts>/ ab,

  Schritt 3 - rechnet die Pixelkoordinaten in physikalische Werte um und
              haengt sie an vier fortlaufende CSV-Tabellen an:
                data/tables/L0_cycle.csv     (1000 hi/low je Snapshot)
                data/tables/L1_response.csv  (800 Werte je Snapshot)
                data/tables/L2_zoom.csv      (neuester Punkt je Snapshot)
                data/tables/L3_run.csv       (neuester Punkt je Snapshot)

Funktioniert eigenstaendig, ohne Netzwerk - einfach mit dem Pfad einer
gespeicherten HTML-Datei aufrufen:
    python3 2_parse_and_convert.py data/raw/20260809T120000_000.html
"""

import re
import sys
import csv
import pathlib
import datetime as dt

RAW_DIR = pathlib.Path("data/raw")
SVG_DIR = pathlib.Path("data/svg")
TABLE_DIR = pathlib.Path("data/tables")

# ---- feste Pixel-Geometrie dieser Firmware-Version (v1.0.1) ------------
# (per Hand aus der SVG-Struktur ermittelt: Rahmen 40,40 - 1040,440;
#  vier uebereinanderliegende Charts mit fixen Grenzen)
RESPONSE_Y = (64, 140)     # oben, unten
CYCLE_Y = (140, 190)
ZOOM_Y = (190, 340)
RUN_Y = (340, 440)


# ---------------------------------------------------------------- utils
def read_points(svg_fragment: str) -> list[tuple[float, float]]:
    m = re.search(r'points="([^"]+)"', svg_fragment)
    if not m:
        return []
    return [
        (float(a), float(b))
        for a, b in re.findall(r'(-?\d+(?:\.\d+)?)\s*,\s*(-?\d+(?:\.\d+)?)', m.group(1))
    ]


def polyline_after(data: str, marker: str, span: int = 60000) -> str | None:
    i = data.find(marker)
    if i == -1:
        return None
    j = data.find('/>', i)
    return data[i:j + 2]


def polyline_by_style(data: str, region_start: int, region_end: int, stroke: str,
                       min_points: int) -> str | None:
    """Findet die <polyline> mit gegebener stroke-Farbe innerhalb eines Zeichenbereichs."""
    for m in re.finditer(r'<polyline[^>]*stroke:' + re.escape(stroke) + r'[^>]*points="([^"]+)"',
                          data[region_start:region_end]):
        pts = read_points(m.group(0))
        if len(pts) >= min_points:
            return m.group(0)
    return None


def extract_texts(data: str):
    out = []
    for m in re.finditer(r'<text x="(-?\d+)" y="(-?\d+)"[^>]*>(.*?)</text>', data, re.S):
        x, y, t = m.groups()
        out.append((int(x), int(y), re.sub(r'<[^>]+>', '', t)))
    return out


def linreg(points):
    """einfache lineare Regression y=f(px) -> value, liefert (slope, intercept)"""
    n = len(points)
    sx = sum(p for p, _ in points)
    sy = sum(v for _, v in points)
    sxx = sum(p * p for p, _ in points)
    sxy = sum(p * v for p, v in points)
    slope = (n * sxy - sx * sy) / (n * sxx - sx * sx)
    intercept = (sy - slope * sx) / n
    return slope, intercept


# ------------------------------------------------------------ scalars
def extract_scalars(texts):
    s = {}
    for x, y, t in texts:
        if m := re.match(r'Line delay:\s*([\d.]+)\s*ns', t):
            s['line_delay_ns'] = float(m.group(1))
        if m := re.match(r'Tuning:\s*([\d.\-]+).*?\(#(\d+):\s*([\d.\-]+)\s*\+\s*fix:\s*([\d.\-]+)\s*ns\)', t):
            s['tuning_deg'] = float(m.group(1))
            s['tuning_index'] = int(m.group(2))
            s['tuning_fix_ns'] = float(m.group(4))
        if 'Temperature:' in t:
            if m := re.search(r'Temperature:([\d.\-]+)', t):
                s['temperature_c'] = float(m.group(1))
            if m := re.search(r'Voltage:([\d.]+)\s*V', t):
                s['voltage_v'] = float(m.group(1))
        if m := re.match(r'burst \((-?\d+)\s*\)', t):
            s['cycle_burst'] = int(m.group(1))
        if m := re.match(r'pause \((-?\d+)\s*\)', t):
            s['cycle_pause'] = int(m.group(1))
    return s


# ------------------------------------------------------------ L0 cycle
def decode_L0(data: str, region: tuple[int, int]) -> list[int]:
    frag = polyline_after(data, '<!-- cycle curve -->')
    pts = read_points(frag)
    ys = [y for _, y in pts]
    hi_level, lo_level = min(ys), max(ys)   # kleineres y = weiter oben = 'hi'
    # sortiert nach x, damit die Reihenfolge der 1000 Samples stimmt
    pts.sort(key=lambda p: p[0])
    return [1 if y == hi_level else 0 for _, y in pts]


# ------------------------------------------------------------ L1 response
def decode_L1(data: str, texts) -> list[float]:
    # Zwei Polylinien liegen im Response-Bereich uebereinander:
    #   - die vollstaendige 800-Punkte-Kurve (stroke:green) = eigentliche L1-Daten
    #   - eine kuerzere rot hervorgehobene Teilkurve (stroke:red) = nur optisches
    #     Highlight des Zoom-Ausschnitts, KEINE eigene Messreihe
    # => explizit nach der gruenen Kurve mit >=800 Punkten suchen, nicht nach dem
    #    (irrefuehrend platzierten) HTML-Kommentar "<!-- response curve -->".
    frag = polyline_by_style(data, 0, data.find('<!-- cycle curve -->'), 'green', 700)
    pts = read_points(frag)
    pts.sort(key=lambda p: p[0])

    ticks = [(y, float(t)) for x, y, t in texts if 15 <= x < 45 and re.fullmatch(r'\d+', t)
             and RESPONSE_Y[0] <= y <= RESPONSE_Y[1] + 10]
    slope, intercept = linreg(ticks)
    return [slope * y + intercept for _, y in pts]


# ------------------------------------------------------------ L2/L3 zoom+run
def full_scale_ps(texts, y_lo, y_hi) -> float:
    for x, y, t in texts:
        if y_lo <= y <= y_hi and 'ps' in t:
            m = re.search(r'([\d.]+)\s*ps', t)
            if m:
                return float(m.group(1))
    raise ValueError("full-scale Achsentext nicht gefunden")


def decode_curve_set(data: str, texts, chart: str, y_bounds: tuple[int, int]):
    """chart: 'zoom' oder 'run'. Liefert dict avg/min/max/burst/pause -> Wert (neuester Punkt)."""
    colors = {'avg': 'black', 'min': 'red', 'max': 'blue',
              'burst': 'darkgoldenrod', 'pause': 'olive'}
    if chart == 'run':
        colors['avg'] = 'gray'

    scale = full_scale_ps(texts, y_bounds[0] - 10, y_bounds[0] + 10)
    y_mid = (y_bounds[0] + y_bounds[1]) / 2
    height = y_bounds[1] - y_bounds[0]
    ps_per_px = scale / height

    out = {}
    for name, color in colors.items():
        marker = f'<!-- {chart} {name} curve -->'
        frag = polyline_after(data, marker)
        pts = read_points(frag)
        pts.sort(key=lambda p: p[0])
        # Beobachtung: die aeusserste rechte Pixelspalte (jeweils neuester Punkt)
        # ist manchmal noch nicht befuellt und liegt dann exakt auf der
        # Chart-Vertikalmitte (Platzhalter). In dem Fall den vorletzten
        # (bereits vollstaendigen) Punkt verwenden.
        newest_x, newest_y = pts[-1]
        if round(newest_y) == round(y_mid) and len(pts) > 1:
            newest_x, newest_y = pts[-2]
        # (Annahme: Skala liegt symmetrisch um die vertikale Chartmitte.
        #  Fuer eine feinere Kalibrierung siehe Hinweis in der Antwort:
        #  Offset ueber mehrere Polls gegen 'line_delay_ns' nachjustieren.)
        out[name] = (y_mid - newest_y) * ps_per_px
    return out


# ------------------------------------------------------------------ main
def process(html_path: pathlib.Path):
    data = html_path.read_text(encoding='utf-8', errors='replace')
    ts = html_path.stem
    texts = extract_texts(data)
    scalars = extract_scalars(texts)

    # ---- Schritt 2: rohe SVG-Ausschnitte je Chart-Typ ablegen ----------
    out_dir = SVG_DIR / ts
    out_dir.mkdir(parents=True, exist_ok=True)
    (out_dir / "L0_cycle.svg").write_text(polyline_after(data, '<!-- cycle curve -->') or "")
    (out_dir / "L1_response.svg").write_text(
        polyline_by_style(data, 0, data.find('<!-- cycle curve -->'), 'green', 700) or "")
    for name in ('avg', 'min', 'max', 'burst', 'pause'):
        (out_dir / f"L2_zoom_{name}.svg").write_text(
            polyline_after(data, f'<!-- zoom {name} curve -->') or "")
        (out_dir / f"L3_run_{name}.svg").write_text(
            polyline_after(data, f'<!-- run {name} curve -->') or "")

    # ---- Schritt 3: Umrechnung in physikalische Werte -------------------
    l0 = decode_L0(data, CYCLE_Y)
    l1 = decode_L1(data, texts)
    l2 = decode_curve_set(data, texts, 'zoom', ZOOM_Y)
    l3 = decode_curve_set(data, texts, 'run', RUN_Y)

    TABLE_DIR.mkdir(parents=True, exist_ok=True)

    # L0: eine Zeile pro Snapshot, 1000 Bit als String (kompakt) + burst/pause-Scalar
    write_row(TABLE_DIR / "L0_cycle.csv",
              ["timestamp", "bits_1000", "hi_count", "cycle_burst", "cycle_pause"],
              [ts, ''.join(map(str, l0)), sum(l0),
               scalars.get('cycle_burst'), scalars.get('cycle_pause')])

    # L1: eine Zeile pro Snapshot, 800 Werte als String (oder alternativ 800 Zeilen -
    # hier kompakt, da bei 2s-Takt jedes Fenster komplett neu ist)
    write_row(TABLE_DIR / "L1_response.csv",
              ["timestamp"] + [f"p{i}" for i in range(len(l1))],
              [ts] + [round(v, 1) for v in l1])

    # L2: ein Messpunkt (neuester) pro Snapshot-Poll
    write_row(TABLE_DIR / "L2_zoom.csv",
              ["timestamp", "avg_ps", "min_ps", "max_ps", "burst_ps", "pause_ps"],
              [ts, l2['avg'], l2['min'], l2['max'], l2['burst'], l2['pause']])

    # L3: ein Messpunkt (neuester) pro Snapshot-Poll, inkl. Line delay/Tuning/Temperatur
    write_row(TABLE_DIR / "L3_run.csv",
              ["timestamp", "avg_ps", "min_ps", "max_ps", "burst_ps", "pause_ps",
               "line_delay_ns", "tuning_deg", "tuning_fix_ns", "temperature_c", "voltage_v"],
              [ts, l3['avg'], l3['min'], l3['max'], l3['burst'], l3['pause'],
               scalars.get('line_delay_ns'), scalars.get('tuning_deg'),
               scalars.get('tuning_fix_ns'), scalars.get('temperature_c'),
               scalars.get('voltage_v')])

    return dict(l0_hi=sum(l0), l1_last=l1[-1], l2=l2, l3=l3, scalars=scalars)


def write_row(path: pathlib.Path, header, row):
    new = not path.exists()
    with path.open('a', newline='') as f:
        w = csv.writer(f)
        if new:
            w.writerow(header)
        w.writerow(row)


if __name__ == "__main__":
    p = pathlib.Path(sys.argv[1])
    result = process(p)
    print("hi-Anzahl L0:", result['l0_hi'], " (Referenz High/Switch:", result['scalars'].get('high_switch'), ")")
    print("letzter L1-Wert:", round(result['l1_last'], 1))
    print("L2 (Zoom) neuester Punkt [ps]:", {k: round(v, 1) for k, v in result['l2'].items()})
    print("L3 (Run) neuester Punkt [ps]:", {k: round(v, 1) for k, v in result['l3'].items()})
