# Translating CCC SF

The settings page (through SFSE Menu Framework) and the listen prompt ("Listen in on nearby
conversation. Press N") can be shown in your language, the same way SFSE Menu Framework
translates its own menus. Since 1.1.1.

Last updated: 1.1.1 (2026-11-28).

## How it works

`Data\SFSE\Plugins\CinematicConversationCameraSFStrings.json` holds every text the mod shows,
English to English:

```json
{
  "_about": "...",
  "Preset": "Preset",
  "Default: {}": "Default: {}",
  ...
}
```

A translation changes the values and keeps the keys:

```json
  "Preset": "Пресет",
  "Default: {}": "По умолчанию: {}",
```

The mod reads the file once, when the game starts. Anything missing or left empty stays in
English, so a translation made for one version keeps working on the next (new texts just show
in English until it's updated).

## Rules

- **Translate the values, never the keys.** The key is how the mod finds the text.
- **Keep `{}` placeholders,** as many as the English has, in the order the English uses
  them. They're filled in with a name or a value (`Default: {}` becomes `Default: On`). If a
  translation's placeholders don't fit, the mod shows that text in English instead.
- **Keep JSON's escapes:** `\"` for a quote inside a text, `\\` for a backslash (the paths,
  `Data\\SFSE\\Plugins`). Your editor's JSON checker catches mistakes; if the file can't be
  read, the log says so and the whole page stays in English.
- **Save as UTF-8.**
- **What isn't translated:** the preset names (Standard, Close, Room, Frontier, Starborn,
  Constellation, Nebula), the INI's keys and values, the log, and the testing messages.

## Fonts

SFSE Menu Framework draws only the letters its font settings include. For Cyrillic, Greek,
Polish, Turkish, Vietnamese, Thai, Korean, Japanese or Chinese, turn on your language's glyphs
in the framework's own settings (its Fonts page). The listen prompt uses the game's font,
which has your language's letters if the game itself is in your language.

## Sharing a translation

Upload it as a separate mod (with only the JSON file, at `SFSE\Plugins\`) or send it to me to
include. Mod managers let the translation's file overwrite the English one. The mod is GPL, so
a translation is welcome either way; please credit it as a translation of CCC SF.

## For developers

The file is written from the source by `tools/strings.py`: every text in an `N_()`, `T()`,
`Fmt()` or `Literal()` (src/Strings.h). After changing any shown text, run
`python tools/strings.py`; the build fails (`--check`) while the file is out of date.
