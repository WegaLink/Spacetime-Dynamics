═══════════════════════════════════════════════════════════════
  Kimi K3.1 Pro Phasendetektor-Anpassung
═══════════════════════════════════════════════════════════════

Exportiert: 2. November 2026 um 23:37
Nachrichten: 6
Modell: kimi-latest

───────────────────────────────────────────────────────────────

[👤 Sie]
In einem Team aus 12 KIs und mir als Mensch sollen Verbesserungen für einen Phasendetektor diskutiert und erarbeitet werden, mit welchem Phänomene im Bereich Raumzeitdynamik untersucht werden sollen. Die Zusammenarbeit findet mit einem markdown-Dokument statt, welches im GitHub-Repository unter folgendem Link verfügbar ist:
https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md Als Start für „Physik 2“ soll die Anpassung für Oszillator, Injektion und ADC an ein 250 m Sat-Kabel HQ 135 dB, 4-fach geschirmt von HB Digital mit 75 Ohm Impedanz und 5 dB Dämpfung bei 10 MHz sowie einem Verkürzungsfaktor von 0.82  entworfen werden. Der Oszillator ist ein DXPatrol GPSDO V2.4 mit 3 Sinus- und einem Rechteck-Ausgang bei 10 MHz/+7dB Signal an 50 Ohm. Die Injektion soll durch Eclypse Z7 AWG mit einem programmierbaren Signal von 1 V Amplitude an 50 Ohm in einer Weise erfolgen, dass die Phase eines kombinierten Signals aus GPSDO und AWG im Bereich von etwa 300 ps moduliert werden kann. Die Eingangsimpedanz des Eclypse Z7 Scope ist mit 1 MOhm/18 pF angegeben. Ein Eingang soll mit dem GPSDO als Referenz verbunden werden und der andere als Messeingang mit dem anderen Ende des Sat-Kabels. Wie kann eine impedanzrichtige Anpassung dieser Anordnung realisiert werden?

───────────────────────────────────────────────────────────────

[🤖 KI]
Ich habe die Dokumentation gelesen. Der Kern der Frage ist die korrekte Impedanzanpassung einer Anordnung aus GPSDO (50 Ω), AWG (50 Ω), 250 m Sat-Kabel (75 Ω) und Scope (1 MΩ). Hier ist ein konkreter Vorschlag.

---

## 1. Grundprinzip: Impedanztransformation statt direkter Verbindung

Die zentrale Herausforderung ist die Mischung aus 50 Ω (Quellen) und 75 Ω (Kabel). Eine direkte Verbindung erzeugt Reflexionen, die die Phasenmessung verfälschen. Die Lösung sind **Mindestverlust-Anpassungsglieder (Minimum Loss Pads, MLP)** an den Übergängen.

### Anpassung 50 Ω ↔ 75 Ω
Ein resistiver L-Pad mit folgenden Werten:
- **Serienwiderstand am 50-Ω-Port:** R₁ = 43,3 Ω (E24: 43 Ω)
- **Parallelwiderstand am 75-Ω-Port:** R₂ = 86,6 Ω (E24: 86,6 Ω oder 82 Ω + 4,7 Ω)

**Eigenschaften:**
- Dämpfung: 5,7 dB (Leistung), entsprechend ~11,4 dB Spannungsdämpfung bei hochohmiger Messung
- Beidseitige Anpassung: 50 Ω-Seite sieht 50 Ω, 75 Ω-Seite sieht 75 Ω
- Verlustleistung unkritisch bei +7 dBm (5 mW)

---

## 2. Konkrete Signalkette

### A. Referenzpfad (GPSDO → Scope CH1)
```
GPSDO Sinus 10 MHz/+7 dBm 
    → 50 Ω Koax (kurz) 
    → 3 dB Dämpfungsglied (50 Ω) 
    → 6 dB Power-Splitter (50 Ω, z.B. Mini-Circuits ZFSC-2-1) 
    → Ausgang A: direkt an Scope CH1 (1 MΩ) 
    → Ausgang B: weiter zum Kombinierer
```
**Hinweis:** Der Splitter hat 6 dB Dämpfung (3 dB ideal + 3 dB durch interne Terminierung). Bei +7 dBm Eingang liegen an CH1 etwa **+1 dBm** an (ca. 0,7 Vpp an 1 MΩ). Das ist für den Scope-Eingang gut aussteuerbar.

### B. Messpfad (GPSDO + AWG → Kabel → Scope CH2)
```
GPSDO (Splitter Ausgang B, +1 dBm) 
    → 10 dB Dämpfungsglied (50 Ω) 
    → Kombinierer-Eingang 1

AWG Eclypse Z7 (1 V Amplitude, 50 Ω) 
    → 20 dB Dämpfungsglied (50 Ω) 
    → Kombinierer-Eingang 2

Kombinierer: 6 dB Power-Splitter als Combiner (50 Ω)
    → Summenausgang 
    → MLP 50→75 Ω 
    → 250 m Sat-Kabel (75 Ω) 
    → MLP 75→50 Ω 
    → Scope CH2 (1 MΩ)
```

**Pegelplan Messpfad:**
| Punkt | Pegel (50 Ω) | Bemerkung |
|-------|-------------|-----------|
| GPSDO nach Splitter | +1 dBm | ~0,7 Vpp |
| Nach 10 dB Dämpfung | −9 dBm | ~0,22 Vpp |
| AWG nach 20 dB Dämpfung | ~0,1 Vpp | 1 V → 0,1 V |
| Nach Combiner (6 dB) | GPSDO: −15 dBm, AWG: ~0,05 Vpp | |
| Nach MLP (5,7 dB) | GPSDO: −20,7 dBm | ~0,06 Vpp am Kabelanfang |
| Nach 250 m Kabel (5 dB @ 10 MHz) | GPSDO: −25,7 dBm | ~0,03 Vpp |
| Nach MLP 75→50 (5,7 dB) | GPSDO: −31,4 dBm | ~0,017 Vpp an CH2 |

**Problem:** Der Pegel an CH2 ist sehr niedrig. Der Eclypse Z7 Scope hat einen Eingangsbereich von typischerweise ±1 V oder ±2,5 V. Bei ~17 mVpp ist das Signal-zu-Rausch-Verhältnis schlecht.

---

## 3. Verbesserter Vorschlag: Vorverstärkung und Pegelanpassung

Um den Pegel an CH2 auf ein sinnvolles Maß zu heben (ca. 0,5–1 Vpp), ist ein **rauscharmer Vorverstärker** am Kabelende erforderlich:

```
... Kabelende 
    → MLP 75→50 Ω 
    → rauscharmer Verstärker (z.B. 20 dB, 50 Ω, NF < 2 dB, z.B. Mini-Circuits ZX60-33LN-S+) 
    → 10 dB Dämpfungsglied (zur Pegeljustage) 
    → Scope CH2
```

**Alternativ (einfacher, aber weniger flexibel):** Den GPSDO-Pegel vor dem Combiner nicht so stark dämpfen und den AWG-Anteil entsprechend anpassen. Allerdings muss das AWG-Signal für die Phasenmodulation ohnehin klein sein gegenüber dem Träger.

---

## 4. Phasenmodulation durch AWG-Injektion

Die geforderte Phasenmodulation von ~300 ps bei 10 MHz entspricht einer Phasenänderung von:
$$\Delta\phi = 2\pi \cdot f \cdot \Delta t = 2\pi \cdot 10^7 \cdot 300 \cdot 10^{-12} \approx 0,01885 \text{ rad} \approx 1,08°$$

### Methode: Vektorsumme (Quadratur-Injektion)
Die sauberste Methode ist die **Addition eines schwachen, um 90° phasenverschobenen Signals** zum Träger. Die resultierende Phase ist:
$$\phi(t) = \arctan\left(\frac{A_{inj} \cdot \sin(\omega t + \delta(t))}{A_{träger}}\right) \approx \frac{A_{inj}}{A_{träger}} \cdot \cos(\delta(t))$$

Für kleine Injektionen ($A_{inj} \ll A_{träger}$) gilt näherungsweise:
$$\Delta\phi \approx \frac{A_{inj}}{A_{träger}}$$

**Umsetzung:**
- Der AWG erzeugt ein 10-MHz-Signal, das **phasenstarr** zum GPSDO läuft (gemeinsame 10-MHz-Referenz oder Trigger).
- Die Phasenlage des AWG wird auf **90° ± δ** relativ zum GPSDO eingestellt.
- Die Amplitude des AWG wird so gewählt, dass $A_{inj}/A_{träger} \approx 0,01885$ (für 300 ps).

**Beispielrechnung:**
- Träger am Combiner-Ausgang: 0,1 Vpp (nach Dämpfung)
- Benötigte Injektionsamplitude: $0,1 \cdot 0,01885 \approx 1,885$ mVpp
- Bei 20 dB AWG-Dämpfung: AWG-Ausgang $1,885 \cdot 10 = 18,85$ mVpp → AWG auf ~19 mVpp einstellen (oder Dämpfung reduzieren).

**Wichtig:** Die Phasenmodulation erfolgt durch Variation von $\delta$ (AWG-Phase) oder durch Variation der AWG-Amplitude. Für eine **lineare Phasenrampe** (zur Kalibrierung) ist die Amplitudenvariation einfacher.

---

## 5. Alternative: Einfachere Injektion über Kabel-Laufzeitvariation

Falls die vektorielle Injektion zu komplex ist, gibt es eine pragmatischere Alternative:

**Phasenmodulation durch variable Verzögerung im AWG-Pfad:**
- Der AWG erzeugt kein Sinussignal, sondern einen **kurzen Impuls oder eine Stufe**, die über einen **programmierbaren Delay-Chip** (z.B. 3D3428, wie im bestehenden System) auf den Träger addiert wird.
- Das ist jedoch komplexer in der analogen Signalverarbeitung.

**Empfehlung:** Die vektorielle Injektion (Punkt 4) ist mit dem Eclypse Z7 AWG direkt machbar, da dieser zwei Kanäle hat und phasenkohärent arbeiten kann.

---

## 6. Zusammenfassung der Hardware-Liste

| Komponente | Anzahl | Spezifikation |
|------------|--------|---------------|
| MLP 50↔75 Ω | 2 | R₁=43,3 Ω, R₂=86,6 Ω, 0,1 %, SMD 0805 |
| Power-Splitter 50 Ω | 2 | 6 dB, 5–1000 MHz (z.B. Mini-Circuits ZFSC-2-1) |
| Dämpfungsglieder | divers | 3 dB, 10 dB, 20 dB, 50 Ω, SMA |
| Rauscharmer Verstärker | 1 | 20 dB, 50 Ω, 10 MHz, NF < 2 dB |
| Koaxkabel 50 Ω | divers | kurz, SMA, doppelt geschirmt |
| 75 Ω Abschlusswiderstand | 1 | zur Kabelterminierung bei Tests |

---

## 7. Kritische Punkte für die Dokumentation

Für den Eintrag in **Physik 2** sollten folgende Aspekte festgehalten werden:

1. **Reflexionen als Störquelle:** Jedes MLP hat eine endliche Anpassgüte. Bei 250 m Kabel und 10 MHz (Wellenlänge ~24 m im Kabel) sind stehende Wellen möglich. Die MLPs müssen eine Return Loss > 20 dB haben.
2. **Thermische Drift der MLPs:** Die Widerstände müssen präzise und temperaturstabil sein (Metallfilm, 25 ppm/K oder besser). Eine Temperaturdifferenz zwischen den MLPs erzeugt eine Phasenverschiebung.
3. **AWG-Phasenrauschen:** Der Eclypse Z7 AWG hat eine eigene Taktjitter. Dieser geht direkt in die Injektion ein. Für 300 ps Modulation muss der Jitter < 30 ps sein (10 %).
4. **Kabel als Sensor:** Das 250-m-Kabel hat eine Laufzeit von $\tau = L \cdot \sqrt{\epsilon_r}/c = 250 / (0,82 \cdot 3 \cdot 10^8) \approx 1,016$ µs. Eine Phasenänderung von 300 ps entspricht $\Delta\tau/\tau \approx 3 \cdot 10^{-4}$. Das ist die Größenordnung, die das System auflösen muss.

---

Soll ich diesen Vorschlag als formatierten Markdown-Eintrag für **Physik 2** im Dokument vorbereiten?

**Citations:**
[1] [https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md](https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md)

───────────────────────────────────────────────────────────────

[👤 Sie]
Ein Widerstandsnetzwerk wurde wie im Dokument beschrieben mit folgenden Komponenten aufgebaut: GPSDO - Dämpfung (-5dB) - 3-Wege-Kombinierer - 50/75 Ohm Anpassung - SATA Kabel 1 m (für Null-Test) - 75/50 Ohm Anpassung - SDR-1. Die Dämpfung der gesamten Strecke wurde mit Nano VNA bei 10 MHz etwa wie erwartet mit -22.5dB gemessen. Nach Wechsel des 1m SATA-Kabels gegen ein 250m SATA-Kabel stieg die Dämpfung wie im NanoVNA Screenshot gezeigt auf -55dB an. Es zeigte sich nun ein Tiefpasscharakter mit einer -3dB Frequenz bei etwa 250 kHz  wie ebenfalls in einem angehängten Screenshot vom Nano VNA gezeigt. Nach Datenblatt sollte die zusätzliche Dämpfung bei 250m/10MHz etwa -5dB sein, sie ist jedoch mehr als -30dB. Der ohmsche Widerstand der Schirmung ist etwa 8 Ohm und des Innenleiters etwa 34 Ohm. Welche Erklärung könnte es dafür geben? Wie kann das System verändert werden, damit das 10MHz Signal des GPSDO mit der nominellen Dämpfung am Ausgang ankommt?

📎 Anhänge: Anpassung und Kombinierer 3 MHz (GPSDO - SATA_250m - SDR-1).jpg, Anpassung und Kombinierer 10 MHz (GPSDO - SATA_250m - SDR-1).jpg

───────────────────────────────────────────────────────────────

[🤖 KI]
Die von Ihnen gemessenen Werte (DC-Widerstand Innenleiter ~34 Ω, Schirm ~8 Ω, Gesamtdämpfung -55 dB bei 10 MHz mit starkem Tiefpasscharakter bei ~250 kHz) entlarven die physikalische Ursache sofort: 

Es handelt sich bei dem verwendeten Kabel um ein **Satellitenkabel mit Stahldraht-Kupfer-Kaschierung (CCS – Copper Clad Steel) oder einem massiven Stahldraht als Innenleiter**, und sehr dünnem Aluminium-Geflecht bzw. Folie als Schirm.

### 1. Die Erklärung für das Phänomen

* **Gleichstromwiderstand vs. HF (Skineffekt):** Bei DC messen Sie 34 Ω für den Innenleiter. Bei HF (10 MHz) drängt der Strom durch den Skineffekt an die Oberfläche. Wenn der Innenleiter aus Stahl mit einer dünnen Kupfer-Plattierung besteht (typisch für billige Koax-/Sat-Kabel), ist der HF-Widerstand bei 10 MHz **dramatisch höher** als bei purem Kupfer.
* **Skin-Eindringtiefe bei 10 MHz:** In Kupfer liegt sie bei ca. 21 µm, in Eisen/Stahl (durch Permeabilität) noch deutlich niedriger. Wenn die Kupferschicht auf dem CCS-Draht dünner als einige Mikrometer ist oder Risse hat, fließt ein Teil des HF-Stroms durch den hochohmigen Stahlkern.
* **Tiefpass-Charakteristik:** Ein 250-m-Kabel mit derartigen Leitungsverlusten wirkt wie ein verteilter $RC$- oder $RLC$-Tiefpass. Die -3-dB-Grenze bei ~250 kHz und die gewaltige Dämpfung von -55 dB bei 10 MHz (statt erwarteter ~10 dB inklusive Anpassungen) sind das klassische Resultat eines hochohmigen, verlustbehafteten Leitermaterials. Die NanoVNA-Messung bei 3 MHz und 10 MHz bestätigt diesen steilen Abfall der Übertragungsfunktion ($S_{21}$).
* **Schirmwiderstand:** 8 Ω für 250 m Schirm ist ebenfalls sehr hoch (entspricht dünner Alu-Folie + lockerem Geflecht). Das führt zu unzureichender Schirmung und erhöht die ohmschen Verluste im Rückleiter weiter.

---

### 2. Wie kann das System verändert werden?

Um das 10-MHz-Signal mit der nominellen Dämpfung (ca. 5 dB Kabeldämpfung + unvermeidliche Steckdämpfung) durch 250 m zu bekommen, sind folgende Schritte zwingend erforderlich:

#### A. Kabeltausch gegen echtes Vollkupfer-Kabel (Solid Copper)
Das wichtigste Upgrade ist ein Kabelwechsel. Das HB Digital HQ 135 dB ist oft als CCS (Steel) ausgeführt, um reißfest zu sein. 
* **Lösung:** Ein Sat- oder Koaxkabel mit **reinem Kupfer-Innenleiter (OFC / Solid Copper)** und **vollflächigem Kupfer-Geflecht** (bzw. verzinntem Kupfer) wählen (z.B. hochwertiges professionelles SDI-Videokabel oder RG-6/U mit Vollkupfer-Seele).
* **Ziel:** Der DC-Widerstand des Innenleiters bei 250 m darf bei RG-6 / 75-Ω-Kabeln im Bereich von **2,5 bis 4 Ω liegen** (nicht 34 Ω!). Damit sinkt die Dämpfung bei 10 MHz sofort auf die im Datenblatt versprochenen Werte (~5 dB).

#### B. DC-Einspeisung / Fernspeisung (Notlösung falls Kabel nicht tauschbar)
Falls das Kabel physisch nicht gewechselt werden kann (z.B. fest verlegt), hilft nur:
* **Verstärkung direkt vor dem Kabel:** Ein rauscharmes Verstärkermodul (Line-Amplifier) mit genau dem Betrag an Verstärkung (hier ca. +30 dB bei 10 MHz), der dem Kabelverlust entspricht, direkt *vor* das Kabel setzen, um den Pegelverlust auszugleichen. 
* *Nachteil:* Das Rauschen wird mitverstärkt, und die Phasenstabilität des Verstärkers muss extrem hoch sein (Temperaturdrift!), um die Raumzeit-Messung nicht zu verfälschen.

#### C. Überprüfung der Impedanzanpassung bei 75 Ω
Da das Kabel nun niederohmiger wird, greifen die 75/50-Ω-Anpassungsglieder wieder wie berechnet. Stellen Sie sicher, dass die Anpassungsnetzwerke (L-Pads oder Trafo-Glieder) direkt an den Kabelenden sitzen, um stehende Wellen und zusätzliche Resonanzen (wie im 3-MHz-NanoVNA-Plot zu sehen) zu vermeiden.

───────────────────────────────────────────────────────────────

[👤 Sie]
Danke für die Hinweise. Das Sat-Kabel (nicht SATA, war falsch) war mit CCS Innenleiter (Fehlkauf, wider besseren Wissens) soll mit folgendem Typ mit Kupfer ausgetauscht werden: https://www.willisat.de/sat-empfang/kabel-co/premiumx-profi-250-m-koaxialkabel-135-db-4-fach-geschirmt-cu-reines-kupfer-satelliten-koaxkabel-antennenkabel-digital-sat-tv-koax-kabel-rg-6-class-a-250m-auf-holz-trommel-mit-10x-f-stecker-vergoldet-mit-dichtring_122626_17917 Sind von diesem Kabel die technischen Daten bekannt? Gibt es eventuell noch andere Kabel, die besser geeignet wären? Aktuell werden Nullmessungen mit direktem GPSDO-Signal an beiden Kanälen sowie mit einem 1m-Kabel am Widerstandsnetzwerk wie im Anhang gezeigt durchgeführt. Bei direktem GPSDO-Signal an beiden Kanälen wird ein geringfügiger Offset von 26.1 ps mit einer Schwankungsbreite von 9 ps wie im Anhang gezeigt sichtbar. Bei Anschluss eines 1m-Kabels mit Anpassung und Kombinierer wird neben der Phasenverschiebung durch die Kabellänge eine Schwankungsbreite von ca. 600 ps sichtbar. Zu deren Reduktion sind bereits eine vollständige Abschirmung in einem Stahlblech-Behälter sowie Ferritringe auf den Versorgungsleitungen versucht worden, jedoch ohne erkennbaren Einfluss. Was könnte die Ursache der hohen Schwankungsbreite des Phasensignals sein? Wie könnte sie sich verringern lassen?

📎 Anhänge: Widerstandsnetzwerk.jpg, GPSDO zu Sat-1m zu SDR1 und GPSDO zu SDR2.jpg, GPSDO zu SDR1 und SDR2.jpg

───────────────────────────────────────────────────────────────

[🤖 KI]
### 1. Technische Daten des gewählten PremiumX Vollkupfer-Kabels

Das von Ihnen herausgesuchte **PremiumX Profi (bzw. Deluxe) Vollkupfer-Koaxialkabel (RG-6, Klasse A, 135 dB)** hat hervorragende, für diesen Einsatzzweck passende Kenndaten:
* **Innenleiter:** 1,02 mm (bzw. 1,13 mm bei Deluxe) aus **100 % reinem Kupfer (CU)**. Der Gleichstromwiderstand sinkt damit bei 250 m Länge von schockierenden ~34 Ω (CCS) auf ca. **4,3 bis 5,5 Ω** für den gesamten Innenleiter.
* **Wellenwiderstand:** 75 Ω (±3 Ω).
* **Verkürzungsfaktor ($v_f$):** ca. 0,82 bis 0,85 (durch geschäumtes Zell-PE-Dielektrikum).
* **Dämpfung bei 10 MHz:** Liegt bei diesem Kabeltyp extrem niedrig, typischerweise bei unter **0,8 bis 1,2 dB pro 100 Meter** bei 10 MHz. Für 250 Meter bedeutet dies eine zu erwartende Gesamtkabeldämpfung von **ca. 2 bis 3 dB** bei 10 MHz (zuzüglich der Anpassungsglieder). Damit wird der Tiefpass-Effekt komplett eliminiert.

#### Gibt es noch besser geeignete Kabel?
* **RG-11 Vollkupfer (75 Ω):** Ein RG-11-Kabel hat einen deutlich dickeren Innenleiter (ca. 1,6 mm Cu) und einen größeren Außendurchmesser (~10 mm). Die Dämpfung bei 10 MHz sinkt damit nochmals um etwa die Hälfte. *Nachteil:* Es ist spürbar dicker, schwerer, steifer und erfordert spezielle F-Stecker für RG-11. Für Labor- oder Testaufbauten ist das RG-6 Vollkupfer in der Regel der beste Kompromiss aus Handhabbarkeit und extrem geringer Dämpfung.
* **Professionelles SDI-Videokabel (z.B. Belden 1694A):** Das ist der absolute Goldstandard für präzise analoge HF- und Digitalsignale bei 75 Ω mit massivem Kupferinnenleiter und gasgeschäumtem PE. Kostet allerdings im Vergleich zum Sat-Kabel ein Vielfaches. Das PremiumX Vollkupfer-Sat-Kabel ist für Ihr Projekt bei diesem Preis-Leistungs-Verhältnis eine exzellente Wahl.

---

### 2. Ursache für die hohe Schwankungsbreite (ca. 600 ps) beim 1m-Kabel-Test

Dass die Nullmessung direkt am GPSDO (ohne Netzwerk) eine winzige Schwankung von nur **9 ps** zeigt, beweist, dass der GPSDO und der Eclypse Z7 AWG/Scope phasenstarr und rauscharm arbeiten. Sobald jedoch das 1m-Kabel mit dem Widerstandsnetzwerk (Kombinierer + 50/75-Ω-Anpassung) dazwischengeschaltet wird, explodiert die Schwankungsbreite auf **~600 ps**. 

Da eine Stahlblech-Abschirmung und Ferrite keinen Einfluss hatten, handelt es sich **nicht um klassische elektromagnetische Einstrahlung (EMV-Netzbrummen oder HF-Störungen)**, sondern um schaltungstechnische bzw. reflexionsbedingte Effekte:

1. **Fehlende Entkopplung (Rückwirkung des AWG auf den GPSDO):** 
   Im Widerstandsnetzwerk sind GPSDO und AWG über den 3-Wege-Kombinierer zusammengeschaltet. Wenn die Entkopplung (Isolation) des passiven Widerstandsnetzwerks unzureichend ist, moduliert das AWG-Signal (oder interne Schaltvorgänge des AWG/Scopes) direkt auf den GPSDO-Referenzeingang zurück. Das führt zu einer Phasenverwirrung am Trigger.
2. **Reflexionen an den Impedanzübergängen (Stehende Wellen):**
   Das Netzwerk wandelt 50 Ω (GPSDO/AWG) auf 75 Ω (das kurze 1m-Kabel) und zurück auf 50 Ω (SDR/Scope). Wenn die Widerstände im L-Pad nicht exakt passen (z. B. Standardwerte statt exakter Berechnungen oder Toleranzen der SMD-Widerstände) oder Streuinduktivitäten durch lange ungeschirmte Beinchen/Leiterbahnen vorliegen, entstehen **Reflexionen**. Diese Mehrfachreflexionen im Kabel (auch bei nur 1 m Länge bei hohen Flankensteilheiten) verändern den Nulldurchgang des Sinussignals massiv und jitter-artig.
3. **AM-zu-PM-Konversion (Amplituden- zu Phasen-Jitter):**
   Wenn durch Fehlanpassung Signalanteile reflektiert werden und sich überlagern, führt jede kleinste Amplituden- oder Frequenzschwankung der Quellen zu einer scheinbaren Phasenverschiebung. 600 ps Jitter bei 10 MHz entspricht gerade einmal ca. **2,16 Grad** Phasenjitter – bei einer ungünstigen Resonanz im Widerstandsnetzwerk schwingt sich das System hier leicht auf.
4. **Masse-Schleifen / Ground-Bouncing über die USB-Masse des Eclypse Z7:**
   Der Eclypse Z7 ist oft über USB mit einem PC verbunden, der GPSDO hat eventuell ein eigenes Netzteil. Über die Schirmungen der Koaxkabel und die Masse des AWG/Scopes fließt ein Ausgleichsstrom (Ground Loop), der hochfrequentes Rauschen direkt in den Eingang mischt.

---

### 3. Wie lässt sich die Schwankungsbreite verringern?

Um den Jitter beim 1m-Test (und später beim 250m-Kabel) wieder auf den Bereich von unter 20–50 ps zu drücken, sollten folgende Maßnahmen ergriffen werden:

* **Aktive Entkopplung statt reinem Widerstandsnetzwerk:** 
  Verwenden Sie für die Injektion keinen passiven Widerstands-Kombinierer, sondern einen **aktiven HF-Kombinierer / RF-Summierer** oder entkoppeln Sie den AWG über einen Operationsverstärker / HF-Buffer mit hoher Rückflussdämpfung (Reverse Isolation > 30 dB), damit das AWG-Signal den GPSDO nicht beeinflusst.
* **Prüfung der Widerstandswerte im L-Pad:** 
  Messen Sie die Widerstände des Anpassungsnetzwerks nach. Für 50 Ω ↔ 75 Ω müssen die exakten Werte (Serienwiderstand $R_1 = 43,3\,\Omega$, Querwiderstand $R_2 = 86,6\,\Omega$) möglichst genau getroffen werden (1%-Metallfilm-Widerstände oder Parallelschaltung von E24-Widerständen).
* **Gezielte Masse-Trennung (Galvanische Trennung):** 
  Trennen Sie eventuelle Masseschleifen auf. Versorgen Sie den GPSDO und den Eclypse Z7 testweise über getrennte, potentialfreie Netzteile oder nutzen Sie Gleichtakt-Drosseln (Common-Mode Chokes) auf den Signalleitungen (keine einfachen Ferritperlen, sondern echte HF-Übertrager / Baluns für 10 MHz), um Mantelwellen auf den Koaxkabeln zu unterbinden.
* **Abschluss mit 75 Ω direkt am Eingang:** 
  Stellen Sie sicher, dass das 1m-Kabel am Ende exakt mit 75 Ω abgeschlossen ist, bevor es in die 75/50-Ω-Anpassung und an das Scope (1 MΩ) geht. Ohne echten 75-Ω-Wellenwiderstand am Kabelende entstehen stehende Wellen, die den Jitter extrem antreiben.

**Citations:**
[1] [https://www.mediamarkt.de/de/product/_premiumx-500m-deluxe-pro-koaxial-antennenkabel-96178714.html#1](https://www.mediamarkt.de/de/product/_premiumx-500m-deluxe-pro-koaxial-antennenkabel-96178714.html#1)
[2] [https://www.mediamarkt.de/de/product/_premiumx-50m-profi-pro-koaxial-antennenkabel-96178719.html#1](https://www.mediamarkt.de/de/product/_premiumx-50m-profi-pro-koaxial-antennenkabel-96178719.html#1)
[3] [https://www.mediamarkt.de/de/product/_premiumx-basic-100w-123399-antennenkabel-97539402.html#1](https://www.mediamarkt.de/de/product/_premiumx-basic-100w-123399-antennenkabel-97539402.html#1)
[4] [https://www.otto.de/p/premiumx-25m-profi-koaxialkabel-schwarz-135db-4-fach-geschirmt-reines-kupfer-tv-kabel-S072P0KQ/#1](https://www.otto.de/p/premiumx-25m-profi-koaxialkabel-schwarz-135db-4-fach-geschirmt-reines-kupfer-tv-kabel-S072P0KQ/#1)
[5] [https://www.otto.de/p/premiumx-5x-100m-koaxialkabel-135db-4-fach-antennenkabel-rg6-class-a-digital-sat-kabel-S0L340FG/#variationId=S0L340FGRL8A#1](https://www.otto.de/p/premiumx-5x-100m-koaxialkabel-135db-4-fach-antennenkabel-rg6-class-a-digital-sat-kabel-S0L340FG/#variationId=S0L340FGRL8A#1)
[6] [https://www.otto.de/p/premiumx-deluxe-pro-kupfer-koaxialkabel-135db-5-fach-100m-sat-antennenkabel-tv-kabel-S072P0J8/#1](https://www.otto.de/p/premiumx-deluxe-pro-kupfer-koaxialkabel-135db-5-fach-100m-sat-antennenkabel-tv-kabel-S072P0J8/#1)
[7] [https://www.otto.de/p/premiumx-profi-pro-koaxialkabel-135db-5-fach-reines-kupfer-50-meter-tv-kabel-S072P0JM/#variationId=S072P0JMLGN3#1](https://www.otto.de/p/premiumx-profi-pro-koaxialkabel-135db-5-fach-reines-kupfer-50-meter-tv-kabel-S072P0JM/#variationId=S072P0JMLGN3#1)
[8] [https://www.saturn.de/de/product/_premiumx-px-basic-250w-122625-antennenkabel-97539340.html](https://www.saturn.de/de/product/_premiumx-px-basic-250w-122625-antennenkabel-97539340.html)
[9] [https://www.voelkner.de/products/3429869/PremiumX-25m-PROFI-Koaxialkabel-Schwarz-135dB-4-fach-geschirmt-REINES-KUPFER-Koax-Kabel-Satellitenkabel-SAT-Antennenkabel.html?offer=58d91faa31195edc9b4f019bee668de5](https://www.voelkner.de/products/3429869/PremiumX-25m-PROFI-Koaxialkabel-Schwarz-135dB-4-fach-geschirmt-REINES-KUPFER-Koax-Kabel-Satellitenkabel-SAT-Antennenkabel.html?offer=58d91faa31195edc9b4f019bee668de5)
[10] [https://www.voelkner.de/products/4810478/PremiumX-100m-BASIC-Koaxialkabel-schwarz-135dB-4-fach-geschirmt-SAT-TV-Koax-Kabel-Antennenkabel-Entmanteler-0-31-m.html?itm_source=&itm_medium=last_seen&itm_campaign=4810478#1](https://www.voelkner.de/products/4810478/PremiumX-100m-BASIC-Koaxialkabel-schwarz-135dB-4-fach-geschirmt-SAT-TV-Koax-Kabel-Antennenkabel-Entmanteler-0-31-m.html?itm_source=&itm_medium=last_seen&itm_campaign=4810478#1)

═══════════════════════════════════════════════════════════════
  Ende der Konversation
═══════════════════════════════════════════════════════════════
