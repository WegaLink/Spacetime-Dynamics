═══════════════════════════════════════════════════════════════
  GPT-6 Phasendetektor impedanzrichtig anpassen
═══════════════════════════════════════════════════════════════

Exportiert: 2. November 2026 um 23:34
Nachrichten: 6
Modell: gpt-6

───────────────────────────────────────────────────────────────

[👤 Sie]
In einem Team aus 12 KIs und mir als Mensch sollen Verbesserungen für einen Phasendetektor diskutiert und erarbeitet werden, mit welchem Phänomene im Bereich Raumzeitdynamik untersucht werden sollen. Die Zusammenarbeit findet mit einem markdown-Dokument statt, welches im GitHub-Repository unter folgendem Link verfügbar ist:
https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md Als Start für „Physik 2“ soll die Anpassung für Oszillator, Injektion und ADC an ein 250 m Sat-Kabel HQ 135 dB, 4-fach geschirmt von HB Digital mit 75 Ohm Impedanz und 5 dB Dämpfung bei 10 MHz sowie einem Verkürzungsfaktor von 0.82  entworfen werden. Der Oszillator ist ein DXPatrol GPSDO V2.4 mit 3 Sinus- und einem Rechteck-Ausgang bei 10 MHz/+7dB Signal an 50 Ohm. Die Injektion soll durch Eclypse Z7 AWG mit einem programmierbaren Signal von 1 V Amplitude an 50 Ohm in einer Weise erfolgen, dass die Phase eines kombinierten Signals aus GPSDO und AWG im Bereich von etwa 300 ps moduliert werden kann. Die Eingangsimpedanz des Eclypse Z7 Scope ist mit 1 MOhm/18 pF angegeben. Ein Eingang soll mit dem GPSDO als Referenz verbunden werden und der andere als Messeingang mit dem anderen Ende des Sat-Kabels. Der Messbereich ist jeweils +/-1 V. Wie kann eine impedanzrichtige Anpassung dieser Anordnung realisiert werden?

───────────────────────────────────────────────────────────────

[🤖 KI]
**Kurzfassung:** Das 250-m-Kabel ist eine 75-Ω-Übertragungsleitung. Die 1 MΩ/18 pF-Angabe des Scope-Eingangs bedeutet bei 10 MHz **keine hochohmige HF-Last**: 18 pF haben dort nur etwa **884 Ω** kapazitiven Blindwiderstand. Direkt am Kabelende angeschlossen, wäre das deshalb keine saubere 75-Ω-Terminierung und die gemessene Phase könnte von Reflexionen und Eingangskapazität abhängen.

## Empfohlener Signalweg

```text
GPSDO Sinusausgang, 10 MHz / ca. +7 dBm
       │
       ├── Referenzzweig ───────────────► Scope CH1
       │                                  kurzer 50-Ω-Koax
       │                                  ggf. Pegelabschwächer
       │
       └── 50↔75-Ω-Anpassung ─► 250 m / 75-Ω-Kabel
                                      │
                                      └── 75-Ω-Abschluss
                                          + breitbandiger,
                                            niederkapazitiver Puffer
                                               │
                                               └────────► Scope CH2
```

Die 75-Ω-Abschlusslast gehört **am Ende des Kabels**, unmittelbar vor einem Puffer. Wählen Sie dafür einen Puffer beziehungsweise einen Messadapter, dessen Eingang für 75-Ω-Leitungen geeignet ist und dessen Eingangskapazität spezifiziert ist. **Nicht** einfach den Scope-Eingang als Kabelabschluss behandeln. Ein 75-Ω-Abschluss direkt am Scope könnte die Leitung zwar terminieren, aber die parallel liegenden 18 pF würden die Last bei 10 MHz stark verändern.

Am GPSDO-Ausgang sind 50 Ω angegeben; der Kabeltyp ist 75 Ω. Für eine definierte Anpassung sollte zwischen 50-Ω-Quelle und 75-Ω-Leitung ein passendes 50/75-Ω-Anpassglied beziehungsweise ein bei 10 MHz geeigneter Übertrager verwendet werden. Ein direkter Anschluss kann für einen ersten Funktionstest ausreichen, ist aber keine impedanzrichtige Messkette. Das Kabel wird als 75-Ω-Kabel angeboten. ([hb-digital.de](https://www.hb-digital.de/10m-coaxial-cable-HQ-135-dB-4-fold-shielded-steel-copper-white-4-F-plugs?utm_source=openai))

## Laufzeit und Pegel

Mit \(L=250\ \mathrm{m}\) und Verkürzungsfaktor \(v_f=0{,}82\) ergibt sich näherungsweise

\[
\tau=\frac{L}{v_f c}\approx 1{,}02\ \mu\mathrm{s}.
\]

Bei 10 MHz entspricht das rund 10,16 Perioden; die Phasenlage ist also zyklisch und muss relativ zur Referenz ausgewertet werden. Die angegebene Dämpfung von 5 dB entspräche bei passenden Abschlussimpedanzen einer Spannungsübertragung von etwa \(10^{-5/20}\approx0{,}56\). Der GPSDO-Ausgang mit ca. +7 dBm an 50 Ω entspricht ungefähr 0,28 V RMS beziehungsweise 0,40 V Spitze an einer 50-Ω-Last. Damit sollte der Eingangsbereich von ±1 V grundsätzlich nicht überschritten werden — dennoch Pegel am tatsächlichen Messpunkt prüfen. Der DXPatrol wird mit drei Sinusausgängen um +7 dBm und einem Rechteckausgang beschrieben. ([wimo.com](https://www.wimo.com/en/dxpatrol-gpsdo-10mhz-reference?utm_source=openai))

## 300-ps-Injektion

Bei 10 MHz entspricht eine Zeitverschiebung von 300 ps einer Phasenmodulation von

\[
\Delta\phi=2\pi f\Delta t\approx 0{,}01885\ \mathrm{rad}\approx1{,}08^\circ.
\]

Eine praktikable Injektion ist, das GPSDO-Signal mit einem **phasenstabilen, quadraturliegenden** AWG-Signal zu kombinieren. Für kleine Winkel gilt näherungsweise \(V_\text{AWG}/V_\text{Träger}\approx\Delta\phi\), also etwa **1,9 % des GPSDO-Trägerpegels am Summierpunkt**. Das ist eine Startabschätzung, keine Kalibrierung: Kabelverluste, Teiler, Abschluss und tatsächliche AWG-Ausgangskalibrierung verändern den nötigen Pegel.

Wichtig: Das AWG muss **synchron zum GPSDO** arbeiten oder seine Phase muss anderweitig stabil bekannt sein. Zwei lediglich nominell auf 10 MHz eingestellte, nicht synchronisierte Signale erzeugen keine verlässliche Phasenmodulation. Kombinieren Sie GPSDO und AWG über einen **isolierten 50-Ω-Leistungsteiler/Combiner** oder eine dafür ausgelegte Summierstufe; danach muss die 50/75-Ω-Anpassung zur Kabelstrecke berücksichtigt werden. Kalibrieren Sie die tatsächlich erzeugten 300 ps zunächst elektrisch, etwa mit einer bekannten Verzögerungsänderung oder einem geeigneten Phasen-/Zeitintervallmessgerät.

## Bezug auf das Eclypse Z7

Die Eclypse Z7 ist eine modulare Plattform; die analogen Eigenschaften hängen vom eingesetzten Scope-Zmod ab. ([digilent.com](https://digilent.com/shop/eclypse-z7/?utm_source=openai)) Falls für **Ihr konkretes Scope-Modul** tatsächlich 1 MΩ/18 pF und ±1 V gelten, sind das die entscheidenden Angaben für den direkten Anschluss: **18 pF bei 10 MHz sind zu viel, um den Eingang als hochohmigen Abschluss einer 250-m-Leitung zu behandeln.** Zusätzlich sollte geprüft werden, ob ±1 V die zulässige Momentanspannung, den ADC-Vollbereich oder eine Softwareeinstellung bezeichnet.

Vor der eigentlichen Messreihe empfiehlt sich daher:

1. Mit einem VNA oder TDR Kabel, Steckverbinder und Abschlüsse prüfen.
2. Am Kabelende die 75-Ω-Terminierung und den Puffer einsetzen.
3. Beide Scope-Kanäle mit einem gemeinsamen, aufgeteilten 10-MHz-Signal speisen und Kanalversatz sowie Phasenrauschen bestimmen.
4. Die AWG-Injektion bei angeschlossener, vollständiger Messkette auf 300 ps kalibrieren.
5. Erst danach die 250-m-Strecke messen und die Temperatur der Leitung mitprotokollieren.

**Fazit:** Die saubere Lösung ist nicht, das 75-Ω-Kabel direkt an einen nominell 1-MΩ-Scope-Eingang zu hängen, sondern die Leitung am Ende definiert mit 75 Ω abzuschließen und das Signal dort mit einem niederkapazitiven Puffer für den ADC aufzubereiten. Die konkrete Anpassungs- und Schutzschaltung lässt sich erst endgültig dimensionieren, wenn das genaue Scope-Zmod und dessen zulässiger Eingangspegel samt Eingangsnetzwerk feststehen.

**Citations:**
[1] [https://digilent.com/shop/eclypse-z7/](https://digilent.com/shop/eclypse-z7/)
[2] [https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md](https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md)
[3] [https://www.hb-digital.de/10m-coaxial-cable-HQ-135-dB-4-fold-shielded-steel-copper-white-4-F-plugs](https://www.hb-digital.de/10m-coaxial-cable-HQ-135-dB-4-fold-shielded-steel-copper-white-4-F-plugs)
[4] [https://www.wimo.com/en/dxpatrol-gpsdo-10mhz-reference](https://www.wimo.com/en/dxpatrol-gpsdo-10mhz-reference)

───────────────────────────────────────────────────────────────

[👤 Sie]
Ein Widerstandsnetzwerk wurde wie im Dokument beschrieben mit folgenden Komponenten aufgebaut: GPSDO - Dämpfung (-5dB) - 3-Wege-Kombinierer - 50/75 Ohm Anpassung - SATA Kabel 1 m (für Null-Test) - 75/50 Ohm Anpassung - SDR-1. Die Dämpfung der gesamten Strecke wurde mit Nano VNA bei 10 MHz etwa wie erwartet mit -22.5dB gemessen. Nach Wechsel des 1m SATA-Kabels gegen ein 250m SATA-Kabel stieg die Dämpfung wie im NanoVNA Screenshot gezeigt auf -55dB an. Es zeigte sich nun ein Tiefpasscharakter mit einer -3dB Frequenz bei etwa 250 kHz  wie ebenfalls in einem angehängten Screenshot vom Nano VNA gezeigt. Nach Datenblatt sollte die zusätzliche Dämpfung bei 250m/10MHz etwa -5dB sein, sie ist jedoch mehr als -30dB. Der ohmsche Widerstand der Schirmung ist etwa 8 Ohm und des Innenleiters etwa 34 Ohm. Welche Erklärung könnte es dafür geben? Wie kann das System verändert werden, damit das 10MHz Signal des GPSDO mit der nominellen Dämpfung am Ausgang ankommt?

📎 Anhänge: Anpassung und Kombinierer 3 MHz (GPSDO - SATA_250m - SDR-1).jpg, Anpassung und Kombinierer 10 MHz (GPSDO - SATA_250m - SDR-1).jpg

───────────────────────────────────────────────────────────────

[🤖 KI]
Die von Ihnen gemessenen DC-Widerstände des 250-m-Kabels – **ca. 34 Ω Innenleiter** und **ca. 8 Ω Schirmung** – liefern die sofortige und eindeutige physikalische Erklärung für den extremen Abfall und den Tiefpasscharakter:

Es handelt sich bei diesem spezifischen HQ-135-dB-Sat-Kabel (und vielen vergleichbaren günstigen Koaxialkabeln für den Hausgebrauch) um ein Kabel mit **CCS-Innenleiter** (Copper-Clad Steel, also verkupferter Stahldraht) und oft einem Schirm aus Aluminium-Geflecht mit verkupferter Aluminiumfolie (CCA) oder einem sehr dünnen Drahtgeflecht. 

### Die Ursachen im Detail:

1. **Massiver Ohmscher DC-Widerstand (Gleichstromwiderstand):**
   * Ein Innenleiter-Widerstand von 34 Ω auf 250 m (bzw. 68 Ω Schleifenwiderstand mit Schirm) ist für HF-Anwendungen im MHz-Bereich bei Fehlanpassung oder bei falscher Betrachtung tödlich, vor allem aber wirkt dieses Kabel bei DC wie ein hochohmiger Vorwiderstand.
   * Noch wichtiger bei 10 MHz ist jedoch:

2. **Skin-Effekt und Stahlinnenleiter (CCS):**
   * Bei 10 MHz ist die Eindringtiefe (Skin-Tiefe) in Kupfer ca. 20 µm. Die Kupferschicht auf dem Stahldraht (CCS) ist extrem dünn (oft nur wenige Mikrometer). 
   * Sobald die HF-Ströme tiefer in den billigen Stahlkern vordringen müssen, steigt der HF-Widerstand (AC-Resistance) im Vergleich zu purem Kupfer um ein Vielfaches an.
   * Ein 250-m-Stahl-Kupfer-Kabel hat bei 10 MHz einen dramatisch höheren Wirkwiderstand als die für reine TV-Frequenzen (ab ~50 MHz aufwärts) ausgelegten Katalogwerte vermuten lassen. Bei Sat-Kabeln ist die Dämpfung im Datenblatt meist für **1000 MHz bis 2000 MHz** angegeben. Bei 10 MHz schlägt der ohmsche Widerstand des Innenleiters voll durch und erzeugt eine gigantische axiale Verlustleistung, während das Kabel bei noch höheren Frequenzen durch die kapazitiven/induktiven Beläge zusätzlich als verteilter RC-Tiefpass wirkt.

3. **Fehlanpassung und Zusatzdämpfung:**
   * Wenn das Kabel über 250 m einen so hohen Serienwiderstand hat, bricht die Impedanz an den Enden komplett zusammen (keine saubere 75-Ω-Welle mehr aufrechtzuerhalten). Die gemessene –55 dB Dämpfung setzen sich aus den ohmschen Leitungsverlusten und massiven Reflexions- bzw. Anpassungsverlusten an den Übergängen zusammen.

---

### Wie kann das System verändert werden, damit das 10-MHz-Signal ankommt?

Um die 250 m mit einem 10-MHz-Referenzsignal zu überbrücken, gibt es im Wesentlichen zwei Wege:

#### Weg A: Wechsel auf ein echtes HF-Koaxialkabel (Vollkupfer)
Ersetzen Sie das CCS-Sat-Kabel durch ein **professionelles 50-Ω- oder 75-Ω-Koaxialkabel mit massivem Kupfer-Innenleiter (Solid Bare Copper)** und gutem Kupfergeflecht (z. B. RG-213, AIRCELL 7 / ECOFLEX 10 für 50 Ω oder ein hochwertiges professionelles RG-11 bzw. Broadcast-Kabel für 75 Ω). 
* Ein Vollkupfer-Kabel mit größerem Querschnitt hat bei 10 MHz auf 250 m nur einen Bruchteil des Widerstands (oft unter 2 Ω Schleifenwiderstand) und dämpft das 10-MHz-Signal tatsächlich im Rahmen der erwarteten wenigen Dezibel.

#### Weg B: Aktive Signalaufbereitung / Repeater (falls das Kabel physikalisch bleiben muss)
Wenn das 250-m-SATA-Kabel aus mechanischen Gründen (Verlegung, Bestand) zwingend im System verbleiben muss, lässt sich das 10-MHz-Signal **nicht** passiv verlustfrei durchschleusen:
1. **Sendeseitige Anhebung (Pre-emphasis):** Den Pegel am Sender massiv erhöhen (wobei das CCS-Kabel bei hoher Leistung thermisch stark belastet wird).
2. **Empfangsseitige Verstärkung (Line-Receiver / Breitbandverstärker):** Da das Kabel wie ein RC-Tiefpass mit Grenzfrequenz bei ~250 kHz wirkt, ist das 10-MHz-Signal am Ende extrem schwach und phasenverzerrt. Ein direkt am Kabelende (vor dem SDR/ADC) platzierter, rauscharmer Breitbandverstärker mit hoher Eingangsimpedanz (oder ein auf 10 MHz abgestimmter Resonanzverstärker / Filter) muss das Signal aufbereiten. 
   * *Achtung für den Phasendetektor:* Jeder analoge Verstärker fügt phasenjitter und temperaturabhängige Phasenverschiebungen hinzu, was die Messung der Raumzeitdynamik verfälschen kann. **Weg A (Vollkupfer-Kabel) ist daher physikalisch dringend zu bevorzugen.**

───────────────────────────────────────────────────────────────

[👤 Sie]
Danke für die Hinweise. Das Sat-Kabel (nicht SATA, war falsch) war mit CCS Innenleiter (Fehlkauf, wider besseren Wissens) soll mit folgendem Typ mit Kupfer ausgetauscht werden: https://www.willisat.de/sat-empfang/kabel-co/premiumx-profi-250-m-koaxialkabel-135-db-4-fach-geschirmt-cu-reines-kupfer-satelliten-koaxkabel-antennenkabel-digital-sat-tv-koax-kabel-rg-6-class-a-250m-auf-holz-trommel-mit-10x-f-stecker-vergoldet-mit-dichtring_122626_17917 Sind von diesem Kabel die technischen Daten bekannt? Gibt es eventuell noch andere Kabel, die besser geeignet wären? Aktuell werden Nullmessungen mit direktem GPSDO-Signal an beiden Kanälen sowie mit einem 1m-Kabel am Widerstandsnetzwerk wie im Anhang gezeigt durchgeführt. Bei direktem GPSDO-Signal an beiden Kanälen wird ein geringfügiger Offset von 26.1 ps mit einer Schwankungsbreite von 9 ps wie im Anhang gezeigt sichtbar. Bei Anschluss eines 1m-Kabels mit Anpassung und Kombinierer wird neben der Phasenverschiebung durch die Kabellänge eine Schwankungsbreite von ca. 600 ps sichtbar. Zu deren Reduktion sind bereits eine vollständige Abschirmung in einem Stahlblech-Behälter sowie Ferritringe auf den Versorgungsleitungen versucht worden, jedoch ohne erkennbaren Einfluss. Was könnte die Ursache der hohen Schwankungsbreite des Phasensignals sein? Wie könnte sie sich verringern lassen?

📎 Anhänge: Widerstandsnetzwerk.jpg, GPSDO zu Sat-1m zu SDR1 und GPSDO zu SDR2.jpg, GPSDO zu SDR1 und SDR2.jpg

───────────────────────────────────────────────────────────────

[🤖 KI]
### 1. Zum neuen Kabel (PremiumX Profi 250 m mit Vollkupfer-Innenleiter)

* **Bekannte Technische Daten:** Es handelt sich um ein klassisches **RG-6**-Äquivalent (Class A, 4-fach geschirmt) mit einer Impedanz von **75 Ω**, einem Schirmungsmaß von bis zu **135 dB** und – im Gegensatz zum vorherigen Fehlkauf – einem **Innenleiter aus 100 % reinem Kupfer (CU)**. Der Innenleiterdurchmesser bei RG-6 liegt typischerweise bei ca. 1,0 mm. Damit sinkt der Gleichstrom- und HF-Widerstand drastisch (der Schleifenwiderstand über 250 m sinkt von über 40 Ω auf nur noch wenige Ohm), sodass der im vorherigen Test gesehene massive Tiefpass und die –55 dB Dämpfung verschwinden und sich die erwarteten ca. 5 dB Dämpfung bei 10 MHz einstellen werden.
* **Gibt es bessere Alternativen?** Für den Frequenzbereich von 10 MHz und Längen von 250 m ist RG-6 mit Vollkupfer gut geeignet. Wenn es noch verlustärmer sein soll (oder wenn mechanisch robuster verlegt wird), greift man im Profi-/Messbereich oft zu **RG-11** (hat einen dickeren Innenleiter von ca. 1,63 mm Vollkupfer und dadurch nochmals spürbar geringere Dämpfung) oder zu professionellen 50-Ω-Koaxkabeln wie **Ecoflex 10 / Aircell 7** (falls man das System auf 50 Ω umbaut). Da Ihr System jedoch auf 75 Ω ausgelegt ist, stellt das gewählte PremiumX-Vollkupferkabel einen sehr guten und praxisgerechten Kompromiss dar.

---

### 2. Ursache für die hohe Schwankungsbreite (600 ps) beim 1m-Kabel mit Netzwerk

Wenn die Nullmessung direkt am Scope stabil ist (~9 ps Jitter), aber mit dem 1-m-Widerstandsnetzwerk, den Anpassungen und dem Kombinierer **600 ps** Jitter bzw. Schwankung auftreten, obwohl Abschirmung und Ferrite nichts brachten, kommen primär folgende Ursachen in Betracht:

1. **Massive Gleichtakt-Störungen (Common-Mode / Ground Loops) über die Masse-Ebene:**
   * Das Eclypse Z7 hängt über USB am PC, der GPSDO hat oft ein eigenes Netzteil (oder Netzanschluss mit Schutzleiter), und AWG/Oszilloskop sind über die Ground-Ebene des Gehäuses oder Netzteile miteinander verkoppelt. 
   * Über das passive Widerstandsnetzwerk und die Kombinierer entsteht eine galvanische Kopplung. Wenn hochfrequente Störströme (z. B. 100-Hz/kHz-Schaltnetzteilreste, Digitalsplitter-Taktungen des Z7) über die Kabelschirme oder die Signalmasse fließen, moduliert dies direkt den Nulldurchgang des 10-MHz-Sinussignals. 
   * *Gegenmaßnahme:* **Galvanische Trennung (RF-Transformer / Baluns)** in den Signalweg einbauen, anstatt alles rein ohmsch und galvanisch direkt zu koppeln.

2. **Reflexionen durch unvollständige Anpassung (Stehende Wellen):**
   * Das Netzwerk kombiniert 50-Ω-Quellen (GPSDO, AWG) und wandelt über Widerstände auf 75 Ω (bzw. zurück). Wenn die Widerstände im Netzwerk (oder die 50/75-Ω-Anpassglieder) auch nur geringfügig von den idealen Werten abweichen (oder parasitäre Kapazitäten auf der Platine/Steckverbindungen haben), entstehen Rückreflexionen. 
   * Diese Reflexionen laufen zwischen GPSDO/AWG und dem Netzwerk hin und her. Da ein Oszilloskop-Eingang (oder ein ungepufferter SDR-Eingang) oft kein perfekter 50/75-Ω-Abschluss ist, verändert sich die effektive Phase am Triggerpunkt massiv in Abhängigkeit von kleinsten thermischen oder Laständerungen.

3. **Übersprechen (Crosstalk) vom AWG auf den Referenzkanal:**
   * Der AWG (Eclypse Z7 Ausgänge) liefert ein programmierbares Signal zur Phasenmodulation. Wenn dieses Signal im Kombinierer oder auf der Platine kapazitiv oder induktiv in den GPSDO-Referenzkanal überspricht, verzerrt das die Nulldurchgänge des Sinus asymmetrisch. Das Oszilloskop misst dann keinen sauberen Phasenversatz mehr, sondern einen durch Amplitudenmodulation/Übersprechen verursachten, stark schwankenden scheinbaren Jitter.

---

### 3. Wie lässt sich die Schwankungsbreite verringern?

* **Gezielte HF-Entkopplung statt nur DC-Ferriten:** Ferrite auf DC-Leitungen helfen oft nicht gegen HF-Ströme, die direkt auf den Signalmassen fließen. Nutzen Sie kleine HF-Breitband-Übertrager (Baluns / Impedanzwandler mit Ferritkernen wie Mini-Circuits-Übertrager oder selbstgewickelte Breitbandtrafos auf Binokularkernen BN43-2402), um die Massekreise zwischen GPSDO, AWG und Messgerät galvanisch zu trennen.
* **Aktive Pufferung / Impedanztrennung:** Statt das Signal über ein rein passives Widerstandsnetzwerk zu kombinieren (welches Signalquellen aufeinander rückwirken lässt), sollte die Injektion über eine entkoppelte aktive Summierstufe (z. B. mit schnellen, rauscharmen Breitband-OPVs oder HF-Transistoren mit hoher Bandbreite) erfolgen, die Rückwirkungen des AWG auf den GPSDO verhindert.
* **Prüfung mit reiner Terminierung:** Testen Sie das Widerstandsnetzwerk schrittweise: Erst nur GPSDO durch das Netzwerk auf Kanal 1 und 2 (ohne AWG, ohne langes Kabel) – sinkt der Jitter hierbei bereits auf wenige Pikosekunden, liegt das Problem beim AWG-Übersprechen oder der Einspeisung. Bleibt der Jitter hoch, ist das passive Widerstandsnetzwerk bezüglich seiner Rückflussdämpfung (Return Loss) an den Port-Übergängen fehlangepasst.

**Citations:**
[1] [https://picclick.de/SAT-Koaxialkabel-135dB-5-fach-VOLLKUPFER-TV-Antennenkabel-CCS-403268601853.html#1](https://picclick.de/SAT-Koaxialkabel-135dB-5-fach-VOLLKUPFER-TV-Antennenkabel-CCS-403268601853.html#1)
[2] [https://www.kaufland.at/product/310250791/](https://www.kaufland.at/product/310250791/)
[3] [https://www.kaufland.de/product/324473356/#1](https://www.kaufland.de/product/324473356/#1)
[4] [https://www.kaufland.de/product/358228566/#1](https://www.kaufland.de/product/358228566/#1)
[5] [https://www.testberichte.de/pdb/p/6L8ONUTjTDmwNDOinp6tZg.html#1](https://www.testberichte.de/pdb/p/6L8ONUTjTDmwNDOinp6tZg.html#1)

═══════════════════════════════════════════════════════════════
  Ende der Konversation
═══════════════════════════════════════════════════════════════
