# bin/ — Kompilierte Binaries

Dieser Ordner nimmt die fertig kompilierten Binaries auf, die nach einem erfolgreichen Build hierher kopiert werden können.

## Nach dem Build verfügbare Binaries

Die Binaries werden primär in `../build/` abgelegt. Für die Abgabe / Weitergabe können sie hier abgelegt werden:

| Binary | Plattform | Beschreibung |
|--------|-----------|-------------|
| `stoneforge_client` | macOS / Linux | Spielbarer Client (raylib) |
| `stoneforge_client.exe` | Windows | Spielbarer Client (raylib) |
| `stoneforge_headless` | macOS / Linux | Headless-Simulator |
| `stoneforge_sim*.so` | macOS / Linux | Python-Binding |
| `stoneforge_sim*.pyd` | Windows | Python-Binding |

## Binaries kopieren (nach Build)

```bash
# macOS / Linux
cp build/stoneforge_client bin/
cp build/stoneforge_sim*.so bin/

# Windows
copy build\Release\stoneforge_client.exe bin\
copy build\Release\stoneforge_sim*.pyd bin\
```

## Hinweis

Die Binaries sind plattformspezifisch und werden nicht ins Git-Repository eingecheckt.
Nach einem Checkout muss neu gebaut werden — siehe `../admin/README.md`.
