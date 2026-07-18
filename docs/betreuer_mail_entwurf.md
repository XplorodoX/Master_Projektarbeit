# Entwurf: Mail an den Betreuer (Kriterienrevision)

> ⚠️ **Entwurf — nicht versendet.** Bitte lesen, an deinen Ton anpassen, Namen einsetzen und
> selbst verschicken. Der Inhalt beruht auf dem Stand vom 17.07.2026 (n=7).

**Warum überhaupt:** Das Exposé setzt ≥ 90 % über 100 unbekannte Seeds an, die Projektarbeit
berichtet gegen 70 %/60 %. Diese Differenz findet ein Prüfer beim Nebeneinanderlegen der beiden
Dokumente in zwei Minuten. Sie proaktiv anzusprechen kostet nichts und nimmt dem Punkt die
Schärfe — sie zu übergehen, macht aus einer begründeten Revision einen entdeckten Widerspruch.

---

**Betreff:** Projektarbeit Stoneforge RL — Revision des Erfolgskriteriums, kurze Rückmeldung erbeten

Sehr geehrte/r Frau/Herr [Name],

im Rahmen der Endauswertung meiner Projektarbeit möchte ich Sie auf einen Punkt hinweisen, den
ich in der Dokumentation offengelegt habe und der eine Abweichung vom Exposé betrifft.

**Der Sachverhalt:** Im Exposé hatte ich als Evaluierungsmetrik eine Success Rate von ≥ 90 % über
100 unbekannte Seeds angesetzt. In der Dokumentation berichte ich gegen ein abgesenktes Kriterium
(≥ 70 % auf dem Testset, ≥ 60 % auf einem Holdout-Set). Diese Absenkung ist im Projektverlauf
entstanden und in der Arbeit als solche gekennzeichnet — ich wollte sie nicht stillschweigend
vornehmen. Wichtig zur Einordnung: Die revidierten Kriterien sind seit dem 08.06.2026 im
Experiment-Changelog dokumentiert, also einen Monat vor den finalen Trainingsläufen — sie wurden
nicht in Kenntnis der Endergebnisse gewählt. Dass das Testset-Kriterium am Ende dennoch knapp
verfehlt wird, zeigt zugleich, dass auch die revidierte Schwelle keine bequeme war.

**Die Begründung**, die ich im Abschnitt „Revision des Erfolgskriteriums" ausführe:

1. Die Aufgabe ist partiell beobachtbar (Sichtradius 7, Ziel nicht sichtbar). In POMDPs verliert
   eine deterministische Politik ihre Optimalitätsgarantie [Singh et al. 1994] — ein Abstand
   zwischen stochastischer und deterministischer Auswertung ist dort erwartbar. Ein Schwellenwert
   von 90 % unterstellt implizit eine nahezu vollständig lösbare Aufgabe.
2. Eine Umgebungsrevision im Juli deckte auf, dass die Distanzvorgabe bis dahin als Luftlinie
   gemessen wurde: „35–45 Felder" entsprach real einem Laufweg von 42–75 Feldern. Die Aufgabe war
   also schwerer, als sie deklariert war.
3. Die Streuung über Trainingsläufe beträgt in dieser Umgebung rund ± 12 Prozentpunkte. Ein
   Schwellenwert von 90 % liegt außerhalb dessen, was die Methode hier stabil liefert.

**Der aktuelle Stand** (sieben unabhängige Trainingsläufe, Mittel ± Std):

- Testset A: 65,7 % ± 12,4 → Kriterium (≥ 70 %) **verfehlt**
- Holdout B: 66,9 % ± 12,8 → Kriterium (≥ 60 %) erfüllt
- Gegen die Exposé-Metrik gerechnet (100 unbekannte Seeds): 66,3 % ± 12,1 → **deutlich unter 90 %**

Ich berichte den Exposé-Wert bewusst mit, damit die Revision nicht wie das Austauschen eines
unbequemen Maßstabs wirkt.

**Ein Zwischenergebnis halte ich für den methodisch interessantesten Teil:** Eine Auswertung über
zunächst nur drei Läufe hatte 73,3 % / 80,0 % ergeben und beide Kriterien scheinbar erfüllt. Da
der Abstand zur Schwelle kleiner war als die Streuung, habe ich vier weitere Läufe gestartet — und
beide Werte fielen deutlich. Der Dreier-Mittelwert war zu optimistisch. Das ist genau der Effekt,
vor dem Henderson et al. (2018) für Deep RL warnen. Ich habe anschließend geprüft und
ausgeschlossen, dass ein Mess- oder Code-Fehler dahintersteckt (die deterministischen Werte der
ersten drei Läufe reproduzieren sich exakt; der Simulationscode ist zwischen beiden Chargen
unverändert).

**Meine Frage an Sie:** Ist die dokumentierte Revision in dieser Form für Sie in Ordnung, oder
sollte ich zusätzlich/anders gegen das ursprüngliche Kriterium berichten? Falls Sie den Abschnitt
vorab ansehen möchten, schicke ich Ihnen gerne die aktuelle Fassung.

Die im Exposé unter „Projektziele" formulierten Muss- und Nice-to-Have-Kriterien (C++-Engine,
pybind11-Gymnasium-Brücke, PPO-Agent auf unbekannten Seeds; Raylib-Client, PPO-LSTM,
Crafting-System) sind vollständig umgesetzt.

Vielen Dank und viele Grüße
Florian Merlau

---

## Hinweise zum Entwurf

- **Ton:** bewusst sachlich, keine Entschuldigung. Eine begründete Revision ist normal — nur das
  Verschweigen wäre ein Fehler.
- **Nicht defensiv werden.** Der Text nennt das Verfehlen zweimal ungeschönt. Das ist Absicht: Wer
  die schlechte Zahl selbst zuerst ausspricht, kontrolliert den Rahmen.
- **Die Frage am Ende ist echt**, keine rhetorische Floskel. Sie verwandelt eine Mitteilung in ein
  Gespräch und gibt dem Betreuer die Möglichkeit, die Revision mitzutragen.
- **Prüfen vor dem Senden:** Name/Anrede, ob dein Betreuer die Arbeit schon kennt, und ob dein
  Projektpartner (Laurin Röseler) mitzeichnen sollte.
