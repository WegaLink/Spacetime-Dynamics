═══════════════════════════════════════════════════════════════
  Muse Spark 1.3 Phasendetektor-Impedanzanpassung
═══════════════════════════════════════════════════════════════

Exportiert: 26. Oktober 2026 um 18:51
Nachrichten: 2
Modell: muse-spark-1.3

───────────────────────────────────────────────────────────────

[👤 Sie]
In einem Team aus 12 KIs und mir als Mensch sollen Verbesserungen für einen Phasendetektor diskutiert und erarbeitet werden, mit welchem Phänomene im Bereich Raumzeitdynamik untersucht werden sollen. Die Zusammenarbeit findet mit einem markdown-Dokument statt, welches im GitHub-Repository unter folgendem Link verfügbar ist:
https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md Als Start für „Physik 2“ soll die Anpassung für Oszillator, Injektion und ADC an ein 250 m Sat-Kabel HQ 135 dB, 4-fach geschirmt von HB Digital mit 75 Ohm Impedanz und 5 dB Dämpfung bei 10 MHz sowie einem Verkürzungsfaktor von 0.82  entworfen werden. Der Oszillator ist ein DXPatrol GPSDO V2.4 mit 3 Sinus- und einem Rechteck-Ausgang bei 10 MHz/+7dB Signal an 50 Ohm. Die Injektion soll durch Eclypse Z7 AWG mit einem programmierbaren Signal von 1 V Amplitude an 50 Ohm in einer Weise erfolgen, dass die Phase eines kombinierten Signals aus GPSDO und AWG im Bereich von etwa 300 ps moduliert werden kann. Die Eingangsimpedanz des Eclypse Z7 Scope ist mit 1 MOhm/18 pF angegeben. Ein Eingang soll mit dem GPSDO als Referenz verbunden werden und der andere als Messeingang mit dem anderen Ende des Sat-Kabels. Der Messbereich ist jeweils +/-1 V. Wie kann eine impedanzrichtige Anpassung dieser Anordnung realisiert werden?

───────────────────────────────────────────────────────────────

[🤖 KI]
Für `Physik 2` als Start: Anpassung `50 Ohm <-> 75 Ohm`, Pegel und Injektion für ca. `300 ps`.

## 1. Gegebene Grössen

* **Kabel:** 250m SAT HQ135, `Z=75 Ohm`, `VF=0.82`, `~5 dB @10MHz`
* Laufzeit ein Weg:

```
tau = L / (c*v) = 250 / (3e8*0.82) = ca. 1016 ns = ca. 1.02 us
```

Bei `10 MHz, T=100 ns` sind das ca. `10.16 Perioden`. Zähler/Phasenmessung muss daher nur `delta-t`, nicht absolut messen.

* **GPSDO DXPatrol V2.4:** `10 MHz Sinus, +7 dBm @50 Ohm`
  `P=5.0 mW, Vrms=0.5V, Vpk=0.707V, Vpp=1.414V`
* **AWG Eclypse Z7:** `1 V Amplitude @50 Ohm`, d.h. `Vpk=1V, Vpp=2V` programmierbar
* **Scope Eclypse Z7:** `1 MOhm / 18pF, +/-1V`. Bei 10 MHz sind 18 pF = ca. 884 Ohm kapazitiv -> ohne Abschluss nicht brauchbar. Unbedingt extern mit 75 Ohm abschliessen.

Messbereich +/-1V einhalten: nach 5 dB Kabeldämpfung + Kombiniererdämpfung sind wir automatisch <1Vpk.

## 2. Prinzip: Quelle 50 Ohm auf Kabel 75 Ohm

Keine LC-Anpassung nötig bei 10 MHz. Breitbandig resistiv:

**Sendeseite:** `50 Ohm Quelle + 25 Ohm Serie = 75 Ohm` ins Kabel.
Reflexionsfreiheit rückwärts, keine Resonanz auf 250m (ca. 1 us Hin-Rück 2 us).

**Empfangsseite:** `75 Ohm Shunt` am Scope-Eingang gegen Masse, dann hochohmig abgreifen.
Ideal: 75-Ohm-Durchgangsabschluss (Feedthrough) + T-Stück.

Damit Restfehlanpassung:
Quelle exakt 75 Ohm, Last exakt 75 Ohm, nur Kabel selbst mit Toleranz. Rückflussdämpfung >20 dB erreichbar, völlig ausreichend für Phasenmessung.

> Keinen 50/75 Minimum-Loss-Pad mit 5.7 dB nehmen, wenn nicht nötig. 25-Ohm-Serie ist verlustloser und phasenstabiler.

Konkret:
```
GPSDO 50R --[Att 3-6dB]--+--[R1]--+--[Rs=25R]--> 75R Kabel 250m --> [Rt=75R nach GND] --> Scope Hi-Z
AWG 50R   --[Att]--------+         |
                                    Rsum
```

## 3. Kombinierer GPSDO + AWG für 300 ps Modulation

Kombiniertes Signal am Kabeleingang:

```
V(t) = A*sin(wt) + a*sin(wt+phi_inj)
```

Für `a << A` gilt Phasenhub:

```
delta-phi_max ~ a/A  [rad]
delta-t = delta-phi / (2*pi*f0)
```

Gefordert `delta-t = 300 ps @10 MHz`:

```
delta-phi = 2*pi*10e6*300e-12 = 0.01885 rad = 1.08 Grad
a/A = 0.01885 = -34.5 dB Spannungsverhältnis
```

D.h. Injektion muss **ca. 34 dB unter Träger** liegen, in Quadratur (90 Grad) für reine PM. In Phase gäbe es nur AM.

Beispiel Pegelrechnung:
Träger am Summationspunkt `A = 0.5 Vrms` (0 dB).
Dann `a = 0.5 * 0.01885 = 9.4 mVrms = 26.6 mVpp`.

AWG kann 1 V Amplitude = 0.707 Vrms. Nötige Dämpfung:

```
20*log10(0.707/0.0094) = 37.5 dB
```

Praktisch: **30 dB Fix-Attenuator + AWG digital -7.5 dB** stellen, dann per Software den Hub 50...500 ps programmierbar durch Amplitude. Das lässt Headroom für Linearitätstest `Messung 3`.

Resistiver 2:1 Kombinierer, phasenstabil mit Metallfilm 0.1%:

```
GPSDO über R1=50R auf Knoten K
AWG über R2=1.5kR (ca. 30 dB Entkopplung) auf Knoten K
K über Rs=43R (?) auf Kabel
```

Vereinfacht und robust: kommerziellen 50-Ohm 2-Way Resistive Splitter/Combiner (z.B. Mini-Circuits ZFSC-2-1, 0-100MHz) nehmen, dann dahinter:

```
Combiner Ausgang 50R --[Rs=25R Serie]--> Kabel 75R
```

Die 25R + 75R Kabellast = 100R Last am 50R Combiner -> VSWR 2:1, bei 10 MHz unkritisch, zusätzlich 3 dB Pad vor Combiner zur Entkopplung des GPSDO einsetzen. Das ist stabiler als diskret gerechnet und reproduzierbar dokumentierbar.

Empfohlene Kette Senden:

```
GPSDO (+7dBm) -> 6dB Pad 50R -> Combiner Port A
AWG (1Vpk max) -> 30dB Pad 50R -> Combiner Port B
Combiner Sum -> 25R (in Serie, 0.1% Metallfilm, kurze Leitung) -> Innenleiter SAT-Kabel
Schirm SAT-Kabel beidseitig direkt auf Gehäuse-GND, einseitig HF-dicht, keine Schleife über Netz.
```

## 4. Empfangsseite impedanzrichtig

```
Kabel-Innenleiter -> T-Stück -> a) 75R (0.1%, 0.25W, kurze Masse) nach GND
                              b) Scope Hi-Z Eingang (1M/18pF)
Referenz: GPSDO 2. Ausgang -> 50R Feedthrough-Abschluss -> Scope Kanal 2 Hi-Z, oder 50R direkt wenn Scope umschaltbar.
```

Scope misst damit:

* CH-Ref: sauberer 10 MHz Sinus ca. 0.3-0.5 Vpk
* CH-Mess: ca. 0.5x durch Kabeldämpfung + Kombinierer, also ca. 0.15-0.25 Vpk, innerhalb +/-1V, SNR gut.

Wichtig: **beide Scope-Kabel gleich kurz (<30cm RG174/Urg), gleiche Länge**, sonst Temperaturdrift als Schein-Phase. Rt=75R direkt am T-Stück, nicht 1m Stichleitung.

4-fach Schirm SAT-Kabel: nicht erden über beide Enden + USB-GND-Schleife. PoE-Pfad aus Physik-1-Erkenntnis beachten: Z7 über isolierten Hub / PoE-Trennung versorgen, USB-Lastspitzen vermeiden.

## 5. Kalibrierung für Physik 2 / Physik 3

1. **Leerlauf / Kurzschluss / 75R-Test:** Kabelende offen, kurz, 75R -> Phase muss nur bei 75R stabil sein.
2. **AWG aus / 50R abgeschlossen:** Rauschen / Allan ohne Injektion.
3. **AWG Quadratur-Injektion:** AWG 90 Grad versetzt zum GPSDO programmieren, Amplitude sweepen -> Nachweis `delta-t linear in a/A`, Vorzeichen dreht mit 90°/270°.
4. **AM-Nulltest:** AWG in Phase (0°) -> darf fast keine Phase, nur Amplitude ändern. Trennt echte PM von AM-PM des Komparators.

Damit ist `Physik 7: 1-10 ppm` direkt prüfbar: `1 ppm von 1016 ns = 1.0 ps`, `10 ppm = 10 ps`. Der geforderte 300-ps-Hub entspricht ca. 300 ppm und liegt damit bewusst ca. 3-30x über Kozyrev-Erwartung, aber noch klein genug für Linearität — ideal als `Messung 3 Injektionskalibrierung`.

Vorschlag für Doku-Eintrag `Physik 2`: Topologie B-neu mit 250m/75R/0.82, 25R-Quellenanpassung, 75R-Senkenabschluss, resistiver Combiner + 30dB AWG-Pad für 300ps-PM bei Quadratur-Injektion.


**Citations:**
[1] [https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md](https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md)

═══════════════════════════════════════════════════════════════
  Ende der Konversation
═══════════════════════════════════════════════════════════════
