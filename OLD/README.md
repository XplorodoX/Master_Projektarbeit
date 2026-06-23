# OLD/ — Archiv veralteter Dateien

Dieser Ordner enthält Dateien, die nicht mehr aktiv genutzt werden, aber aus Dokumentationsgründen erhalten bleiben.

> Nichts hier löschen — der Ordner ist das Archiv, nicht der Papierkorb.

---

## Inhalt

### `scripts/`

| Datei | Warum archiviert |
|-------|-----------------|
| `launcher.py` | Alter CLI-Launcher (Terminal-Menü). **Ersetzt durch** `scripts/launcher_gui.py` (grafische GUI mit Live-Metriken, Model-Picker, Eval-Dashboard). |
| `LAUNCHER.md` | Dokumentation des alten CLI-Launchers. |
| `start_dqn_play.sh` | Shell-Skript das `python/ai_play.py` aufruft — diese Datei existiert nicht mehr. Außerdem war es nur für DQN; GUI unterstützt alle Algorithmen. |
| `start_dqn_play.bat` | Windows-Version von `start_dqn_play.sh`, gleiche Probleme. |

### `build_validation/`

Windows MSVC-Build-Artefakt (`.vcxproj`, `.sln`, `CMakeCache.txt`).  
Wurde auf einem Windows-System erzeugt, um die Build-Kompatibilität zu validieren.  
Kein Quellcode — automatisch generiert durch `cmake -G "Visual Studio ..."`.  
Bleibt hier als Referenz für die Windows-Build-Konfiguration.

---

## Was die Nachfolger bieten

| Alt | Neu | Verbesserungen |
|-----|-----|----------------|
| `launcher.py` | `scripts/launcher_gui.py` | GUI, Live-Training-Metriken, Model-Picker, System-Status, Schnellstart-Dashboard |
| `start_dqn_play.sh` | Launcher → Abspielen | Alle Algorithmen (PPO/DQN/A2C), Monster-Toggle, Dual-Modus |
| DQN-only | PPO + DQN + A2C | Alle drei auf Discrete Action Space trainierbar |
