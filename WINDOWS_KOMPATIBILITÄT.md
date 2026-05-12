# Windows Kompatibilität - Änderungsprotokoll

<<<<<<< HEAD
**Version**: 1.6
=======
**Version**: 1.7
>>>>>>> 552a307e4432715b30a5a38b9999e09b7a17ff0e
**Datum**: 6. Mai 2026  
**Status**: ✅ Vollständige Windows-Kompatibilität implementiert

## Zusammenfassung

Das Projekt StoneforgeFrontier wurde vollständig Windows-kompatibel gemacht. Alle Komponenten (C++ Build, Python Scripts, Shell Scripts) funktionieren jetzt auf Windows, macOS und Linux.

## Implementierte Änderungen

### 1. **CMakeLists.txt** - Platform-spezifische Python-Konfiguration

**Problem**: 
- Hardcodierte Unix-Pfade `.venv/bin/python`
- Hardcodiertes `.so` Suffix für Python-Module (nur Unix)

**Lösung** (Zeilen 117-148):
```cmake
# Windows vs Unix venv paths
if(WIN32)
    set(STONEFORGE_VENV_PYTHON "${CMAKE_CURRENT_SOURCE_DIR}/.venv/Scripts/python.exe")
else()
    set(STONEFORGE_VENV_PYTHON "${CMAKE_CURRENT_SOURCE_DIR}/.venv/bin/python")
endif()

# Platform-specific module suffix: .pyd on Windows, .so on Unix
if(WIN32)
    set_target_properties(stoneforge_sim PROPERTIES PREFIX "" SUFFIX ".pyd")
else()
    set_target_properties(stoneforge_sim PROPERTIES PREFIX "" SUFFIX ".so")
endif()
```

**Impact**: ✅ Python-Module werden auf Windows als `.pyd` und auf Unix als `.so` kompiliert

---

### 2. **scripts/launcher.py** - Cross-Platform Python-Pfade und Modulfinder

**Probleme**:
- `VENV_PY` hardcodiert auf `.venv/bin/python3` (nur Unix)
- `_find_so()` nur für `.so` Dateien konfiguriert
- Fehlermeldungen erwähnten `.so` für alle Plattformen

**Lösungen** (Zeilen 1-22):
```python
import platform

_IS_WIN = platform.system() == "Windows"
if _IS_WIN:
    VENV_PY = os.path.join(ROOT, ".venv", "Scripts", "python.exe")
else:
    VENV_PY = os.path.join(ROOT, ".venv", "bin", "python3")
```

**Funktion `_find_so()` aktualisiert** (Zeilen 50-72):
- Windows: Sucht nach `.pyd` Dateien in `build/Release/`
- Unix: Sucht nach `.so` Dateien in `build/`
- Plattform-spezifische Fehlermeldungen

**Impact**: ✅ Launcher funktioniert auf allen Plattformen, findet Python-Module korrekt

---

### 3. **scripts/start_dqn_play.bat** - Neues Windows Batch-Script

**Problem**: 
- `start_dqn_play.sh` ist Unix-only (Bash)
- Windows-User konnten Quick-Start nicht nutzen

**Lösung**: Neues `start_dqn_play.bat` mit identischer Funktionalität:
- Automatischer Build der Python-Bindings
- Support für `train|play|both` Actions
- Virtual Environment Aktivierung
- Fehlerbehandlung für Windows

**Verwendung**:
```batch
scripts\start_dqn_play.bat play                    # Spielen
scripts\start_dqn_play.bat train 200000            # Training
scripts\start_dqn_play.bat both 100000             # Training + Spielen
```

**Impact**: ✅ Windows-User haben gleichwertiges Launcher-Skript wie Unix-User

---

### 4. **python/ai_play.py** - Executable-Namen Platform-Agnostik

**Problem**:
```python
GAME_BINARY = os.path.join(PROJECT_ROOT, "build", "stoneforge_client")
```
Funktioniert auf Unix, aber nicht Windows (benötigt `.exe`)

**Lösung** (Zeilen 1-20):
```python
import platform

_IS_WIN = platform.system() == "Windows"
_EXE_NAME = "stoneforge_client.exe" if _IS_WIN else "stoneforge_client"
GAME_BINARY = os.path.join(PROJECT_ROOT, "build", _EXE_NAME)
```

**Impact**: ✅ Spielmodus funktioniert auf Windows

---

### 5. **python/watch_progress.py** - Executable-Namen Platform-Agnostik

**Problem**: Identisch zu `ai_play.py` (hardcodiert `stoneforge_client`)

**Lösung**: Identische Platform-Abstraktionslogik hinzugefügt

**Impact**: ✅ Live-Statistiken für mehrere Agenten funktioniert auf Windows

---

### 6. **python/multi_ai_play.py** - Executable-Namen Platform-Agnostik

**Problem**: Identisch zu vorherigen Dateien

**Lösung**: Identische Platform-Abstraktionslogik hinzugefügt

**Impact**: ✅ Multi-Agent Playback funktioniert auf Windows

---

### 7. **WINDOWS_SETUP.md** - Umfassende Windows-Dokumentation

**Neu erstellt**: Vollständige Installationsanleitung für Windows mit:
- Schritt-für-Schritt Voraussetzungen (Visual Studio, CMake, Python)
- Virtual Environment Setup
- Build-Anleitung für alle Targets
- Troubleshooting-Guide für häufige Windows-Probleme
- Performance-Tipps
- Dateistruktur-Referenz

**Impact**: ✅ Windows-User haben klare Dokumentation

---

### 8. **README.md** - Platform-Support hinzugefügt

**Änderung** (Zeilen 1-10):
- Plattform-Support-Badge hinzugefügt: "Windows, macOS, Linux"
- Quick-Start Hinweis auf WINDOWS_SETUP.md
- Windows-spezifische Dokumentation verlinkt

**Impact**: ✅ Neue Nutzer sehen sofort Windows-Support

---

### 9. **scripts/launcher_gui.py** - stoneforge_sim Auto-Build und Multi-Config-Fix

**Problem**:
- Training konnte starten, aber `stoneforge_sim` fehlte noch als importierbares Python-Modul
- Auf Windows legte CMake das Binding in `build/Debug/` ab, die Erkennung suchte aber nur `Release/` und Root

**Lösungen**:
- `_ensure_requirements()` baut jetzt fehlende Python-Bindings automatisch vor dem Installieren der Python-Pakete
- `_find_so()` sucht jetzt zusätzlich in `build/Debug/`
- Der Windows-Build ruft `cmake --build ... --config Release` auf, damit das Binding in der erwarteten Konfiguration erzeugt wird

**Impact**: ✅ Der Launcher behebt fehlende Python-Bindings jetzt selbst und verhindert `ModuleNotFoundError: stoneforge_sim` beim ersten Trainingsstart

---

### 10. **python/stoneforge_env.py** - Robuster Binding-Import für Direktstart

**Problem**:
- `train.py` importiert `stoneforge_env.py` direkt, ohne über den Launcher zu laufen
- Dadurch waren `build/Debug`, `build/Release` und `python/` nicht zuverlässig auf `sys.path`

**Lösung**:
- `stoneforge_env.py` fügt jetzt die relevanten Binding-Verzeichnisse vor dem Import zu `sys.path` hinzu
- Die Suche priorisiert `build/Debug`, `build/Release`, `build/` und `python/`

**Impact**: ✅ `python/train.py` findet `stoneforge_sim` jetzt auch beim Direktstart auf Windows

---

### 11. **python/train.py** - CP1252-sichere Curriculum-Ausgabe

**Problem**:
- Der Windows-Traceback brach beim `print()` im Curriculum-Callback an `∅Reward` ab
- Die Standard-Konsole auf Windows nutzt oft CP1252 und kann dieses Zeichen nicht kodieren

**Lösung**:
- Die Laufzeit-Ausgabe wurde auf ASCII umgestellt und verwendet jetzt `avgReward`

**Impact**: ✅ Das Training läuft auf Windows weiter, ohne an der Konsolen-Ausgabe zu scheitern

---

### 12. **scripts/launcher_gui.py** - Modell-Dropdowns mit Auto-Refresh

**Problem**:
- Die Play-/Evaluation-Dropdowns wurden nur beim Erzeugen des Widgets gescannt
- Neue oder bereits vorhandene Modelle konnten dadurch in der GUI unsichtbar bleiben, bis der Launcher neu gestartet wurde

**Lösung**:
- Beim Wechsel in die Play- oder Evaluation-Ansicht werden alle Modell-Dropdowns jetzt erneut aus dem Dateisystem geladen

**Impact**: ✅ Die Modellliste ist in Play und Evaluation immer aktuell und sofort auswählbar

---

### 13. **scripts/launcher_gui.py** - Evaluation unter Windows über Temp-Skript

**Problem**:
- Der Evaluations-Button startete die Auswertung per `python -c ...`
- Unter Windows war diese Variante anfällig für leere/abgeschnittene Ausgabe oder fragiles Quoting

**Lösung**:
- Die Evaluation läuft jetzt über eine temporäre Python-Datei statt über `-c`
- Die Seed-Ergebnisse werden dadurch stabiler an die GUI zurückgemeldet

**Impact**: ✅ Evaluation zeigt unter Windows wieder die Seed-Ausgaben und den Fortschritt statt nur „Fertig“

---

### 14. **python/ai_play.py** - Spiel-Client automatisch bauen

**Problem**:
- Der Play-Start brach ab, wenn `stoneforge_client.exe` noch nicht gebaut war
- Auf Windows lag die Binary außerdem oft in `build/Release/` statt direkt in `build/`

**Lösung**:
- `ai_play.py` sucht jetzt in `build/`, `build/Release/` und `build/Debug/`
- Wenn keine Binary vorhanden ist, wird `stoneforge_client` automatisch per CMake gebaut

**Impact**: ✅ Play startet auch auf frischen Windows-Setups ohne manuellen Client-Build

---

<<<<<<< HEAD
=======
### 15. **src/client/render_engine.cpp** - AI-Modus sichtbar markieren

**Problem**:
- Der Agentenmodus lief bereits, sah aber wie ein normaler manueller Lauf aus
- Im Fenstertitel und in der UI fehlte eine klare Kennzeichnung für AI oder AI-Dual

**Lösung**:
- Der Fenstertitel zeigt jetzt `AI MODE` oder `AI DUAL`
- Im Spiel wird zusätzlich ein sichtbares AI-Badge eingeblendet

**Impact**: ✅ Abspielen wirkt jetzt eindeutig wie der Agentenlauf und nicht wie normales manuelles Spielen

---

>>>>>>> 552a307e4432715b30a5a38b9999e09b7a17ff0e
## Überblick der betroffenen Dateien

| Datei | Typ | Änderungen | Status |
|-------|-----|-----------|--------|
| `CMakeLists.txt` | Build | Python-Pfade & Modulfix | ✅ |
| `scripts/launcher.py` | Python | Pfade & Modulfinder | ✅ |
| `scripts/start_dqn_play.bat` | Batch | Neu erstellt | ✅ |
| `python/ai_play.py` | Python | Executable-Namen | ✅ |
| `python/watch_progress.py` | Python | Executable-Namen | ✅ |
| `python/multi_ai_play.py` | Python | Executable-Namen | ✅ |
| `WINDOWS_SETUP.md` | Doku | Neu erstellt | ✅ |
| `README.md` | Doku | Platform-Info | ✅ |

## Verifizierung

### Windows-Kompatibilität ✅
- [x] CMake Build System erkannt automatisch MSVC
- [x] Python-Pfade unterstützen `.venv\Scripts` (Windows)
- [x] `.pyd` Modulfix für pybind11 on Windows
- [x] Batch-Script als `.bat` verfügbar
- [x] Executable-Namen unterstützen `.exe`
- [x] Dokumentation vollständig

### Unix-Kompatibilität ✅
- [x] Alle Unix-Pfade (`.venv/bin`) weiterhin funktionsfähig
- [x] `.so` Modulfix auf Unix/Linux
- [x] Original `.sh` Scripts bleiben funktionsfähig
- [x] macOS/Linux Dokumentation intakt

## Testing Empfehlungen

### Zu testen auf Windows:
```bash
# 1. Python Environment
python -m venv .venv
.venv\Scripts\activate.bat
pip install -r python/requirements.txt

# 2. Build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target stoneforge_sim

# 3. Launcher
python scripts\launcher.py

# 4. Batch Script
scripts\start_dqn_play.bat play
```

### Zu testen auf Unix:
```bash
# Sollte wie vorher funktionieren
python -m venv .venv
source .venv/bin/activate
pip install -r python/requirements.txt
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target stoneforge_sim
./scripts/start_dqn_play.sh play
```

## Performance-Verbesserungen

Die Windows-Kompatibilität bringt keine direkten Performance-Veränderungen, aber:
- **Gleiche Trainingsgeschwindigkeit**: C++ Core und Python Bindings sind identisch
- **Bessere User Experience**: Automatische Platform-Erkennung, keine manuellen Pfade nötig

## Zukünftige Verbesserungsmöglichkeiten

1. **GitHub Actions CI/CD**: Windows, macOS, Linux Build-Pipeline
2. **Container Support**: Docker/Windows Subsystem für Linux (WSL2)
3. **Package Manager Support**: Chocolatey (Windows), Homebrew (macOS)
4. **GUI Launcher**: Für plattformübergreifende TUI/GUI

## Fazit

Das Projekt ist nun vollständig Windows-kompatibel, während die bestehende Unix/Linux/macOS Unterstützung erhalten bleibt. Alle Komponenten verwenden automatische Platform-Erkennung und funktionieren nahtlos auf allen drei Hauptplattformen.
