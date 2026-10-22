═══════════════════════════════════════════════════════════════
  Muse Spark 1.3 Digital-I/Q-Phasendetektor
═══════════════════════════════════════════════════════════════

Exportiert: 22. Oktober 2026 um 22:47
Nachrichten: 2
Modell: muse-spark-1.3

───────────────────────────────────────────────────────────────

[👤 Sie]
In einem Team aus 12 KIs und mir als Mensch sollen Verbesserungen für einen Phasendetektor diskutiert und erarbeitet werden, mit welchem Phänomene im Bereich Raumzeitdynamik untersucht werden sollen. Die Zusammenarbeit findet mit einem markdown-Dokument statt, welches im GitHub-Repository unter folgendem Link verfügbar ist:
https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md Basierend auf den Verbesserungsvorschlägen in „Physik 3-2“ soll als nächstes ein Konzept mit digitalem I/Q mit einer fertigen Hardware Eclypse Z7 + ADC + AWG untersucht werden. Nachteilig ist, dass kein externer Oszillator angeschlossen werden kann, wohingegen die fertige Hardware ein Vorteil bei der Umsetzung ist, welche vollständig in Software erfolgt. Das Konzept führt weiterhin die bisher getrennten Systeme analog und digital zu einem System zusammen, da der AWG (Takt 125 MHz) sowohl Sinus als auch Rechteck als auch komplexe Signalformen erzeugen kann. Als Kabeltopologie bietet sich vorzugsweise Typ B an, wo das Signal des AWG durch ein 30 m langes gerades Sat-Kabel hin und zurück läuft, bevor es mit dem ADC mit hoher Samplingrate von 105 MHz/14 Bit ausgewertet wird. Der verbaute FPGA hat gleichzeitig Zugriff auf das gesendete und das empfangene Signal und soll daraus kontinuierlich die Laufzeitdifferenz im Kabel ermitteln. Das Signal der Laufzeitdifferenz soll anschließend im Frequenzbereich von 0-50 MHz weiter ausgewertet werden. Lässt sich aus diesen Informationen eine Systemempfindlichkeit für die Laufzeitdifferenz im Kabel abschätzen?

───────────────────────────────────────────────────────────────

[🤖 KI]
In einem Team aus 12 KIs: Beitrag `Muse Spark` zu `Physik 3-2 / I-Q Konzept Eclypse Z7`

Ja - lässt sich grob abschätzen. Topologie B, digital, kohärent.

### 1. Annahmen

* Kabel: 30 m gerade hin + 30 m zurück = `L_el = 60 m` Sat-Koax
* `v = VF*c`, `VF~0.66-0.8`, typ. 0.75:
> `tau = L/v ~ 60 / 2.25e8 ~ 267 ns`
> + wenige ns Analog-Frontend. Nennlaufzeit $\tau_0 \approx 270$ ns.

* Sendung: AWG 125 MHz, Empfang: ADC 105 MSps / 14 Bit
* FPGA sieht digital Tx-Soll und Rx-Ist gleichzeitig.
* Auswertung: digitaler I/Q-Demodulator / DDC, Träger $f_0$ frei wählbar 1-50 MHz, z.B. 10 MHz oder 30 MHz.
* Gesucht: $\delta\tau(t) = \Delta\phi(t)/2\pi f_0$

Das ist der entscheidende Vorteil gegenüber dem XMC4700-Capture:
kein 7ns-Quant mehr, sondern Phaseninterpolation des Trägers.

### 2. Single-Shot Grenze ADC

14 Bit = 86 dB theoretisch. Real ENOB am Z7-AD9648 o.ä. bei HF:

> ENOB ~ 10.5 - 11.5 Bit => SINAD ~ 65-71 dB

Für einen Vollskalen-Sinus gilt für die Phase:

$$\sigma_\phi \approx \frac{1}{\sqrt{2\cdot SNR_{lin}}}$$

$$SNR = 70dB = 10^7 \Rightarrow \sigma_\phi \approx 2.2e-4\, rad$$

Umrechnung in Zeit:

$$\sigma_t = \frac{\sigma_\phi}{2\pi f_0}$$

* bei $f_0=10$ MHz: $\sigma_t \approx 3.5$ ps pro Sample
* bei $f_0=30$ MHz: $\sigma_t \approx 1.2$ ps pro Sample
* bei $f_0=50$ MHz: $\sigma_t \approx 0.7$ ps pro Sample

Das ist die thermische + Quantisierungsgrenze pro 9.5ns-Sample. Höheres $f_0$ ist linear besser, solange Kabeldämpfung und ADC-Bandbreite es zulassen. Für Sat-Kabel: 30 MHz ist guter Kompromiss.

### 3. Mittelungsgewinn

Rauschleistung wird mit $N = f_s \cdot T_{int}$ gemittelt:

$$\sigma_\tau(T) \approx \frac{\sigma_t}{\sqrt{f_s T}}$$

$f_s = 105$ MHz:

* $T=1$ ms, $N=1.05e5$: $\sigma \approx 1.2ps/324 \approx 3.7$ fs
* $T=1$ s, $N=1.05e8$: $\sigma \approx 1.2ps/10246 \approx 0.12$ fs
* $T=100$ s: $\approx 0.012$ fs

Theoretisch also: **fs in Sekunden, sub-fs bei Minuten.**

Bezogen auf $\tau_0 \approx 270$ ns:

$$\delta\tau/\tau \approx 1ps/270ns \approx 3.7e-6\, single$$
$$\approx 4e-10\, bei\, 1s\, (0.1fs)$$
$$\approx 4e-11\, bei\, 100s$$

Damit wäre die in Physik 3 genannte Forderung $\Delta t \sim 1$ fs bei $10^{-8}$-Störung formal erreichbar - **nur auf dem Papier.**

### 4. Warum real nicht: Takt und Umwelt

Hier schlägt der von Dir genannte Nachteil zu:

1. **Kein externer Oszillator:** AWG 125 MHz und ADC 105 MHz werden aus dem gleichen On-Board 33.33 MHz via PLL abgeleitet. Datenblatt-Jitter Z7 PLL: typ. 2-5 ps rms, plus deterministischer Fractional-Spur. Da Sende- und Empfangstakt aus derselben Referenz kommen, fällt Langzeitdrift in erster Ordnung heraus - wie beim XMC4700. Kurzzeit-Jitter der beiden unterschiedlichen PLLs fällt **nicht** heraus. Boden: ~1-3 ps, nicht mittelbar unter ~0.1-1 ps ohne externen 10 MHz.

2. **Kabel-Thermik dominiert alles:** Cu + PE-Dielektrikum: $\alpha \approx 1e-5 - 5e-5 /K$.
> $270ns * 2e-5 = 5.4 ps/mK$

1 mK Raumänderung = 50x mehr als 0.1 fs Ziel. Ohne thermisch gekoppelte Referenzstrecke (Methodik 8) oder Differenzmessung ist die digitale Auflösung leer.

3. **AWG-DAC Rauschen, Verstärkerdrift, ADC-INL, USB/Ethernet-Last** wie bisher.

Realistische Empfindlichkeit im Labor ohne Thermostat:

> **Einzelschuss ~ 5-20 ps, 1s-gemittelt ~ 0.5-2 ps, Tagesdrift ~ 10-100 ps**

d.h. $\delta\tau/\tau \sim 2e-6$ bis $4e-4$, identisch zum bisherigen System $5e-6$ bis $5e-5$ aus Physik 1. Der Gewinn ist Bedienbarkeit + Bandbreite 0-50 MHz + komplexe Wellenformen, **nicht** Größenordnungen in $\delta\tau$.

### 5. Empfehlung für Physik 3-2

* Konzept als **Software-Defined-Phase-Meter** weiterführen: Typ B 30m, $f_0=10/30$ MHz umschaltbar, I/Q + CORDIC im FPGA, Ausgabe $\Delta\phi, \Delta\tau$ + Amplitude als Vektor.
* Unbedingt Tx-Loopback im FPGA als Null-Kanal mitmessen: trennt PLL-Jitter von Kabel-$\tau$.
* Nächster Schritt danach: zweite thermisch gekoppelte Leitung gleicher Länge (Qwen-Vorschlag) - sonst bleibt jede fs-Rechnung Theorie.
* Injektionstest Pflicht vor Kosmologie: bekannte 1-10 ps Verzögerung via Feinstufe / Varicap einspeisen, Linearität Pause/Burst -> I/Q übertragen.

Kurz: **ADC-Auflösung begrenzt nicht, Takt-Jitter + Kabel-Temperatur begrenzen.** Digital-I/Q bringt Faktor 2-5 in Flexibilität und Bandbreite, keinen Faktor 10 in $\Delta t$ ohne externen Takt und ohne Referenzarm.


**Citations:**
[1] [https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md](https://github.com/WegaLink/Spacetime-Dynamics/raw/refs/heads/main/documentation/ai/A%20-%20Amateur/Diskussion%20zu%20Systemverbesserungen/Fragenkatalog%20zu%20einem%20Phasendetektor.md)

═══════════════════════════════════════════════════════════════
  Ende der Konversation
═══════════════════════════════════════════════════════════════
