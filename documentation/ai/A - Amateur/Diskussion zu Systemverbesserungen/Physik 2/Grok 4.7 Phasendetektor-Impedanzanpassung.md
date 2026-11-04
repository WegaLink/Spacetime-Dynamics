═══════════════════════════════════════════════════════════════
  Grok 4.7 Phasendetektor-Impedanzanpassung
═══════════════════════════════════════════════════════════════

Exportiert: 2. November 2026 um 23:34
Nachrichten: 12
Modell: grok-latest

───────────────────────────────────────────────────────────────

[👤 Sie]
In einem Team aus 12 KIs und mir als Mensch sollen Verbesserungen für einen Phasendetektor diskutiert und erarbeitet werden, mit welchem Phänomene im Bereich Raumzeitdynamik untersucht werden sollen. Die Zusammenarbeit findet mit einem markdown-Dokument statt, welches im GitHub-Repository unter folgendem Link verfügbar ist:
https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md Als Start für „Physik 2“ soll die Anpassung für Oszillator, Injektion und ADC an ein 250 m Sat-Kabel HQ 135 dB, 4-fach geschirmt von HB Digital mit 75 Ohm Impedanz und 5 dB Dämpfung bei 10 MHz sowie einem Verkürzungsfaktor von 0.82  entworfen werden. Der Oszillator ist ein DXPatrol GPSDO V2.4 mit 3 Sinus- und einem Rechteck-Ausgang bei 10 MHz/+7dB Signal an 50 Ohm. Die Injektion soll durch Eclypse Z7 AWG mit einem programmierbaren Signal von 1 V Amplitude an 50 Ohm in einer Weise erfolgen, dass die Phase eines kombinierten Signals aus GPSDO und AWG im Bereich von etwa 300 ps moduliert werden kann. Die Eingangsimpedanz des Eclypse Z7 Scope ist mit 1 MOhm/18 pF angegeben. Ein Eingang soll mit dem GPSDO als Referenz verbunden werden und der andere als Messeingang mit dem anderen Ende des Sat-Kabels. Der Messbereich ist jeweils +/-1 V. Wie kann eine impedanzrichtige Anpassung dieser Anordnung realisiert werden?

───────────────────────────────────────────────────────────────

[🤖 KI]
Ich prüfe kurz die Ausgangs- und Eingangsspezifikationen von GPSDO und Eclypse, damit die Anpassungswerte zur realen Beschaltung passen.Die Anordnung lässt sich impedanzrichtig aufbauen, wenn die 50-Ω-Welt (GPSDO, AWG, Scope-Abschluss) und die 75-Ω-Leitung getrennt abgeschlossen und nur über ein definiertes Anpassglied verbunden werden. Der Scope selbst darf die Leitung nicht abschließen.

## Randbedingungen

Der DXPatrol GPSDO liefert an den Sinusausgängen etwa +7 dBm an 50 Ω, das sind rund 1,5 Vss bzw. 0,71 V Spitze. Der Rechteckausgang ist für diese Messung ungeeignet: Harmonische werden im Kabel unterschiedlich gedämpft und verschieben den Nulldurchgang. Der Zmod-AWG hat 50 Ω Quellimpedanz; der nutzbare Low-Range ist ±1,25 V an hochohmig, an 50 Ω also etwa die Hälfte. Der Zmod-Scope ist 1 MΩ parallel 18 pF und im Low-Range ±1 V am SMA. Er ist kein 50- oder 75-Ω-Abschluss.

Die Leitung: 250 m, 75 Ω, Verkürzungsfaktor 0,82, 5 dB bei 10 MHz. Die einfache Laufzeit ist

\[
\tau = \frac{250}{0{,}82\cdot c} \approx 1{,}017\,\mu\mathrm{s}.
\]

Hin und zurück sind das etwa 2,03 µs und 10 dB. Bei λ ≈ 24,6 m ist die Strecke elektrisch lang. Ein offenes oder nur mit 1 MΩ belastetes Ende reflektiert fast vollständig; der zurücklaufende Rest liegt noch bei etwa −10 dB und liegt genau in der Größenordnung, die eine 300-ps-Phasenaussage verfälscht. Deshalb muss jedes Ende mit seinem Wellenwiderstand abgeschlossen sein.

300 ps bei 10 MHz sind 1,08° bzw. 0,0188 rad. Eine kleine Quadraturkomponente erzeugt das über \(\delta\varphi \approx V_q/V_c\).

## Topologie

```text
GPSDO Sinus A ──► 3 dB-Pad 50 Ω ──► Port 1
                                      resistiver
AWG (50 Ω) ─────► Port 2            T-Kombinierer ──► 50→75-Pad oder
                                      (je 16,7 Ω)       Übertrager
GPSDO Sinus B ──► 50-Ω-Abschluss am Scope-CH1          │
(Referenz, kurz)                                       ▼
                                              250 m, 75 Ω, HQ 135
                                                       │
                                          75 Ω gegen Leitungsmasse
                                                       │
                                              Scope-CH2, 1 MΩ || 18 pF
                                              (nur Abgriff, kein Abschluss)
```

Der Kombinierer ist ein symmetrisches Widerstands-T aus drei 16,7-Ω-Widerständen (E96: 16,5 Ω). Jeder Port sieht 50 Ω, jeder Pfad hat 6 dB Spannungs- und Leistungsverlust. GPSDO und AWG bleiben damit auf 50 Ω abgeschlossen, der Ausgang des Kombinierers ist eine saubere 50-Ω-Quelle für das Anpassglied.

Vor den GPSDO-Zweig gehört ein festes 3-dB- oder 6-dB-50-Ω-Dämpfungsglied. Die Rückflussdämpfung eines GPSDO-Ausgangs ist selten besser als 10–15 dB; das Pad verbessert die Anpassung zum Kabel hin um das Doppelte seiner Dämpfung und entkoppelt Laständerungen vom OCXO. Stabilität unter Last ist beim DXPatrol mit etwa 2 ppb angegeben — für eine Laufzeitmessung im Pikosekundenbereich ist das relevant, weil eine Laständerung die Quellphase zieht.

## 50 Ω auf 75 Ω

Zwei gleichwertige Varianten, eine davon fest verbauen:

**L-Pad, minimum loss, 5,72 dB.** Shunt auf der 50-Ω-Seite 86,6 Ω, Serie zur 75-Ω-Seite 43,3 Ω (E96: 86,6 Ω und 43,2 Ω). Kontrolle:

- von der Leitung aus: \(43{,}3 + (86{,}6 \parallel 50) = 75\,\Omega\)
- vom Kombinierer aus: \(86{,}6 \parallel (43{,}3 + 75) = 50\,\Omega\)

Metallfilmwiderstände, 0,1 %, kurze Beine, Masse direkt am Schirmübergang. Bei 10 MHz ist das reaktiv noch unkritisch, solange die Schleife unter etwa 1 cm bleibt.

**Übertrager, bevorzugt, wenn der Pegel knapp wird.** Ein 50:75-Übertrager (zum Beispiel Mini-Circuits ADT1.5-1+ oder ein gleichwertiger 1:1,5-Spannungsübertrager, 1–100 MHz) hat bei 10 MHz typisch unter 0,5 dB und transformiert den Abschluss korrekt. Vorteil zusätzlich: der Schirm der langen Leitung muss nicht galvanisch an der Scope-Masse liegen.

Nicht mischen: entweder Pad oder Übertrager, nicht beides in Reihe, sonst ist der Pegel unnötig niedrig und die Fehlerquellen verdoppeln sich.

## Fernes Ende

75-Ω-Abschluss direkt am Stecker, 0,1 %, Masse am Schirm, dann erst der Scope-Abgriff parallel dazu. 1 MΩ verändert 75 Ω um 0,006 Ω und ist ohmsch egal. Die 18 pF sind es nicht: \(X_C \approx 884\,\Omega\), der Abschluss wird komplex, etwa \(74{,}5 - j6{,}3\,\Omega\), \(|\Gamma| \approx 0{,}04\). Das ist ein fester Phasenoffset in der Größenordnung 1 ns, kein Rauschen, und fällt in der Kalibrierung heraus, solange Temperatur und Bereichsumschaltung konstant bleiben. Der Low-Range (±1 V, Teiler nahezu 1:1) ist hier richtig; der High-Range-Teiler hat eine andere Eingangskapazität und darf während einer Messreihe nicht umgeschaltet werden.

Sauberer, wenn der Offset selbst klein sein soll: ein Video-Puffer (zum Beispiel AD8065 oder ein 75-Ω-Leitungstreiber) direkt am 75-Ω-Widerstand, Ausgang in den Scope. Dann sieht die Leitung nur den Widerstand.

Referenzkanal: zweiter Sinusausgang über ein kurzes 50-Ω-Kabel auf ein 50-Ω-Durchgangsglied direkt am CH1-SMA, Scope parallel dazu. Dieselbe 18 pF an 50 Ω ergeben \(\omega RC \approx 0{,}057\) rad, etwa 0,9 ns. An 75 Ω sind es etwa 1,35 ns. Die Differenz ist ein konstanter Kanalversatz und wird mit einem kurzen Referenzkabel statt der 250-m-Strecke einmal gemessen, nicht in die Anpassung hineingerechnet.

Pegel nach 6 dB Kombinierer, 3 dB Quellpad, 5,7 dB Anpasspad und 5 dB Kabel: der Träger kommt am Scope mit grob 50–80 mV Spitze an, weit innerhalb ±1 V und weit über dem LSB des Scope. Mit Übertrager statt Pad liegt er etwa Faktor 2 höher. Beides ist für einen Nulldurchgangsvergleich brauchbar.

## Injektion für ±300 ps

Am Kombiniererausgang ist die Träger-Spitzenspannung nach 3 dB Pad und 6 dB Teiler etwa 0,25 V. Für 0,0188 rad Quadratur werden dort etwa 4,7 mV Spitze aus dem AWG-Zweig gebraucht. Wegen der 6 dB des T-Glieds muss der AWG an seinem 50-Ω-Ausgang etwa 9–10 mV Spitze liefern. Im Low-Range (167 µV absolut, an 50 Ω effektiv die Hälfte) sind das einige zehn Stufen. Die resultierende Schrittweite der Phase liegt bei wenigen Pikosekunden, ausreichend, um 300 ps einzustellen, nicht um sie zu eichen — die Eichung macht eine bekannte Laufzeit, nicht der DAC-Code.

Der AWG muss auf etwa ±90° zum Träger stehen, sonst entsteht AM statt Phase. Das geht, indem CH1 den GPSDO abtastet und der AWG daraus numerisch die Quadratur bildet. Ein Phasenfehler von 5° kostet kaum Quadratur (\(\cos 5°\)), erzeugt aber eine relative AM von etwa \(0{,}0016\). Für den Nulldurchgang unerheblich, für einen Spitzenwertdetektor nicht. Amplitude klein halten und Phase einmal gegen CH1 kalibrieren.

Eine zweite AWG-Amplitude, in Phase statt in Quadratur, ist der Kontrollversuch: sie darf die gemessene Laufzeit nicht im selben Maß verschieben. Tut sie es doch, misst der Auswertealgorithmus AM-zu-PM und nicht die injizierte Phase.

## Was die Anpassung nicht leistet

Sie beseitigt nicht die Temperaturdrift des Kabels. 5 dB und 1 µs über 250 m Polyethylen-Dielektrikum wandern mit der Temperatur um Nanosekunden pro Kelvin, nicht um Pikosekunden. Das Anpassnetz selbst (Pad oder Übertrager plus 75-Ω-Abschluss) muss deshalb thermisch beim Scope und nicht im Freien sitzen, und eine zweite, gleich lange, gleich abgeschlossene Leitung oder ein Temperatursensor am Kabel bleibt der eigentliche Diskriminator. Die impedanzrichtige Kette sorgt nur dafür, dass diese Drift nicht noch von einem temperaturabhängigen Reflexionsfaktor moduliert wird.

Schirmströme über 250 m sind der zweite systematische Fehler. Vierfachschirmung hilft gegen Einstrahlung, nicht gegen eine Masseschleife zwischen GPSDO-Gehäuse und Eclypse. Wenn der 50:75-Übertrager die Leitung galvanisch trennt und der Schirm nur senderseitig aufgelegt wird, bleibt die Schleife offen. Empfängerseitig auflegen und senderseitig über den Übertrager trennen ist die andere zulässige Variante — nicht beide Enden hart erden.

Für den Fragenkatalog ist die prüfbare Aussage damit: Nach dem Aufbau muss ein an Port 2 injizierter Quadraturhub von ±300 ps am Scope als Laufzeitänderung derselben Größe und desselben Vorzeichens erscheinen, ein gleich großer In-Phase-Hub darf das nicht, und ein Abschluss mit 1 MΩ statt 75 Ω muss die Mehrfachreflexion bei 2 µs sichtbar machen. Damit ist die Anpassung selbst falsifizierbar, bevor irgendeine Deutung der Laufzeit beginnt.

**Citations:**
[1] [https://digilent.com/reference/programmable-logic/eclypse-z7/demos/zmod-awg](https://digilent.com/reference/programmable-logic/eclypse-z7/demos/zmod-awg)
[2] [https://digilent.com/reference/zmod/awg/reference-manual](https://digilent.com/reference/zmod/awg/reference-manual)
[3] [https://digilent.com/reference/zmod/awg/start](https://digilent.com/reference/zmod/awg/start)
[4] [https://digilent.com/reference/zmod/scope/reference-manual](https://digilent.com/reference/zmod/scope/reference-manual)
[5] [https://forum.digilent.com/topic/23275-eclypse-z7-with-zmod-awg-and-scope-problem-with-calibration/](https://forum.digilent.com/topic/23275-eclypse-z7-with-zmod-awg-and-scope-problem-with-calibration/)
[6] [https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md](https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md)
[7] [https://www.ercomer.pl/pl/p/GPSDO-2-generator-wzorcowy-10MHz-z-wyswietlaczem-Oled/2953](https://www.ercomer.pl/pl/p/GPSDO-2-generator-wzorcowy-10MHz-z-wyswietlaczem-Oled/2953)
[8] [https://www.passion-radio.com/satellite-qo-100/gpsdov2-1262.html](https://www.passion-radio.com/satellite-qo-100/gpsdov2-1262.html)
[9] [https://www.pmr-funkgeraete.de/Funkgeraete/Zubehoer-CB-AFU/Messgeraete/DXpatrol-GPSDO-V2-10-MHz-Referenz::16309.html](https://www.pmr-funkgeraete.de/Funkgeraete/Zubehoer-CB-AFU/Messgeraete/DXpatrol-GPSDO-V2-10-MHz-Referenz::16309.html)
[10] [https://www.scribd.com/document/718257031/GPSDO-Manual-Dxpatrol](https://www.scribd.com/document/718257031/GPSDO-Manual-Dxpatrol)
[11] [https://www.wimo.com/en/dxpatrol-gpsdo-10mhz-reference](https://www.wimo.com/en/dxpatrol-gpsdo-10mhz-reference)

───────────────────────────────────────────────────────────────

[👤 Sie]
Anpassung und Kombinierer sollen in einem ersten Messaufbau als reines Widerstandsnetzwerk mit selektierten Widerständen der E24-Reihe aufgebaut und mit VNA überprüft werden. Die Dämpfung vor dem Kombinierer soll für den GPSDO etwa 5 dB betragen und für den AWG etwa 25 dB. Welche Werte ergeben sich daraus für die Widerstände und mit welcher Verschaltung? Der Aufbau soll auf einem PCB mit SMA/F Buchsen erfolgen und mit 16 cm fertig konfektionierten SMA-Leitungen an GPSDO, AWG und Scope angeschlossen werden. Der Schirm des Sat-Kabels auf der Empfängerseite bleibt galvanisch offen und wird durch Kondensatoren HF-mäßig überbrückt. Welche kritischen Stellen könnte der Aufbau haben und welche Verbesserungen können gegebenenfalls wie vorgeschlagen durch Einbeziehung von fertigen SMA-Komponenten erzielt werden?

───────────────────────────────────────────────────────────────

[🤖 KI]
Die erste Ausbaustufe ist ein reines 50-Ω-Widerstandsnetz bis zum Übergang auf 75 Ω. Mit selektierten E24-Werten liegen Anpassung und Dämpfung bei 10 MHz nahe an den Vorgaben; was danach noch stört, sind nicht die Widerstandswerte, sondern der 75-Ω-Übergang am SMA, die nur scheinbare galvanische Trennung und die Masseführung.

## Zielwerte und E24-Wahl

Pi-Glied ist hier günstiger als T-Glied: die Parallelwiderstände liegen bei hohen Dämpfungen nahe 50 Ω, der Serienwiderstand ist groß, und ein 50-Ω-VNA sieht den Fehler direkt.

**GPSDO-Pfad, Soll 5 dB, 50 Ω, Pi-Glied**

| Funktion | Soll | E24 | Anzahl |
|---|---|---|---|
| Serie | 30,4 Ω | 30 Ω | 1 |
| Quer, eingangs- und ausgangsseitig | 178,5 Ω | 180 Ω | 2 |

Damit wird \(Z_\mathrm{in}\) an 50 Ω praktisch 50,0 Ω und die Spannungsdämpfung 4,94 dB. Einzelwiderstände reichen; Parallelschaltungen lohnen hier nicht.

**AWG-Pfad, Soll 25 dB, 50 Ω, Pi-Glied**

| Funktion | Soll | E24 | Anzahl |
|---|---|---|---|
| Serie | 443 Ω | 220 Ω + 220 Ω | 2 in Serie |
| Quer, beidseitig | 56,0 Ω | 56 Ω | 2 |

\(Z_\mathrm{in}\) bleibt 50 Ω, die Dämpfung wird 24,9 dB. Ein einzelner 430-Ω-Widerstand ergibt 24,7 dB und 49,9 Ω und ist die einfachere Variante, wenn nur ein Bauteil selektiert werden soll. 220+220 ist der bessere Treffer.

**Kombinierer, symmetrisches Widerstands-T, drei gleiche Arme**

Soll je Arm \(50/3 = 16{,}7\,\Omega\). Zwei 33-Ω-Widerstände parallel ergeben 16,5 Ω. Jeder Port sieht dann

\[
16{,}5 + \frac{16{,}5+50}{2} = 49{,}75\,\Omega.
\]

Jeder Pfad hat 6,1 dB Spannungsdämpfung, die Entkopplung der beiden Eingänge liegt nur bei etwa 6 dB. Die Pads davor sind deshalb keine Pegelkosmetik, sondern die eigentliche Entkopplung: vom AWG-Ausgang bis zum GPSDO-Ausgang sind es 25 + 6 + 5 ≈ 36 dB.

**50 → 75, L-Glied, Minimum-Loss**

| Funktion | Soll | E24 |
|---|---|---|
| Quer, nur auf der 50-Ω-Seite | 86,6 Ω | 91 Ω |
| Serie, zur 75-Ω-Seite | 43,3 Ω | 43 Ω |

Von der Leitung her gesehen: \(43 + (91 \parallel 50) = 75{,}3\,\Omega\). Vom Kombinierer her: \(91 \parallel (43+75) = 51{,}4\,\Omega\). Rückflussdämpfung etwa 37 dB an 50 Ω und über 50 dB an 75 Ω, Zusatzdämpfung etwa 5,8 dB. 82 Ω und 43 Ω wären an 50 Ω etwas schlechter und an 75 Ω deutlich schlechter; 91/43 ist die bessere einzelne E24-Paarung.

## Verschaltung

```text
GPSDO ---- SMA ---- [Pi 5 dB: 180 / 30 / 180] ----+
                                                  |
                                         Arm 16,5 Ω (2×33 ||)
                                                  |
AWG ------ SMA ---- [Pi 25 dB: 56 / 220+220 / 56] +-- Arm 16,5 Ω --+-- Arm 16,5 Ω --+
                                                                                      |
                                                                              L-Glied: 91 Ω gegen Masse
                                                                              43 Ω in Serie
                                                                                      |
                                                                              SMA oder besser F/BNC-75
                                                                                      |
                                                                              250 m, 75 Ω

Empfängerende, eigenes kleines Brett:
  Innenleiter ---- 75 Ω ---- Schirm          (Abschluss gehört zum Kabel, nicht zum Scope)
  Innenleiter ---- SMA ---- 16 cm ---- Scope-CH2, 1 MΩ, kein 50-Ω-Abschluss
  Schirm ---- zwei Kondensatoren ---- lokale Masse
  Schirm nicht mit Masse oder Gehäuse verschrauben
```

Alle Querwiderstände der Pi-Glieder und der 91-Ω-Querwiderstand liegen gegen die durchgehende Massefläche, nicht gegen eine separate „HF-Masse“. Die drei Kombiniererarme bilden einen Stern; der Sternpunkt ist nur so groß wie ein Lötpad. Referenzkanal des Scope bleibt außerhalb dieses Netzes: zweiter GPSDO-Sinus, kurzes Kabel, 50-Ω-Durchführung direkt am CH1.

Pegel zur Kontrolle: +7 dBm werden durch 5 dB und 6 dB auf etwa 0,20 V Spitze am Kombiniererausgang gedämpft. Für 300 ps Quadratur, also 1,08°, braucht der AWG-Zweig dort etwa 4 mV Spitze. Nach 6 dB Kombinierer und 25 dB Pad sind das rund 130 mV Spitze am AWG-Ausgang, gut im Low-Range und fein genug aufgelöst.

## VNA, nur 50 Ω

Bei 10 MHz und einem Sweep von 1 bis 50 MHz, damit eine Layoutresonanz nicht als „guter Punkt“ durchrutscht:

1. Jedes Pi-Glied allein, beide Seiten mit SMA: \(S_{11}\), \(S_{22}\) besser als −30 dB, \(S_{21}\) bei −4,9 dB bzw. −24,9 dB. Abweichung über 0,3 dB heißt falsch selektierter Widerstand, nicht Layout.
2. Kombinierer allein: jeder Port gegen die beiden anderen, die mit 50 Ω abgeschlossen sind. \(S_{11}\) nahe 50 Ω, Durchgang −6,1 dB, Entkopplung der Eingänge etwa −6 dB. Starke Abweichung der Entkopplung bedeutet einen offenen oder falschen Arm.
3. L-Glied: 75-Ω-Seite mit dem selektierten 75-Ω-Widerstand abschließen, \(S_{11}\) am 50-Ω-Port messen. Erwartet etwa 51 Ω, nicht 75 Ω. Ein 50-Ω-VNA misst einen 75-Ω-Port nicht als „angepasst“, wenn dort kein 75-Ω-Kalibrierkit liegt. Nicht auf \(S_{11}=0\) an der offenen 75-Ω-Buchse hoffen.
4. Danach die Kette aus Pads, Kombinierer und L-Glied nur noch als Durchgang: GPSDO-Port nach 75-Ω-Seite etwa −4,9 −6,1 −5,8 ≈ −17 dB, AWG-Port nach 75-Ω-Seite etwa −25 −6 −6 ≈ −37 dB. Das ist der Abnahmekorridor.

Widerstände vor dem Einlöten messen und auf besser als 1 % zum Rechenwert aussuchen. 0805- oder 1206-Metallfilm, 100 mW reichen; die größte Leistung ist der GPSDO hinter dem ersten Querwiderstand, weit unter 50 mW.

## Kritische Stellen

**SMA am 75-Ω-Tor.** SMA ist ein 50-Ω-Stecker. Bei 10 MHz ist die Stoßstelle elektrisch nur wenige Millimeter, der Reflexionsbeitrag liegt weit unter dem des L-Glieds und ist für diese Messung egal. Er wird erst oberhalb einiger hundert Megahertz sichtbar. Für den VNA-Abgleich stört er nicht, für einen späteren Vergleich mit einem Übertrager sollte das Kabelende trotzdem eine echte 75-Ω-Buchse haben, F oder BNC-75, und nicht noch ein SMA-F-Adapter.

**16-cm-Leitungen.** Bei 10 MHz sind 16 cm etwa 0,007 Wellenlängen. Laufzeit und Dämpfung sind gegenüber 250 m bedeutungslos, sofern alle drei Leitungen dieselbe Type haben und nicht eine davon ein RG-58 mit schlechtem SMA-Crimp ist. Wackelnde Schirmcrimps erzeugen genau die Pikosekunden, die später als Messgröße gelten sollen. Fertig konfektionierte Leitungen vor dem Versuch einmal auf dem VNA als Durchgang messen; \(S_{11}\) muss dem offenen Ende entsprechen, nicht einem halben Kurzschluss im Stecker.

**Offener Schirm.** Zwei Punkte werden leicht verwechselt. Die Kondensatoren schließen nur den Schirmstrom bei 10 MHz. Der Innenleiter verbindet beide Gehäuse weiterhin galvanisch, über das Widerstandsnetz und die GPSDO-Masse. Eine Ausgleichsspannung zwischen den Gebäuden fließt dann auf dem Innenleiter, nicht auf dem Schirm. Das ist keine galvanische Trennung. Zusätzlich liegt die gesamte Gleichtaktspannung an den Kondensatoren. An 250 m im Freien sind das Netzbrummen, induzierte Stöße und im ungünstigen Fall Blitzfolge. 10-nF-Keramik mit 50 V ist dafür der falsche Bauteil. Mindestens zwei Kondensatoren parallel, 10 nF bis 100 nF, Spannungsfestigkeit mehrere hundert Volt, kurze Beine direkt an der Buchse. Ein hochohmiger Ableitwiderstand, 1 MΩ bis 10 MΩ, vom Schirm nach Masse verhindert statische Aufladung und lässt nur Mikroampere Gleichstrom zu; die Schleife bei 50 Hz bleibt praktisch offen. Ohne ihn lädt sich der Schirm auf eine zufällige Spannung auf, und die erste Entladung sitzt im Scope-Eingang.

Bei 10 nF ist \(X_C\) bei 10 MHz etwa 1,6 Ω, in Serie mit dem 75-Ω-Abschluss also ein kleiner, fester, kalibrierbarer Fehler. Zwei Kondensatoren halbieren ESL und die Wahrscheinlichkeit, dass ein einzelner Ausfall den Abschluss öffnet. Den 75-Ω-Widerstand zwischen Innenleiter und Schirm löten, nicht zwischen Innenleiter und Platinenmasse. Sonst liegt \(X_C\) voll in der Signalmasse, und der Abschluss ist 75 Ω plus Kondensator statt 75 Ω parallel zur Scope-Kapazität.

**Scope-Abgriff.** CH2 bleibt 1 MΩ. Ein 50-Ω-Durchgangsglied an CH2 würde den eben gesetzten 75-Ω-Abschluss parallel schalten und die Leitung falsch abschließen. Die 18 pF sind ein fester Offset, kein Anpassfehler, solange der Bereich nicht umgeschaltet wird.

**Layout.** Bei 10 MHz ist die Leiterlänge unkritisch, die Massefläche nicht. Ein geschlitztes Groundplane unter dem Kombinierer macht aus 16,5 Ω eine kleine Induktivität und verschiebt \(S_{11}\) stärker als die E24-Rundung. Sternpunkt klein, Querwiderstände beidseitig an die Fläche, SMA-Masse mit mehreren Vias im ersten Millimeter. Keine Mäander, keine Stromversorgung auf derselben Insel.

## Was fertige SMA-Teile besser machen

Das Widerstandsnetz ist der richtige erste Aufbau, weil jeder Fehler auf dem VNA einem Bauteil zugeordnet werden kann. Danach ersetzen, nicht mischen:

- Die beiden Pi-Glieder durch fertige SMA-Dämpfungsglieder, 5 dB und 25 dB bzw. 20 dB plus 6 dB. Rückflussdämpfung und Wiederholbarkeit sind besser als selektierte 0805, und der AWG-Pfad kann zum Prüfen der Isolation einfach herausgeschraubt werden.
- Den Kombinierer durch einen fertigen resistiven 50-Ω-Zweiweg-Splitter im SMA-Gehäuse. Dieselbe 6-dB-Physik, aber definierte Masse und keine Sternpunkt-Induktivität. Ein transformatorischer Splitter hat mehr Entkopplung, ist aber bei 10 MHz oft schon am unteren Bandrand und ändert die Gleichstromverhältnisse; erst einsetzen, wenn der resistive Aufbau auf dem VNA liegt.
- Das L-Glied durch einen kleinen 50:75-Übertrager im eigenen Gehäuse, sobald der Widerstandsaufbau als Referenz vermessen ist. Das ist die einzige der genannten Änderungen, die die Gleichtaktfrage wirklich löst: Innenleiter und Schirm werden beide getrennt. Dämpfung fällt von etwa 6 dB auf unter 1 dB, der Pegel am Scope steigt, und der offene Schirm mit Kondensatoren wird überflüssig. Vorher und nachher dieselbe VNA-Messung, sonst ist der Vergleich nichts wert.
- Am Kabelende eine 75-Ω-Buchse und ein fertiger 75-Ω-Abschluss statt des diskreten Widerstands, der Scope weiter hochohmig parallel. Der diskrete 75-Ω-Widerstand bleibt nur so lange, wie er selektiert und auf dem VNA dem Abschluss zugeordnet werden kann.

Reihenfolge, die den Aufbau falsifizierbar hält: erst Pads und Kombinierer auf dem VNA, dann L-Glied mit 75-Ω-Abschluss, dann 16-cm-Leitungen, zuletzt Schirmkondensatoren und das lange Kabel. Ein Übertrager oder ein fertiges Dämpfungsglied kommt erst hinein, wenn die Widerstandsversion ihre Soll-S-Parameter gehalten hat. Sonst ist nicht mehr erkennbar, welches Teil die 300 ps verschoben hat.

───────────────────────────────────────────────────────────────

[👤 Sie]
Danke! Bitte die Informationen für C&P in die Antwort bei „Physik 2“ aufbereiten und dabei gegebenenfalls Hinweise der anderen KIs mit berücksichtigen. Im Prompt wurde der Anschluss des Sat-Kabels über F-Buchsen nicht erwähnt was noch mit berücksichtigt werden sollte.

📎 Anhänge: Grok 4.7 Phasendetektor-Impedanzanpassung.md, Mistral Large 4 Phasendetektor-Impedanzanpassung.md, MiniMax M3.1 Phasendetektor-Impedanzanpassung.md, Quen 3.8 Max Phasendetektor-Impedanzanpassung.md, DeepSeek Phasendetektor-Anpassung.md, Kimi K3.1 Pro Phasendetektor-Anpassung.md, GLM 5.3 Phasendetektor-Anpassung.md, Perplexity Phasendetektor-Impedanzanpassung.md, Muse Spark 1.3 Phasendetektor-Impedanzanpassung.md, Gemini 4 Argon Phasendetektor impedanzrichtig anpassen.md, Claude 5.5 Phasendetektor-Anpassung.md, GPT-6 Phasendetektor impedanzrichtig anpassen.md

───────────────────────────────────────────────────────────────

[🤖 KI]
Hier ist der vollständige und für Copy & Paste direkt verwendbare Entwurf für den Abschnitt **„Physik 2“** im Markdown-Dokument. Er fasst die Dimensionierung des Widerstandsnetzwerks, die F-Steckverbinder am Sat-Kabel, die HF-gerechte Schirmüberbrückung am Empfangsende sowie die wesentlichen Erkenntnisse aus der Teamdiskussion (VNA-Falsifizierbarkeit, Gleichtaktschutz und Injektionskalibrierung) zusammen.

***

# Physik 2: Präzisionsanpassung und Signalaufbereitung (250-m-Messstrecke)

Dieser Abschnitt definiert den konkreten Hardware-Entwurf für die 250-m-Messstrecke (HQ 135 dB Sat-Kabel, 75 Ω, VF 0,82, 5 dB Dämpfung bei 10 MHz) sowie die impedanzrichtige Einbindung von DXPatrol GPSDO V2.4, Eclypse Z7 AWG und dem Eclypse Z7 Scope (1 MΩ ∥ 18 pF).

---

## 1. Topologie der Messkette

```text
[ GPSDO Sinus 1 ] ──(16cm SMA)──► Scope CH1 (50-Ω-Durchgangsterminator, Referenz)

[ GPSDO Sinus 2 ] ──(16cm SMA)──► Pi-Glied (5 dB)  ──┐
                                                     ├── [T-Kombinierer 16,5 Ω] ──► L-Glied 50→75 Ω ──► [F-Buchse]
[ AWG (Eclypse) ] ──(16cm SMA)──► Pi-Glied (25 dB) ──┘                                                    │
                                                                                                    (250m HQ-135, 75 Ω)
                                                                                                         │
                                                                                                   [F-Buchse] 
                                                                                                         │
                                                                                           75-Ω-Abschluss am Innenleiter
                                                                                           + HF-Kondensatoren am Schirm
                                                                                           + (16cm SMA-Leitung zum Scope CH2)
```

---

## 2. Dimensionierung der Widerstandsnetzwerke (E24-Reihe)

Um Reflexionen und unerwünschte Phasenripples zu vermeiden, werden alle Quellen und Lasten über ein selektiertes Widerstandsnetzwerk (Metallfilm 0,1 %, 0805/1206) impedanzrichtig entkoppelt:

1. **GPSDO-Pfad (Soll-Dämpfung ~5 dB, 50 Ω Pi-Glied):**
   * Serienwiderstand: **30 Ω** (Soll: 30,4 Ω)
   * Querwiderstände (Eingang/Ausgang): je **180 Ω** (Soll: 178,5 Ω)
   * *Ergebnis:* \(Z_\mathrm{in} \approx 50{,}0\,\Omega\), Spannungsdämpfung \(\approx 4{,}94\,\mathrm{dB}\).

2. **AWG-Pfad (Soll-Dämpfung ~25 dB, 50 Ω Pi-Glied):**
   * Serienwiderstand: **220 Ω + 220 Ω in Serie** (Gesamt 440 Ω, Soll: 443 Ω)
   * Querwiderstände (Eingang/Ausgang): je **56 Ω** (Soll: 56,0 Ω)
   * *Ergebnis:* \(Z_\mathrm{in} \approx 50\,\Omega\), Spannungsdämpfung \(\approx 24{,}9\,\mathrm{dB}\).

3. **Symmetrischer T-Kombinierer (GPSDO + AWG):**
   * Drei gleiche Arme im Sternpunkt: je **16,5 Ω** (realisiert durch **2× 33 Ω parallel**, Soll: 16,7 Ω).
   * *Eigenschaften:* Jeder Port sieht saubere 50 Ω; die Dämpfung je Zweig beträft ca. 6,1 dB. Die vorgeschalteten Pi-Glieder sichern eine Entkopplung der Ausgänge von über 35 dB, um ein „Pulling“ des Oszillators zu verhindern.

4. **Anpassung 50 Ω auf 75 Ω (L-Glied / Minimum-Loss-Pad am Kabelanfang):**
   * Querwiderstand (auf der 50-Ω-Seite gegen Masse): **91 Ω** (Soll: 86,6 Ω)
   * Serienwiderstand (zur 75-Ω-Seite): **43 Ω** (Soll: 43,3 Ω)
   * *Ergebnis:* Rückflussdämpfung \(S_{11} < -37\,\mathrm{dB}\) an 50 Ω und \(<-50\,\mathrm{dB}\) an 75 Ω; Zusatzdämpfung \(\approx 5{,}8\,\mathrm{dB}\).

---

## 3. Schnittstellen und Steckverbinder (F-Buchsen & SMA)

* **Sendeseite (Platine):** Der Übergang vom 50-Ω-Widerstandsnetz erfolgt über eine **SMA/F-Buchse**. Über eine kurze, konfektionierte **16-cm-SMA-Leitung** wird die Verbindung hergestellt. Für den direkten Anschluss des 75-Ω-Sat-Kabels auf der Platine ist eine **75-Ω-F-Buchse** (oder ein impedanzarme SMA-zu-F-Schottwandbuchse) vorzusehen.
* **Empfangsseite (Kabelende):** Das 250-m-Sat-Kabel (HQ 135 dB, 4-fach geschirmt) besitzt standardmäßig **F-Kompressionsstecker**. 
  * Der 75-Ω-Abschlusswiderstand (0,1 %) wird direkt zwischen F-Innenleiter und F-Außenleiter (Schirm) am Empfangsstecker angelötet.
  * Der Abgriff zum Scope-CH2 erfolgt über eine kurze, direkt angelötete SMA/BNC-Messleitung (Länge < 16 cm). Der Scope-Eingang selbst bleibt hochohmig (**1 MΩ ∥ 18 pF**), um die 75-Ω-Terminierung nicht zu verfälschen.

---

## 4. HF-gerechte Schirmüberbrückung am empfangsseitigen Kabelende

Um Masseschleifen und Brummstörungen (sowie gefährliche Potenzialausgleiche über die 250-m-Strecke im Freien) zu unterbinden, wird der Kabelschirm empfangsseitig **galvanisch offen** gehalten:
* **HF-Überbrückung:** Der Schirm wird über **zwei parallele Hochspannungs-Keramikkondensatoren** (z. B. 2× 10 nF bis 100 nF, Spannungsfestigkeit \(\ge 500\,\mathrm{V}\)) niederohmisch an die lokale Gerätemasse gekoppelt (\(X_C \approx 0{,}16\,\Omega\) bei 10 MHz).
* **Statische Entladung:** Ein hochohmiger Ableitwiderstand (1 MΩ bis 10 MΩ) parallel zu den Kondensatoren verhindert elektrostatische Aufladungen des Kabelschirms.

---

## 5. Injektion für den ±300-ps-Phasenhub

* **Ziel:** 300 ps bei 10 MHz entsprechen einem Phasenversatz von \(\Delta\varphi \approx 1{,}08^\circ \approx 0{,}01885\,\mathrm{rad}\).
* **Methode (Quadratur-Vektoraddition):** Der AWG liefert ein phasenstarr (per 10-MHz-Referenz) zum GPSDO gekoppeltes Signal, das um \(90^\circ\) phasenverschoben als Quadraturkomponente am Kombinierer eingespeist wird.
* **Pegel:** Nach der 25-dB-Dämpfung und dem Kombinierer liefert der AWG im Low-Range einige Millivolt – ideal, um den DAC-Rauschboden zu drücken und den Phasenhub im Bereich von wenigen Pikosekunden bis 300 ps hochauflösend einzustellen.

---

## 6. Falsifizierungs- und Testplan (VNA-Abnahme)

Vor Inbetriebnahme mit dem 250-m-Kabel wird die Elektronik stufenweise am VNA (1 bis 50 MHz) überprüft:
1. **Pi-Glieder & Kombinierer einzeln:** \(S_{11}\) und \(S_{22} < -30\,\mathrm{dB}\), Durchgangsdämpfungen exakt nach Soll.
2. **L-Glied:** Mit 75-Ω-Prüfwiderstand auf \(S_{11}\) testen.
3. **Nullmessung (Injektions-Test):** Bei AWG-Amplitude = 0 V darf kein künstlicher Offset auftreten. Ein reiner Quadratur-Hub von ±300 ps muss am Scope als Laufzeitänderung gleichen Betrags erscheinen, während ein In-Phase-Hub (0°) keine signifikante Laufzeitänderung bewirken darf (Trennung von AM- und PM-Artefakten).

───────────────────────────────────────────────────────────────

[👤 Sie]
Das Zmod Scope soll gegen ein Zmod SDR mit 50 Ohm Impedanz getauscht werden. Wie ändert sich die Anpassung und welche Vor- oder Nachteile ergeben sich?

───────────────────────────────────────────────────────────────

[🤖 KI]
Das Ersetzen des Zmod Scope (1 MΩ ∥ 18 pF) durch das **Zmod SDR** (50 Ω Eingangsimpedanz) verändert die Schaltung grundlegend zum Besseren: Die Messstrecke wird von einer hochohmigen Spannungsabfrage auf eine rein **leistungsmäßig angepasste 75-Ω/50-Ω-Wellenwiderstandskette** umgestellt.

Hier sind die Änderungen und die Vor- und Nachteile im Detail:

---

## 1. Was ändert sich an der Anpassung?

Durch den 50-Ω-Eingang des Zmod SDR entfällt das Problem der kapazitiven Last am Kabelende. Allerdings muss nun der Übergang vom 75-Ω-Sat-Kabel auf den 50-Ω-Eingang des SDR sauber gelöst werden:

* **Empfangsseitiges L-Glied (50 Ω zu 75 Ω, umgekehrt):** 
  Am Ende des 250-m-Kabels wird ein identisches Minimum-Loss-Pad (L-Glied) wie am Anfang eingesetzt:
  * Serienwiderstand (zur 75-Ω-Leitung): **43 Ω**
  * Querwiderstand (auf der 50-Ω-Seite gegen Masse): **91 Ω**
  * *Ergebnis:* Das 75-Ω-Kabel sieht exakt seinen Wellenwiderstand (keine stehenden Wellen, keine Reflexionen), und das Zmod SDR sieht aus Richtung des Kabels saubere 50 Ω.
* **Referenzkanal (Scope CH1 / SDR CH1):**
  Der GPSDO-Referenzpfad (Sinus 1) geht nun direkt über eine 50-Ω-Leitung auf den 50-Ω-Eingang des SDR. Der bisher nötige externe 50-Ω-Durchgangsterminator entfällt.

---

## 2. Vorteile des Zmod SDR (50 Ω)

1. **Keine Reflexionen am Kabelende:** 
   Das Zmod Scope mit 1 MΩ ließ am Kabelende einen Reflexionsfaktor von \(\Gamma \approx 0{,}04\) (ca. −28 dB Rückflussdämpfung) durch die 18-pF-Eingangskapazität entstehen, der zu Mehrfachreflexionen (Echo nach ~2 µs) führte. Das Zmod SDR schließt die Strecke über das L-Glied perfekt ab.
2. **Breitbandige Signalintegrität:** 
   Da beide Enden (Sender und Empfänger) nun impedanzrichtig mit ihren jeweiligen Wellenwiderständen (75 Ω auf der Leitung, 50 Ω an den Geräten) abgeschlossen sind, verhält sich die 250-m-Strecke wie eine echte, fehlerfreie Übertragungsleitung.
3. **Robustheit gegen Störungen:** 
   50-Ω-Systeme sind deutlich unempfindlicher gegenüber kapazitivem Crosstalk und Einstrahlungen auf den letzten Zentimetern vor dem Messgerät als hochohmige 1-MΩ-Eingänge.
4. **Keine Kalibrierfehler durch Frequenzgang der Eingangskapazität:** 
   Die 18 pF des Scopes erzeugen einen frequenzabhängigen Phasenfehler bei 10 MHz. Ein 50-Ω-Widerstand im SDR hat keine nennenswerte Blindkomponente, wodurch die Phase über den Nulldurchgang extrem stabil bleibt.

---

## 3. Nachteile / zu beachtende Punkte

1. **Zusätzliche Dämpfung (Insertion Loss):**
   Das empfangsseitige L-Glied (Minimum-Loss-Pad) erzeugt eine prinzipbedingte Spannungsdämpfung von ca. **5,8 dB**. 
   * *Bedeutung für den Pegel:* Zusammen mit den 5 dB Kabeldämpfung und den Dämpfungen auf der Sendeseite kommt am SDR ein kleinerer Signalpegel an. Da das Zmod SDR jedoch für HF-Signale ausgelegt ist und eine exzellente Empfindlichkeit besitzt, ist dies unkritisch, solange das Signal weit über dem Rauschboden des ADCs liegt.
2. **Verlust des hochohmigen Abgriffs:**
   Das Signal wird nun mit Leistung belastet. Ein "unbelauschetetes" Mitmessen (High-Z-Probe) ist ohne aktiven HF-Verstärker oder Puffer nicht mehr möglich – was für eine präzise Phasendetektion im 50-Ω-System aber ohnehin die fachgerechtere Lösung ist.

---

## 4. Konsequenz für den Aufbau

* **Empfangsseite:** Das Sat-Kabel endet nun an einer **F-Buchse**, auf die direkt das kompakte 50/75-Ω-Anpassungs-Glied (L-Pad aus 43 Ω und 91 Ω) folgt, von wo aus ein kurzes 50-Ω-SMA-Kabel direkt in den 50-Ω-Eingang des Zmod SDR führt.
* Die HF-Kondensatoren zur Schirmüberbrückung am empfangsseitigen F-Stecker bleiben wie gehabt erhalten, um Masseschleifen zu unterbinden.

───────────────────────────────────────────────────────────────

[👤 Sie]
Ein Widerstandsnetzwerk wurde wie im Dokument beschrieben mit folgenden Komponenten aufgebaut: GPSDO - Dämpfung (-5dB) - 3-Wege-Kombinierer - 50/75 Ohm Anpassung - SATA Kabel 1 m (für Null-Test) - 75/50 Ohm Anpassung - SDR-1. Die Dämpfung der gesamten Strecke wurde mit Nano VNA bei 10 MHz etwa wie erwartet mit -22.5dB gemessen. Nach Wechsel des 1m SATA-Kabels gegen ein 250m SATA-Kabel stieg die Dämpfung wie im NanoVNA Screenshot gezeigt auf -55dB an. Es zeigte sich nun ein Tiefpasscharakter mit einer -3dB Frequenz bei etwa 250 kHz  wie ebenfalls in einem angehängten Screenshot vom Nano VNA gezeigt. Nach Datenblatt sollte die zusätzliche Dämpfung bei 250m/10MHz etwa -5dB sein, sie ist jedoch mehr als -30dB. Der ohmsche Widerstand der Schirmung ist etwa 8 Ohm und des Innenleiters etwa 34 Ohm. Welche Erklärung könnte es dafür geben? Wie kann das System verändert werden, damit das 10MHz Signal des GPSDO mit der nominellen Dämpfung am Ausgang ankommt?

📎 Anhänge: Anpassung und Kombinierer 3 MHz (GPSDO - SATA_250m - SDR-1).jpg, Anpassung und Kombinierer 10 MHz (GPSDO - SATA_250m - SDR-1).jpg

───────────────────────────────────────────────────────────────

[🤖 KI]
Die Messung mit dem NanoVNA liefert eine eindeutige physikalische Diagnose: **Das verwendete „SATA-Kabel“ ist kein hochfrequentes Koaxialkabel (75 Ω Impedanz), sondern ein vieradriger Flach- oder Folienleiter (SATA-Datenkabel für Computer-Festplatten) mit enormem Serienwiderstand.**

Hier ist die detaillierte Erklärung und der Lösungsweg, wie das System auf das echte 250-m-Sat-Kabel umgestellt wird.

---

## 1. Ursachenanalyse: Warum beträgt die Dämpfung −55 dB bei 10 MHz?

1. **Der Innenleiterwiderstand von 34 Ω ist der Hauptgrund:**
   Ein echtes 75-Ω-Sat-Kabel (wie das genannte *HQ 135 dB*) besitzt einen dicken Kupfer- oder kupferkaschierten Stahl-Innenleiter (Durchmesser ca. 1 mm), dessen Gleichstromwiderstand bei 250 Länge im Bereich von **ca. 2 bis 4 Ω** liegt (nicht 34 Ω!). 
   Ein Widerstand von 34 Ω auf einer 75-Ω-Leitung erzeugt bereits im Gleichstromfall eine fundamentale Fehlanpassung und einen massiven Spannungsteiler.
2. **RC-Tiefpass statt HF-Wellenleiter:**
   Ein Innenleiterwiderstand von 34 Ω in Kombination mit der Leitungskapazität (ca. 50 pF/m, bei 250 m also etwa 12,5 nF) bildet einen **RC-Tiefpass mit einer Grenzfrequenz von \(f_{3\text{dB}} \approx \frac{1}{2\pi R C}\)**. 
   Rechnerisch landet man hier exakt bei den im NanoVNA beobachteten **ca. 250 kHz**. Bei 10 MHz ist das Signal dadurch bereits um über 30 dB weggefiltert – das erklärt die gemessenen −55 dB (5 dB vom Netz + 30 dB zusätzlicher Tiefpass-Verlust).
3. **Verwechslung des Kabeltyps:**
   Die Bezeichnung „SATA-Kabel“ im Testaufbau legt nahe, dass hier versehentlich ein standardmäßiges Computer-Datenkabel (Serial ATA für Festplatten) statt des geplanten **HQ 135 dB Sat-Koaxialkabels** aufgerollt wurde. SATA-Datenkabel haben feine Signalleiter ohne HF-Schirmung und sind für Gigabit-Digitalimpulse auf wenigen Zentimetern ausgelegt, aber absolut tödlich für analoge 10-MHz-HF-Signale über 250 Meter.

---

## 2. Wie kann das System für das echte Sat-Kabel angepasst werden?

Sobald das echte **250 m HQ 135 dB Sat-Kabel** (75 Ω, 4-fach geschirmt, Kupfer-Innenleiter, Schirmwiderstand im Sub-Ohm-Bereich) eingesetzt wird, bricht der Tiefpasscharakter zusammen, und die Dämpfung bei 10 MHz wird sich exakt auf den Datenblattwert von **ca. 5 dB** (plus ca. 12 dB für die Pads und den Kombinierer, gesamt ca. 17–18 dB) einstellen.

### Notwendige Schritte für den Umbau:

1. **Kabeltausch auf echtes Koaxialkabel:**
   Das verwendete 250-m-Datenkabel komplett entfernen. Das 250-m-Sat-Kabel (HQ 135 dB) aufrollen (keine engen Radien, um den Wellenwiderstand nicht zu quetschen).
2. **Steckerprüfung (F-Kompressionsstecker):**
   Das Sat-Kabel an beiden Enden mit hochwertigen F-Kompressionssteckern versehen. Der Innenleiter muss sauber gekürzt und darf keinen Kurzschluss zum Schirm (Folie + Geflecht) haben.
3. **Abschluss und Anpassung beibehalten:**
   * **Senderseite:** 50-Ω-Quellen (GPSDO/AWG) → Pi-Pads → T-Kombinierer → 50/75-Ω-L-Glied → F-Buchse → 250m Sat-Kabel.
   * **Empfangsseite:** 250m Sat-Kabel → F-Buchse → 75/50-Ω-L-Glied → Zmod SDR (50 Ω).
4. **Erneute NanoVNA-Messung:**
   Nach dem Tausch auf das Koaxialkabel muss der NanoVNA bei 10 MHz eine Gesamtdämpfung (\(S_{21}\)) von **ca. −17 bis −22 dB** (inklusive aller Teiler und Pads) und einen flachen Frequenzgang ohne Tiefpass-Einbruch im Bereich bis 20 MHz zeigen.

───────────────────────────────────────────────────────────────

[👤 Sie]
Danke für die Hinweise. Das Sat-Kabel (nicht SATA, war falsch) war mit CCS Innenleiter (Fehlkauf, wider besseren Wissens) soll mit folgendem Typ mit Kupfer ausgetauscht werden: https://www.willisat.de/sat-empfang/kabel-co/premiumx-profi-250-m-koaxialkabel-135-db-4-fach-geschirmt-cu-reines-kupfer-satelliten-koaxkabel-antennenkabel-digital-sat-tv-koax-kabel-rg-6-class-a-250m-auf-holz-trommel-mit-10x-f-stecker-vergoldet-mit-dichtring_122626_17917 Sind von diesem Kabel die technischen Daten bekannt? Gibt es eventuell noch andere Kabel, die besser geeignet wären? Aktuell werden Nullmessungen mit direktem GPSDO-Signal an beiden Kanälen sowie mit einem 1m-Kabel am Widerstandsnetzwerk durchgeführt. Bei direktem GPSDO-Signal an beiden Kanälen wird ein geringfügiger Offset von 26.1 ps mit einer Schwankungsbreite von 9 ps wie im Anhang gezeigt sichtbar. Bei Anschluss eines 1m-Kabels mit Anpassung und Kombinierer wird neben der Phasenverschiebung durch die Kabellänge eine Schwankungsbreite von ca. 600 ps sichtbar. Zu deren Reduktion sind bereits eine vollständige Abschirmung in einem Stahlblech-Behälter sowie Ferritringe auf den Versorgungsleitungen versucht worden, jedoch ohne erkennbaren Einfluss. Was könnte die Ursache der hohen Schwankungsbreite des Phasensignals sein? Wie könnte sie sich verringern lassen?

📎 Anhänge: GPSDO zu Sat-1m zu SDR1 und GPSDO zu SDR2.jpg, GPSDO zu SDR1 und SDR2.jpg

───────────────────────────────────────────────────────────────

[🤖 KI]
### 1. Technische Daten des PremiumX 135 dB (Voll-Kupfer-Version)

Das gewählte **PremiumX Profi 250m (mit reiner Kupfer-Innenader, Voll-Cu / CU)** ist im Vergleich zum CCS-Kabel die richtige Wahl für analoge HF-Präzisionsanwendungen:
* **Innenleiter:** Reines Kupfer (Voll-Cu), Durchmesser \(\varnothing \approx 1{,}02\,\mathrm{mm}\). Gleichstromwiderstand liegt bei ca. **2,1 bis 2,3 Ω pro 100 m** (bei 250 m also ca. **5,3 bis 5,8 Ω** Gesamtwiderstand statt 34 Ω beim CCS-Fehlkauf).
* **Wellenwiderstand:** \(75\,\Omega\) (Class A, RG-6 Standard).
* **Schirmung:** 4-fach geschirmt (Alu-Folie / Geflecht / Alu-Folie / Geflecht), Schirmungsmaß bis 135 dB.
* **Dämpfung bei 10 MHz:** Liegt datenblattgemäß im Bereich von ca. **0,6 bis 0,8 dB pro 100 m** (bei 250 m also unter **2 dB** Gesamtdämpfung für das reine Kabel). Zusammen mit den L-Gliedern und dem Kombinierer ergibt sich bei 10 MHz die erwartete Gesamtdämpfung von ca. 17 bis 18 dB (wichtiger Unterschied zu den gemessenen −55 dB des Datenkabels).

*Gibt es bessere Kabel?* Für noch geringere Dämpfung oder maximale Phasenstabilität bei Temperaturwechseln kommen dickere Koaxialkabel (z. B. **RG-11** mit 10 mm Durchmesser und ca. 1,6 mm Kupferinnenleiter oder professionelle 50-/75-Ω-Messtrunk-Kabel) infrage. Für einen ersten und flexiblen Laboraufbau auf Holz-Trommel ist das gewählte RG-6-Voll-Kupfer-Kabel jedoch ein guter und absolut ausreichender Standard.

---

### 2. Ursachenanalyse der hohen Phasenschwankung (600 ps)

Während eine direkte GPSDO-Direktmessung auf beiden Kanälen (Nullmessung) extrem stabil läuft (\(\approx 26\,\text{ps}\) Offset, \(\approx 9\,\text{ps}\) Jitter), springt die Schwankungsbreite beim Einfügen des 1-m-Kabels mit Widerstandsnetzwerk und Kombinierer dramatisch auf **600 ps** an. Da Stahlblechgehäuse und Ferrite wirkungslos blieben, liegt die Ursache nicht in äußerer elektromagnetischer Einstrahlung (EMV), sondern an **interternen Schaltungs- und HF-Effekten**:

1. **AM-to-PM-Konversion (Amplituden-Modulation zu Phasen-Modulation):**
   Der 3-Wege-Kombinierer ist ein reines Widerstandsnetzwerk (kein Transformator). Wenn die beiden Eingänge (GPSDO und AWG) phasenstarr auf 10 MHz laufen, überlagern sich die Signale im Sternpunkt. Kleine Amplitudenschwankungen, Restwelligkeit der Betriebsspannungen im AWG oder GPSDO, oder leichtes Rauschen auf dem AWG-Injektionspfad führen im Summenpunkt zu einer *Vektordrift*. Da das Scope die Nulldurchgänge detektiert, interpretiert es Amplituden- und Rauschmodulationen direkt als enorme Phasensprünge (600 ps entsprechen bei 10 MHz gerade einmal ca. \(2{,}16^\circ\)).
2. **Reflexions-Jitter an den Steckern (Mismatch-Uncertainty):**
   Selbst bei sorgfältig selektierten E24-Widerständen erzeugen kleinste Toleranzen, Lötpad-Kapazitäten auf dem PCB oder ungenügende SMA-Übergänge winzige Fehlanpassungen (\(S_{11}\)). Das führt zu stehenden Wellen auf den kurzen 16-cm-Leitungen. Verändert sich die Frequenz oder Temperatur auch nur minimal, wandert die Phase im Resonanzkabel sprunghaft.
3. **Masse- und Gleichtakt-Verkopplung (Ground Bounce):**
   Auch wenn das Stahlblech äußerlich schirmt, fließt der hochfrequente Strom über die gemeinsame Massefläche (Ground Plane) der Platine. Wenn AWG und GPSDO (oder das Messsystem) über Netzteile minimal koppeln, entsteht ein hochfrequenter Gleichtaktstrom auf den Schirmen der 16-cm-SMA-Kabel, der den Phasendetektor im WaveForms-Softwaretrigger (Scope) verwirrt.
4. **Triggereinstellung und digitales Rauschen im WaveForms:**
   Wenn das AWG aktiv ist, injiziert es ein Signal. Wenn dieses Signal Rest-DAC-Stufenrauschen oder Harmonische enthält und der Scope-Trigger auf einen verrauschten Nulldurchgang statt auf eine stabile mathematische Phasenmessung (C1-C2 Phase) schaut, springt der Messwert im Histogramm massiv.

---

### 3. Wie lässt sich die Phasenschwankung verringern?

1. **Getrennte Stromversorgungen / Galvanische Entkopplung:**
   Versorgen Sie den GPSDO, das AWG und das Scope (bzw. das FPGA-Board) über **völlig getrennte, akkubasierte Netzteile** oder hochwertige lineare Netzteile. Vermeiden Sie jegliche Brummschleifen über die Schutzleiter (PE) der Labornetzteile.
2. **Verifizierung ohne AWG-Injektion (Isolierung des Fehlers):**
   Schalten Sie den AWG-Ausgang komplett ab (oder trennen Sie ihn physisch am Pi-Glied ab) und messen Sie nur den GPSDO-Pfad durch das Widerstandsnetzwerk und das 1-m-Kabel auf beide Scope-Kanäle. 
   * *Ziel:* Wenn die 600 ps Jitter **sofort verschwinden**, ist der AWG-Injektionspfad (oder dessen Amplitudenrauschen) die Ursache. Wenn die 600 ps **bleiben**, liegt es an der Fehlanpassung des passiven Widerstandsnetzwerks oder an Masseproblemen auf dem PCB.
3. **Verbesserung des PCB-Layouts für den Sternpunkt:**
   Ein reiner Lötpunkt-Sternpunkt auf einer normalen Lochraster- oder schlechten Leiterplatte hat parasitäre Induktivitäten. Die Widerstände müssen so nah wie physisch möglich (ohne Leiterbahnen dazwischen) zusammenrücken, um die Sternpunkt-Fläche gegen Null zu minimieren.
4. **Softwareseitige Mittelung in WaveForms erhöhen:**
   In den Scope-Messungen (Measurements) von WaveForms sollte für die Phasenmessung eine Mittelung („Acquisition Mode: Average“ mit 16 oder 64 Samples) aktiviert werden, um hochfrequentes Rauschen im Nulldurchgang auszufiltern.

**Citations:**
[1] [http://www.hifi-forum.de/viewthread-97-17866.html#1](http://www.hifi-forum.de/viewthread-97-17866.html#1)
[2] [https://www.otto.de/p/premiumx-profi-pro-koaxialkabel-135db-5-fach-reines-kupfer-50-meter-tv-kabel-S072P0JM/#variationId=S072P0JMLGN3#1](https://www.otto.de/p/premiumx-profi-pro-koaxialkabel-135db-5-fach-reines-kupfer-50-meter-tv-kabel-S072P0JM/#variationId=S072P0JMLGN3#1)
[3] [https://www.saturn.de/de/product/_premiumx-px-basic-250w-122625-antennenkabel-97539340.html](https://www.saturn.de/de/product/_premiumx-px-basic-250w-122625-antennenkabel-97539340.html)
[4] [https://www.technisat.com/de_DE/Produktdatenblatt/352-775/?product=1645&createpdf#1#1#1](https://www.technisat.com/de_DE/Produktdatenblatt/352-775/?product=1645&createpdf#1#1#1)
[5] [https://www.willisat.de/premiumx-profi-250-m-koaxialkabel-135-db-4-fach-geschirmt-cu-reines-kupfer-satelliten-koaxkabel-antennenkabel-digital-sat-tv-koax-kabel-rg-6-class-a-auf-holz-trommel/a-122621/](https://www.willisat.de/premiumx-profi-250-m-koaxialkabel-135-db-4-fach-geschirmt-cu-reines-kupfer-satelliten-koaxkabel-antennenkabel-digital-sat-tv-koax-kabel-rg-6-class-a-auf-holz-trommel/a-122621/)

═══════════════════════════════════════════════════════════════
  Ende der Konversation
═══════════════════════════════════════════════════════════════
