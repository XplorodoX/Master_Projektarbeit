# GPU- und Cloud-Training (MacBook & Kaggle)

Diese Dokumentation beschreibt, wie das Reinforcement Learning-Training (speziell für das CNN-Netzwerk in `train_cnn.py`) beschleunigt und auf GPUs (entweder lokal auf dem MacBook oder kostenlos in der Cloud auf Kaggle) ausgelagert werden kann.

---

## 1. Lokales GPU-Training auf Apple Silicon (MacBook)

Moderne MacBooks mit Apple Silicon (M-Chips) besitzen eine integrierte GPU, die von PyTorch über **MPS (Metal Performance Shaders)** angesprochen werden kann.

### Schritt 1: SB3-Konfiguration anpassen
Stable-Baselines3 verwendet standardmäßig `"cpu"`, wenn es auf macOS läuft. Du musst das Device explizit in den Algorithmus-Optionen in [train_cnn.py](file:///Users/merluee/Master_Projektarbeit/scripts/train_cnn.py) setzen:

```python
# In scripts/train_cnn.py
RPPO_KWARGS = dict(
    policy="MlpLstmPolicy",
    device="mps",  # Aktiviert Apple Silicon GPU-Beschleunigung
    n_steps=256,
    batch_size=64, # Erhöht für effizientere GPU-Auslastung (Standard war 8)
    ...
)
```

### Schritt 2: Batch-Größe erhöhen
Bei einem sehr kleinen Batch (z. B. `batch_size=8`) übersteigt der Datentransfer-Overhead zwischen CPU (Simulation) und GPU (Lernen) oft die Rechenersparnis. 
* **Empfehlung:** Setze `batch_size` auf mindestens `64` oder `128` bei GPU-Lauf.

### Schritt 3: Simulation parallelisieren
Nutze beim Starten den Parallelisierungsmodus für die CPU-Umgebungen:
```bash
.venv/bin/python scripts/train_cnn.py --subproc
```
Dies verteilt das Generieren der Rollouts über `SubprocVecEnv` parallel auf alle CPU-Kerne deines MacBooks.

### Schritt 4: Release-Build der C++ Simulation sicherstellen
Stelle sicher, dass die C++-Erweiterung im `Release`-Modus gebaut wurde, um maximale Geschwindigkeit beim Environment-Stepping zu garantieren:
```bash
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j
```

---

## 2. Cloud-Training auf Kaggle (Kostenlose Nvidia GPUs)

Kaggle stellt wöchentlich 30 Stunden kostenlose GPU-Laufzeit (Nvidia Tesla T4 oder P100) zur Verfügung. Da dort eine vollwertige Linux-Umgebung läuft, lässt sich das C++-Simulationsmodul dort einfach kompilieren.

### Schritt 1: Code hochladen
* **Über GitHub (Empfohlen):** Pushe dein Repository (kann privat sein) auf GitHub und klone es im Kaggle Notebook.
* **Als Kaggle Dataset:** Erstelle ein `.zip` deines Projekts (ohne `.venv` und `build/`) und lade es als privates Dataset in Kaggle hoch.

### Schritt 2: C++ Modul auf Kaggle kompilieren
Führe folgende Befehle in einer Notebook-Zelle aus, um das C++-Modul auf der Kaggle-Linux-Instanz zu kompilieren:

```python
# (Ggf. zuerst ins Projektverzeichnis wechseln, z.B. %cd Master_Projektarbeit)

!mkdir -p build
%cd build
!cmake -DCMAKE_BUILD_TYPE=Release ..
!make -j$(nproc)
%cd ..
```
*Hinweis: Dies erzeugt das Linux-kompatible Modul `stoneforge_sim.so`.*

### Schritt 3: Python-Bibliotheken installieren
Da PyTorch bereits vorinstalliert ist, müssen nur die RL-Bibliotheken ergänzt werden:

```python
!pip install stable-baselines3[extra] sb3-contrib gymnasium
```

### Schritt 4: PYTHONPATH einrichten
Kopiere das kompilierte Modul direkt in den `python/`-Ordner deines Repos, damit es beim Importieren von `stoneforge_env` gefunden wird:

```python
!cp build/stoneforge_sim*.so python/
```

### Schritt 5: Accelerator aktivieren & Training starten
1. Klicke im Kaggle-Notebook rechts unter **Settings** -> **Accelerator** und wähle **GPU T4 x2** oder **GPU P100**.
2. Führe das Training aus:
   ```python
   !python scripts/train_cnn.py --subproc
   ```

### Schritt 6: Modelle sichern
Nach dem Training liegen die Modelle im temporären Kaggle-Dateisystem (`/kaggle/working/`). Lade sie über das Dateimenü rechts herunter oder lade sie programmatisch in dein Google Drive hoch.

---

## 3. Verbindung mit VS Code (VS Code Remote Tunnels)

Kaggle bietet von Haus aus keinen direkten SSH-Zugang an. Du kannst dich aber über ein **VS Code Remote Tunnel** direkt aus deinem lokalen VS Code mit dem laufenden Kaggle-Container verbinden. Dadurch kannst du Code auf Kaggle bearbeiten und ausführen, als wäre er lokal.

### Schritt 1: VS Code CLI in Kaggle herunterladen und starten
Führe in einer Zelle deines Kaggle-Notebooks folgenden Code aus:

```python
# VS Code CLI herunterladen und entpacken
!curl -Lk 'https://code.visualstudio.com/sha/download?build=stable&os=cli-alpine-x64' --output vscode_cli.tar.gz
!tar -xf vscode_cli.tar.gz

# Tunnel starten (einmalige GitHub/Microsoft Authentifizierung erforderlich)
!./code tunnel --accept-server-license-terms
```

### Schritt 2: Tunnel authentifizieren
1. In den Ausgaben der Zelle erscheint ein Link (z. B. `https://github.com/login/device`) und ein achtstelliger Code.
2. Öffne den Link in deinem Browser, logge dich in deinen GitHub- oder Microsoft-Account ein und gib den Code ein.
3. Im Kaggle-Output siehst du kurz darauf, dass der Tunnel erfolgreich unter einem Namen wie `dev-box` registriert wurde.

### Schritt 3: Lokal in VS Code verbinden
1. Öffne VS Code auf deinem MacBook.
2. Installiere die offizielle Extension **Remote - Tunnels** (von Microsoft).
3. Klicke unten links auf das blaue Verbindungssymbol `><` (oder drücke `F1` und tippe `Remote-Tunnels: Connect to Tunnel...`).
4. Logge dich mit demselben GitHub/Microsoft-Account ein.
5. Wähle die aktive Kaggle-Instanz aus der Liste aus.

Du bist nun direkt mit dem Kaggle-Container verbunden, hast vollen Zugriff auf das Dateisystem (`/kaggle/working/`), kannst Terminals öffnen und das Training direkt aus VS Code heraus steuern.

