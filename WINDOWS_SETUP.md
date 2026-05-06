# Windows Setup für StoneforgeFrontier

Diese Anleitung erklärt, wie Sie das Projekt unter Windows einrichten und ausführen.

## Voraussetzungen

1. **Visual Studio Build Tools 2022** oder **Visual Studio Community 2022**
   - Installieren Sie mit C++ Development Tools (MSVC Compiler, Windows SDK)
   - Download: https://visualstudio.microsoft.com/

2. **CMake 3.20+**
   - Download: https://cmake.org/download/
   - Oder via Chocolatey: `choco install cmake`

3. **Python 3.11+**
   - Download: https://www.python.org/downloads/
   - **WICHTIG**: Aktivieren Sie "Add Python to PATH" während Installation

4. **Git** (optional, aber empfohlen)
   - Download: https://git-scm.com/download/win

## Schritt 1: Project Setup

```bash
# Repository klonen (falls nicht bereits vorhanden)
git clone <repo_url>
cd Master_Projektarbeit

# Virtual Environment erstellen
python -m venv .venv

# Virtual Environment aktivieren
.venv\Scripts\activate.bat

# Python Requirements installieren
pip install -r python/requirements.txt
```

## Schritt 2: Build-Verzeichnis vorbereiten

```bash
# Im Repository root (mit aktiviertem venv)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

**Hinweis**: CMake erkennt automatisch MSVC-Compiler. Falls Probleme auftreten, können Sie explizit angeben:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022"
```

## Schritt 3: Binärprogramme kompilieren

```bash
# Headless Simulator
cmake --build build --target stoneforge_headless -j 4

# Raylib Client (optional, benötigt Raylib)
cmake --build build --target stoneforge_client -j 4

# Python Bindings
cmake --build build --target stoneforge_sim -j 4
```

Das `.pyd` Modul wird in `build/Release/stoneforge_sim.pyd` erstellt und sollte nach `python/` kopiert werden.

## Schritt 4: Training und Playback ausführen

### Option A: Über Batch-Skript (empfohlen)

```bash
# Nur Spielen (mit Default-Modell)
scripts\start_dqn_play.bat play

# Training für 200k Schritte
scripts\start_dqn_play.bat train 200000

# Training + Spielen
scripts\start_dqn_play.bat both 100000
```

### Option B: Direkt über Python-Launcher

```bash
# Alle Befehle als Menü
python scripts/launcher.py

# Oder direkt
python scripts/launcher_gui.py
```

### Option C: Manuelle Ausführung

```bash
# Sicherstellen, dass venv aktiviert ist
.venv\Scripts\activate.bat

# Training
python python/train.py --algo dqn --timesteps 1000000

# Spielen
python python/ai_play.py --model best_models_dqn/best_model.zip --seed 42
```

## Troubleshooting

### Problem: "Python not found" oder "cmake not found"

**Lösung**: Stellen Sie sicher, dass die Tools zur PATH hinzugefügt wurden:
- Python: Während Installation "Add Python to PATH" aktivieren
- CMake: Manuell zur PATH hinzufügen unter System Properties > Environment Variables
- Oder öffnen Sie PowerShell/CMD neu nach Installation

### Problem: "Microsoft Visual C++ 14.0 is required"

**Lösung**: Installieren Sie Visual Studio Build Tools:
```bash
# Oder Quick Install
choco install visualstudio2022buildtools --package-parameters "--add Microsoft.VisualStudio.Workload.NativeDesktop"
```

### Problem: ".pyd file not found"

**Lösung**: Build-Verzeichnis ist wahrscheinlich `Release` subfolder:
```bash
# Manuell kopieren
copy build\Release\stoneforge_sim.pyd python\
```

Der Launcher macht das automatisch, aber Sie können auch manuell prüfen.

### Problem: "PYTHONPATH" Fehler beim Import

**Lösung**: Stellen Sie sicher, dass `.venv` mit aktiviertem venv erstellt wurde:
```bash
# Venv neu erstellen
rmdir /s .venv
python -m venv .venv
.venv\Scripts\activate.bat
pip install -r python/requirements.txt
```

## Dateistruktur nach Setup

```
Master_Projektarbeit/
├── .venv/
│   ├── Scripts/           # Windows: Python executable hier
│   │   ├── python.exe
│   │   ├── activate.bat   # Activation script
│   │   └── pip.exe
│   └── Lib/               # Installed packages
├── build/
│   ├── Release/           # Windows Release build output
│   │   └── stoneforge_sim.pyd
│   ├── stoneforge_headless.exe
│   └── stoneforge_client.exe
├── python/
│   ├── stoneforge_sim.pyd  # Kopiert von build/Release/
│   ├── stoneforge_env.py
│   ├── train.py
│   └── train_ppo.py
└── scripts/
    ├── launcher.py
    └── start_dqn_play.bat  # Windows batch script
```

## Environment Variables für Entwicklung

Falls Sie häufig in PowerShell arbeiten, können Sie ein Profil-Skript erstellen:

```powershell
# PowerShell Profile öffnen (falls nicht existiert, erstellen)
# $PROFILE wird in PowerShell angezeigt

# Hinzufügen:
function Enter-StoneforgeDev {
    Set-Location "C:\path\to\Master_Projektarbeit"
    .\.venv\Scripts\Activate.ps1
}
```

Dann:
```powershell
Enter-StoneforgeDev
```

## Cross-Platform Tests

Das Projekt ist jetzt Windows-, Mac- und Linux-kompatibel:

- **Windows**: `.venv\Scripts\python.exe`, `.pyd` Module, `.bat` Skripte
- **Mac/Linux**: `.venv/bin/python3`, `.so` Module, `.sh` Skripte

Das Launcher-System (`scripts/launcher.py`) erkennt automatisch die Plattform.

## Performance-Tipps für Windows

1. **SSD verwenden**: Training mit GPU/schnellen Durchsätzen ist auf SSD deutlich schneller
2. **Windows Defender ausschließen**: 
   - Ausnahmen > Ordner hinzufügen > `build/` und `python/`
3. **Raylib Client disabled**:
   - Standard-Build unterstützt nur Headless. Für GUI-Client, Raylib installieren:
   ```bash
   # vcpkg (Microsoft's package manager)
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   .\vcpkg\vcpkg install raylib:x64-windows
   ```

## Weitere Ressourcen

- CMake Windows Guide: https://cmake.org/cmake/help/latest/manual/cmake.1.html
- Python Virtual Environments: https://docs.python.org/3/library/venv.html
- Gymnasium Docs: https://gymnasium.farama.org/
- Stable-Baselines3: https://stable-baselines3.readthedocs.io/
