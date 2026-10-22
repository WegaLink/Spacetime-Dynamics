═══════════════════════════════════════════════════════════════
  Quen 3.8 Max Phasendetektor-Empfindlichkeit-Abschätzung
═══════════════════════════════════════════════════════════════

Exportiert: 22. Oktober 2026 um 22:48
Nachrichten: 6
Modell: alibaba-qwen-3.8-max

───────────────────────────────────────────────────────────────

[👤 Sie]
In einem Team aus 12 KIs und mir als Mensch sollen Verbesserungen für einen Phasendetektor diskutiert und erarbeitet werden, mit welchem Phänomene im Bereich Raumzeitdynamik untersucht werden sollen. Die Zusammenarbeit findet mit einem markdown-Dokument statt, welches im GitHub-Repository unter folgendem Link verfügbar ist:
https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md Basierend auf den Verbesserungsvorschlägen in „Physik 3-2“ soll als nächstes ein Konzept mit digitalem I/Q mit einer fertigen Hardware Eclypse Z7 + ADC + AWG untersucht werden. Nachteilig ist, dass kein externer Oszillator angeschlossen werden kann, wohingegen die fertige Hardware ein Vorteil bei der Umsetzung ist, welche vollständig in Software erfolgt. Das Konzept führt weiterhin die bisher getrennten Systeme analog und digital zu einem System zusammen, da der AWG (Takt 125 MHz) sowohl Sinus als auch Rechteck als auch komplexe Signalformen erzeugen kann. Als Kabeltopologie bietet sich vorzugsweise Typ B an, wo das Signal des AWG durch ein 30 m langes gerades Sat-Kabel hin und zurück läuft, bevor es mit dem ADC mit hoher Samplingrate von 105 MHz/14 Bit ausgewertet wird. Der verbaute FPGA hat gleichzeitig Zugriff auf das gesendete und das empfangene Signal und soll daraus kontinuierlich die Laufzeitdifferenz im Kabel ermitteln. Das Signal der Laufzeitdifferenz soll anschließend im Frequenzbereich von 0-50 MHz weiter ausgewertet werden. Lässt sich aus diesen Informationen eine Systemempfindlichkeit für die Laufzeitdifferenz im Kabel abschätzen?

───────────────────────────────────────────────────────────────

[🤖 KI]
# Systemempfindlichkeit des Eclypse-Z7-Phasendetektors – Abschätzung

## 1. Systemparameter

| Parameter | Wert |
|-----------|------|
| AWG-Takt | 125 MHz |
| ADC-Abtastrate | 105 MHz |
| ADC-Auflösung | 14 Bit |
| Kabel | 30 m Sat-Kabel (Typ B), hin und zurück |
| Physische Signallaufstrecke | 60 m |
| Verkürzungsfaktor (Sat-Kabel, z. B. RG6 Foam-PE) | v_f ≈ 0,83 |
| Elektrische Laufzeit τ | 60 m / (0,83 · 3·10⁸ m/s) ≈ **241 ns** |
| FPGA | Zynq-7000 (gemeinsamer Takt für AWG + ADC) |

---

## 2. Theoretische Auflösung (Quantisierungsrauschen)

### 2.1 SNR des ADC

$$\text{SNR}_{\text{theor}} = 6{,}02 \cdot 14 + 1{,}76 \approx 86 \text{ dB}$$

Praktisch ist mit einer effektiven Auflösung (ENOB) von **12 Bit** zu rechnen:

$$\text{SNR}_{\text{eff}} \approx 74 \text{ dB} \quad \Rightarrow \quad \text{SNR}_{\text{lin}} \approx 2{,}5 \times 10^7$$

### 2.2 Phasenrauschen pro I/Q-Stützstelle

Bei digitaler I/Q-Demodulation (Multiplikation mit cos/sin, Tiefpass):

$$\sigma_\varphi = \frac{1}{\sqrt{2 \cdot \text{SNR}_{\text{lin}}}} = \frac{1}{\sqrt{2 \cdot 2{,}5 \times 10^7}} \approx 1{,}4 \times 10^{-4} \text{ rad}$$

### 2.3 Zeitliche Auflösung pro Stützstelle

Für eine Signalfrequenz $f_0$:

$$\sigma_{\Delta t}^{(1)} = \frac{\sigma_\varphi}{2\pi f_0}$$

| $f_0$ | $\sigma_{\Delta t}^{(1)}$ |
|--------|---------------------------|
| 10 MHz | 2,2 ps |
| 25 MHz | 0,9 ps |
| 50 MHz | 0,45 ps |

### 2.4 Mittelung über N Stützstellen

Bei einer Messbandbreite $B$ und Abtastrate $f_s = 105$ MHz:

$$N = \frac{f_s}{2B}, \qquad \sigma_{\Delta t}(B) = \sigma_{\Delta t}^{(1)} \cdot \sqrt{\frac{2B}{f_s}}$$

---

## 3. Empfindlichkeit vs. Bandbreite (Tabelle)

Für $f_0 = 10$ MHz, ENOB = 12, $\tau = 241$ ns:

| Bandbreite $B$ | Mittelungszeit | $\sigma_{\Delta t}$ (theor.) | $\Delta\tau/\tau$ |
|:-:|:-:|:-:|:-:|
| 50 MHz | ~10 ns | 2,2 ps | 9 × 10⁻⁶ |
| 1 MHz | ~0,5 µs | 0,30 ps | 1,3 × 10⁻⁶ |
| 100 kHz | ~5 µs | 96 fs | 4 × 10⁻⁷ |
| 10 kHz | ~50 µs | 30 fs | 1,3 × 10⁻⁷ |
| 1 kHz | ~0,5 ms | 9,6 fs | 4 × 10⁻⁸ |
| 1 Hz | ~0,5 s | 0,30 fs | 1,3 × 10⁻⁹ |

> **Hinweis:** Diese Werte gelten nur für das Quantisierungsrauschen. In der Praxis dominieren andere Effekte (siehe Abschnitt 4).

---

## 4. Praktische Begrenzungen

### 4.1 Kabeltemperatur (dominant!)

Der Temperaturkoeffizient der Laufzeit für Koaxialkabel liegt bei:

$$\alpha_\tau \approx 50\text{–}150 \text{ ppm/K}$$

Für $\tau = 241$ ns und $\alpha_\tau = 100$ ppm/K:

$$\frac{\partial \tau}{\partial T} = 241 \text{ ns} \times 100 \times 10^{-6} = 24 \text{ ps/K}$$

| Ziel-$\sigma_{\Delta t}$ | Erforderliche Temperaturstabilität |
|:-:|:-:|
| 1 ps | ± 42 mK |
| 100 fs | ± 4,2 mK |
| 10 fs | ± 0,42 mK |

**Fazit:** Ohne aktive thermische Stabilisierung des Kabels ist die Empfindlichkeit auf ca. **1–10 ps** (bei ±0,05–0,5 K Schwankung) begrenzt. Das Quantisierungsrauschen wird ab ca. 1 kHz Bandbreite irrelevant.

### 4.2 Takt-Jitter (Common-Mode-Unterdrückung)

Da AWG und ADC denselben 125-MHz-Takt auf der Eclypse Z7 nutzen, hebt sich der Taktjitter in erster Ordnung auf. Resteffekte entstehen durch:
- Unterschiedliche Signalpfadlängen im FPGA (Routenverzögerung)
- Jitter im ADC-Sample-and-Hold (typisch < 1 ps RMS)

Diese sind vernachlässigbar gegenüber der Kabeltemperatur.

### 4.3 ADC/DAC-Nichtlinearität

DNL/INL des 14-Bit-ADC erzeugen harmonische Verzerrungen, die bei der I/Q-Demodulation als Phasenfehler erscheinen. Typisch: < 0,1° bei guter Kalibrierung → vernachlässigbar.

### 4.4 Kabeldispersion

Bei $f_0 = 10$ MHz ist die Dispersion in Sat-Kabel vernachlässigbar. Bei $f_0 > 100$ MHz könnte sie relevant werden.

---

## 5. Realistische Systemempfindlichkeit

Unter Berücksichtigung der praktischen Begrenzungen:

| Szenario | Bandbreite | $\sigma_{\Delta t}$ (realistisch) | $\Delta\tau/\tau$ |
|:---------|:-:|:-:|:-:|
| Unkontrollierte Umgebung (Labor) | 1 Hz | 5–20 ps | 2–8 × 10⁻⁵ |
| Thermisch stabilisiert (±10 mK) | 1 Hz | 0,5–1 ps | 2–4 × 10⁻⁶ |
| Thermisch stabilisiert (±1 mK) | 1 Hz | 100–200 fs | 4–8 × 10⁻⁷ |
| Kurzzeitmessung (< 1 s) | 10 kHz | 2–5 ps | 1–2 × 10⁻⁵ |

---

## 6. Vergleich mit dem bisherigen System

| | Altes System (XMC4700) | Neues System (Eclypse Z7) |
|:--|:-:|:-:|
| Laufzeit τ | ~2 µs (400 m) | ~241 ns (60 m) |
| Auflösung δτ | 10–100 ps | 1–10 ps (thermisch begrenzt) |
| **Δτ/τ** | **5 × 10⁻⁶ – 5 × 10⁻⁵** | **4 × 10⁻⁶ – 4 × 10⁻⁵** |
| Takt | PWM + Capture, gemeinsamer 144-MHz-Takt | AWG + ADC, gemeinsamer 125-MHz-Takt |
| Vorteil | Längere Laufzeit | Flexiblere Signalformen, I/Q in Software |

Die relative Empfindlichkeit ist vergleichbar. Der Vorteil des neuen Systems liegt in:
- **Software-Definierbarkeit** (beliebige Signalformen, adaptive Filterung)
- **Gleichzeitiger Zugriff** auf TX und RX im FPGA
- **Potenzial zur Verbesserung** durch längeres Kabel oder Resonanzbetrieb

---

## 7. Empfehlungen zur Empfindlichkeitssteigerung

| Maßnahme | Erwarteter Gewinn | Aufwand |
|:---------|:-:|:-:|
| Kabel in thermisch isoliertes Gehäuse (±5 mK) | Faktor 5–10 | gering |
| Zweites, thermisch gekoppeltes Referenzkabel (Common-Mode-Rejection) | Faktor 2–5 | mittel |
| Signalfrequenz auf 25–50 MHz erhöhen | Faktor 2–5 (theor.) | gering |
| Kabel auf 100 m verlängern (τ → 800 ns) | Faktor 3 in Δτ/τ | gering |
| Resonanzbetrieb (mehrfache Umläufe) | Faktor 5–50 | hoch |

---

## 8. Zusammenfassung

**Die abgeschätzte Systemempfindlichkeit liegt bei:**

$$\boxed{\sigma_{\Delta t} \approx 1\text{–}10 \text{ ps} \quad \Leftrightarrow \quad \frac{\Delta\tau}{\tau} \approx 4 \times 10^{-6} \text{ bis } 4 \times 10^{-5}}$$

wobei die untere Grenze durch die thermische Stabilität des Kabels bestimmt wird, nicht durch das Quantisierungsrauschen des ADC. Das System erreicht damit eine vergleichbare relative Empfindlichkeit wie der bisherige Aufbau, bietet aber durch die digitale I/Q-Verarbeitung und die gemeinsame Taktbasis ein deutlich robusteres Fundament für zukünftige Verbesserungen.

**Citations:**
[1] [https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md](https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md)

───────────────────────────────────────────────────────────────

[👤 Sie]
wie funktioniert die common-mode-rejection mit einem zweiten, thermisch gekoppelten referenzkabel?

───────────────────────────────────────────────────────────────

[🤖 KI]
# Common-Mode-Rejection mit thermisch gekoppeltem Referenzkabel

## 1. Grundprinzip

```
                    ┌──────────────────────────────────────────┐
                    │            Eclypse Z7 (FPGA)             │
                    │                                          │
   AWG Ch.1 ──┬────┤  TX_A ──→ [Kabel A: 60 m] ──→ RX_A ──→ φ_A  │
              │    │                                          │
              │    │  TX_B ──→ [Kabel B: 60 m] ──→ RX_B ──→ φ_B  │
              │    │                                          │
              │    │         Δφ = φ_A − φ_B                   │
              │    └──────────────────────────────────────────┘
              │
              └──→ Splitter (oder AWG Ch.2)
```

Beide Kabel liegen **physisch nebeneinander** (z. B. zusammen aufgewickelt oder im selben Schlauch), sodass sie denselben Temperaturschwankungen ausgesetzt sind.

---

## 2. Mathematische Beschreibung

### Einzelnes Kabel

Die gemessene Laufzeit eines Kabels ist:

$$\tau_i(T) = \tau_{i,0} + \alpha_\tau \cdot \tau_{i,0} \cdot \Delta T + \delta_i$$

wobei:
- $\tau_{i,0}$: Nennlaufzeit
- $\alpha_\tau$: Temperaturkoeffizient (~100 ppm/K)
- $\Delta T$: Temperaturänderung
- $\delta_i$: nichtthermische Störgröße (das eigentliche Messsignal)

### Differenzbildung

$$\Delta\tau = \tau_A - \tau_B = \underbrace{(\tau_{A,0} - \tau_{B,0})}_{\text{konstant, kalibrierbar}} + \alpha_\tau \cdot \underbrace{(\tau_{A,0} - \tau_{B,0})}_{\approx 0} \cdot \Delta T + (\delta_A - \delta_B)$$

Wenn $\tau_{A,0} \approx \tau_{B,0}$ (gleiche Kabellänge), hebt sich der Temperaturterm auf:

$$\boxed{\Delta\tau \approx \delta_A - \delta_B}$$

Die Temperatur wirkt nur noch über den **Mismatch** der beiden Kabel.

---

## 3. Restempfindlichkeit durch Mismatch

### 3.1 Längen-Mismatch

Wenn die Kabel um $\Delta L$ voneinander abweichen:

$$\tau_{A,0} - \tau_{B,0} = \frac{\Delta L}{v_f \cdot c}$$

Der verbleibende Temperaturterm ist dann:

$$\sigma_{\Delta\tau,\text{temp}} = \alpha_\tau \cdot \frac{\Delta L}{v_f \cdot c} \cdot \sigma_T$$

**Beispiel:** $\Delta L = 1$ cm, $\alpha_\tau = 100$ ppm/K, $\sigma_T = 1$ K:

$$\sigma_{\Delta\tau} = 100 \times 10^{-6} \times \frac{0{,}01}{0{,}83 \times 3 \times 10^8} \times 1 = 4 \text{ fs}$$

### 3.2 Mismatch im Temperaturkoeffizienten

Selbst bei gleicher Länge können die Kabel leicht unterschiedliche $\alpha_\tau$ haben (Fertigungstoleranz):

$$\sigma_{\Delta\tau} = \tau_0 \cdot \Delta\alpha_\tau \cdot \sigma_T$$

| $\Delta\alpha_\tau / \alpha_\tau$ | $\sigma_T = 1$ K | $\sigma_T = 0{,}1$ K |
|:-:|:-:|:-:|
| 1 % | 240 fs | 24 fs |
| 0,1 % | 24 fs | 2,4 fs |
| 0,01 % | 2,4 fs | 0,24 fs |

---

## 4. Vergleich: Einzelkabel vs. Referenzkabel

| | Einzelkabel | Mit Referenzkabel |
|:--|:-:|:-:|
| Temperaturabhängigkeit | 24 ps/K | 0,02–0,24 ps/K (bei 0,01–0,1 % Mismatch) |
| Empfindlichkeit bei ±1 K | 24 ps | 20–240 fs |
| Empfindlichkeit bei ±0,1 K | 2,4 ps | 2–24 fs |
| **Verbesserung** | – | **Faktor 100–1000** |

---

## 5. Praktische Umsetzung mit der Eclypse Z7

### 5.1 Hardware-Topologie

```
AWG Ch.1 ──→ Splitter ──┬──→ Kabel A (60 m) ──→ ADC Ch.1
                         │
                         └──→ Kabel B (60 m) ──→ ADC Ch.2
```

**Wichtig:** Der Splitter selbst muss ebenfalls thermisch stabil sein, da er eine zusätzliche Laufzeit einbringt. Alternativ: AWG Ch.1 → Kabel A, AWG Ch.2 → Kabel B (dann muss die Phasenbeziehung der beiden AWG-Kanäle kalibriert werden).

### 5.2 FPGA-Verarbeitung

```
┌─────────────────────────────────────────────────┐
│  ADC Ch.1 → I/Q-Demod → φ_A(t)                │
│                                    → Δφ(t) → LP → Δτ(t)
│  ADC Ch.2 → I/Q-Demod → φ_B(t)                │
└─────────────────────────────────────────────────┘
```

Die Differenzbildung erfolgt **samplegenau** im FPGA, da beide ADC-Kanäle synchron abgetastet werden.

### 5.3 Kalibrierung

1. **Offset-Kalibrierung:** Beide Kabel kurzschließen (oder durch gleich lange Kalibrierkabel ersetzen), Offset $\Delta\tau_0$ messen und subtrahieren.
2. **Drift-Kalibrierung:** Langzeitmessung unter konstanten Bedingungen, um thermische Asymmetrien zu charakterisieren.

---

## 6. Grenzen der Methode

| Grenze | Ursache | Gegenmaßnahme |
|:-------|:--------|:--------------|
| Thermischer Gradient zwischen A und B | Kabel nicht perfekt gekoppelt | Gemeinsames Gehäuse, Wärmeleitpaste |
| Splitter-Asymmetrie | Temperaturabhängigkeit des Splitters | Splitter ebenfalls thermisch einbinden |
| ADC-Kanal-Mismatch | Unterschiedliche Gruppenlaufzeit der ADC-Eingänge | Regelmäßige Kalibrierung, Kanalwechsel |
| AWG-Kanal-Mismatch | Phasenversatz zwischen Ch.1 und Ch.2 | Nur einen Kanal + Splitter verwenden |
| Kabelalterung | $\alpha_\tau$ driftet über Monate | Periodische Rekalibrierung |

---

## 7. Zusammenfassung

Die Common-Mode-Rejection wandelt das Problem der **absoluten Temperaturstabilität** in das (viel einfachere) Problem der **relativen Temperaturgleichheit** zweier Kabel um.

$$\text{Einzelkabel: } \sigma_{\Delta t} = \alpha_\tau \cdot \tau \cdot \sigma_T \approx 24 \text{ ps/K}$$

$$\text{Referenzkabel: } \sigma_{\Delta t} = \alpha_\tau \cdot \tau \cdot \frac{\Delta\alpha_\tau}{\alpha_\tau} \cdot \sigma_T \approx 2\text{–}240 \text{ fs/K}$$

Der entscheidende praktische Aufwand liegt darin, die beiden Kabel **thermisch so gut wie möglich zu koppeln** – idealerweise durch gemeinsames Aufwickeln auf einen temperaturstabilen Träger (z. B. Aluminiumspule in einer Styroporbox).

───────────────────────────────────────────────────────────────

[👤 Sie]
wie verhält sich das Nutzsignal, welches in beiden Kabeln etwa gleich ist, wird dieses nicht auch durch die Differenzbildung unterdrückt?

───────────────────────────────────────────────────────────────

[🤖 KI]
# Das Common-Mode-Problem des Nutzsignals

Das ist eine fundamentale und berechtigte Frage. Die Antwort hängt davon ab, **wie das Nutzsignal räumlich auf die Kabel wirkt**.

---

## 1. Das Dilemma

```
Temperatur:     wirkt gleich auf beide Kabel  → Common-Mode  → soll unterdrückt werden
Nutzsignal:     wirkt ??? auf beide Kabel     → ???          → soll gemessen werden
```

Wenn das Nutzsignal ebenfalls gleich auf beide Kabel wirkt, wird es zusammen mit der Temperatur unterdrückt. Das wäre kontraproduktiv.

---

## 2. Fälle je nach physikalischem Modell

### Fall A: Nutzsignal wirkt global und isotrop

**Beispiel:** Eine gleichmäßige Skalierung der Raumzeit (hypothetische „Atmung" der Raumzeit)

```
Kabel A:  ████████████████████  → τ_A + δ
Kabel B:  ████████████████████  → τ_B + δ
                                    ─────────────
Differenz:                         τ_A − τ_B  → δ hebt sich auf ✗
```

**Ergebnis:** Das Nutzsignal wird vollständig unterdrückt. Die Common-Mode-Rejection arbeitet hier **gegen** die Messung.

---

### Fall B: Nutzsignal wirkt richtungsabhängig (tensoriell)

**Beispiel:** Gravitationswellen-ähnliche Störung mit quadrupolarer Charakteristik

```
Raumzeitstörung:  ←→  (Streckung in x, Stauchung in y)

Kabel A (x-Richtung):  ████████████████████  → τ_A + δ
Kabel B (y-Richtung):  ████████████████████  → τ_B − δ
                                    ─────────────
Differenz:                                  2δ  → Signal verdoppelt ✓
```

**Ergebnis:** Das Nutzsignal erscheint **differentiell** und wird sogar verdoppelt.

---

### Fall C: Nutzsignal wirkt lokal

**Beispiel:** Eine lokale Raumzeitfluktuation, die nur einen Raumbereich betrifft

```
         Störzone
            ↓
Kabel A:  ████▓▓▓▓████████████  → τ_A + δ
Kabel B:  ████████████████████  → τ_B
                                    ─────────────
Differenz:                            δ  → Signal sichtbar ✓
```

**Ergebnis:** Messbar, wenn die Kabel räumlich getrennt verlegt werden.

---

## 3. Konsequenz für das Kabeldesign

| Nutzsignal-Modell | Kabelanordnung | Common-Mode-Rejection |
|:--|:--|:--|
| Global isotrop | **Gleich orientiert, überlagert** | ✗ unterdrückt Nutzsignal |
| Tensoriell (richtungsabhängig) | **Senkrecht zueinander** | ✓ unterdrückt nur Temperatur |
| Lokal | **Räumlich getrennt** | ✓ unterdrückt nur Temperatur |
| Unbekannt | **Mehrere Konfigurationen parallel** | → Unterscheidung möglich |

---

## 4. Praktische Lösung: Dreifach-Messung

Um das Dilemma aufzulösen, kann man **drei Kabel** verwenden:

```
┌─────────────────────────────────────────────────────┐
│                                                     │
│  Kabel A ──── x-Richtung ────┐                     │
│                               ├→ Δφ_AB = φ_A − φ_B │
│  Kabel B ──── y-Richtung ────┘                     │
│                                                     │
│  Kabel C ──── x-Richtung ────┐                     │
│  (parallel zu A, thermisch    ├→ Δφ_AC = φ_A − φ_C │
│   gekoppelt mit A)            ┘                     │
│                                                     │
└─────────────────────────────────────────────────────┘
```

| Differenz | Misst | Unterdrückt |
|:--|:--|:--|
| $\Delta\varphi_{AB}$ | Tensorielles Signal + Temperatur-Mismatch | Common-Mode-Temperatur |
| $\Delta\varphi_{AC}$ | Nur Temperatur-Mismatch | Alles andere |

Durch Kombination:

$$\text{Nutzsignal}_{\text{tensoriell}} = \Delta\varphi_{AB} - \Delta\varphi_{AC}$$

Damit lässt sich ein **tensorielles Nutzsignal** von der **Temperatur** trennen.

---

## 5. Was bedeutet das für das aktuelle Konzept?

### 5.1 Einzelkabel (Typ B, hin und zurück)

```
AWG ──→ Kabel (30 m hin) ──→ Kabel (30 m zurück) ──→ ADC
```

Hier liegt das Kabel **doppelt**, aber in **entgegengesetzter Richtung**. Das hat eine interessante Eigenschaft:

- **Isotropes Signal:** Wirkt gleich auf Hin- und Rückweg → addiert sich → **messbar** ✓
- **Tensorielles Signal (x-Richtung):** Hinweg wird gestreckt, Rückweg wird gestaucht → hebt sich auf → **nicht messbar** ✗

Das Einzelkabel in Typ-B-Topologie ist also ein **Detektor für isotrope Laufzeitänderungen**, aber blind für richtungsabhängige Effekte.

### 5.2 Mit Referenzkabel

| Anordnung | Isotropes Signal | Tensorielles Signal | Temperatur |
|:--|:-:|:-:|:-:|
| A und B parallel, gleiche Richtung | unterdrückt ✗ | unterdrückt ✗ | unterdrückt ✓ |
| A und B senkrecht zueinander | unterdrückt ✗ | **messbar** ✓ | unterdrückt ✓ |
| A hin+zurück, B nur Referenz | **messbar** ✓ | unterdrückt ✗ | unterdrückt ✓ |

---

## 6. Empfehlung

Da das physikalische Modell der Raumzeitdynamik noch nicht feststeht, ist eine **kombinierte Messung** sinnvoll:

```
┌──────────────────────────────────────────────────────────┐
│                                                          │
│  Kabel A: 30 m hin + 30 m zurück (Typ B)               │
│           → empfindlich auf isotrope Laufzeitänderung   │
│                                                          │
│  Kabel B: 30 m gerade, parallel zu Kabel A              │
│           → Referenz für Temperatur                     │
│                                                          │
│  Kabel C: 30 m gerade, senkrecht zu Kabel A             │
│           → empfindlich auf tensorielle Komponente      │
│                                                          │
│  Auswertung:                                            │
│    φ_A − φ_B  → isotropes Signal (temperaturkorrigiert)│
│    φ_A − φ_C  → tensorielles + isotropes Signal        │
│    φ_B − φ_C  → tensorielles Signal (temperaturkorrig.)│
│                                                          │
└──────────────────────────────────────────────────────────┘
```

Damit lassen sich **alle drei Fälle unterscheiden**, und die Common-Mode-Rejection unterdrückt nur die Temperatur, nicht das Nutzsignal.

---

## 7. Zusammenfassung

| Frage | Antwort |
|:--|:--|
| Wird das Nutzsignal unterdrückt? | **Ja**, wenn es common-mode ist (beide Kabel gleich betroffen) |
| Ist das ein Problem? | **Nur**, wenn das physikalische Modell ein isotropes Signal vorhersagt und die Kabel parallel liegen |
| Lösung? | Kabelgeometrie an das erwartete Signalmodell anpassen; im Zweifel mehrere Geometrien parallel messen |
| Typ-B-Topologie (hin+zurück)? | Wirkt selbst als „Common-Mode-Rejection" für tensorielle Signale, ist aber empfindlich für isotrope |

Die Kabeltopologie ist somit kein rein technisches Detail, sondern ein **physikalisches Designelement**, das bestimmt, auf welche Art von Raumzeitphänomen das System empfindlich ist.

═══════════════════════════════════════════════════════════════
  Ende der Konversation
═══════════════════════════════════════════════════════════════
