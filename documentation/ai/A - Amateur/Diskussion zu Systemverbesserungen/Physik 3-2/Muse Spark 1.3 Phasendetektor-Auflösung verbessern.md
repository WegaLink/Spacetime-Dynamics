═══════════════════════════════════════════════════════════════
  Muse Spark 1.3 Phasendetektor-Auflösung verbessern
═══════════════════════════════════════════════════════════════

Exportiert: 20. Oktober 2026 um 23:42
Nachrichten: 2
Modell: muse-spark-1.3

───────────────────────────────────────────────────────────────

[👤 Sie]
In einem Team aus 12 KIs und mir als Mensch sollen Verbesserungen für einen Phasendetektor diskutiert und erarbeitet werden, mit welchem Phänomene im Bereich Raumzeitdynamik untersucht werden sollen. Die Zusammenarbeit findet mit einem markdown-Dokument statt, welches im GitHub-Repository unter folgendem Link verfügbar ist:
https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md Nach der Analyse der Phasendetektor-Auflösung eines analogen und eines digitalen Systems in Physik 3-1 sollen für beide Systeme Vorschläge zu möglichen Verbesserungen der Auflösung generiert werden. Störeinflüsse durch Umwelt, Umgebung und Gerätekanäle sollen einer nächsten Diskussion in Physik 2 vorbehalten bleiben und soweit möglich zunächst nicht betrachtet werden. Wie kann das technische Konzept verändert werden, um Verbesserungen zu erhalten? Welche kritischen elektronischen Bauteile sollten zum Einsatz kommen, um Verbesserungen der Phasendetektor-Auflösung zu erhalten? Welche Größenordnung einer Verbesserung kann damit erreicht werden?

───────────────────────────────────────────────────────────────

[🤖 KI]
# Physik 3-1 : Auflösung Analog vs. Digital - Verbesserungsvorschläge

> Fokus: nur **intrinsische Auflösung** des Phasenkomparators.
> Umwelt/Umgebung/Gerätekanäle = Physik 2, hier ausgeklammert.

## 1. Ausgangslage

Zwei getrennte Ketten, dürfen nicht vermischt werden:

**A - Historisch analog:** 2x freier 16 MHz Oszillator, ~25m RG58, Mischer + Bänder 0-5 MHz / 5-15 MHz. Primärgröße: $\int \Delta f/f \, dt$

**B - Aktuell digital:** 1x 144 MHz XMC4700, PWM -> 4x100m Cat-6a, $v\approx 0.66c$, $\tau \approx 2\mu s$ + ~60 ns Offset, RS422 LTC1687CS Return, Capture + 3D3428 Feinstufen ~30 ps + Statistik ~1000 Zyklen. Effektiv 10-100 ps.

Nachweisgrenze aktuell:

$$\delta\tau/\tau = (10-100ps)/2\mu s = 5\cdot10^{-6} \text{ bis } 5\cdot10^{-5}$$

Ziel: Faktor 2, 10, 100 in $\delta\tau_{min}$ bei gleichem $\tau$.

Grundformel Digital:

$$\sigma_{\tau}(N) = \frac{T_{clk}}{\sqrt{12 \cdot N}} \oplus \sigma_{jitter} \oplus \sigma_{quant,fein}$$

mit $T_{clk}=6.94ns$, $N\approx 1000$. Reines Quantisierungs-Limit $\approx 63ps$, mit Feinstufe + Mittelung 10-100 ps - konsistent.

Analog:

$$\sigma_{\phi} \approx \frac{v_n}{A \cdot \sqrt{BW}} , \quad \delta\tau = \sigma_{\phi}/2\pi f_0$$

limitiert durch Mischer-Rauschen, AM-PM, Offsetdrift, PLL-Phasenrauschen.

## 2. Digitales System B - Konzeptänderungen

### B1. Takt von 144 MHz -> 500 MHz - 1 GHz FPGA-TDC
Weg von MCU-Capture zu dediziertem TDC in FPGA / ASIC.

* Carry-Chain TDC (Xilinx UltraScale, Intel Cyclone 10): LSB 5-15 ps, RMS 8-20 ps single-shot.
* Mit $N=10^4$ Mittelung: $0.1-0.3 ps$ statistisch, praktisch 1-3 ps durch INL.
* Kritische Bauteile: Xilinx Artix-7 / Lattice ECP5 + TDC-IP, TI TDC7201 als Referenz (55 ps LSB, 35 ps RMS), AMS GPX2 (10 ps).
* **Gewinn: Faktor 10-30** : 100 ps -> 3-10 ps.

### B2. Feininterpolation Dual-Slope / Vernier / Nutt
3D3428 (30 ps Schritt, hohe INL, Tempdrift) ersetzen:

* AD9508 Clock-Delay, HMC856 programmierbare Delayline 3 ps, oder DS1100.
* Vernier-Oszillator TDC: Auflösung 1-5 ps.
* Nutt-Methode: grob Zähler + fein Time-Stretcher x100-1000.
* Kritisch: ADCMP580/581 Komparator (<100 fs jitter), LTC6957 Clock-Buffer (60 fs), Si5345 Jitter-Cleaner.
* **Gewinn: Faktor 3-10**

### B3. Trägerfrequenz erhöhen 5 MHz -> 50-200 MHz
$\Delta\phi = 2\pi f_0 \Delta\tau$. Bei 10x $f_0$ gibt gleiche $\Delta\tau$ 10x $\Delta\phi$.

* Cat-7 bis 600 MHz spezifiziert, 50 MHz problemlos, Dämpfung ~ 0.1 dB/m.
* Treiber: LVDS / CML statt RS422, z.B. DS90LV011A, MC100EP16, LTC1687 ersetzen durch FIN1002 + Equalizer DS15EA101.
* ADC-Demodulation statt Flanken-Capture: 16-bit 250 MSPS ADC (ADS54J60, LTC2107) + DDC in FPGA, Phasenauflösung $<10 \mu rad/\sqrt{Hz}$.
* **Gewinn: Faktor 5-20**, begrenzt durch Kabeldämpfung/Dispersion.

### B4. Mehrfachumlauf / Resonanz - Topologie B -> Resonator
Signal N-mal durch Schleife statt 1x hin-rück:

$$\Delta t_{res}=N\cdot\Delta t_{einfach}$$

* Passive Reflexion mit HF-Schalter ADG901, aktive Re-Zirkulation mit Verstärker ADA4898 + AGC.
* N=10-20 realistisch bei Cat-7 Q~20-50 bei 10 MHz.
* Kritisch: rauscharmer Verstärker NF <1nV/rtHz, Oszillator-Phasenrauschen <-150 dBc/Hz @1kHz.
* **Gewinn: Faktor 5-15**, aber nur für quasistatische Störung mit $\omega_m \cdot N\tau \ll 1$.

### B5. Differentielle Zweitstrecke + Cross-Correlation
Nicht zur Driftkompensation (Physik 2), sondern zur Auflösungssteigerung:

* 2 identische Capture-Kanäle am selben Takt, Kreuzkorrelation: unkorreliertes Quantisierungsrauschen fällt mit $1/\sqrt{N_{ch}\cdot N_{avg}}$.
* Dual-ADC: LTC2185 2x 125 MSPS + FPGA-Mittelung 10s-100s.
* **Gewinn: Faktor 1.4-3** pro Verdopplung, mit 4 Kanälen Faktor 2.

> **Digital gesamt realistisch kaskadiert:** 100 ps -> 1-3 ps = **Faktor 30-100**, mit neuer Elektronik + höherem Träger + FPGA-TDC. Ohne Kabeländerung. Mit Resonator N=10 zusätzlich Faktor ~5, total bis ~500 theoretisch.

## 3. Analoges System A - Konzeptänderungen

### A1. Doppel-Balanced-Mischer + Limiting
Historischer einfacher Diodenmischer ersetzen:

* ADE-1L / HMC207 (IP3 +20dBm, Isolation 50dB), Vorverstärker PGA-103+, Limiter AD8306 gegen AM-PM.
* Basisband-Verstärker ADA4522 chopper (5nV/rtHz), 24-bit ADC ADS127L01 statt Zähler.
* **Gewinn: Faktor 3-5**

### A2. OCXO / Rubidium gemeinsame Referenz statt 2x frei
Größter Hebel historisch:

* 2x freier 16 MHz Quarz: Allan ~1e-8 @1s dominiert alles. Ersetzen durch 1x OCXO Morion MV89A ($\sigma_y \approx 5e-13$ @1s) gesplittet, oder 2x相干 PLL auf 10 MHz Rb SRS PRS10.
* Kritisch: Wenzel / Connor-Winfield OCXO, SiTime SiT5356 als Low-Cost.
* **Gewinn: Faktor 10-100** im Allan-Floor, macht Mischer-Rauschen erst sichtbar.

### A3. Lock-in / IQ-Demodulation statt Bänder
0-5 / 5-15 MHz Bänder ersetzen durch:

* AD8302 Gain/Phase-Detector (10 mV/dB, 10 mV/Grad bis 2.7 GHz) oder ADL5380 IQ-Demodulator +同步 Detection SR860.
* PLL ADF4351 + HMC7044 Jitter-Cleaner als LO.
* Phasenauflösung $<0.01^\circ$ = bei 16 MHz $<1.7ps$ äquivalent.
* **Gewinn: Faktor 10-30** gegenüber Nulldurchgangszähler.

### A4. Höhere Zwischenfrequenz / Heterodyn
16 MHz direkt mischen -> auf 100-200 MHz hochsetzen, dann runtermischen:

* $\delta\tau = \delta\phi/2\pi f$, höheres f = feineres Lineal.
* Kritisch: rauscharme Multiplier LTC6957 + BPF.
* **Gewinn: Faktor 3-10**

> **Analog gesamt:** von geschätzt ~1 ns (frei + einfach) -> 10-50 ps = **Faktor 20-100**, fast ausschließlich durch Referenz-Oszillator + IQ-Detektion.

## 4. Kritische Bauteile - Einkaufsliste Priorität

1. **Zeitbasis:** OCXO Morion MV335 / Vectron OX-204, ggf. SRS PRS10 Rb - größter Einzelhebel für A, Common-Mode für B.
2. **Clock-Distribution:** LTC6957-3, Si5345A, HMC7044 - <100 fs additive jitter.
3. **Komparator/Receiver:** ADCMP580 (80ps, 100fs), DS15EA101 Equalizer für Cat - ersetzt LTC1687 Flankenunsicherheit.
4. **TDC/FPGA:** Artix-7 + Carry-TDC oder TDC7201/GPX2 - ersetzt 3D3428 + XMC Capture.
5. **ADC für Phase:** ADS127L01 (Analog-Basisband), ADS54J60 / LTC2185 (Digital-DDC).
6. **Mischer/Detektor (nur A):** ADE-1L, AD8302, ADL5380.
7. **Delay-Fein:** HMC856, AD9508 statt 3D3428.

## 5. Größenordnung Zusammenfassung

| Pfad | Maßnahme | Faktor | Kosten/Risiko |
|---|---|---|---|
| B | FPGA-TDC 5ps LSB | 10-30 | mittel, empfohlen |
| B | $f_0$ 5->50MHz + LVDS + ADC-DDC | 5-20 | mittel, empfohlen |
| B | N=10 Re-Zirkulation | 5-15 | hoch, Phase 2 |
| B | 2-4 Kanal Korrelation | 2-4 | niedrig |
| A | OCXO/Rb Referenz | 10-100 | niedrig, Pflicht |
| A | IQ/Lock-in AD8302 | 10-30 | niedrig |
| **B total** | TDC + HF + 2ch | **30-100** | -> 1-3 ps, $\delta\tau/\tau \approx 10^{-6}$ |
| **A total** | Ref + IQ | **20-100** | -> 10-50 ps |

Kein Umbau ohne vorherige **Injektionskalibrierung (Messung 3)**: bekannte $\Delta\tau$ via HMC856 einspeisen, Linearität Pause/Burst vs. Zähler prüfen. Sonst wird bessere Auflösung nur besseres Rauschen messen.

Vorschlag Reihenfolge: 1) OCXO + Clocktree, 2) FPGA-TDC Evalboard parallel zum XMC loggen, 3) $f_0$-Erhöhung, 4) Resonator erst nach Transferfunktion Physik 3.


**Citations:**
[1] [https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md](https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md)

═══════════════════════════════════════════════════════════════
  Ende der Konversation
═══════════════════════════════════════════════════════════════
