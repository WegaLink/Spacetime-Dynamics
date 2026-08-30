#!/usr/bin/env python3
"""
5_spectral_analysis.py

FFT-Spektralanalyse der L2-Zeitreihen (avg/min/max/burst/pause) mehrerer
Detektoren und Zeitstempel. Erzeugt eine interaktive HTML-Grafik (Plotly),
in der pro Signal ein Panel mit gemeinsamer X-Achse (Periodendauer in
Sekunden) dargestellt wird. Farbe = Detektor, Linienstil = Zeitstempel.

Erwartete Verzeichnisstruktur (relativ zu diesem Skript):
    data/preprocessed/TimeWavesDetector#<ID>/L2_zoom_<YYYYMMDD>T<HHMMSS>_<ms>.csv

CSV-Format (Header vorhanden, keine eigene Zeitstempel-Spalte):
    avg_ps,min_ps,max_ps,burst_ps,pause_ps
Die Zeilen sind lückenlos im festen SAMPLE_INTERVAL_S-Abstand aufgezeichnet.

Beispielaufrufe:
    python3 5_spectral_analysis.py \
        --detectors 51,60,61,78 --timestamps 17:35,19:35,20:10 \
        --signals avg,burst,pause

    python3 5_spectral_analysis.py \
        --detectors 51 --timestamps 20260812T1735 --signals avg

    python3 5_spectral_analysis.py -d 51,60 -t 17:35 -s avg,min,max,burst,pause -o eclipse.html

    # lineare X-Achse (Default ist log):
    python3 5_spectral_analysis.py -d 51,60,61,78 -t 17:35,19:35,20:10 \
        -s avg,min,max,burst,pause --xscale linear

Die erzeugte HTML-Datei enthaelt zusaetzlich zur Plotly-Legende ein
Kontrollpanel mit Checkboxen fuer Detektor/Zeitstempel/Signal, mit denen
sich einzelne Kombinationen nachtraeglich im Browser ein-/ausblenden lassen,
ohne das Skript erneut aufzurufen. Pro Parameter muss stets mindestens eine
Option aktiv bleiben - das Panel verhindert das Abwaehlen der letzten
verbleibenden Checkbox einer Gruppe.
"""

import argparse
import json
import os
import re
import sys

import numpy as np
import pandas as pd
import plotly.graph_objects as go
from plotly.subplots import make_subplots

# ---------------------------------------------------------------------------
# Konfiguration
# ---------------------------------------------------------------------------

SAMPLE_INTERVAL_S = 2.0  # festes Abtastintervall der L2-Statistik

BASE_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                         "data", "preprocessed")

SIGNAL_COLUMNS = {
    "avg": "avg_ps",
    "min": "min_ps",
    "max": "max_ps",
    "burst": "burst_ps",
    "pause": "pause_ps",
}

# Qualitative Farbpalette für Detektoren
DETECTOR_COLORS = [
    "#1f77b4", "#d62728", "#2ca02c", "#9467bd",
    "#ff7f0e", "#8c564b", "#e377c2", "#17becf",
]

# Linienstile für Zeitstempel
LINESTYLES = ["solid", "dash", "dot", "dashdot", "longdash", "longdashdot"]


# ---------------------------------------------------------------------------
# Datei-Auflösung
# ---------------------------------------------------------------------------

def find_detector_dir(detector_id: str) -> str:
    path = os.path.join(BASE_DIR, f"TimeWavesDetector#{detector_id}")
    if not os.path.isdir(path):
        raise FileNotFoundError(f"Detektor-Verzeichnis nicht gefunden: {path}")
    return path


def build_timestamp_pattern(ts: str) -> re.Pattern:
    """
    Akzeptiert zwei Formate:
      - 'HH:MM'                 -> matcht beliebiges Datum mit dieser Uhrzeit
      - 'YYYYMMDDTHHMM' / 'YYYYMMDDTHHMMSS' -> matcht genau dieses Datum/Uhrzeit
    """
    ts = ts.strip()

    if re.fullmatch(r"\d{2}:\d{2}", ts):
        hhmm = ts.replace(":", "")
        return re.compile(rf"^L2_zoom_\d{{8}}T{hhmm}\d{{2}}_\d+\.csv$")

    if re.fullmatch(r"\d{8}T\d{4}", ts):
        return re.compile(rf"^L2_zoom_{ts}\d{{2}}_\d+\.csv$")

    if re.fullmatch(r"\d{8}T\d{6}", ts):
        return re.compile(rf"^L2_zoom_{ts}_\d+\.csv$")

    raise ValueError(
        f"Unbekanntes Zeitstempel-Format: '{ts}'. "
        f"Erlaubt: 'HH:MM' oder 'YYYYMMDDTHHMM[SS]'."
    )


def find_file(detector_dir: str, ts_pattern: re.Pattern) -> str | None:
    matches = sorted(f for f in os.listdir(detector_dir) if ts_pattern.match(f))
    if not matches:
        return None
    if len(matches) > 1:
        print(f"  Hinweis: mehrere Treffer in {detector_dir} für Zeitstempel-Muster "
              f"'{ts_pattern.pattern}', verwende erste Datei: {matches[0]}",
              file=sys.stderr)
    return os.path.join(detector_dir, matches[0])


# ---------------------------------------------------------------------------
# FFT
# ---------------------------------------------------------------------------

def compute_fft(values: np.ndarray):
    """
    Reine FFT ohne Fensterung/Overlap. Mittelwert wird vorher abgezogen
    (Detrending), DC-Bin (Periode = unendlich) wird entfernt.
    Rückgabe: (periods_sorted, relative_amplitude_sorted), aufsteigend nach Periodendauer.
    """
    values = np.asarray(values, dtype=float)
    values = values - np.mean(values)

    n = len(values)
    spectrum = np.fft.rfft(values)
    freqs = np.fft.rfftfreq(n, d=SAMPLE_INTERVAL_S)

    # relative Amplitude (normiert auf Signallänge, Faktor 2 fuer einseitiges Spektrum)
    amplitude = np.abs(spectrum) / n * 2

    # DC-Anteil (freq = 0) entfernen
    freqs = freqs[1:]
    amplitude = amplitude[1:]

    periods = 1.0 / freqs
    order = np.argsort(periods)
    return periods[order], amplitude[order]


# ---------------------------------------------------------------------------
# Interaktives Kontrollpanel (Checkboxen fuer Detektor/Zeitstempel/Signal)
# ---------------------------------------------------------------------------

def unique_ordered(values):
    seen = []
    for v in values:
        if v not in seen:
            seen.append(v)
    return seen


def build_control_panel_html(trace_meta):
    """
    Erzeugt HTML+JS fuer ein Checkbox-Panel, das die Sichtbarkeit der
    Plotly-Traces per Plotly.restyle steuert. Pro Gruppe (det/ts/sig)
    muss mindestens eine Checkbox aktiv bleiben.
    trace_meta: Liste von dicts {'det':..., 'ts':..., 'sig':...} in exakt
    derselben Reihenfolge wie die Traces in fig.data.
    """
    dets = unique_ordered(m["det"] for m in trace_meta)
    tss = unique_ordered(m["ts"] for m in trace_meta)
    sigs = unique_ordered(m["sig"] for m in trace_meta)

    def render_group(group_key, label, values):
        boxes = "\n".join(
            f'<label class="ctrl-item">'
            f'<input type="checkbox" data-group="{group_key}" '
            f'data-value="{v}" checked onchange="onCtrlChange(this)"> {v}'
            f"</label>"
            for v in values
        )
        return f'<fieldset class="ctrl-group"><legend>{label}</legend>{boxes}</fieldset>'

    panel_html = f"""
<div id="spectral-ctrl-panel" style="font-family: Arial, Helvetica, sans-serif;
     font-size: 13px; padding: 10px 14px; border: 1px solid #ddd;
     border-radius: 6px; margin: 10px 14px; display: flex; gap: 24px;
     flex-wrap: wrap; background: #fafafa;">
  {render_group('det', 'Detektor', dets)}
  {render_group('ts', 'Zeitstempel', tss)}
  {render_group('sig', 'Signal', sigs)}
</div>
<style>
  .ctrl-group {{ border: 1px solid #ccc; border-radius: 4px; padding: 6px 10px; }}
  .ctrl-group legend {{ font-weight: bold; padding: 0 4px; }}
  .ctrl-item {{ display: inline-block; margin: 2px 10px 2px 0; white-space: nowrap; }}
</style>
"""

    trace_meta_json = json.dumps(trace_meta)

    script_html = f"""
<script>
  var spectralTraceMeta = {trace_meta_json};
  var spectralGraphDiv = "spectral-plot";

  function onCtrlChange(checkbox) {{
    var group = checkbox.dataset.group;
    var groupBoxes = document.querySelectorAll('input[data-group="' + group + '"]');
    var checkedCount = 0;
    groupBoxes.forEach(function(cb) {{ if (cb.checked) checkedCount++; }});
    if (checkedCount === 0) {{
      // mindestens eine Option pro Gruppe muss aktiv bleiben
      checkbox.checked = true;
      return;
    }}
    updateSpectralVisibility();
  }}

  function updateSpectralVisibility() {{
    function selectedValues(group) {{
      var s = new Set();
      document.querySelectorAll('input[data-group="' + group + '"]').forEach(function(cb) {{
        if (cb.checked) s.add(cb.dataset.value);
      }});
      return s;
    }}
    var selDet = selectedValues('det');
    var selTs = selectedValues('ts');
    var selSig = selectedValues('sig');

    var visibility = spectralTraceMeta.map(function(m) {{
      return selDet.has(m.det) && selTs.has(m.ts) && selSig.has(m.sig);
    }});

    var gd = document.getElementById(spectralGraphDiv);
    if (gd) {{
      Plotly.restyle(gd, {{visible: visibility}});
    }}
  }}
</script>
"""
    return panel_html, script_html


def inject_control_panel(html_path, trace_meta):
    panel_html, script_html = build_control_panel_html(trace_meta)
    with open(html_path, "r", encoding="utf-8") as f:
        html = f.read()

    html = html.replace("<body>", "<body>\n" + panel_html, 1)
    html = html.replace("</body>", script_html + "\n</body>", 1)

    with open(html_path, "w", encoding="utf-8") as f:
        f.write(html)


# ---------------------------------------------------------------------------
# Hauptprogramm
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="FFT-Spektralanalyse der L2-Zeitreihen mehrerer Detektoren."
    )
    parser.add_argument("-d", "--detectors", required=True,
                         help="Kommagetrennte Detektor-IDs, z.B. 51,60,61,78")
    parser.add_argument("-t", "--timestamps", required=True,
                         help="Kommagetrennte Zeitstempel, z.B. 17:35,19:35,20:10 "
                              "oder volles Datum 20260812T1735")
    parser.add_argument("-s", "--signals", required=True,
                         help=f"Kommagetrennte Signale aus {list(SIGNAL_COLUMNS)}")
    parser.add_argument("-o", "--output", default="spectral_analysis.html",
                         help="Pfad der Ausgabe-HTML-Datei (Default: spectral_analysis.html)")
    parser.add_argument("--xscale", choices=["linear", "log"], default="log",
                         help="Skalierung der X-Achse (Periodendauer). Default: log")
    args = parser.parse_args()

    detectors = [d.strip() for d in args.detectors.split(",") if d.strip()]
    timestamps = [t.strip() for t in args.timestamps.split(",") if t.strip()]
    signals = [s.strip() for s in args.signals.split(",") if s.strip()]

    for s in signals:
        if s not in SIGNAL_COLUMNS:
            parser.error(f"Unbekanntes Signal: '{s}'. Erlaubt: {list(SIGNAL_COLUMNS)}")

    color_map = {det: DETECTOR_COLORS[i % len(DETECTOR_COLORS)]
                 for i, det in enumerate(detectors)}
    style_map = {ts: LINESTYLES[i % len(LINESTYLES)]
                 for i, ts in enumerate(timestamps)}

    fig = make_subplots(
        rows=len(signals), cols=1,
        shared_xaxes=True,
        subplot_titles=signals,
        vertical_spacing=0.04,
    )

    any_data = False
    trace_meta = []  # exakt parallel zu den tatsaechlich hinzugefuegten Traces

    for det in detectors:
        try:
            det_dir = find_detector_dir(det)
        except FileNotFoundError as e:
            print(f"Warnung: {e}", file=sys.stderr)
            continue

        for ts in timestamps:
            try:
                pattern = build_timestamp_pattern(ts)
            except ValueError as e:
                parser.error(str(e))
                return

            filepath = find_file(det_dir, pattern)
            if filepath is None:
                print(f"Warnung: keine Datei fuer Detektor {det}, Zeitstempel '{ts}' gefunden.",
                      file=sys.stderr)
                continue

            df = pd.read_csv(filepath)

            for row_idx, sig in enumerate(signals, start=1):
                col = SIGNAL_COLUMNS[sig]
                if col not in df.columns:
                    print(f"Warnung: Spalte '{col}' fehlt in {filepath}", file=sys.stderr)
                    continue

                periods, amps = compute_fft(df[col].values)
                any_data = True
                trace_meta.append({"det": det, "ts": ts, "sig": sig})

                fig.add_trace(
                    go.Scatter(
                        x=periods,
                        y=amps,
                        mode="lines",
                        name=f"Det#{det} | {ts} | {sig}",
                        legendgroup=f"{det}-{ts}",
                        line=dict(color=color_map[det], dash=style_map[ts], width=1),
                        showlegend=(row_idx == 1),
                        hovertemplate=(
                            f"Det#{det} | {ts} | {sig}<br>"
                            "Periode: %{x:.2f} s<br>"
                            "rel. Amplitude: %{y:.3f}<extra></extra>"
                        ),
                    ),
                    row=row_idx, col=1,
                )

    if not any_data:
        print("Keine Daten gefunden - Abbruch.", file=sys.stderr)
        sys.exit(1)

    fig.update_xaxes(title_text="Periodendauer [s]", row=len(signals), col=1)
    if args.xscale == "log":
        fig.update_xaxes(type="log")
    for row_idx in range(1, len(signals) + 1):
        fig.update_yaxes(title_text="rel. Amplitude", row=row_idx, col=1)

    fig.update_layout(
        title="FFT-Spektralanalyse L2-Zeitreihen &mdash; Detektor x Zeitstempel x Signal",
        height=max(300 * len(signals), 400),
        hovermode="x unified",
        legend=dict(itemsizing="constant"),
        template="plotly_white",
    )

    fig.write_html(args.output, div_id="spectral-plot")
    inject_control_panel(args.output, trace_meta)
    print(f"Grafik gespeichert: {args.output}")


if __name__ == "__main__":
    main()
