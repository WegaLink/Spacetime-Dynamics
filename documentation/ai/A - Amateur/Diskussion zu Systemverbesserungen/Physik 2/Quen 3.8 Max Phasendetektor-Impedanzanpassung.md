═══════════════════════════════════════════════════════════════
  Quen 3.8 Max Phasendetektor-Impedanzanpassung
═══════════════════════════════════════════════════════════════

Exportiert: 26. Oktober 2026 um 18:52
Nachrichten: 2
Modell: alibaba-qwen-3.8-max

───────────────────────────────────────────────────────────────

[👤 Sie]
In einem Team aus 12 KIs und mir als Mensch sollen Verbesserungen für einen Phasendetektor diskutiert und erarbeitet werden, mit welchem Phänomene im Bereich Raumzeitdynamik untersucht werden sollen. Die Zusammenarbeit findet mit einem markdown-Dokument statt, welches im GitHub-Repository unter folgendem Link verfügbar ist:
https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md Als Start für „Physik 2“ soll die Anpassung für Oszillator, Injektion und ADC an ein 250 m Sat-Kabel HQ 135 dB, 4-fach geschirmt von HB Digital mit 75 Ohm Impedanz und einem Verkürzungsfaktor von 0.82  entworfen werden. Der Oszillator ist ein GPSDO mit 10 MHz/+7dB Signal an 50 Ohm. Die Injektion soll durch Eclypse Z7 AWG mit einem programmierbaren Signal von 1 V Amplitude an 50 Ohm in einer Weise erfolgen, dass die Phase eines kombinierten Signals aus GPSDO und AWG im Bereich von etwa 300 ps moduliert werden kann. Die Eingangsimpedanz des Eclypse Z7 Scope ist mit 1 MOhm/18 pF angegeben. Ein Eingang soll mit dem GPSDO als Referenz verbunden werden und der andere als Messeingang mit dem anderen Ende des Sat-Kabels. Wie kann eine impedanzrichtige Anpassung dieser Anordnung realisiert werden?

───────────────────────────────────────────────────────────────

[🤖 KI]
# Impedanzangepasste Messkette für den Phasendetektor mit 250 m Sat-Kabel

## 1. Systemübersicht

```
                    ┌─────────────────────────────────────────────────────────────────────┐
                    │                        Eclypse Z7                                    │
                    │                                                                      │
 GPSDO             │  ┌──────────┐         ┌──────────────┐         ┌──────────────┐     │
 10 MHz  ──────────────▶│ Splitter │──Port A──▶│ 50Ω Feed-  │──────────▶│ Scope Ch 1   │     │
 +7 dBm              │  │  (2-way) │         │ through Term │         │ (Referenz)   │     │
 50 Ω               │  └────┬─────┘         └──────────────┘         └──────────────┘     │
                    │       │                                                              │
                    │       │ Port B                                                       │
                    │       ▼                                                              │
                    │  ┌──────────┐         ┌──────────────┐         ┌──────────────┐     │
 AWG  ─────────────────▶│Combiner│──────────▶│ 50Ω→75Ω     │──────────▶│ 250 m Kabel  │     │
 10 MHz             │  │(asymm.)│         │ Anpassung    │         │ 75 Ω, VF 0.82│     │
 50 Ω, ≤1 V        │  └──────────┘         └──────────────┘         └──────┬───────┘     │
                    │                                                        │              │
                    │         ┌──────────────┐         ┌──────────────┐     │              │
                    │         │ Scope Ch 2   │◀──────────│ 75Ω Feed-  │◀────┘              │
                    │         │ (Messeingang)│         │ through Term │                    │
                    │         └──────────────┘         └──────────────┘                    │
                    └─────────────────────────────────────────────────────────────────────┘
```

## 2. Kabelparameter

| Parameter | Wert |
|-----------|------|
| Länge | 250 m |
| Impedanz | 75 Ω |
| Verkürzungsfaktor | 0,82 |
| Ausbreitungsgeschwindigkeit | 2,46 × 10⁸ m/s |
| **Einweg-Laufzeit** | **1,017 µs** |
| Elektrische Länge bei 10 MHz | 10,17 λ |
| Dämpfung bei 10 MHz (typ.) | ca. 3–5 dB/100 m → 7,5–12,5 dB gesamt |

> **Hinweis:** Die Angabe „135 dB" bezieht sich auf die Schirmdämpfung, nicht auf die Signaldämpfung. Die tatsächliche Leitungsdämpfung bei 10 MHz ist dem Datenblatt zu entnehmen.

---

## 3. Anpassung an den drei kritischen Übergängen

### 3.1 GPSDO (50 Ω) → Kabel (75 Ω)

**Empfohlene Lösung: Breitband-Übertrager**

Bei 10 MHz ist ein Übertrager die verlustärmste und breitbandigste Lösung:

| Option | Bauteil | Einfügedämpfung | Bandbreite |
|--------|---------|-----------------|------------|
| **A (empfohlen)** | Mini-Circuits ADT1.5-1 (50:75 Ω) | < 1 dB | 0,1–200 MHz |
| B (DIY) | 5 Windungen Primär / 6 Windungen Sekundär auf FT50-43 Ringkern | < 1 dB | 1–50 MHz |
| C (resistiv) | L-Pad: R_ser = 43,3 Ω, R_shunt = 150 Ω | ≈ 5,7 dB | DC–∞ |

**Schaltung Option C (resistives L-Pad):**

```
GPSDO (50Ω) ──┤├── 43,3 Ω ──┬──── Kabel (75Ω)
                              │
                             150 Ω
                              │
                             GND
```

Das L-Pad transformiert 50 Ω auf 75 Ω mit minimaler Fehlanpassung, kostet aber 5,7 dB. Für eine Kalibriermessung ist das akzeptabel, reduziert aber den SNR-Abstand.

**Begründung für den Übertrager:** Bei 10,17 Wellenlängen Kabellänge erzeugt jede Fehlanpassung (VSWR > 1) Mehrfachreflexionen, die das Phasenmaß verfälschen. Der Übertrager hält die Rückflussdämpfung unter −20 dB, das resistive Pad unter −30 dB. Beide sind geeignet; der Übertrager erhält mehr Signalpegel.

---

### 3.2 Kabel (75 Ω) → Scope-Eingang (1 MΩ / 18 pF)

**Problem:** Der hochohmige Scope-Eingang (1 MΩ ∥ 18 pF) stellt für das 75-Ω-Kabel praktisch einen Leerlauf dar. Bei 10,17 λ Kabellänge erzeugt dies eine totale Reflexion (Γ ≈ +1) und stehende Wellen – die Phasenmessung wird mehrdeutig.

**Lösung: 75-Ω-Durchführungsabschluss (Feed-Through Terminator)**

```
Kabel (75Ω) ──→ BNC-T-Stück ──→ 75Ω Abschluss (Feed-Through) ──→ Scope Ch2 (1MΩ/18pF)
```

Ein 75-Ω-Feed-Through-Terminator (z. B. BNC-Stecker mit eingebautem 75-Ω-Widerstand) wird direkt am Scope-Eingang aufgesteckt. Er präsentiert dem Kabel 75 Ω und leitet das Signal an den hochohmigen Scope-Eingang weiter.

**Nachweis der Vernachlässigbarkeit der Scope-Kapazität:**

$$X_C = \frac{1}{2\pi \cdot 10^7 \cdot 18 \times 10^{-12}} = 884\;\Omega \gg 75\;\Omega$$

Die 18 pF belasten den 75-Ω-Abschluss bei 10 MHz mit weniger als 9 % – der Fehler bleibt unter 0,1 dB und ist für die Phasenmessung vernachlässigbar.

---

### 3.3 AWG-Injektion (50 Ω) in den Signalpfad

**Ziel:** Eine quadrature (90°) Komponente zum GPSDO-Signal hinzufügen, um eine definierte Phasenmodulation von 0–300 ps zu erzeugen.

**Berechnung der erforderlichen AWG-Amplitude:**

Die Phasenverschiebung durch Vektoraddition beträgt:

$$\Delta\varphi = \arctan\!\left(\frac{B}{A}\right) \approx \frac{B}{A} \quad \text{für } B \ll A$$

Für 300 ps bei 10 MHz:

$$\Delta\varphi_{max} = 2\pi \cdot 10^7 \cdot 300 \times 10^{-12} = 0{,}01885 \text{ rad} = 1{,}08°$$

**Pegelberechnung durch die Kette:**

| Punkt | Pegel |
|-------|-------|
| GPSDO-Ausgang | +7 dBm = 0,708 V_peak an 50 Ω |
| Nach 2-Way-Splitter (−6 dB) | 0,354 V_peak |
| Nach Combiner (−6 dB, symmetrisch) | 0,177 V_peak |
| Erforderliche Quadratur-Komponente am Combiner-Ausgang | 0,177 × 0,01885 = **3,34 mV_peak** |
| Erforderliche AWG-Amplitude am Combiner-Eingang | 3,34 × 2 = **6,67 mV_peak** |

Die AWG muss also nur 6,67 mV_peak liefern – weit unterhalb der 1-V-Fähigkeit.

---

## 4. Combiner-Design: Asymmetrische Einspeisung

Da das AWG-Signal sehr klein gegenüber dem GPSDO-Signal ist, empfiehlt sich eine **asymmetrische Einspeisung** statt eines symmetrischen 3-dB-Combiners:

### Variante A: Resistive Einspeisung über hochohmigen Widerstand

```
                        R_inj
GPSDO ──▶ Splitter ──▶──┤├──────┬──────▶ 50→75Ω ──▶ Kabel
  Port B                470 Ω   │
                                 │
AWG ──▶──────────────────────────┘
  (50 Ω)
```

**Dimensionierung:**

Der Einspeisewiderstand R_inj bestimmt das Übersprechverhältnis:

$$V_{inj} = V_{AWG} \cdot \frac{Z_{node}}{R_{inj} + Z_{node}}$$

Mit Z_node ≈ 50 Ω (Eingangsimpedanz der Anpassungsstufe):

$$R_{inj} = Z_{node}\left(\frac{V_{AWG}}{V_{inj}} - 1\right) = 50\left(\frac{1\;\text{V}}{6{,}67\;\text{mV}} - 1\right) \approx 7{,}45\;\text{kΩ}$$

**Praktischer Wert:** 7,5 kΩ (E96-Reihe)

**Vorteil:** Die GPSDO-Pfad-Dämpfung beträgt nur ca. 0,3 dB (Spannungsteiler 7,5 kΩ ∥ 50 Ω ≈ 49,7 Ω). Die AWG sieht eine Last von ≈ 7,5 kΩ, was für den 50-Ω-Ausgang unkritisch ist (der Ausgang wird kaum belastet, die Spannung bleibt nahezu konstant).

**Nachteil:** Die AWG ist nicht impedanzangepasst (sieht 7,5 kΩ statt 50 Ω). Bei 10 MHz und der kurzen Verbindung ist das tolerierbar, da die Wellenlänge 30 m beträgt und die Leitung zum Combiner kurz ist (< 0,5 m → < λ/60).

### Variante B: Symmetrischer 3-Port-Combiner mit externem Dämpfungsglied

```
GPSDO ──▶ Splitter ──▶ Port 1 ──┐
                                  ├──▶ [3-Port Combiner] ──▶ 50→75Ω ──▶ Kabel
AWG ──▶ [40 dB Dämpfungsglied] ──▶ Port 2 ──┘
```

Mit einem 40-dB-Dämpfungsglied:
- AWG 1 V → 10 mV am Combiner-Eingang
- Combiner (−6 dB): 10 mV → 5 mV am Ausgang
- GPSDO: 0,354 V → 0,177 V am Ausgang
- Phasenhub: arctan(5 mV / 177 mV) = 0,028 rad = 450 ps

Für exakt 300 ps: AWG auf 0,67 V einstellen oder 43 dB Dämpfung verwenden.

**Vorteil:** Alle Ports sind impedanzangepasst (50 Ω). Bessere Isolierung zwischen GPSDO und AWG.

**Empfehlung:** Variante B für höchste Messgenauigkeit, Variante A für schnellen Aufbau.

---

## 5. Referenzpfad (Scope Ch1)

```
GPSDO ──▶ [Splitter] ──▶ Port A ──▶ [50Ω Feed-Through] ──▶ Scope Ch1
```

Der 50-Ω-Feed-Through-Terminator am Scope-Eingang stellt sicher, dass der GPSDO impedanzrichtig abgeschlossen ist und keine Reflexionen in den Referenzpfad zurücklaufen.

**Wichtig:** Die interne Laufzeitdifferenz zwischen Ch1 und Ch2 des Eclypse Z7 muss einmalig kalibriert werden (z. B. durch Verbindung beider Kanäle mit demselben Signal über gleich lange Kabel). Diese Offset-Kalibrierung ist Voraussetzung für die absolute Phasenmessung.

---

## 6. Phasenmodulation: Ansteuerung der AWG

Die AWG muss ein 10-MHz-Sinussignal **in Quadratur** (90° Phasenversatz) zum GPSDO erzeugen. Dafür gibt es zwei Möglichkeiten:

### Option 1: Externe Referenz (empfohlen)
Falls der Eclypse Z7 einen externen Referenzeingang (10 MHz) besitzt:
- GPSDO 10 MHz → Referenzeingang des Z7
- AWG-Phase auf 90° programmieren
- Amplitude der AWG steuert den Phasenhub

### Option 2: Software-Quadratur
Falls keine externe Referenz vorhanden ist:
- AWG erzeugt 10 MHz mit einstellbarer Phase
- Phase wird softwareseitig auf 90° relativ zum GPSDO kalibriert
- Dies erfordert eine einmalige Kalibrierung der relativen Phase

**Modulationsbetrieb:**
- AWG-Amplitude von 0 bis V_max sweepen → Phasenhub 0 bis 300 ps
- Oder: AWG-Amplitude konstant, Phase um ±δ modulieren → äquivalent

---

## 7. Zusammenfassung der Bauteile

| Position | Bauteil | Zweck |
|----------|---------|-------|
| Splitter | Mini-Circuits ZFSC-2-1 oder resistiv (3× 16,7 Ω) | GPSDO-Signal aufteilen |
| Combiner | Variante A: 7,5 kΩ Widerstand / Variante B: ZFSC-2-1 + 40 dB Pad | AWG-Signal einspeisen |
| 50→75 Ω | Mini-Circuits ADT1.5-1 oder L-Pad (43,3 Ω / 150 Ω) | Impedanzanpassung |
| Kabelabschluss | 75 Ω BNC Feed-Through Terminator | Reflexionsfreier Abschluss |
| Referenzabschluss | 50 Ω BNC Feed-Through Terminator | Reflexionsfreier Abschluss |
| Optional | 10 dB Dämpfungsglied vor Scope Ch1 | Pegelanpassung falls nötig |

---

## 8. Kritische Hinweise

1. **Thermische Stabilität:** 250 m Kabel haben einen typischen Temperaturkoeffizienten der Laufzeit von 10–50 ppm/K. Bei 1,017 µs Laufzeit entspricht 1 K Temperaturänderung einer Laufzeitänderung von **10–50 ns** – das ist 30–170× größer als die 300 ps, die injiziert werden sollen. Für die Kalibriermessung muss das Kabel thermisch stabil sein oder die Messung muss als **Differenzmessung** (mit/ohne Injektion) im schnellen Wechsel erfolgen.

2. **Dämpfung des Kabels:** Bei geschätzten 7–12 dB Dämpfung auf 250 m kommt am fernen Ende nur noch ein Bruchteil des Signals an. Bei +7 dBm Eingang und −10 dB Kabeldämpfung bleiben −3 dBm (0,158 V_peak) am Scope. Das ist für den Eclypse Z7 (Eingangsbereich typ. ±1 V bis ±25 V) ausreichend.

3. **Phasenmehrdeutigkeit:** Bei 10,17 λ elektrischer Länge ist die absolute Phase mehrdeutig. Für die Kalibriermessung ist nur die **Phasenänderung** (Δφ durch AWG-Injektion) relevant, nicht die absolute Phase.

4. **Scope-Abtastrate:** Bei 100 MSa/s und 10 MHz erhält man 10 Abtastwerte pro Periode. Für eine Phasenauflösung von 300 ps (≈ 1,08°) ist eine Mittelung über mindestens 100–1000 Perioden erforderlich, um das Quantisierungsrauschen zu unterdrücken.

**Citations:**
[1] [https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md](https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md)

═══════════════════════════════════════════════════════════════
  Ende der Konversation
═══════════════════════════════════════════════════════════════
