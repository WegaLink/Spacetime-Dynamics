═══════════════════════════════════════════════════════════════
  Mistral Large 4 I/Q-Laufzeitempfindlichkeitabschätzung
═══════════════════════════════════════════════════════════════

Exportiert: 22. Oktober 2026 um 22:49
Nachrichten: 4
Modell: mistral-large

───────────────────────────────────────────────────────────────

[👤 Sie]
In einem Team aus 12 KIs und mir als Mensch sollen Verbesserungen für einen Phasendetektor diskutiert und erarbeitet werden, mit welchem Phänomene im Bereich Raumzeitdynamik untersucht werden sollen. Die Zusammenarbeit findet mit einem markdown-Dokument statt, welches im GitHub-Repository unter folgendem Link verfügbar ist:
https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md Basierend auf den Verbesserungsvorschlägen in „Physik 3-2“ soll als nächstes ein Konzept mit digitalem I/Q mit einer fertigen Hardware Eclypse Z7 + ADC + AWG untersucht werden. Nachteilig ist, dass kein externer Oszillator angeschlossen werden kann, wohingegen die fertige Hardware ein Vorteil bei der Umsetzung ist, welche vollständig in Software erfolgt. Das Konzept führt weiterhin die bisher getrennten Systeme analog und digital zu einem System zusammen, da der AWG sowohl Sinus als auch Rechteck als auch komplexe Signalformen erzeugen kann. Als Kabeltopologie bietet sich vorzugsweise Typ B an, wo das Signal des AWG durch ein 30 m langes gerades Kabel hin und zurück läuft, bevor es mit dem ADC mit hoher Samplingrate von 105 MHz ausgewertet wird. Der verbaute FPGA hat gleichzeitig Zugriff auf das gesendete und das empfangene Signal und soll daraus kontinuierlich die Laufzeitdifferenz im Kabel ermitteln. Das Signal der Laufzeitdifferenz soll anschließend im Frequenzbereich von 0-50 MHz weiter ausgewertet werden. Lässt sich aus diesen Informationen eine Systemempfindlichkeit für die Laufzeitdifferenz im Kabel abschätzen?

───────────────────────────────────────────────────────────────

[🤖 KI]
Ich analysiere dein geplantes Eclypse Z7-Konzept und schätze die erreichbare Systemempfindlichkeit für die Laufzeitdifferenz ab.

## Ausgangspunkt: Bekannte Größen

| Parameter | Wert |
|-----------|------|
| Kabellänge (hin & zurück, Typ B) | 2 × 30 m = 60 m |
| Verkürzungsfaktor Cat-6a/7 | ~0,66–0,78 |
| Elektrische Laufzeit τ | ~2 µs (wie im bestehenden System) |
| ADC-Sample-Rate | 105 MS/s (Eclypse Z7) |
| FPGA-Takt | typisch 100–200 MHz (Zynq-7000) |

## Kritische Analyse der Empfindlichkeitskette

### 1. ADC-Auflösung als primäre Grenze

Der Eclypse Z7 nutzt typischerweise einen **12- oder 14-Bit-ADC** (je nach konkretem Modell – bitte prüfen, welches Board genau vorliegt). Bei 105 MS/s und einem Sinus bei z. B. 10 MHz:

- **Quantisierungsrauschen (SNR)**: Für 12 Bit idealerweise ~74 dB, realistisch ~65–70 dB mit Jitter und Nichtlinearitäten
- **Effektive Auflösung**: ~10–11 Bit effektiv

### 2. Phasen-/Laufzeitmessung per digitalem I/Q

Der entscheidende Vorteil deines Konzepts: Der FPGA sieht **beide Signale** (gesendet und empfangen) und kann eine **digitale Korrelation/Phasenmessung** durchführen.

**Mögliche Verfahren:**

| Verfahren | Prinzip | Erwartbare Auflösung |
|-----------|---------|----------------------|
| **Direkte Kreuzkorrelation** | Verschiebung des Empfangssignals gegen das Referenzsignal | ~1 Sample-Periode = ~9,5 ns (zu grob allein) |
| **I/Q-Demodulation + Phasenvergleich** | Beide Signale auf gemeinsame Zwischenfrequenz mischen, Phasenwinkel vergleichen | Sub-Sample durch Interpolation |
| **Digitale PLL / Costas-Loop** | Nachführung der Phase mit numerischem Oszillator | Sehr hohe Auflösung bei langen Mittelungszeiten |
| **Fourier-Phasenmethode** | FFT beider Signale, Phasendifferenz bei Trägerfrequenz | Gut bei hohem SNR, begrenzt durch Fensterung |

### 3. Realistische Abschätzung der Laufzeitauflösung

**Schritt 1: Einzelmessung (ein Sample-Fenster)**

Bei direkter Korrelation mit parabolischer Interpolation des Korrelationspeaks:

$$\delta\tau_{einzel} \approx \frac{T_{sample}}{SNR_{eff}} \approx \frac{9,5 \text{ ns}}{10^{65/20}} \approx \frac{9,5 \text{ ns}}{1778} \approx 5 \text{ ps}$$

Das ist optimistisch; realistisch mit Nichtlinearitäten und Jitter eher **10–50 ps pro Einzelmessung**.

**Schritt 2: Mittelung über N Messungen**

Da der FPGA kontinuierlich messen kann:

$$\delta\tau_{gemittelt} = \frac{\delta\tau_{einzel}}{\sqrt{N}}$$

Bei 1 MHz effektiver Messrate über 1 Sekunde: N = 10⁶ → Verbesserung um Faktor 1000

$$\delta\tau_{1s} \approx \frac{10 \text{ ps}}{1000} = 10 \text{ fs}$$

**Aber Achtung**: Diese Mittelung funktioniert nur, wenn das Rauschen **unkorreliert** ist. Systematische Effekte (Temperaturdrift des Kabels, ADC-Offset, Taktjitter) mitteln sich **nicht** heraus.

### 4. Taktjitter als fundamentale Grenze

Das ist der **kritischste Punkt** bei deinem Konzept:

| Jitter-Quelle | Typischer Wert | Auswirkung |
|-------------|--------------|------------|
| Interner FPGA-Takt (PLL) | 10–100 ps RMS (je nach Qualität) | Direkt auf Phasenmessung |
| ADC-Aperturjitter | 1–10 ps (bei guten ADCs) | Begrenzt Einzelmessung |
| AWG-Ausgangsjitter | 10–100 ps | Referenzsignal verrauscht |

**Wichtig**: Da AWG und ADC vom **gleichen Takt** abgeleitet sind (kein externer Oszillator!), korrelieren sich einige Jitter-Komponenten. Das kann **Vorteil** (Common-Mode-Unterdrückung) oder **Nachteil** (keine unabhängige Referenz) sein.

Für die **Laufzeitdifferenz im Kabel** ist entscheidend:
- Ändert sich die Laufzeit des Kabels, verschiebt sich das Empfangssignal relativ zum gesendeten
- Gemeinsamer Taktjitter verschiebt beide Signale gleich → **fällt heraus**
- **Differentieller Jitter** (AWG-Pfad vs. ADC-Pfad) bleibt als Fehler

Realistischer differentieller Jitter: **~1–10 ps** → begrenzt die Langzeitstabilität

### 5. Kabel-Temperaturdrift als systematische Grenze

Cat-6a/7: Temperaturkoeffizient der Laufzeit ~ **10–50 ppm/K** (je nach Hersteller, Dielektrikum)

Bei 2 µs Laufzeit:
- 1 mK Temperaturänderung → **20–100 fs** Laufzeitänderung
- 1 K → **20–100 ps**

Das ist **deutlich größer** als die statistische Auflösung! Ohne aktive Temperaturstabilisierung oder differentielle Messung (zweite Leitung) dominiert dieser Effekt.

## Geschätzte Systemempfindlichkeit

| Szenario | Bedingung | δτ (Laufzeitdifferenz) | Relativ zu τ = 2 µs |
|----------|-----------|------------------------|---------------------|
| **Statistisch, 1 s Mittelung** | Gutes SNR, weißes Rauschen | ~10–50 fs | 5×10⁻⁹ – 2,5×10⁻⁸ |
| **Statistisch, 1000 s Mittelung** | Weißes Rauschen hält an | ~0,3–1,5 fs | 1,5×10⁻¹⁰ – 7,5×10⁻¹⁰ |
| **Systematisch begrenzt** | Temperaturdrift 1 mK/h, nicht korrigiert | ~20–100 fs/h Drift | — |
| **Mit differentieller Zweitleitung** | Kabeldrift kompensiert | ~1–10 fs (Jitter-limitiert) | 5×10⁻¹⁰ – 5×10⁻⁹ |
| **Vergleich: bestehendes System** | 10–100 ps effektiv | 10–100 ps | 5×10⁻⁶ – 5×10⁻⁵ |

## Vergleich mit dem bestehenden System

| Aspekt | Bestehend (XMC4700, Capture) | Neu (Eclypse Z7, I/Q) |
|--------|---------------------------|----------------------|
| Primäre Messung | Zeitintervall zwischen Flanken | Kontinuierliche Phase/I/Q |
| Auflösung (roh) | ~7 ns (Takt), verfeinert durch Statistik | ~10 ps (einzel), fs (gemittelt) |
| Mittelung | Über ~1000 Zyklen, Pause/Burst | Kontinuierlich im FPGA |
| Flexibilität | Fest verdrahtet | Vollständig software-definiert |
| Externer Takt | Möglich (wahrscheinlich genutzt) | **Nicht möglich** |
| Gleichzeitiger Zugriff auf Tx & Rx | Nein (nur Rx gemessen) | **Ja** |
| Frequenzbereich der Auswertung | Begrenzt | 0–50 MHz (dein Ziel) |

## Kritische Bewertung deines Konzepts

### Stärken
1. **Vollständige Software-Flexibilität**: Signalformen, Filter, Algorithmen änderbar ohne Hardwareeingriff
2. **Kohärente Tx/Rx-Messung**: Direkter Phasenvergleich ohne externe Referenz
3. **Hohe Sample-Rate**: 105 MS/s erlaubt Oversampling und digitale Filterung
4. **FPGA-Echtzeitverarbeitung**: Korrelation, FFT, PLL in Hardware

### Schwächen & Risiken

| Problem | Auswirkung | Mögliche Lösung |
|---------|-----------|-----------------|
| **Kein externer Oszillator** | Langzeitdrift des internen Taktgebers; Frequenzfehler akkumulieren | Interne Kalibrierung gegen GPS-DO (falls verfügbar), oder Akzeptanz für Kurzzeitmessungen |
| **Gemeinsamer Takt für AWG & ADC** | Common-Mode-Unterdrückung gut, aber: systematische Fehler beider Pfade nicht unterscheidbar | Sorgfältiges PCB-Layout, getrennte Stromversorgung, differentielle Messung mit zweitem Kanal |
| **Temperaturdrift des Kabels** | Dominanter systematischer Fehler | Zweite, thermisch gekoppelte Leitung (Methodik 8); oder aktive Temperaturregelung |
| **ADC-Nichtlinearitäten** | Spektrale Artefakte, Phasenfehler | Kalibrierung mit bekanntem Signal, Dithering |
| **Begrenzte ADC-Auflösung (12/14 Bit)** | SNR begrenzt Einzelmessung | Oversampling (105 MS/s >> 10 MHz Träger), digitale Filterung |

## Konkrete Empfehlung für die Empfindlichkeit

Für eine **realistische, konservative Schätzung** deines Systems:

> **Kurzzeit (1 s, statistisch):** δτ ≈ 10–50 fs → **δτ/τ ≈ 5×10⁻⁹ – 2,5×10⁻⁸**

> **Langzeit (>100 s, systematisch begrenzt):** δτ ≈ 1–10 ps (durch Temperaturdrift und Taktinstabilität) → **δτ/τ ≈ 5×10⁻⁷ – 5×10⁻⁶**

> **Mit differentieller Zweitleitung + Temperaturstabilisierung:** δτ ≈ 10–100 fs → **δτ/τ ≈ 5×10⁻⁸ – 5×10⁻⁷**

Das wäre eine **Verbesserung um Faktor 10–1000** gegenüber dem bestehenden System (10–100 ps), aber nur wenn die systematischen Effekte (Temperatur, Takt) beherrscht werden.

## Offene Fragen für die Konzeptphase

1. **Welches genaue Eclypse Z7-Board?** (ADC-Typ, Auflösung, Jitter-Spezifikation)
2. **Ist GPS-disciplinierter Oszillator als Referenz möglich?** (auch wenn nicht als Haupttakt – für Kalibrierung)
3. **Zweite Leitung für differentielle Messung vorgesehen?** (Methodik 8 aus deinem Katalog)
4. **Welche Trägerfrequenz(en)?** (10 MHz? Höher für bessere Phasenauflösung?)
5. **Bandbreite des Kabels bei höheren Frequenzen?** (Cat-7 bis 600 MHz, aber Dämpfung steigt)

Soll ich einen dieser Punkte vertiefen – z. B. die konkrete I/Q-Phasenmessung im FPGA, die differentielle Zweikanal-Topologie, oder den Vergleich mit alternativen Hardware-Plattformen (z. B. Red Pitaya, ADALM-PLUTO mit externem Takt)?

**Citations:**
[1] [https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md](https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md)

───────────────────────────────────────────────────────────────

[👤 Sie]
das eclypse z7 ist ein bundle mit ADC 14bit/105MS/s, GPS als referenz muss noch untersucht werden, sollte über den PMOD möglich sein, trägerfrequenz ist durch quantisierung mit dem 125 MHz takt des AWG begrenzt, eventuell daher rechteck oder pulse generieren, als kabel steht neben Cat 6a auch RG58 und Sat-Kabel zur Verfügung, bandbreite dann 2-3 GHz, wie kann eine differentielle Messung mit zweiter Leitung realisiert werden?

───────────────────────────────────────────────────────────────

[🤖 KI]
Danke für die Präzisierung! Das ändert die Abschätzung deutlich – insbesondere die **14-Bit-Auflösung**, die **GPS-Referenz über PMOD** und die **höheren Kabelbandbreiten** eröffnen neue Möglichkeiten. Lass mich die Empfindlichkeit neu bewerten und ein konkretes Konzept für die **differentielle Zweileitungsmessung** entwickeln.

---

## 1. Aktualisierte Systemparameter

| Parameter | Wert | Anmerkung |
|-----------|------|-----------|
| ADC | 14 Bit, 105 MS/s | ~84 dB SNR (theoretisch), ~72–76 dB praktisch |
| AWG-Takt | 125 MHz | Nyquist: 62,5 MHz; Rechteck/Pulse bis ~20–30 MHz nutzbar |
| FPGA-Takt | 125 MHz (Zynq-7020/7030) | Gleiche Domäne wie AWG → kohärente Erzeugung |
| GPS-Referenz | PMOD (z. B. PMOD GPS oder externer OCXO) | Noch zu untersuchen: 1 PPS, 10 MHz, NMEA |
| Kabeloptionen | Cat 6a (~500 MHz), RG58 (~1 GHz), Sat-Kabel (~2–3 GHz) | Sat-Kabel bevorzugt für hohe Bandbreite |
| Signallaufweg | 60 m (hin und zurück) | ~300–400 ns elektrische Länge (je nach Verkürzungsfaktor) |

---

## 2. Trägerfrequenz und Signalform: Rechteck/Pulse vs. Sinus

### Problem: Sinus bei 125 MHz AWG-Takt
- Reiner Sinus bei >30 MHz wird durch **Quantisierung und endliche Auflösung** des AWG verzerrt
- Effektive Auflösung sinkt durch **Jitter** und **Nichtlinearität**

### Lösung: Rechteck oder Pulse mit Bandbreitenbegrenzung

| Signalform | Vorteile | Nachteile | Empfohlene Nutzung |
|------------|----------|-----------|-------------------|
| **Rechteck (50% Duty)** | Einfache Erzeugung, starke Grundwelle + Harmonische | Hoher Jitter bei steilen Flanken, EMV-Probleme | Nur mit **Bandpassfilter** auf Grundwelle |
| **Gauss-Pulse / raised-cosine** | Kontrolliertes Spektrum, geringe Nebenkeulen | Komplexere Erzeugung im FPGA | **Bevorzugt** für präzise Laufzeitmessung |
| **PN-Sequenz (Barker, M-Sequenz)** | Korrelationsgewinn, Störfestigkeit | Höherer DSP-Aufwand | Für rauscharme Umgebungen |

### Konkrete Empfehlung: **Bandpass-gefiltertes Rechtecksignal**

```
FPGA (125 MHz) → AWG DAC → Tiefpass (anti-aliasing) → Bandpass (Träger) → Kabel
                                    ↓
                              z. B. 10–20 MHz Rechteck
                                    ↓
                              Bandpass 10 MHz (Q ~ 5–10)
                                    ↓
                              ~Sinusförmiges Signal mit scharfer Phaseninformation
```

**Warum das funktioniert:**
- Die **steilen Flanken** des Rechtecks liefern präzise Zeitinformation (hohe dV/dt)
- Der **Bandpassfilter** entfernt Harmonische und begrenzt das Spektrum
- Der **ADC** sieht ein quasi-sinusförmiges Signal → optimale Ausnutzung der 14 Bit

**Alternative: Direkte I/Q-Erzeugung im FPGA**
- Zwei DAC-Kanäle (falls vorhanden) oder **zeitmultiplexed** I/Q auf einem Kanal
- Ermöglicht **komplexe Signalformen** (chirps, modulierte Träger)

---

## 3. Differentielle Messung mit zweiter Leitung

### Physikalisches Prinzip

Die differentielle Messung eliminiert **gemeinsame Störungen** (common-mode), die auf beiden Leitungen gleich wirken:

| Störgröße | Wirkung auf Einzelmessung | Wirkung auf Differenzmessung |
|-----------|---------------------------|-------------------------------|
| Temperaturdrift des Kabels | Systematischer Fehler | Wird kompensiert (beide Kabel gleich) |
| Taktjitter AWG/ADC | Phasenfehler | Wird kompensiert (gemeinsame Referenz) |
| Elektromagnetische Einkopplung | Additives Rauschen | Wird unterdrückt (Gleichtaktunterdrückung) |
| Phasenrauschen GPS-Referenz | Frequenzfehler | Wird kompensiert |
| **Echte Laufzeitänderung** (z. B. Raumzeitdynamik) | Messsignal | **Bleibt erhalten** (nur wenn unterschiedlich) |

### Kritische Annahme

> Die differentielle Messung funktioniert nur, wenn die **physikalische Laufzeitänderung** in **einem Kabel anders** ist als im anderen. Bei einem homogenen Medium (gleiches Kabel, gleiche Temperatur, gleiche mechanische Belastung) ist die Differenz **null** – auch für echte Effekte!

**Konsequenz für dein Experiment:**
- Die beiden Kabel müssen **unterschiedliche physikalische Bedingungen** erfahren
- Oder: Ein Kabel ist **Referenz** (stabilisiert), das andere **Messobjekt** (exponiert)

---

## 4. Hardware-Topologien für differentielle Messung

### Variante A: Zwei separate Kabel (räumlich getrennt)

```
                    ┌─────────────────┐
    AWG ────────────┤  Kabel 1 (Mess) ├────┬────── ADC CH1
    (Signal S(t))   │  30 m hin/zurück│    │
                    └─────────────────┘    │
                                           ├──→ FPGA: Δφ = φ₁ - φ₂
                    ┌─────────────────┐    │
    AWG ────────────┤ Kabel 2 (Ref)   ├────┴────── ADC CH2
    (Signal S(t))   │  30 m hin/zurück│
    (gleiche Quelle)└─────────────────┘
```

**Vorteile:**
- Einfache Trennung von Mess- und Referenzpfad
- Unterschiedliche Kabeltypen möglich (z. B. Cat 6a vs. RG58)

**Nachteile:**
- Zwei Kabel = doppelte mechanische Komplexität
- Unterschiedliche elektrische Eigenschaften (Impedanz, Dämpfung) müssen kalibriert werden

**Empfohlene Kabelkombination:**
| Kabel | Rolle | Begründung |
|-------|-------|------------|
| **Sat-Kabel (75 Ω, 2–3 GHz)** | Messkabel | Höchste Bandbreite, geringe Dämpfung |
| **RG58 (50 Ω, ~1 GHz)** | Referenzkabel | Gut verfügbar, ausreichend für Referenz |
| *oder* zweites Sat-Kabel | Referenz | Identische elektrische Eigenschaften |

---

### Variante B: Ein Kabel mit zwei Paaren (Cat 6a/7)

Cat 6a/7 hat **4 verdrillte Paare**. Nutze zwei Paare:

```
        ┌─────────────────────────────────────┐
        │  Cat 6a/7 Kabel (4 Paare)           │
        │                                     │
  Paar 1├─ AWG → TX+ ──────── RX+ → ADC CH1 │
        │  (Hinleitung)         (Rückleitung)  │
        │                                     │
  Paar 2├─ AWG → TX+ ──────── RX+ → ADC CH2 │
        │  (Hinleitung)         (Rückleitung)  │
        │                                     │
  Paar 3├─ (Reserve / Schirmung / Strom)     │
  Paar 4├─ (Reserve)                         │
        └─────────────────────────────────────┘
```

**Verdrahtung pro Paar (galvanisch getrennt):**
```
AWG-Ausgang ──┬── TX+ ───┬── 30 m ──┬── RX+ ─── ADC+
              │          │          │
             GND      (Schirm)    GND
```

**Wichtig:** Die beiden Paare müssen **galvanisch getrennt** sein (kein gemeinsames GND am Kabelende), sonst koppeln Störungen ein.

**Vorteile:**
- Kompakte Lösung, ein Kabel
- Gleiche mechanische Belastung (Temperatur, Biegung)
- Gleiche elektrische Länge (bei sorgfältiger Konfektionierung)

**Nachteile:**
- Übersprechen zwischen Paaren (NEXT/FEXT) begrenzt die Gleichtaktunterdrückung
- Cat 6a Bandbreite (~500 MHz) niedriger als Sat-Kabel

---

### Variante C: Hybrid – Sat-Kabel mit zwei Leitern (bevorzugt)

Sat-Kabel (RG6-ähnlich) hat **Innenleiter + Schirm**. Für differentielle Messung:

```
        ┌─────────────────────────────────────┐
        │  Zwei separate Sat-Kabel            │
        │  (oder ein Kabel mit zwei Leitern)  │
        │                                     │
  Kabel 1: Innenleiter = Signal, Schirm = GND → ADC CH1 (single-ended)
  Kabel 2: Innenleiter = Signal, Schirm = GND → ADC CH2 (single-ended)
        │                                     │
        │  Oder: differentielle Übertragung    │
        │  (Innenleiter 1 = +, Innenleiter 2 = -)│
        └─────────────────────────────────────┘
```

**Für echte differentielle Übertragung** (höchste Störunterdrückung):

```
    AWG differentiell ──┬── Kabel 1 (+) ───┬── ADC CH1 (+)
    (z. B. mit Balun     │                  │
     oder zwei DACs)     └── Kabel 2 (-) ───┴── ADC CH2 (-)
```

Das Eclypse Z7 hat typischerweise **single-ended ADC-Eingänge** (0–1 V oder ±1 V). Für differentielle Messung brauchst du:
- **Balun/Übertrager** am AWG-Ausgang (z. B. Mini-Circuits TC1-1-13M+)
- **Differenzverstärker** am ADC-Eingang (z. B. AD8138, THS4521)

---

## 5. FPGA-Implementierung der Differenzmessung

### Prinzipschaltbild im FPGA

```
┌─────────────────────────────────────────────────────────┐
│  Zynq-7000 FPGA                                         │
│                                                         │
│  ┌─────────┐    ┌─────────┐    ┌─────────────────────┐ │
│  │  AWG    │    │  AWG    │    │  Signalgenerator    │ │
│  │  CH1    │    │  CH2    │    │  (NCO, 125 MHz)     │ │
│  │ (I)     │    │ (Q)     │    │                     │ │
│  └────┬────┘    └────┬────┘    └─────────────────────┘ │
│       │              │                                  │
│       └──────┬───────┘                                  │
│              │ (gemeinsames Signal, ggf. verzögert)     │
│              ▼                                          │
│  ┌─────────────────────────────────────────────────┐    │
│  │  Mischer / Korrelator                           │    │
│  │  ┌─────────┐      ┌─────────┐                  │    │
│  │  │ Mischer │      │ Mischer │                  │    │
│  │  │ CH1     │      │ CH2     │                  │    │
│  │  │ (I/Q)   │      │ (I/Q)   │                  │    │
│  │  └────┬────┘      └────┬────┘                  │    │
│  │       │                │                        │    │
│  │       ▼                ▼                        │    │
│  │  ┌─────────┐      ┌─────────┐                  │    │
│  │  │ Tiefpass│      │ Tiefpass│                  │    │
│  │  │ (CIC)   │      │ (CIC)   │                  │    │
│  │  └────┬────┘      └────┬────┘                  │    │
│  │       │                │                        │    │
│  │       ▼                ▼                        │    │
│  │  ┌─────────┐      ┌─────────┐                  │    │
│  │  │ Phase   │      │ Phase   │                  │    │
│  │  │ CH1     │      │ CH2     │                  │    │
│  │  │ (atan2) │      │ (atan2) │                  │    │
│  │  └────┬────┘      └────┬────┘                  │    │
│  │       │                │                        │    │
│  │       └───────┬────────┘                        │    │
│  │               ▼                                 │    │
│  │  ┌─────────────────────────┐                    │    │
│  │  │  Differenz: Δφ = φ₁ - φ₂ │                   │    │
│  │  │  → Laufzeitdifferenz Δτ  │                   │    │
│  │  │  → 0–50 MHz Auswertung   │                   │    │
│  │  └─────────────────────────┘                    │    │
│  └─────────────────────────────────────────────────┘    │
│                                                         │
│  ┌─────────────────────────────────────────────────┐    │
│  │  GPS-Referenz (PMOD)                            │    │
│  │  • 1 PPS → Zeitstempel, Synchronisation         │    │
│  │  • 10 MHz → Taktkalibrierung (PLL-Feedback)      │    │
│  └─────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────┘
```

### Mathematische Beschreibung

**Empfangene Signale:**
$$s_1(t) = A_1 \cos(\omega t + \phi_1 + \phi_{n,1}(t))$$
$$s_2(t) = A_2 \cos(\omega t + \phi_2 + \phi_{n,2}(t))$$

Wobei:
- $\phi_1, \phi_2$: Nutzphasen (enthalten Laufzeitinformation)
- $\phi_{n,1}, \phi_{n,2}$: Rauschphasen (Temperatur, Jitter, EMV)

**I/Q-Demodulation:**
$$I_k = \text{LPF}\{s_k(t) \cdot \cos(\omega_{LO} t)\}$$
$$Q_k = \text{LPF}\{s_k(t) \cdot \sin(\omega_{LO} t)\}$$

**Phasenberechnung:**
$$\phi_k = \text{atan2}(Q_k, I_k)$$

**Differenz:**
$$\Delta\phi = \phi_1 - \phi_2 = (\phi_{1,nutz} - \phi_{2,nutz}) + (\phi_{n,1} - \phi_{n,2})$$

**Laufzeitdifferenz:**
$$\Delta\tau = \frac{\Delta\phi}{2\pi f_{träger}}$$

---

## 6. Aktualisierte Empfindlichkeitsschätzung

### Mit 14-Bit ADC und differentieller Messung

| Parameter | Einzelkanal | Differentiell (2 Kabel) |
|-----------|-------------|------------------------|
| ADC-SNR (praktisch) | ~72 dB | ~72 dB (pro Kanal) |
| Phasenrauschen (1 Hz BW, 10 MHz Träger) | ~1–3 mrad | ~1–3 mrad (unkorreliert) |
| **Phasenauflösung nach Mittelung** | | |
| • 1 ms Mittelung | ~30–100 µrad | ~20–70 µrad (Wurzel-2 besser) |
| • 1 s Mittelung | ~1–3 µrad | ~0,7–2 µrad |
| • 1000 s Mittelung | ~30–100 nrad | ~20–70 nrad |
| **Laufzeitauflösung** | | |
| • 1 s Mittelung | ~0,5–1,5 ps | ~0,3–1 ps |
| • 1000 s Mittelung | ~5–15 fs | ~3–10 fs |

### Systematische Fehler (nach Kompensation)

| Fehlerquelle | Größe | Kompensation |
|--------------|-------|--------------|
| Kabellängendifferenz (mechanisch) | ~1–10 mm → 5–50 ps | Kalibrierung mit bekanntem Delay |
| Impedanzfehlanpassung | Reflexionen, stehende Wellen | Abschlusswiderstände, TDR-Kalibrierung |
| Temperaturdrift (Restfehler) | ~0,1–1 ppm/K × ΔT | Differentielle Messung, aktive Regelung |
| Taktjitter (nach GPS-Sync) | ~1–10 ps (kurzzeitig) | GPS-Referenz, Mittelung |
| ADC-Offset/Drift | ~1–10 LSB | Chopping, Kalibrierung |

**Realistische Gesamtempfindlichkeit (differentiell, GPS-stabilisiert, 1000 s Mittelung):**
$$\boxed{\delta(\Delta\tau) \approx 10\text{–}100 \text{ fs} \hat{=} 10^{-14}\text{–}10^{-13} \text{ s}}$$

**Vergleich:**
- Bisheriges System (aus "Physik 3-2"): ~1–10 ps
- **Verbesserungsfaktor: 10–1000×**

---

## 7. GPS-Referenz über PMOD: Konzept

### Mögliche PMOD-Module

| Modul | Hersteller | Funktionen | Eignung |
|-------|-----------|------------|---------|
| **PMOD GPS** | Digilent | NMEA, 1 PPS, UART | Gut für Zeitstempel |
| **PMOD OCXO** | Digilent | 100 MHz OCXO, 1 PPS In/Out | Sehr gut für Taktstabilität |
| **PMOD RTCC** | Digilent | Echtzeituhr, Batterie | Nicht direkt relevant |
| **Custom PMOD** | — | Externer OCXO + GPS-Disziplinierung | Optimal, aber Aufwand |

### Empfohlene Konfiguration: GPS-disziplinierter OCXO

```
┌─────────────┐     ┌─────────────┐     ┌─────────────────┐
│  GPS-Modul  │────→│  FPGA       │────→│  OCXO (10 MHz)  │
│  (PMOD)     │ 1PPS│  (PLL/Software)   │     │  (extern, PMOD) │
│  • NMEA     │     │  • Phasenvergleich│     │  • 10 MHz out   │
│  • 1 PPS    │     │  • Frequenzregelung│    │  • 1 PPS in/out │
└─────────────┘     └─────────────┘     └────────┬────────┘
                                                  │
                                                  ▼
                                           ┌─────────────┐
                                           │  Eclypse Z7 │
                                           │  (externer  │
                                           │   Takteingang)│
                                           │  → AWG/ADC  │
                                           └─────────────┘
```

**Wichtig:** Das Eclypse Z7 hat **keinen externen Takteingang** für den Haupttakt! (Wie du erwähnt hast.)

**Lösungen:**
1. **Software-PLL im FPGA:** GPS-1-PPS synchronisiert die **Messzeitbasis**, nicht den Takt selbst
2. **Taktkalibrierung:** Messung der tatsächlichen Taktfrequenz gegen GPS, Korrektur in Software
3. **Externe Taktquelle für AWG/ADC:** Falls PMOD-Pins für Takt-Routing verfügbar (prüfen!)

### Software-PLL (empfohlen)

```python
# Pseudocode für FPGA/Software
class GPSDisciplinedPLL:
    def __init__(self):
        self.nominal_freq = 125e6  # Hz
        self.actual_freq = 125e6   # gemessen
        self.phase_error = 0

    def update(self, gps_1pps_timestamp, local_1pps_timestamp):
        # Phasenfehler zwischen GPS-1PPS und lokalem 1PPS
        self.phase_error = gps_1pps_timestamp - local_1pps_timestamp

        # Frequenzkorrektur (PI-Regler)
        self.actual_freq += Kp * self.phase_error + Ki * integral(self.phase_error)

        # Anwendung: Korrektur der Zeitbasis für Messungen
        return self.actual_freq
```

---

## 8. Kabelauswahl: Empfehlung

| Kabel | Impedanz | Bandbreite | Dämpfung (100 MHz) | Verkürzungsfaktor | Empfehlung |
|-------|----------|------------|---------------------|-------------------|------------|
| **Cat 6a** | 100 Ω (differentiell) | ~500 MHz | ~20 dB/100m | ~0,78 | Gut für Referenz, begrenzt für Messung |
| **RG58** | 50 Ω | ~1 GHz | ~15 dB/100m | ~0,66 | Ausreichend, günstig |
| **RG6 (Sat)** | 75 Ω | ~2–3 GHz | ~5 dB/100m | ~0,82 | **Bevorzugt für Messung** |
| **RG213** | 50 Ω | ~1 GHz | ~8 dB/100m | ~0,66 | Robust, schwer |

**Empfohlene Kombination für differentielle Messung:**
- **Messkabel:** RG6 Sat-Kabel (75 Ω, höchste Bandbreite, geringste Dämpfung)
- **Referenzkabel:** Zweites RG6 Sat-Kabel (identische Eigenschaften) **oder** Cat 6a (wenn mechanische Stabilität wichtiger als elektrische Identität)

**Impedanzanpassung:**
- AWG-Ausgang: 50 Ω oder 75 Ω (je nach Kabel)
- ADC-Eingang: 50 Ω oder hochohmig (je nach Board)
- **Terminierung:** Abschlusswiderstände am Kabelende kritisch!

---

## 9. Nächste Schritte und offene Fragen

| # | Aufgabe | Priorität | Verantwortlich |
|---|---------|-----------|----------------|
| 1 | **PMOD-Pinout prüfen:** Welche Pins für GPS, externer Takt, Trigger verfügbar? | Hoch | Hardware |
| 2 | **ADC-Eingangsbeschaltung:** Single-ended oder differentiell? Eingangsimpedanz? | Hoch | Hardware |
| 3 | **AWG-Ausgang:** Single-ended oder differentiell? Maximale Amplitude? | Hoch | Hardware |
| 4 | **Kabelkonfektionierung:** Steckverbinder, Längentoleranz, Schirmung | Mittel | Mechanik |
| 5 | **FPGA-IP-Core:** NCO, Mischer, CIC-Filter, atan2 – vorhanden oder selbst entwickeln? | Hoch | FPGA |
| 6 | **GPS-Modul testen:** 1 PPS Genauigkeit, Jitter, Verfügbarkeit | Mittel | System |
| 7 | **Kalibrierverfahren:** Bekanntes Delay (z. B. 1 m Kabel = ~5 ns) einmessen | Mittel | System |

---

## 10. Zusammenfassung

| Aspekt | Empfehlung |
|--------|------------|
| **Signalform** | Bandpass-gefiltertes Rechtecksignal (~10–20 MHz) oder Gauss-Pulse |
| **Differentielle Messung** | Zwei separate Sat-Kabel (RG6) oder zwei Paare in Cat 6a |
| **GPS-Referenz** | PMOD GPS + Software-PLL (da kein externer Takteingang) |
| **FPGA-Verarbeitung** | I/Q-Demodulation, atan2-Phasenvergleich, Differenzbildung |
| **Erwartete Empfindlichkeit** | **10–100 fs** (differentiell, GPS-stabilisiert, 1000 s Mittelung) |
| **Verbesserung zu bisher** | Faktor **10–1000×** |

---

Soll ich als Nächstes:
- **A)** Einen **konkreten FPGA-Blockdiagramm** (Verilog/VHDL-Struktur) für die I/Q-Demodulation und Differenzbildung skizzieren?
- **B)** Die **Kalibrierprozedur** detaillieren (wie misst man 10 fs Laufzeitdifferenz mit bekanntem Referenzdelay)?
- **C)** Die **GPS-Disziplinierung** vertiefen (Hard- vs. Software-PLL, Allan-Varianz-Analyse)?
- **D)** Ein **Markdown-Dokument** für das GitHub-Repo erstellen, das dieses Konzept strukturiert darstellt?

═══════════════════════════════════════════════════════════════
  Ende der Konversation
═══════════════════════════════════════════════════════════════
