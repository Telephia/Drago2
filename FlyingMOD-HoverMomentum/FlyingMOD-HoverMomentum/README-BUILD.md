# FlyingMOD mit Hover-Momentum (privater Patch)

Modifizierter Quellcode von **Play as a Dragon SE** (Nexus-Mod 28854, v1.9.7)
von Ceruulean und z65536.

**Änderung:** Beim Übergang vom Gleiten in den Schwebezustand behält der Drache
jetzt etwas Restmomentum und bremst exponentiell über ca. 1 Sekunde ab,
statt sofort auf der Stelle stehen zu bleiben. Beschleunigen und der
normale Flug sind unverändert.

---

## Was wurde geändert?

| Datei | Änderung |
|---|---|
| `src/Movement.cpp` | Im Hook `EnableCollisionWhenFlying::Func` wird die Welt-Bewegung pro Frame zusätzlich über eine exponentiell abklingende Momentum-Variable geglättet (nur beim Abbremsen; Beschleunigen bleibt 1:1). Beim Landen/Perchen wird das Momentum zurückgesetzt. |
| `src/Settings.cpp` | Neuer Standardwert `HoverMomentum = 0.4` (Abklingzeit in Sekunden). |
| `src/Plugin.h`, `src/DebugUtils.h` | Neu angelegt, fehlten im Quellcode-Zip des Autors. |
| `xmake.lua`, `.github/workflows/build.yml` | Neu: Build-Konfiguration und CI, damit die DLL in der GitHub-Cloud kompiliert wird. |

---

## DLL bauen (ohne eigene Entwicklungsumgebung)

Der einfachste Weg läuft über GitHub Actions. Du brauchst nur ein
kostenloses GitHub-Konto.

1. **Neues, privates Repository anlegen:** github.com → *New repository* →
   z. B. `FlyingMOD-patched` → **Private** → *Create*. (Häkchen bei
   "Add a README" weglassen.)
2. **Alle Dateien hochladen:** Im neuen Repo auf *"uploading an existing
   file"* klicken und den **Inhalt dieses Ordners** per Drag & Drop
   hineinziehen (also `src`, `.github`, `xmake.lua`, `README-BUILD.md`,
   `.gitignore` zusammen). Wichtig: der versteckte Ordner `.github` muss
   mit dabei sein. Dann *"Commit changes"*.
3. **Build starten:** Der Build läuft automatisch beim Hochladen los
   (sonst: Tab **Actions** → *Build FlyingMOD (Hover Momentum)* →
   *Run workflow* → *Run workflow*).
4. **DLL herunterladen:** Nach ca. 10-15 Minuten: Tab **Actions** →
   den grünen, fertigen Lauf anklicken → ganz unten bei *Artifacts*
   `FlyingMOD-HoverMomentum` herunterladen → entpacken →
   `FlyingMOD.dll` liegt drin.
5. **Ersetzen:** Sichere deine alte `FlyingMOD.dll` (unter
   `Skyrim Special Edition/Data/SKSE/Plugins/`) und ersetze sie durch die
   neue. Das war's.

Sollte der Build fehlschlagen, klicke den roten Lauf an, öffne
*Build* und kopiere die Fehlermeldung - daraus lässt sich die Ursache
meist direkt beheben.

## Momentum-Stärke einstellen (optional)

Erstelle oder bearbeite `Skyrim Special Edition/Data/SKSE/Plugins/FlyingMOD.ini`:

```ini
[Settings]
HoverMomentum=0.4
```

- `0` = aus (verhält sich wie das Original)
- `0.2` = kaum spürbar
- `0.4` = Standard, "ein wenig Momentum"
- `0.8` bis `1.5` = deutlich langes Ausgleiten

Der Wert wird beim nächsten Spielstart gelesen. (Der MCM-"Save"-Vorgang
schreibt die Datei ebenfalls und übernimmt den Wert mit.)

## Wichtig

- **Nur persönliche Nutzung:** Die Assets von "Play as a Dragon SE"
  dürfen laut Autor nicht ohne Erlaubnis weiterverbreitet werden. Die
  gepatchte DLL also bitte nicht hochladen oder weitergeben. Gern kannst
  du aber den gewünschten Effekt beim Autor als Feature vorschlagen.
- Der Patch basiert auf v1.9.7 (Stand September 2026). Bei einem
  Mod-Update musst du den Vorgang mit dem neuen Quellcode wiederholen -
  oder den Autor um das offizielle Feature bitten.
