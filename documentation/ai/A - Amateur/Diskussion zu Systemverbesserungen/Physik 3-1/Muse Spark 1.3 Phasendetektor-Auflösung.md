═══════════════════════════════════════════════════════════════
  Muse Spark 1.3 Phasendetektor-Auflösung
═══════════════════════════════════════════════════════════════

Exportiert: 20. Oktober 2026 um 21:21
Nachrichten: 2
Modell: muse-spark-1.3

───────────────────────────────────────────────────────────────

[👤 Sie]
In einem Team aus 12 KIs und mir als Mensch sollen Verbesserungen für einen Phasendetektor diskutiert und erarbeitet werden, mit welchem Phänomene im Bereich Raumzeitdynamik untersucht werden sollen. Die Zusammenarbeit findet mit einem markdown-Dokument statt, welches im GitHub-Repository unter folgendem Link verfügbar ist:
https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md Aktuell geht es um die Transferfunktion. Als erste Komponente der Messkette wurde die Wandlung einer mit c propagierenden Metrikstörung in eine Laufzeitdifferenz in einem Kabel behandelt und das Ergebnis unter Physik 3 eingetragen. Die zweite Komponente der Messkette liest diese Laufzeitdifferenz aus. Zunächst sollen nochmals ein historisches analoges System mit Leitungstopologie Typ A und ein aktuelle laufendes digitales System mit Leitungstopologie vom Typ B analysiert werden. Das System A hatte zwei 16 MHz Oszillatoren in einem Abstand von 25 m, die mit einem RG58 Kabel an einem Ende auf einen NE612 Mischer geführt wurden. Manuell wurden die Oszillatoren auf Schwebung fein justiert mit einer maximalen Überlagerungsfrequenz von etwa 1-2 Hz, die dann mit einem Hochpassfilter unterdrückt wurde. Anschließend wurde das Signal auf den Eingang 1 eines MEphisto Scope 16-Bit ADC geführt. Gleichzeitig wurde das Signal an den Eingang eines AR5000 Empfängers geführt. Dort wurde nach einer fest eingestellten Verstärkung der ZF-Ausgang durch einen logarithmischen Detektor ADL5519 in ein DC-Signal gewandelt, das auf den Eingang 2 des MEphisto Scope geschaltet wurde. Der AR5000 realisiert zum ZF-Ausgang eine Bandpasscharakteristik im Bereich 5..15 MHz. Das digitale System B am ringförmigen Kabel mit 57,2 m Durchmesser realisiert die Idee der Kategorie B-1 im Dokument. Es sendet ein 500 kHz PWM-Signal von einem XMC4700 Mikrocontroller mit einem Leitungstreiber LTC1687CS in ein Cat 6a Kabel und wandelt das zurück kommende Signal in einen digitalen Pegel. Die Flanke wird mit einer programmierbaren Verzögerung durch einen 3D3428-0.25 einmalig fein abgestimmt bis die Flanke genau auf den Umschaltpunkt des Comparators im XMC4700 fällt. Das Erzeugen eines Messsignals wird durch Auszählen der höheren Werte des Comparators innerhalb von jeweils 1000 Flanken erhalten. Die Übertragungsfunktion dieser Flankentriggerung wurde im Bereich 200..800 als linear mit einem Anstieg von 100 ps/200 Zählwerten ermittelt. Das System stellt daher mit der programmierbaren Verzögerung in 64 Schritten einmalig oder wenn das Signal den linearen Bereich von 200..800 verlässt, einen Mittelwert der Zählwerte von etwa 500 ein (optimaler Arbeitspunkt). Die Verzögerungsschritte wirken auf den 144 MHz Takt des Comparators und können mit einer Schrittweite von 250 ps fein eingestellt werden, was im Zusammenspiel mit der 144 MHz Takt-Zählung des Comparators als Überlagerung eine effektive Schrittweite von etwa 30 ps ergibt. Welche Auflösung beim Auslesen der Laufzeitdifferenz kann aus der Beschreibung für die Systeme A und B abgeschätzt werden?

───────────────────────────────────────────────────────────────

[🤖 KI]
Für beide Leseketten lässt sich aus Ihrer Beschreibung
eine **Auslese-Auflösung für $\delta\tau$** abschätzen -
nicht die Falsifikationsschwelle des Gesamtsystems,
sondern was die Elektronik am Ausgang an kleinster
Laufzeitänderung noch in Zählwerten / Volt umsetzt.

Gemeinsame Umrechnung:

$$\Delta\varphi = 2\pi f_0 \Delta\tau \qquad
\Delta\tau = \Delta\varphi / 2\pi f_0$$

d.h. bei festem $f_0$ sind Phase und Laufzeit
dieselbe Größe in zwei Einheiten.

### System A - historisch, analog, Typ A

Kette:

> 2x 16 MHz frei, 25 m RG58 auf ein Ende,
> NE612 als Mischer/Phasendetektor,
> manuell auf Schwebung 1-2 Hz, Hochpass,
> Ch1 MEphisto Scope 16 Bit direkt,
> parallel AR5000 5-15 MHz ZF -> ADL5519 Log-Detektor -> Ch2.

1. **Direktpfad Ch1 - Phasenflanke:**

Periode bei 16 MHz:

$$T_0 = 62.5\,{\rm ns}$$

$2\pi \triangleq 62.5$ ns.

Ein Diodenring-/Gilbert-Mischer als Phasendetektor
hat im Nulldurchgang typisch:

$$dU/d\varphi \sim 0.1-0.5\,{\rm V/rad}$$

bei einigen 100 mV Eingang.

16 Bit ADC heißt $2^{16}=65536$ Stufen.
Nutzbar nach Verstärkung/Rauschen realistisch
12-14 Bit $\triangleq$ 4000-16000 Stufen.

Für $\pm\pi/2$ linearer Bereich $\approx 3.1$ rad:

$$\delta\varphi_{LSB} \sim 3.1/10000 \approx 3\times 10^{-4}\,{\rm rad}$$

$$\delta\tau_{LSB} = \delta\varphi/2\pi f_0
\approx 3\times 10^{-4} / 10^{8}
\approx 3\,{\rm ps}$$

Das ist die **Quantisierungsgrenze**. Praktisch
begrenzt durch:

* relative Drift zweier freier 16 MHz Quarze:
  $10^{-7}$-$10^{-6}$/K, /Alterung, d.h. ns-Drift
  auf Minuten,
* 1-2 Hz Restschwebung: wurde hochpassgefiltert,
  d.h. quasistationär blind, nur Transienten
  $> \sim$2 Hz sichtbar,
* NE612 Offsetdrift, AM-PM, Kabel als Antenne,
* AR5000-Pfad ist kein Phasen- sondern
  Amplituden/Leistungspfad 5-15 MHz über
  ADL5519: löst keine ps-Laufzeit auf,
  sondern Hüllkurve/Rauschen als
  Veto-/Hilfskanal.

Realistische **Einzelabtast-Auflösung Ch1
nach Filterung: $10-100$ ps**, 
integriert über Sekunden auf $<10$ ps
Elektronik-Rauschen herunter mittelbar,
aber dann von Quarz-Allan und Temperatur
dominiert.

Wichtig für Physik 1/3-Trennung: A misst

$$\int \Delta f/f\,dt$$

zweier Freiläufer plus Kabel-Laufzeitglied,
nicht $\delta\tau$ bei festem gemeinsamem Takt.
Ohne Takt-Referenz ist keine Trennung
Eigenzeit/Takt/Laufzeit möglich.

### System B - digital, Typ B, Kreis 57.2 m

Kette wie von Ihnen beschrieben:

> 500 kHz PWM XMC4700 -> LTC1687CS -> Cat6a
> zurück -> Comparator, 3D3428-0.25 Feinverzögerung,
> Zählung HIGH in 1000 Flanken,
> linear 200..800 $\triangleq$ 100 ps,
> Arbeitspunkt-Regelung auf ~500 über 64x250 ps
> überlagert mit 144 MHz $\triangleq$ ~30 ps effektiv.

Daraus direkt:

Steigung:

$$S = 100\,{\rm ps} / 200\,{\rm counts}
= 0.5\,{\rm ps/count}$$

600 counts linearer Bereich $\triangleq$ ~300 ps
Fenster. 3D3428 Schritt 250 ps grob,
Vernier mit 6.94 ns /144 MHz $\rightarrow$
angegeben ~30 ps effektiv. Das passt:
Regel-DAC stellt Arbeitspunkt, Messgröße ist
Zählwert.

Rauschen einer 1000er-Messung, Binomial:

$$N=1000,\; p\approx 0.5$$
$$\sigma_N = \sqrt{Np(1-p)}\approx 15.8\,{\rm counts}$$

$$\sigma_{\tau,2ms} \approx 15.8 \times 0.5\,{\rm ps}
\approx 8\,{\rm ps}$$

bei 500 kHz sind 1000 Flanken = 2 ms.

Das ist die **Einzelmessung**.

Durch Mittelung:

* 0.5 s : ~250 Messungen -> $8/\sqrt{250}\approx 0.5$ ps
  reines Zählrauschen,
* praktisch begrenzt durch Comparator-Jitter
  XMC4700 + LTC1687, 3D3428-Drift, Cat6a
  thermisch/mechanisch, 144 MHz Phasenrauschen,
  Versorgung/USB.

Deshalb in Physik 1 dokumentiert:

> **effektiv 10-100 ps Systemauflösung**,

1 ps aus Design-Prompt ohne PDF ausdrücklich
**nicht** übernommen.

Regelschleife zu beachten: solange Zählwert
200..800 bleibt reine Zählwert-Zeitreihe
mit 0.5 ps/count umrechenbar. Bei Verlassen
springt 3D3428 um ~30/250 ps - das muss im
Datenstrom als Sprung + Delay-Code geloggt
werden, sonst entsteht künstlicher Chirp/
Sägezahn.

### Kurzvergleich für Physik 3

* **A:** Phasensteigung bei 16 MHz hoch,
  ADC-Auflösung ps-Level, aber Referenz
  zwei Freiläufer -> Drift ns-Level dominiert,
  keine absolute $\tau$-Kalibrierung,
  quasistationär durch Hochpass blind.
  Brauchbar als Event-/Transienten-Detektor,
  nicht als $\Delta\tau/\tau$-Meter.

* **B:** echte Zeitintervall-Quantisierung
  gegen gemeinsamen 144 MHz Takt,
  **~8 ps / 2 ms roh, 10-100 ps systematisch
  begrenzt**, linear 300 ps Fenster,
  darüber Verzögerungs-Regelung.
  Das ist die Zahl, die in die zweite Stufe
  der Transferfunktion eingeht:

$$\delta\tau_{min} \approx 10-100\,{\rm ps}
\;\;{\rm bei}\;\; \tau\approx 2\,\mu{\rm s}-2\,{\rm ms}$$

je nach angenommener elektrischer Länge.

Vorschlag zur Ablage in Physik 3:

Rohgröße, $f_0$, $S=0.5$ ps/count, $N=1000$,
Delay-Code, $T_{sample}$ festhalten,
Nullmessung abgeschlossen/terminiert und
Injektion nach Messung 3 als nächste
Pflichtmessung - erst dann ist $S$ kalibriert
statt nur aus Flankensteilheit abgeleitet.

**Citations:**
[1] [https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md](https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md)

═══════════════════════════════════════════════════════════════
  Ende der Konversation
═══════════════════════════════════════════════════════════════
