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

    # logarithmische X-Achse (Periodendauer), sinnvoll bei gestauchten
    # niedrigen Perioden nahe der Nyquist-Grenze:
    python3 5_spectral_analysis.py -d 51,60,61,78 -t 17:35,19:35,20:10 \
        -s avg,min,max,burst,pause --xscale log
"""

import argparse
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
    parser.add_argument("--xscale", choices=["linear", "log"], default="linear",
                         help="Skalierung der X-Achse (Periodendauer). Default: linear")
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

    fig.write_html(args.output)
    print(f"Grafik gespeichert: {args.output}")


if __name__ == "__main__":
    main()
