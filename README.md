# Cinematic Conversation Camera SF

A cinematic dialogue camera for Starfield: shot/reverse-shot framing that cuts
between you and the person you're talking to, like a film or TV scene.

This is a Starfield port of **Cinematic Conversation Camera** for Skyrim by
**hashhbbrown** ([source](https://github.com/fatalCMD/ccc)). Starfield-specific
dialogue camera handling builds on **Player Camera During Dialogue** by
**rrrafau** ([source](https://github.com/rrrafau1337/player-camera-during-dialogue)).
Both are GPL-3.0-or-later, and so is this project.

## Status: milestone 1 (test build)

This build only proves the basics:

- Takes over the dialogue camera and frames an over-the-shoulder shot.
- Cuts to whoever is talking (`iShotMode=2`), or holds one side (`0` or `1`).
- Gives the camera back to the game when the conversation ends.

Not yet ported: CCC's full shot library, angle presets, letterbox, subtitles in
the bars, menu suspend/resume, ship and scripted-scene handling, NPC-to-NPC filming.

## Requirements

- Starfield **1.16.244** (other versions: the plugin loads but stays off)
- [SFSE](https://www.nexusmods.com/starfield/mods/106) for 1.16.244
- [Address Library for SFSE Plugins](https://www.nexusmods.com/starfield/mods/3256)
- Keep Starfield's own dialogue camera setting **on** (this mod hooks into it)
- Disable other dialogue camera mods while testing

## Install

1. Open the latest successful run under **Actions** → **Build** and download the
   `CinematicConversationCameraSF` artifact.
2. Copy its `Data` folder into your Starfield folder (or install the zip with your
   mod manager).

## Testing and logs

The log is at
`Documents\My Games\Starfield\SFSE\Logs\CinematicConversationCameraSF.log`.
Please include it with any report.

## Building

GitHub Actions builds every push (`.github/workflows/build.yml`). Locally: Visual
Studio 2022 with C++ tools and [xmake](https://xmake.io), then

```
git clone --recurse-submodules <this repo>
xmake config -m releasedbg
xmake build
```

## License

GPL-3.0-or-later. If you distribute a build, you must also publish its source.
