# LOOPHEAD – Multi-Head Tape Echo

Bandecho mit vier Wiedergabeköpfen, Teil der *G Audio Vintage Series*
(gleiches Design wie DUSTBOX 12, SPRINGTANK und REEL 15).
Formate: **VST3** (Windows/macOS), **AU** (macOS), **Standalone**.

## Was LOOPHEAD von normalen Delays unterscheidet

| Feature | Was es macht |
|---|---|
| **4 Wiedergabeköpfe** | Jeder Kopf einzeln schaltbar (Köpfe 1–4 spielen bei 1×, 2×, 3× und 4× Kopfabstand). Kombinationen ergeben rhythmische Echo-Muster. Die Köpfe sind im Stereobild verteilt |
| **REV 4** | Kopf 4 liest das Band rückwärts. Rückwärts-Echos laufen also zusammen mit normalen Echos |
| **Motor-Trägheit** | Der Kopfabstand folgt dem Regler langsam wie ein echter Bandmotor. Drehen ergibt die typischen Pitch-Bends |
| **THROW** | Gedrückt halten: Feedback springt über 100 %, das Echo schaukelt sich auf. Beim Loslassen klingt es aus. Automatisierbar |
| **FREEZE** | Trennt den Eingang und lässt das Band verlustfrei weiterlaufen: Endlosschleife |
| **Federhall** | Eingebauter Federhall aus SPRINGTANK auf den Echos |
| **DUCK** | Echos werden leiser, während du spielst/singst, und kommen in den Pausen hoch |
| **Sättigung im Loop** | DRIVE begrenzt auch die Eigenschwingung musikalisch statt digital zu clippen |
| **Animierter Bandlauf** | Band und Rollen laufen je nach Tempo, aktive Köpfe glühen mit den Echos |

## Regler

- **HEAD SPACING**: Kopfabstand in ms (20–350 ms) oder mit **SYNC** als Notenwert
  (1/32 bis 1/4, auch triolisch und punktiert)
- **FEEDBACK** (bis 110 %) · **DRIVE** · **COLOR** (dunkel ↔ hell) · **WOBBLE** (Wow & Flutter) ·
  **AGE** (Höhenverlust, Rauschen, Dropouts)
- **SPRING** · **DUCK** · **WIDTH** · **MIX** (Dry bleibt bis 50 % voll) · **OUTPUT**

## Werkspresets

01 Space Station · 02 Dub Siren · 03 Slapback 50s · 04 Triplet Bounce ·
05 Backwards Dream · 06 Runaway · 07 Vocal Throw · 08 Clean Digital

## Bauen

**GitHub Actions:** Projekt in ein Repo pushen, dann unter *Actions → Build LOOPHEAD*
die Artefakte für Windows und macOS herunterladen.

**Lokal:**
```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Installation

- **Windows:** `LOOPHEAD.vst3` nach `C:\Program Files\Common Files\VST3\`
- **macOS:** `LOOPHEAD.vst3` nach `~/Library/Audio/Plug-Ins/VST3/`,
  `LOOPHEAD.component` nach `~/Library/Audio/Plug-Ins/Components/`.
  Nicht signiert: `xattr -cr ~/Library/Audio/Plug-Ins/VST3/LOOPHEAD.vst3`

## Lizenz-Hinweis

JUCE 8 ist dual lizenziert (AGPLv3 oder kommerziell). Vor einem Closed-Source-Verkauf
die Bedingungen der JUCE-Lizenzen auf juce.com prüfen.
