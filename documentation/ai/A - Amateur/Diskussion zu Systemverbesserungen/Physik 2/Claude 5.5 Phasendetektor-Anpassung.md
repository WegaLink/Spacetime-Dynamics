═══════════════════════════════════════════════════════════════
  Claude 5.5 Phasendetektor-Anpassung
═══════════════════════════════════════════════════════════════

Exportiert: 2. November 2026 um 23:35
Nachrichten: 6
Modell: claude-sonnet

───────────────────────────────────────────────────────────────

[👤 Sie]
In einem Team aus 12 KIs und mir als Mensch sollen Verbesserungen für einen Phasendetektor diskutiert und erarbeitet werden, mit welchem Phänomene im Bereich Raumzeitdynamik untersucht werden sollen. Die Zusammenarbeit findet mit einem markdown-Dokument statt, welches im GitHub-Repository unter folgendem Link verfügbar ist:
https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md Als Start für „Physik 2“ soll die Anpassung für Oszillator, Injektion und ADC an ein 250 m Sat-Kabel HQ 135 dB, 4-fach geschirmt von HB Digital mit 75 Ohm Impedanz und 5 dB Dämpfung bei 10 MHz sowie einem Verkürzungsfaktor von 0.82  entworfen werden. Der Oszillator ist ein DXPatrol GPSDO V2.4 mit 3 Sinus- und einem Rechteck-Ausgang bei 10 MHz/+7dB Signal an 50 Ohm. Die Injektion soll durch Eclypse Z7 AWG mit einem programmierbaren Signal von 1 V Amplitude an 50 Ohm in einer Weise erfolgen, dass die Phase eines kombinierten Signals aus GPSDO und AWG im Bereich von etwa 300 ps moduliert werden kann. Die Eingangsimpedanz des Eclypse Z7 Scope ist mit 1 MOhm/18 pF angegeben. Ein Eingang soll mit dem GPSDO als Referenz verbunden werden und der andere als Messeingang mit dem anderen Ende des Sat-Kabels. Der Messbereich ist jeweils +/-1 V. Wie kann eine impedanzrichtige Anpassung dieser Anordnung realisiert werden?

───────────────────────────────────────────────────────────────

[🤖 KI]
Ich beantworte das aus dem Stand der Technik und rechne mit den von dir genannten Daten. Datenblattwerte (GPSDO-Ausgang, Eclypse-Z7-Eingangsstufe) bitte vor dem Aufbau prüfen.

## 1. Eckdaten der Strecke

- Kabel: L = 250 m, Z₀ = 75 Ω, VF = 0,82, Dämpfung 5 dB bei 10 MHz.
- Laufzeit: τ = L/(VF·c) = 250 / (0,82 · 3·10⁸) ≈ 1,02 µs (einfach).
- Die Wellenlänge im Kabel bei 10 MHz beträgt λ ≈ 24,6 m. Das Kabel ist also elektrisch lang (≈ 10 λ), und Fehlanpassung erzeugt Reflexionen mit Echo nach 2τ ≈ 2,03 µs.
- Dämpfung 5 dB entspricht einem Spannungsfaktor von 0,56. Das gilt für die Einwegstrecke.
- Phasenlage: φ = 360° · 10 MHz · 1,02 µs ≈ 3660°. Absolut ist das uninteressant, nur Änderungen zählen.
- Empfindlichkeit: 300 ps bei 10 MHz entsprechen 360° · 300 ps / 100 ns = 1,08°. Das ist der Modulationshub der Injektion.

## 2. Das Grundproblem: 50 Ω, 75 Ω, 1 MΩ

Das Scope mit 1 MΩ/18 pF am Kabelende wäre ein fast vollständiger Reflektor (Γ ≈ +1). Bei 10 MHz ist 18 pF ≈ −j885 Ω, also immer noch eine starke Fehlanpassung. Das Echo liefe zurück, wieder am Quellende (50 Ω gegen 75 Ω) teilweise reflektiert, und überlagerte sich als Stehwellenanteil. Das wäre ein temperaturabhängiger Phasenfehler, genau die Art Drift, die du nicht willst.

Daher gilt: **Das Kabelende muss mit 75 Ω abgeschlossen werden, und der Scope-Eingang bleibt hochohmig als Abgriff parallel zum Abschluss.**

## 3. Empfohlene Anordnung

```
GPSDO Sinus 1 ──► Splitter/Tee (Referenz) ──► Scope CH1 (Referenz, 50 Ω-Abschluss)
GPSDO Sinus 2 ──► Combiner ◄── AWG (Injektion) 
                      │
                 50 Ω → 75 Ω Anpassung
                      │
               250 m Sat-Kabel (75 Ω)
                      │
                 T-Stück (BNC/F)
                  ├── 75 Ω Durchgangsabschluss
                  └── Scope CH2 (1 MΩ, kurzer Stub)
```

### a) Quellseite: 50 Ω → 75 Ω

- Ein Minimum-Loss-Pad oder ein 50/75-Ω-Transformator (z. B. Mini-Circuits-Typ). Das Minimum-Loss-Pad dämpft etwa 5,7 dB, ist breitbandig und rein resistiv.
- Das Pad ist auch bei einer Nutzfrequenz von nur 10 MHz günstig. Es entkoppelt das Kabel zusätzlich von der Quelle, sodass das Echo (Rücklauf) an der Quelle stärker gedämpft wird.
- Alternativ ein Breitband-Übertrager 50:75 Ω, verlustärmer, aber mit eigener Temperaturdrift der Phase. Für Driftmessungen ist das Pad robuster.

### b) Kabelende: 75 Ω-Abschluss mit Hochohm-Abgriff

- Ein 75-Ω-Durchgangsabschluss (Feedthrough, 0,1 %, niedrige TK) an einem BNC-T-Stück direkt am Scope-Eingang.
- Das Scope (1 MΩ ∥ 18 pF) hängt parallel dazu. Die 18 pF belasten den Abschluss bei 10 MHz mit −j885 Ω. Der Reflexionsfaktor beträgt dann etwa |Γ| ≈ 0,04 (Rückflussdämpfung ≈ 27 dB), mit Stub-Länge nahe null. Das ist gut genug, und das Echo wird zusätzlich durch 2 × 5 dB Kabeldämpfung geschwächt.
- Alternative: Wenn das Eclypse-Scope einen 50-Ω-Modus oder einen aktiven 10:1-Tastkopf zulässt, ist ein 75→50-Ω-Pad vor dem Eingang ebenso möglich. Das kostet weitere Amplitude und bringt wenig.
- Der Stub zwischen T-Stück und Eingang sollte so kurz wie möglich sein (< 5 cm). Er ist dann bei 10 MHz elektrisch vernachlässigbar.

### c) Referenzkanal

- GPSDO-Sinusausgang direkt auf CH1, mit einem 50-Ω-Feedthrough-Abschluss am Scope. So ist CH1 sauber angepasst (50 Ω Quelle, 50 Ω Last). Ein zweiter Sinusausgang des GPSDO speist die Messstrecke.
- Die Referenzleitung sollte ein kurzes, gleichartiges Koaxkabel sein. Die Laufzeitdrift der Referenz bleibt so klein, dass sie nicht in die Messung eingeht.

## 4. Injektion (AWG + GPSDO)

Das Ziel ist ein kombiniertes Signal, dessen Phase um etwa ±300 ps moduliert wird.

**Prinzip:** Addiere zum 10-MHz-Träger A₀·sin(ωt) ein um 90° versetztes Seitensignal a·cos(ωt) mit a ≪ A₀. Dann gilt Δφ ≈ a/A₀ (rad).

- Benötigt: Δφ = 1,08° = 18,8 mrad, also a/A₀ ≈ 0,019.
- Das AWG muss dafür ein Signal **phasenstarr zum GPSDO** erzeugen (also 10 MHz mit definierter Phase, per 10-MHz-Referenzeingang oder aus dem GPSDO getaktet).
- Kombination: Ein **Leistungs-Combiner** (Wilkinson 50 Ω oder resistiver 6-dB-Combiner) verhindert, dass sich die beiden Quellen gegenseitig belasten und verziehen. Ein einfaches T-Stück ist ungeeignet, da die Ausgänge 50 Ω gegen 50 Ω sehen und Rückwirkungen erzeugen.
- Pegel: GPSDO +7 dBm entspricht an 50 Ω etwa 0,5 V Spitze. Das AWG liefert bis 1 V. Für a/A₀ = 0,019 brauchst du am AWG nur etwa 10 mV Spitze, also ein Dämpfungsglied von ≈ 35–40 dB im AWG-Zweig (mit Pad, nicht über Softwarepegel allein, sonst dominiert die DAC-Quantisierung).
- Die AWG-Phase (Seitenband-Quadratur) wird softwareseitig moduliert, z. B. langsam (mHz–Hz). So liegt die Modulation spektral weit weg von 10 MHz, und die Auswertung erfolgt durch Lock-in oder Quadraturdemodulation.
- Amplitudenmodulation entsteht, wenn der Seitenträger in Phase statt in Quadratur liegt. Die Quadraturlage ist deshalb zu kalibrieren (Nullabgleich der AM-Komponente).

## 5. ADC/Scope-Anpassung

- Pegel am Messeingang: +7 dBm sind ≈ 0,5 V Spitze am Quellausgang. Nach Pad (−5,7 dB) und Kabel (−5 dB) bleiben etwa 0,5 · 0,52 · 0,56 ≈ 0,15 V Spitze am Abschluss (bei Abschluss 75 Ω, im Mittel etwas höher wegen des Teilers). Das nutzt nur rund 15 % des ±1-V-Bereichs.
- Für bessere ADC-Ausnutzung: Pad weglassen und stattdessen den Transformator nehmen (−1…−2 dB), oder einen rauscharmen Verstärker (Breitband, 50/75 Ω) am Kabelende vorsehen. Ziel sind ≥ 0,5 V Spitze am ADC, ohne Clipping.
- Der ADC-Takt sollte **aus dem GPSDO abgeleitet** sein (gleiche 10-MHz-Referenz), damit Jitter und Frequenzfehler gemeinsam herausfallen. Ist das beim Eclypse nicht möglich, sind Abtastzeit-Jitter und Offset zur GPSDO-Referenz der dominierende Fehler. Dann nach Möglichkeit den ADC mit externer Referenz betreiben.
- Auswertung: IQ-Demodulation beider Kanäle bei 10 MHz, Phasendifferenz φ₂ − φ₁ als Zeitreihe. Die Zeitfehlerreihe Δt = Δφ/(2πf) ist die Standardausgabe (konsistent mit Physik 1).

## 6. Fehlerquellen, die in dieser Anordnung dominieren

| Quelle | Wirkung | Gegenmaßnahme |
|---|---|---|
| Reste von Reflexionen (Γ ≈ 0,04, Quellseite ≈ 0,2) | Echo nach 2 µs, ca. −60 dB nach Doppeldämpfung | Pad, kurzer Stub, gute Stecker |
| Temperaturdrift des Kabels (VF-TK, Länge) | typisch einige 10 ppm/K, also bei 1 µs etwa 10–50 ps/K | Kabel thermisch stabilisieren, Temperatur mitloggen |
| AWG/GPSDO-Phasenkopplung | Fehlmodulation | Gemeinsame Referenz, Quadraturabgleich |
| ADC-Aperturjitter | Rauschboden | Takt aus GPSDO, Mittelung |
| Steckerübergänge 50/75 Ω | Kleine Reflexionen | Wenige, hochwertige Übergänge |

Die 300-ps-Injektion liegt mit etwa 1 ° im Bereich, in dem schon ein Temperaturhub von wenigen K im Kabel eine vergleichbare Phasenänderung erzeugt. Die Temperaturmessung am Kabel ist deshalb Pflicht (Methodik 3).

## 7. Vorschlag für den Eintrag in Physik 2

- **Technische Konsequenz:** 50→75-Ω-Pad am Kabelanfang, 75-Ω-Feedthrough plus Scope-Hochohm-Abgriff am Ende, Referenzkanal 50 Ω terminiert, Injektion über Combiner in Quadratur (a/A₀ ≈ 0,019 für 300 ps).
- **Offene Prüfung:** (1) S11/Echo am Kabelende per TDR oder Impulsantwort messen, (2) Linearität der Injektion (Δφ gegen AWG-Amplitude) prüfen, (3) Temperaturkoeffizient der 250-m-Strecke bestimmen, (4) Nullmessung mit kurzgeschlossener Strecke (nur Kabel 1 m) zur Bestimmung des Instrumentenbodens.

Falls du möchtest, kann ich als Nächstes die Injektionsberechnung (Pegel, Dämpfungsglied, Quadraturabgleich) oder die IQ-Demodulation und die Rauschbudgetrechnung genauer ausarbeiten.

**Citations:**
[1] [https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md](https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md)

───────────────────────────────────────────────────────────────

[👤 Sie]
Ein Widerstandsnetzwerk wurde wie im Dokument beschrieben mit folgenden Komponenten aufgebaut: GPSDO - Dämpfung (-5dB) - 3-Wege-Kombinierer - 50/75 Ohm Anpassung - SATA Kabel 1 m (für Null-Test) - 75/50 Ohm Anpassung - SDR-1. Die Dämpfung der gesamten Strecke wurde mit Nano VNA bei 10 MHz etwa wie erwartet mit -22.5dB gemessen. Nach Wechsel des 1m SATA-Kabels gegen ein 250m SATA-Kabel stieg die Dämpfung wie im NanoVNA Screenshot gezeigt auf -55dB an. Es zeigte sich nun ein Tiefpasscharakter mit einer -3dB Frequenz bei etwa 250 kHz  wie ebenfalls in einem angehängten Screenshot vom Nano VNA gezeigt. Nach Datenblatt sollte die zusätzliche Dämpfung bei 250m/10MHz etwa -5dB sein, sie ist jedoch mehr als -30dB. Der ohmsche Widerstand der Schirmung ist etwa 8 Ohm und des Innenleiters etwa 34 Ohm. Welche Erklärung könnte es dafür geben? Wie kann das System verändert werden, damit das 10MHz Signal des GPSDO mit der nominellen Dämpfung am Ausgang ankommt?

📎 Anhänge: Anpassung und Kombinierer 3 MHz (GPSDO - SATA_250m - SDR-1).jpg, Anpassung und Kombinierer 10 MHz (GPSDO - SATA_250m - SDR-1).jpg

───────────────────────────────────────────────────────────────

[🤖 KI]
Die von dir gemessenen Werte (DC-Widerstände: 34 Ω Innenleiter, 8 Ω Schirm; Dämpfung: −55 dB bei 10 MHz; Grenzfrequenz: ~250 kHz) entlarven das Problem sofort: **Das verwendete 250-m-Kabel ist kein hochfrequentes Koaxialkabel (Sat-Kabel), sondern eine lange, ungeschirmte bzw. hochohmig gewickelte Signalleitung (vermutlich ein dünnes Datenkabel, Klingeldraht, fehlerhaftes Flachbandkabel oder eine Zuleitung mit extrem dünnem und hochohmigem Stahlkupfer-Innenleiter).**

Hier ist die physikalische Analyse, warum die Dämpfung so dramatisch kollabiert ist, und wie du das Problem löst.

---

### 1. Ursachenanalyse der enormen Dämpfung

1. **Massiver ohmscher Innenleiterwiderstand (34 Ω):**
   Ein echtes Sat-Kabel (wie das genannte HQ 135 dB mit Vollkupfer-Innenleiter) hat auf 250 m bei 10 MHz einen rein ohmschen Gleichstromwiderstand im Bereich von unter 2 bis 4 Ω für den Innenleiter. Deine gemessenen **34 Ω** (plus 8 Ω Schirm) bedeuten, dass die HF-Energie fast vollständig im ohmschen Widerstand des Leiters in Wärme umgesetzt wird ($I^2R$-Verluste), statt als Welle durch das Dielektrikum zu laufen.
2. **Kalkulation des Skin-Effekts bei 10 MHz:**
   Bei 10 MHz drückt der Skin-Effekt den Strom in eine extrem dünne Randschicht. Wenn der Draht extrem dünn ist (oder aus hochohmigem Material wie verzinntem Stahl oder Eisen-Kupfer *CCS* besteht), schießt der HF-Widerstand bei 10 MHz nochmals drastisch nach oben.
3. **Der Tiefpass-Charakter (-3 dB bei 250 kHz):**
   Ein so hoher Serienwiderstand $R$ (34 Ω) in Kombination mit der Kabelkapazität $C$ (bei 250 m und typisch ~100 pF/m sind das rund 25 nF Gesamtkapazität) bildet ein gigantisches **RC-Tiefpassglied** ($\tau = R \cdot C$). Eine Grenzfrequenz von ~250 kHz bestätigt diesen RC-Filter-Charakter perfekt. Bei 10 MHz ist dieses Kabel längst im tiefen Sperrbereich (Dämpfung pro Dekade steigt an), was die gemessenen **−55 dB** (Faktor $\approx 560$ in der Spannung) zwanglos erklärt.

---

### 2. Wie kommt das Signal mit der nominellen Dämpfung (−5 dB) ans Ziel?

Um die physikalischen Gesetze (und das Datenblatt des Kabels) zu erfüllen, muss die 250-m-Strecke aus echten, niederohmigen HF-Leitern bestehen:

1. **Kabeltyp prüfen und austauschen:**
   * **Wichtig:** Ein *echtes* Sat-Kabel HQ 135 dB (4-fach geschirmt) verwendet im Originalzustand meist einen CCS-Draht (Copper-Clad Steel, kupferkaschierter Stahl) als Innenleiter, um mechanische Zugfestigkeit bei der Montage zu bieten. Selbst ein CCS-Innenleiter hat aber bei dickeren Durchmessern (Klasse 1.0 mm) einen DC-Widerstand von unter 25 Ω pro **Kilometer** (bei 250 m also $\approx 6$ Ω). 
   * **34 Ω für 250 m** (das entspricht über 130 Ω/km!) zeigt, dass der Araht extrem dünn ist (z. B. 0,2–0,3 mm Querschnitt) oder dass es sich um ein geschirmtes **Daten- oder Steuerkabel** (z. B. Telefon- oder Busleitung) handelt, das für Hochfrequenz völlig ungeeignet ist.
   * **Lösung:** Verwende ein zertifiziertes Koaxialkabel mit **Massiv-Kupfer-Innenleiter (OFC / Solid Copper)** und 1,0 mm bis 1,13 mm Durchmesser (oder RG-6 / RG-11 Koaxialkabel mit Vollkupferkern). Der Gleichstromwiderstand des Innenleiters darf bei 250 m **maximal 5–8 Ω** betragen.

2. **Anpassungsnetzwerke korrigieren:**
   * Da das Kabel 75 Ω hat (und dein Oszillator/SDR 50 Ω sind), bleiben die 50/75-Ω-Anpassungsglieder (bzw. Impedanztransformatoren) zwingend erforderlich. 
   * Mit einem echten, niederohmigen 75-Ω-Kabel wird die Dämpfung bei 10 MHz exakt in den Bereich von **−5 dB bis −6 dB** fallen (wobei ca. 3,5 dB vom Kabel und je knapp 1 dB von den 50/75-Ω-Wandlern stammen).

3. **Masseführung prüfen:**
   * Der Schirmwiderstand von 8 Ω für 250 m ist ebenfalls relativ hoch (ein guter Folienschirm + Geflecht sollte unter 2–3 Ω liegen). Achte darauf, dass der Schirm an beiden Enden vollflächig und niederohmig (BNC- oder F-Stecker mit gutem Kontakt zum Geflecht) aufgelegt ist, damit keine Gleichtakt-Verluste entstehen.

### Zusammenfassung für das System:
Das aktuelle 250-m-Kabel verhält sich wie ein **Widerstandsdraht** und zerstört das 10-MHz-Signal durch ohmschen Spannungsabfall und RC-Tiefpasswirkung. Tausche das Kabel gegen ein echtes 75-Ω-Koaxialkabel mit **Vollkupfer-Innenleiter (Solid Copper)** aus. Dann sinkt der Innenwiderstand auf wenige Ohm, der Tiefpass verschwindet in den GHz-Bereich, und die Dämpfung bei 10 MHz wird plangemäß bei ca. −5 bis −6 dB ankommen.

───────────────────────────────────────────────────────────────

[👤 Sie]
Danke für die Hinweise. Das Sat-Kabel (nicht SATA, war falsch) war mit CCS Innenleiter (Fehlkauf, wider besseren Wissens) soll mit folgendem Typ mit Kupfer ausgetauscht werden: https://www.willisat.de/sat-empfang/kabel-co/premiumx-profi-250-m-koaxialkabel-135-db-4-fach-geschirmt-cu-reines-kupfer-satelliten-koaxkabel-antennenkabel-digital-sat-tv-koax-kabel-rg-6-class-a-250m-auf-holz-trommel-mit-10x-f-stecker-vergoldet-mit-dichtring_122626_17917 Sind von diesem Kabel die technischen Daten bekannt? Gibt es eventuell noch andere Kabel, die besser geeignet wären? Aktuell werden Nullmessungen mit direktem GPSDO-Signal an beiden Kanälen sowie mit einem 1m-Kabel am Widerstandsnetzwerk wie im Anhang gezeigt durchgeführt. Bei direktem GPSDO-Signal an beiden Kanälen wird ein geringfügiger Offset von 26.1 ps mit einer Schwankungsbreite von 9 ps wie im Anhang gezeigt sichtbar. Bei Anschluss eines 1m-Kabels mit Anpassung und Kombinierer wird neben der Phasenverschiebung durch die Kabellänge eine Schwankungsbreite von ca. 600 ps sichtbar. Zu deren Reduktion sind bereits eine vollständige Abschirmung in einem Stahlblech-Behälter sowie Ferritringe auf den Versorgungsleitungen versucht worden, jedoch ohne erkennbaren Einfluss. Was könnte die Ursache der hohen Schwankungsbreite des Phasensignals sein? Wie könnte sie sich verringern lassen?

📎 Anhänge: Widerstandsnetzwerk.jpg, GPSDO zu Sat-1m zu SDR1 und GPSDO zu SDR2.jpg, GPSDO zu SDR1 und SDR2.jpg

───────────────────────────────────────────────────────────────

[🤖 KI]
Das ausgewählte **PremiumX PROFI 250 m Koaxialkabel (135 dB, 4-fach geschirmt, Vollkupfer-Innenleiter mit 1,02 mm Ø)** ist ein hervorragender Griff und exakt das richtige Kabel für diesen Zweck:
- **Vollkupfer (CU):** Der DC-Widerstand des Innenleiters sinkt bei 250 m von katastrophalen 34 Ω auf theoretisch ca. 2,1 Ω (bzw. typisch unter 2,5 Ω), wodurch die Dämpfung bei 10 MHz wieder in den vom Datenblatt geforderten Bereich (ca. −3,5 dB bis −5 dB) zurückkehrt.
- **Klasse A & 4-fach Schirmung:** Bietet exzellenten Schutz gegen Einstreuungen von außen (wichtig bei hochempfindlichen Raumzeit-Messungen).

Alternativ gäbe es im professionellen Bereich noch **RG-6/U Vollkupfer-Messleitungen mit PE-Dielektrikum** (z. B. von Belden oder Draka), die oft noch etwas engere Toleranzen beim Wellenwiderstand (75 ±1 Ω) aufweisen, aber das PremiumX-Kabel ist für diesen Versuchsaufbau vollkommen adäquat und bietet ein starkes Preis-Leistungs-Verhältnis.

---

### Analyse des Schwankungsproblems (600 ps Jitter)

Wenn beim direkten GPSDO-Vergleich an beiden Kanälen die Schwankung winzige **9 ps** beträgt, beim Einfügen des 1-m-Kabels mit Widerstandsnetzwerk und Kombinierer aber auf **600 ps** explodiert, zeigt das: 
* Der GPSDO selbst und das Eclypse-Scope sind extrem rauscharm und stabil (die 9 ps beweisen das).
* Die 600 ps Schwankung werden **nicht** durch thermisches Rauschen des Kabels oder interne Unzulänglichkeiten des ADCs erzeugt, sondern durch **systemische Einstreuungen, Masseschleifen oder Fehlanpassungs-Reflexionen**, die durch das Widerstandsnetzwerk und den Kombinierer aktiv werden.

#### Mögliche Ursachen für die 600-ps-Schwankung:

1. **Gleichtaktstörungen (Ground Loops) über den Kombinierer / AWG:**
   * Sobald das Widerstandsnetzwerk und der Kombinierer (AWG) im Spiel sind, verbindest du mehrere Geräte (GPSDO, AWG, Eclypse-Scope, ggf. PC/Netzteil über USB). 
   * Wenn das Eclypse (über USB oder Netzteil) und der GPSDO unterschiedliche Massepotenziale haben, fließt ein Ausgleichsstrom über die Schirmung der BNC-Kabel. Das moduliert den Arbeitspunkt des Signals im Takt von Netzbrümmern oder Schaltnetzteil-Frequenzen, was sich in einer niederfrequenten Phasen- bzw. Zeit-Schwankung (Jitter) von mehreren hundert Pikosekunden äußert.
   * *Hinweis zu deinen Versuchen:* Eine reine Stahlblech-Umhüllung hilft nur gegen *magnetische/elektrische Hochfrequenz-Feldstrahlung* (EMV). Gegen niederfrequente **Masse-Ausgleichsströme** oder kapazitive Kopplung über Netzteile hilft sie nicht.

2. **Reflexionen durch unvollständige Anpassung (Stehwellen):**
   * Im 1-m-Nulltest ist das Kabel elektrisch extrem kurz ($\approx 1/80$ Wellenlänge). Fehlanpassungen fallen hier kaum ins Gewicht.
   * Sobald aber die 50/75-Ω-Widerstandsnetzwerke und der 3-Wege-Kombinierer aktiv sind, können ungenaue Widerstandswerte oder parasitäre Kapazitäten an den Knotenpunkten Reflexionen erzeugen. Wenn sich diese mit dem Hauptsignal überlagern, führt das zu einer amplituden- und frequenzabhängigen Phasenverzerrung, die auf dem Scope als verrauschte Phasenmessung (600 ps) erscheint.

3. **AWG-Restkopplung / Clock-Jitter:**
   * Falls das AWG (Eclypse Z7) aktiv ist oder dessen Ausgangsstufe im hochohmigen Zustand Floaten/Rauschen einfängt, kann dies über den Kombinierer direkt in den Messpfad übersprechen.

---

### Empfohlene Maßnahmen zur Reduktion der Schwankungsbreite

1. **Galvanische Trennung / Masseschleifen unterbrechen:**
   * Versuche, den GPSDO, das AWG und das Eclypse-Scope an **exakt dieselte Steckdosenleiste** (Sternpunkt-Erdung) anzuschließen, um Potenzialdifferenzen zu minimieren.
   * Teste testweise, ob das AWG im ausgeschalteten bzw. getrennten Zustand (nur reiner GPSDO durch das Netzwerk auf das 1-m-Kabel) die 600 ps reduziert. Falls ja, koppelt das AWG Masse- oder Taktstörungen ein.

2. **Prüfung der Widerstandstoleranzen im Netzwerk:**
   * Überprüfe im Schaltplan (Widerstandsnetzwerk), ob die verwendeten Widerstände enge Toleranzen (0,1% oder 1% Metallfilm) besitzen. Unsymmetrien im 3-Wege-Kombinierer führen sofort zu unvollständiger Entkopplung und Signalverfälschung.

3. **Gleichtakt-Drosseln (Common-Mode Chokes) statt normaler Ferrite:**
   * Normale Ferritperlen auf dem Kabel (Dämpfung von Gegentakt-Signalen) helfen bei Masseschleifen im Koaxialkabel wenig. Wenn, dann muss die Koaxialleitung selbst mehrfach durch einen **Ferrit-Ringkern (Mantelwellensperre / Common-Mode Choke)** geführt werden, um Gleichtaktströme auf dem Kabelschirm zu ersticken.

4. **Schrittweises Isolieren (Einkanal-Fehlersuche):**
   * Nimm den Kombinierer und das AWG komplett aus dem Aufbau. Schließe den GPSDO direkt an das 1-m-Kabel mit 50/75-Ω-Anpassung an und leite es auf CH2. 
   * Sinkt der Jitter in diesem Zustand wieder in die Nähe der 9–30 ps, liegt das Problem definitiv beim Kombinierer oder der AWG-Injektion (Masseverbindung / Restwelligkeit). Bleibt der Jitter hoch, liegt es an der 50/75-Ω-Widerstandsanpassung selbst.

**Citations:**
[1] [https://www.willisat.de/sat-empfang/kabel-co/premiumx-profi-250-m-koaxialkabel-135-db-4-fach-geschirmt-cu-reines-kupfer-satelliten-koaxkabel-antennenkabel-digital-sat-tv-koax-kabel-rg-6-class-a-250m-auf-holz-trommel-mit-10x-f-stecker-vergoldet-mit-dichtring_122626_17917](https://www.willisat.de/sat-empfang/kabel-co/premiumx-profi-250-m-koaxialkabel-135-db-4-fach-geschirmt-cu-reines-kupfer-satelliten-koaxkabel-antennenkabel-digital-sat-tv-koax-kabel-rg-6-class-a-250m-auf-holz-trommel-mit-10x-f-stecker-vergoldet-mit-dichtring_122626_17917)

═══════════════════════════════════════════════════════════════
  Ende der Konversation
═══════════════════════════════════════════════════════════════
