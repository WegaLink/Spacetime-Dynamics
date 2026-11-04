═══════════════════════════════════════════════════════════════
  Perplexity Phasendetektor-Impedanzanpassung
═══════════════════════════════════════════════════════════════

Exportiert: 2. November 2026 um 23:36
Nachrichten: 6
Modell: sonar

───────────────────────────────────────────────────────────────

[👤 Sie]
In einem Team aus 12 KIs und mir als Mensch sollen Verbesserungen für einen Phasendetektor diskutiert und erarbeitet werden, mit welchem Phänomene im Bereich Raumzeitdynamik untersucht werden sollen. Die Zusammenarbeit findet mit einem markdown-Dokument statt, welches im GitHub-Repository unter folgendem Link verfügbar ist:
https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md Als Start für „Physik 2“ soll die Anpassung für Oszillator, Injektion und ADC an ein 250 m Sat-Kabel HQ 135 dB, 4-fach geschirmt von HB Digital mit 75 Ohm Impedanz und 5 dB Dämpfung bei 10 MHz sowie einem Verkürzungsfaktor von 0.82  entworfen werden. Der Oszillator ist ein DXPatrol GPSDO V2.4 mit 3 Sinus- und einem Rechteck-Ausgang bei 10 MHz/+7dB Signal an 50 Ohm. Die Injektion soll durch Eclypse Z7 AWG mit einem programmierbaren Signal von 1 V Amplitude an 50 Ohm in einer Weise erfolgen, dass die Phase eines kombinierten Signals aus GPSDO und AWG im Bereich von etwa 300 ps moduliert werden kann. Die Eingangsimpedanz des Eclypse Z7 Scope ist mit 1 MOhm/18 pF angegeben. Ein Eingang soll mit dem GPSDO als Referenz verbunden werden und der andere als Messeingang mit dem anderen Ende des Sat-Kabels. Der Messbereich ist jeweils +/-1 V. Wie kann eine impedanzrichtige Anpassung dieser Anordnung realisiert werden?

───────────────────────────────────────────────────────────────

[🤖 KI]
Die sauberste Lösung ist, **die 75-Ω-Leitung als 75-Ω-Übertragungssystem mit passenden Übergängen und Abschlusswiderständen zu betreiben**. Den GPSDO nicht direkt an das Sat-Kabel und den 1-MΩ-Scope-Eingang nicht direkt ans offene Kabelende anschließen: Das erzeugt Fehlanpassungen und Reflexionen. Der Zmod-Scope-Eingang ist \(1\,\text{M}\Omega \parallel 18\,\text{pF}\), nicht 75 Ω[1]; die GPSDO-Sinusausgänge sind laut Produktangabe etwa \(+7\,\text{dBm}\)[2].

## Empfohlene Messkette

```text
GPSDO 10 MHz ── 50→75 Ω-Anpassung ──┬── 75-Ω-Leitung, 250 m ── 75-Ω-Abschluss ──┬── Scope CH2
                                    │                                           │
                                    └── Referenz/Teiler ── Scope CH1             └── hochohmiger Abgriff
```

- Verwenden Sie einen **50-zu-75-Ω-Anpassbaustein** am GPSDO-Ausgang. Ein passiver Widerstands- oder Übertrageradapter ist möglich, aber mit Einfügedämpfung; für eine präzise Amplituden-/Phasenmessung ist ein spezifizierter RF-Übertrager oder ein 50/75-Ω-Puffer geeigneter.
- Schließen Sie das Kabel am entfernten Ende mit **75 Ω** ab. Nutzen Sie dafür einen SMA-/F-Adapter mit kurzem Anschluss und einem 75-Ω-Abschlusswiderstand am Kabelende.
- Da der Scope-Eingang hochohmig ist, messen Sie am Kabelende über einen **hochohmigen, kapazitätsarmen Puffer**. Alternativ kann ein 75-Ω-Abschluss mit einem hochohmigen Abgriff verwendet werden, sofern der Abgriff die 75-Ω-Terminierung nicht wesentlich belastet. Ein T-Stück mit direkter Scope-Verbindung ist nicht automatisch korrekt: Der Scope verändert die Last und kann die Terminierung am Messpunkt beeinträchtigen.
- Für CH1 führen Sie denselben GPSDO-Referenztakt über einen zweiten passenden Abzweig zum Scope. Achten Sie darauf, dass der Abzweig den GPSDO nicht doppelt oder falsch belastet. Eine **50-Ω-Verteilung mit getrennten Pufferausgängen** ist meist besser als ein passives T-Stück.

## Pegel und Dämpfung

\(+7\,\text{dBm}\) an 50 Ω entspricht ungefähr \(0{,}50\,\text{V}_{\mathrm{rms}}\) beziehungsweise \(1{,}41\,\text{V}_{\mathrm{pp}}\) bei einem Sinus. Die angegebene Kabeldämpfung von 5 dB bei 10 MHz bedeutet, dass am Ende etwa \(0{,}56\) des Eingangsspannungspegels ankommt, sofern die Dämpfungsangabe für die konkrete Länge gilt. **Prüfen Sie, ob „5 dB“ für 250 m oder pro 100 m angegeben ist**; das verändert den Empfangspegel erheblich. Die gefundenen HB-Digital-Daten bestätigen 75 Ω und eine Abschirmungsangabe, liefern aber keine eindeutige Bestätigung der konkreten 250-m-Variante mit genau 5 dB bei 10 MHz[5].

Auch die Scope-Grenze von ±1 V ist zu beachten: Ein GPSDO-Sinus mit \(+7\,\text{dBm}\) kann im 50-Ω-System bereits rund ±0,71 V Spitze betragen. Der Referenzkanal sollte daher **mit einem geeigneten Dämpfungsglied oder Puffer** auf einen sicheren Pegel gebracht werden. Beim AWG gilt die Ausgangsangabe „1 V an 50 Ω“ nur bei korrekter 50-Ω-Last; ein hochohmiger Scope kann je nach AWG-Ausgangsstufe eine andere Spannung anzeigen. Die Digilent-AWG-Angabe beschreibt einen 50-Ω-Ausgang, während die Scope-Eingangsimpedanz hochohmig ist[1][4].

## Phaseninjektion um etwa 300 ps

Bei 10 MHz entspricht 300 ps einer Phasenverschiebung von

\[
\Delta\phi=2\pi f\Delta t\approx 0{,}01885\ \text{rad}\approx1{,}08^\circ.
\]

Die zweckmäßige Injektion ist **keine direkte Addition eines beliebigen AWG-Signals am Kabel**, sondern eine kalibrierte Phasenmodulation eines 10-MHz-Signals:

1. Erzeugen Sie mit dem AWG eine kleine, bekannte Phasen- oder Laufzeitmodulation synchron zur GPSDO-Referenz. Ein praktikabler Weg ist ein **quadraturbasierter I/Q-Aufbau**: GPSDO-Sinus als Träger, dazu ein um 90° verschobener Träger und eine kleine AWG-gesteuerte Quadraturkomponente. Für kleine Winkel gilt näherungsweise \(\Delta\phi \simeq V_Q/V_I\). Die Quadratur muss elektrisch erzeugt und am Messpunkt kalibriert werden.
2. Alternativ kann der AWG eine **kleine FM-Modulation** erzeugen; daraus ergibt sich Phase als Zeitintegral der Frequenzabweichung. Dafür müssen Frequenzhub, Modulationsfrequenz und Modulationsform so gewählt werden, dass die resultierende Zeitabweichung 300 ps beträgt.
3. Kombinieren Sie Signale nur über einen **passenden 50-Ω-Leistungskombinierer** und passen Sie dessen Ausgang anschließend an 75 Ω an. Ein bloßes Widerstands-T-Stück führt zu Pegelverlust, verändert die Quellenlast und kann Rückwirkungen zwischen GPSDO und AWG verursachen.
4. Erfassen Sie die Injektion zunächst direkt am Scope-Eingang und bestimmen Sie die tatsächlich erzeugte Phase beziehungsweise Laufzeit aus den aufgezeichneten Wellenformen. Wiederholen Sie die Kalibrierung durch die vollständige Kabelstrecke.

Für 300 ps muss die Phase bei 10 MHz um etwa \(1{,}08^\circ\) verschoben werden. Das ist eine kleine Phasenänderung, aber nicht automatisch eine 300-ps-Auflösung des Scope: Die effektive Genauigkeit hängt von Abtastrate, Zeitbasis, Triggerung, Rauschen und Auswerteverfahren ab. Das Zmod Scope bietet je nach Modell schnelle simultane Abtastung und ±1-V-Bereich, doch die angegebene Eingangsimpedanz bleibt hochohmig[1][4].

## Wichtiger Punkt zur gemeinsamen Zeitbasis

Wenn CH1 und CH2 über denselben Scope-ADC-Takt erfasst werden, ist eine konstante Kabelverzögerung gut messbar, aber langsame gemeinsame Zeitbasisschwankungen können in der Differenz teilweise verschwinden. Für die Injektionsprüfung sollte die Verzögerung daher **aus einer synchronen Phasen-/Kreuzkorrelationsauswertung** bestimmt und mit einer bekannten elektrischen Verzögerung oder Phasenmodulation verifiziert werden.

Die angegebene Verkürzungsgeschwindigkeit von \(0{,}82c\) ergibt für 250 m eine einfache Laufzeit von rund \(1{,}02\,\mu\text{s}\). Das ist eine erwartete Größenordnung, kein Ersatz für die tatsächliche Messung des Kabels samt Steckern und Adaptern.

**Kurz:** 50→75-Ω-Anpassung am GPSDO, 75-Ω-Abschluss am Kabelende, hochohmiger Puffer zum Scope, gepufferte Referenz für CH1 und AWG-Injektion über einen kontrollierten Phasenmodulator oder kalibrierten Kombinierer. Vor dem Aufbau sind die genaue Kabeldämpfung bei 10 MHz und die elektrische Ausgangsspezifikation des konkreten AWG-Moduls zu prüfen.

**Citations:**
[1] [https://cdkauf-online.de/Fuer-den-Funkamateur/Es-hail-2-OSCAR-100-QO-100/491/DXPatrol-GPSDO-2.0-GPS-disziplinierter-Normaloszillator-mit-Oled-Display](https://cdkauf-online.de/Fuer-den-Funkamateur/Es-hail-2-OSCAR-100-QO-100/491/DXPatrol-GPSDO-2.0-GPS-disziplinierter-Normaloszillator-mit-Oled-Display)
[2] [https://cdkauf-online.de/Fuer-den-Funkamateur/Es-hail-2-OSCAR-100-QO-100/601/DXPATROL-Full-duplex-QO-100-Groundstation](https://cdkauf-online.de/Fuer-den-Funkamateur/Es-hail-2-OSCAR-100-QO-100/601/DXPATROL-Full-duplex-QO-100-Groundstation)
[3] [https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md](https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md)
[4] [https://in.element14.com/digilent-embedded-measurement](https://in.element14.com/digilent-embedded-measurement)
[5] [https://ipxchange.tech/evaluation-boards/digilent-eclypse-z7-processor-fgpa-soc-development-board-for-embedded-measurement-systems/](https://ipxchange.tech/evaluation-boards/digilent-eclypse-z7-processor-fgpa-soc-development-board-for-embedded-measurement-systems/)
[6] [https://www.hb-digital.de/05-m-10-m-SAT-Anschluss-Kabel-mit-2-x-vergoldeten-oder-vernickelten-Vollmetall-F-Schnellstecker-Quickfix-Farbe-waehlbar](https://www.hb-digital.de/05-m-10-m-SAT-Anschluss-Kabel-mit-2-x-vergoldeten-oder-vernickelten-Vollmetall-F-Schnellstecker-Quickfix-Farbe-waehlbar)

───────────────────────────────────────────────────────────────

[👤 Sie]
Ein Widerstandsnetzwerk wurde wie im Dokument beschrieben mit folgenden Komponenten aufgebaut: GPSDO - Dämpfung (-5dB) - 3-Wege-Kombinierer - 50/75 Ohm Anpassung - SATA Kabel 1 m (für Null-Test) - 75/50 Ohm Anpassung - SDR-1. Die Dämpfung der gesamten Strecke wurde mit Nano VNA bei 10 MHz etwa wie erwartet mit -22.5dB gemessen. Nach Wechsel des 1m SATA-Kabels gegen ein 250m SATA-Kabel stieg die Dämpfung wie im NanoVNA Screenshot gezeigt auf -55dB an. Es zeigte sich nun ein Tiefpasscharakter mit einer -3dB Frequenz bei etwa 250 kHz  wie ebenfalls in einem angehängten Screenshot vom Nano VNA gezeigt. Nach Datenblatt sollte die zusätzliche Dämpfung bei 250m/10MHz etwa -5dB sein, sie ist jedoch mehr als -30dB. Der ohmsche Widerstand der Schirmung ist etwa 8 Ohm und des Innenleiters etwa 34 Ohm. Welche Erklärung könnte es dafür geben? Wie kann das System verändert werden, damit das 10MHz Signal des GPSDO mit der nominellen Dämpfung am Ausgang ankommt?

📎 Anhänge: Anpassung und Kombinierer 3 MHz (GPSDO - SATA_250m - SDR-1).jpg, Anpassung und Kombinierer 10 MHz (GPSDO - SATA_250m - SDR-1).jpg

───────────────────────────────────────────────────────────────

[🤖 KI]
Die von Ihnen gemessenen Werte (insbesondere der ohmsche Widerstand des Innenleiters von ca. **34 Ω** für die gesamte Strecke und die massive Dämpfung von ca. \(-55\,\text{dB}\) bei 10 MHz mit starkem Tiefpasscharakter) liefern eine eindeutige physikalische Erklärung:

### 1. Erklärung für das unerwartete Verhalten

- **DC-Widerstand des Innenleiters:** Ein typisches Koaxialkabel für Sat-Anlagen (wie das HQ 135 dB) verwendet im Innenleiter meist **verkupferten Stahldraht (CCS – Copper-Clad Steel)** oder einen sehr dünnen massiven Kupferdraht. Bei einer Länge von **250 m** summiert sich der Gleichstromwiderstand des Innenleiters auf die von Ihnen gemessenen **34 Ω** (ca. \(136\,\Omega/\text{km}\), was typisch für dünne CCS-Innenleiter ist).
- **Gleichstromwiderstand der Schirmung:** Die Schirmung (meist Aluminium-Folie plus Aluminium- oder tinnierte Kupfergeflechte) hat ca. **8 Ω** (\(\approx 32\,\Omega/\text{km}\)).
- **Skineffekt bei 10 MHz:** Hochfrequenzströme fließen aufgrund des Skineffekts nur in der äußeren Schicht des Leiters (Skintiefe bei Kupfer bei 10 MHz liegt bei ca. \(21\,\mu\text{m}\)). Bei einem Stahldraht (CCS) weicht der Strom jedoch in den weitaus schlechter leitenden Stahlkern aus, sobald die Frequenz sinkt oder der Kupfermantel sehr dünn ist. 
- **Zusammenbruch der Übertragung:** 
  1. Die DC-Leitungsverluste und der HF-Widerstand eines 250-m-Stahlkern-Innenleiters sind bei 10 MHz dramatisch höher als bei reinem Kupfer.
  2. Noch schwerwiegender ist: **Sat-Kabel (75 Ω) sind für Gigahertz-Frequenzen (Sat-ZF von 950 MHz bis 2150 MHz) optimiert.** Bei sehr niedrigen Frequenzen wie 10 MHz (und erst recht beim gemessenen Tiefpass bei \(\sim 250\,\text{kHz}\)) spielen die Kapazitäten und Induktivitäten im Zusammenspiel mit dem hochohmigen Innenleiter komplett verrückt. Das Kabel wirkt auf 250 m Länge wie ein extrem verlustbehaftetes RC-Glied bzw. eine verteilte RC-Leitung, nicht mehr wie eine reine Wellenleiter-Übertragungsstrecke. 
  3. Die vom Datenblatt versprochenen „5 dB Dämpfung bei 10 MHz“ beziehen sich fast immer auf **100 Meter** (oder sind ein idealisierter theoretischer Wert für reine Kupferleitungen bei hohen Frequenzen, wo der Skineffekt ideal greift). 250 Meter Sat-Kabel mit CCS-Innenleiter dämpfen 10 MHz *kolossal* stark.

---

### 2. Wie kann das System verändert werden, damit das 10-MHz-Signal ankommt?

Um ein 10-MHz-Referenzsignal verlustarm über 250 Meter zu übertragen, müssen Sie von ungeeigneten Consumer-Sat-Kabeln auf professionelle HF-Leitungen umsteigen:

1. **Kabelwechsel auf massives Kupfer (Solid Copper) / Geringer Gleichstromwiderstand:**
   - Verwenden Sie ein professionelles **Koaxialkabel mit massivem Kupfer-Innenleiter (BC – Bare Copper)** und niedriger Dämpfung im Kurzwellen-/VHF-Bereich (z. B. RG-213, H-1000, Aircom Premium oder professionelle 75-Ω-Messtechnik-Kabel wie RG-11 mit Vollkupfer).
   - Ein typisches 50-Ω- oder 75-Ω-Vollkabel hat einen Innenleiterwiderstand im Bereich von unter \(2\,\Omega\) bis \(5\,\Omega\) für die gesamte 250-m-Strecke (statt 34 Ω). Dadurch bricht die Amplitude bei 10 MHz nicht ein.

2. **Aktive Signalaufbereitung (Repeater / Line-Amplifier):**
   - Wenn das Kabel aus mechanischen Gründen (z. B. Verlegung, Gewicht) beibehalten werden muss, kann das 10-MHz-Signal **nicht** passiv verlustfrei durch 250 m CCS-Kabel gezwungen werden.
   - **Lösung:** Setzen Sie direkt hinter dem GPSDO (oder am Sendeende) einen **HF-Verstärker** ein, der die \(-30\,\text{dB}\) Dämpfung des Kabels ausgleicht, und entkoppeln Sie den Empfängereingang entsprechend. Besser ist jedoch immer eine niederohmige Kupferleitung, da ein Verstärker auf 250 m auch das Rauschen stark anhebt.

3. **Alternativer Ansatz für Raumzeit- / Phasendynamik-Experimente (RF-over-Fiber):**
   - Bei 250 Metern Distanz und extrem hohen Anforderungen an Phasenstabilität (Nanosekunden-/Pikosekunden-Bereich) kämpfen Sie bei Kupferkabeln immer mit thermischen Längenänderungen (Ausdehnung des Kabels bei Temperaturwechseln verfälscht die Phase massiv!).
   - Professionelle Aufbauten in diesem Bereich nutzen oft **Glasfaser-Strecken (RF over Fiber / Analog-Optik-Links)** oder direkt digitale Protokolle (White Rabbit / PTP / Ethernet-basierte Uhrenverteilung), um Laufzeit- und Dämpfungsprobleme auf Langstrecken komplett zu eliminieren.

───────────────────────────────────────────────────────────────

[👤 Sie]
Danke für die Hinweise. Das Sat-Kabel (nicht SATA, war falsch) war mit CCS Innenleiter (Fehlkauf, wider besseren Wissens) soll mit folgendem Typ mit Kupfer ausgetauscht werden: https://www.willisat.de/sat-empfang/kabel-co/premiumx-profi-250-m-koaxialkabel-135-db-4-fach-geschirmt-cu-reines-kupfer-satelliten-koaxkabel-antennenkabel-digital-sat-tv-koax-kabel-rg-6-class-a-250m-auf-holz-trommel-mit-10x-f-stecker-vergoldet-mit-dichtring_122626_17917 Sind von diesem Kabel die technischen Daten bekannt? Gibt es eventuell noch andere Kabel, die besser geeignet wären? Aktuell werden Nullmessungen mit direktem GPSDO-Signal an beiden Kanälen sowie mit einem 1m-Kabel am Widerstandsnetzwerk wie im Anhang gezeigt durchgeführt. Bei direktem GPSDO-Signal an beiden Kanälen wird ein geringfügiger Offset von 26.1 ps mit einer Schwankungsbreite von 9 ps wie im Anhang gezeigt sichtbar. Bei Anschluss eines 1m-Kabels mit Anpassung und Kombinierer wird neben der Phasenverschiebung durch die Kabellänge eine Schwankungsbreite von ca. 600 ps sichtbar. Zu deren Reduktion sind bereits eine vollständige Abschirmung in einem Stahlblech-Behälter sowie Ferritringe auf den Versorgungsleitungen versucht worden, jedoch ohne erkennbaren Einfluss. Was könnte die Ursache der hohen Schwankungsbreite des Phasensignals sein? Wie könnte sie sich verringern lassen?

📎 Anhänge: Widerstandsnetzwerk.jpg, GPSDO zu Sat-1m zu SDR1 und GPSDO zu SDR2.jpg, GPSDO zu SDR1 und SDR2.jpg

───────────────────────────────────────────────────────────────

[🤖 KI]
### 1. Technische Daten des gewählten Kabels (PremiumX Profi 250m CU)

Das von Ihnen verlinkte **PremiumX Profi (CU, reines Kupfer)** ist eine deutliche Verbesserung gegenüber dem alten CCS-Kabel:
- **Innenleiter:** Reines Kupfer (Solid Copper, meist ca. \(1{,}02\,\text{mm}\) Durchmesser), wodurch der DC-Widerstand für 250 m von katastrophalen \(\approx 34\,\Omega\) auf **unter \(4\,\Omega\) bis \(5\,\Omega\) für die gesamte Strecke** sinkt.
- **Wellenwiderstand:** Nominell \(75\,\Omega\).
- **Schirmung:** 4-fach geschirmt (Klasse A), Schirmungsmaß ca. 135 dB.
- **Dämpfung bei 10 MHz:** Da Sat-Kabel primär für den GHz-Bereich optimiert sind, gibt es für 10 MHz selten offizielle Datenblätter, aber mit massivem Kupfer entfällt der massive Skineffekt-Einbruch des Stahlkerns. Die Dämpfung wird sich bei 10 MHz im Bereich von schätzungsweise \(\approx 1\,\text{dB}\) bis \(2\,\text{dB}\) für die 250 m bewegen (deutlich unter den im Datenblatt theoretisch angegebenen \(\sim 5\,\text{dB}\) oder dem gemessenen \(-55\,\text{dB}\)).

#### Gibt es noch besser geeignete Kabel?
Für Raumzeit- und Phasendynamik-Messungen im Nanosekunden-/Pikosekunden-Bereich gibt es zwei physikalische Aspekte, die ein Standard-Sat-Kabel (RG-6-Klasse) an seine Grenzen bringen:
1. **Temperaturabhängigkeit des Dielektrikums (Phasendrift):** Standard-Sat-Kabel nutzen geschäumtes PE (Foam-PE). Dessen mechanische und dielektrische Eigenschaften variieren mit der Temperatur, was zu langsamen Phasenverschiebungen bei Raumklimaschwankungen führt.
2. **Mechanische Stabilität (Biegestress):** Wenn das 250-m-Kabel bewegt oder aufgerollt ist, verändern sich die mechanischen Toleranzen leicht.

*Alternativen:*
- **RG-11 Vollkupfer / 75-Ω-Messtechnik-Kabel:** Haben einen dickeren Innenleiter (oft \(\approx 1{,}6\,\text{mm}\)), noch geringere Dämpfung und sind mechanisch stabiler.
- **Professionelle 50-Ω- oder 75-Ω-Halb-Rigid- oder RG-213-Leitungen** (wobei 50-Ω-Leitungen im bestehenden Setup Anpassungsglieder erfordern). Für ein rein passives 75-Ω-System ist ein hochwertiges RG-11-Vollkupferkabel eine exzellente Wahl.

---

### 2. Ursachen für die hohe Schwankungsbreite (Jitter / Rauschen ~600 ps) bei der 1-m-Messung

Bei der Direktmessung (GPSDO auf beide Kanäle) liegt die Schwankungsbreite bei sauberen **9 ps**. Sobald jedoch das Widerstandsnetzwerk und ein kurzes 1-m-Kabel im Spiel sind, springt die Schwankung auf **ca. 600 ps** (was bei 10 MHz ca. \(2{,}16^\circ\) Jitter/Phasenrauschen entspricht). Dass eine Stahlblech-Kapselung und Ferrite keinen Einfluss hatten, zeigt, dass es **kein klassisches Einstrahlproblem (HF/Netzbrumm)** ist.

Die Hauptursachen für diese massive Verschlechterung im Netzwerk/Kabel-Pfad sind:

1. **Fehlende Signalflanken-Steilheit & Rauschverstärkung im passiven Netzwerk:**
   Ein passives Widerstandsnetzwerk (Teiler, Kombinierer, Anpassungswiderstände) dämpft das Signal massiv (ca. \(-22{,}5\,\text{dB}\) bis \(-25\,\text{dB}\)). Dadurch sinkt das Signal-Rausch-Verhältnis (SNR) am Scope-Eingang dramatisch. Das Rauschen des Oszilloskops bzw. der Eingangsstufe überlagert sich dem flachen Sinus-Nulldurchgang. Bei einem 10-MHz-Sinus führt ein geringes Amplitudenrauschen bei flacher Flanke zu riesigen Zeit-/Phasen-Jittern (\(\Delta t = \frac{\Delta V}{2\pi f V_{\text{peak}}}\)).
2. **Reflexionen durch unvollständige Impedanzanpassung (Mismatched Loads):**
   Das Netzwerk arbeitet mit diskreten Widerständen, kombiniert GPSDO-Signal und (simulierten) AWG-Pfad und trifft auf die hochohmige, kapazitive Last des Scopes (\(1\,\text{M}\Omega \parallel 18\,\text{pF}\)) bzw. des SDRs. Unangepasste Übergänge zwischen 50 Ω, Widerstands-Kombinierern und 75 Ω erzeugen stehende Wellen und Mehrfachreflexionen, die das Signal verformen und von Messung zu Messung fluktuieren lassen.
3. **Masse-Loops / Ground-Bouncing über die gemeinsame Geräte-Masse:**
   Wenn GPSDO, Eclypse Z7 (Scope/AWG) und eventuelle USB-/Netzteile über Schutzleiter oder Masse-Schirme mehrfach verbunden sind, fließen Ausgleichsströme auf den Schirmungen. Selbst wenn Ferrite auf den DC-Leitungen sitzen, reicht hochfrequentes digitales Schaltrauschen des FPGA-Boards (Eclypse Z7), um die empfindliche analoge Masse des Scopes oder der Referenz zu modulieren.

---

### 3. Wie lässt sich die Schwankungsbreite verringern?

1. **Aktive Pufferung statt rein passiver Widerstandsnetzwerke:**
   Verwenden Sie am Ausgang des GPSDO und an den Einspeisepunkten **aktive HF-Buffer / Breitband-Verstärker mit hoher Entkopplung** (oder Operationsverstärker mit hoher slew rate), anstatt Signale über passive Widerstandsnetzwerke zu kombinieren. Das hält das Signal-Rausch-Verhältnis hoch.
2. **Erzwingen einer echten 50-Ω- oder 75-Ω-Last am Scope:**
   Der Eclypse-Z7-Scope-Eingang ist hochohmig (\(1\,\text{M}\Omega\)). Ein offener oder hochohmig abgeschlossener Abschluss am Messpunkt führt zu Reflexionen. Schließen Sie den Messpunkt *direkt am Scope-Eingang* mit einem physischen 50-Ω- oder 75-Ω-Abschlusswiderstand (terminiert) ab und koppeln Sie das Signal entweder per hochohmigem Tastkopf (10:1 mit niedriger Kapazität) oder über einen Impedanzwandler aus.
3. **Galvanische Trennung / Gleichtaktunterdrückung:**
   Da Masse-Schleifen oft für unruhige Phasenmessungen sorgen, prüfen Sie, ob der GPSDO und das Eclypse Z7 über getrennte Netzteile laufen. Ein HF-Trenntrafo (Balun) oder ein optischer/galvanischer Trenner in der Taktleitung kann helfen, Brummen und digitale Störnteppiche des FPGAs fernzuhalten.
4. **Verbesserung der Flankensteilheit (Zero-Crossing-Detektion):**
   Wenn möglich, nutzen Sie nicht den Sinus-Ausgang direkt für die Phasendetektion, sondern formen Sie das Signal über einen schnellen Komparator (Nulldurchgangsdetektor mit schneller Schaltdiskriminierung) in ein Rechtecksignal um. Scharfe Flanken minimieren den Einfluss von Amplitudenrauschen auf die Zeitmessung drastisch.

**Citations:**
[1] [https://www.kaufland.at/product/320666483/](https://www.kaufland.at/product/320666483/)
[2] [https://www.kaufland.at/product/338131801/](https://www.kaufland.at/product/338131801/)
[3] [https://www.kaufland.de/product/324473356/#1](https://www.kaufland.de/product/324473356/#1)
[4] [https://www.kaufland.de/product/358228566/#1](https://www.kaufland.de/product/358228566/#1)
[5] [https://www.testberichte.de/pdb/p/evdTAFFeQTOKa3asbV7yDA.html#1](https://www.testberichte.de/pdb/p/evdTAFFeQTOKa3asbV7yDA.html#1)

═══════════════════════════════════════════════════════════════
  Ende der Konversation
═══════════════════════════════════════════════════════════════
