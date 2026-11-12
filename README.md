# Cinematic Conversation Camera SF

A cinematic dialogue camera for Starfield. Conversations are filmed like a scene from a
film or TV series: shots and reverse shots that cut to whoever is talking, over-the-
shoulder shots, close-ups and three-quarter angles, camera moves, and a cutting rhythm
you can choose. Works whether you play in first or third person.

This is a Starfield port of **Cinematic Conversation Camera** for Skyrim by
**hashhbbrown** ([source](https://github.com/fatalCMD/ccc)). Taking over Starfield's
dialogue camera builds on **Player Camera During Dialogue** by **rrrafau**
([source](https://github.com/rrrafau1337/player-camera-during-dialogue)). Both are
GPL-3.0-or-later, and so is this project. See [CREDITS.md](CREDITS.md) for everything
this mod draws on.

## Status: alpha (0.4.x)

Working:

- **CCC's shot library and director:** all 39 camera setups with CCC's framing, three
  presets (Standard, Close, Room), camera moves (push, pull, crane, tilt, drift, orbit,
  slide), and CCC's cutting rules: each person keeps their angle across the
  back-and-forth and gets a new one after a few of their lines; short lines don't count.
- **Cutting styles**, calmest to liveliest: Cinematic, Odyssey, SpaceWestern, Dogfight,
  SpaceOpera.
- **Your character in shot from first person:** your full body is built at the start of
  the conversation (about a tenth of a second), including with seated NPCs, and you're
  back in first person when it ends.
- **NPCs keep facing you:** only what's on screen moves; the game still thinks the camera
  is at your eyes.
- Clean exits in both perspectives (starts can still show a frame or two of the game's
  own camera), seated characters, and small speakers (robots), which keep the game's
  own camera. Quest scenes you take part in (someone walks up and starts talking,
  several people taking turns) keep the game's own camera.
- **Seated people and merchants** get the plain over-the-shoulder shot instead of the
  shot library, whose angles often put the chair or the shop counter in front of them
  (`bPlainShotsSeated`, `bPlainShotsMerchants`).

Differences from CCC, because of Starfield:

- Starfield's conversation lens is fixed and narrower (about 30 degrees) than the ones
  CCC uses, so its framings want the camera further back. Until wall detection exists the
  camera stays within 3 m of the person it frames indoors and 5 m outdoors; shots that
  would need more room come out tighter, and the two-person and room shots are mostly
  unavailable.
- Extreme close-ups are rarer: CCC reserves them for intense lines, which this mod can't
  read yet.
- There's no in-game settings menu; everything is in the INI.

Planned, roughly in order:

- Depth of field on the person in shot. Outdoors the game focuses about 2 m from the
  camera, so on longer shots the near person can be sharp and the speaker soft. The
  0.4.8 attempt (moving the game's camera) shifted angled shots and is off; 0.4.11
  looks for the game's focus distance so it can be set directly.
- Close-ups driven by each line's emotion.
- Wall and obstacle detection, which also unlocks the longer and room shots.
- Close-ups that follow the face, the 180-degree rule tuned for Starfield.
- Black bars, fading the dialogue options during speech, subtitles in the bars.
- Reaction shots, holding on the answer after a persuasion check, filming NPC-to-NPC
  conversations, NPCs stepping away from work to talk.

## Requirements

- Starfield **1.16.244** (other versions: the plugin loads but stays off)
- [SFSE](https://www.nexusmods.com/starfield/mods/106) for 1.16.244
- [Address Library for SFSE Plugins](https://www.nexusmods.com/starfield/mods/3256)
- Keep Starfield's own dialogue camera setting **on** (this mod hooks into it)
- Disable other dialogue camera mods

## Install

1. Download `CinematicConversationCameraSF-test-build.zip` from the **Test build**
   pre-release under **Releases** (always the newest alpha build), or the
   `CinematicConversationCameraSF` artifact from the latest successful run under
   **Actions** → **Build**.
2. Copy its `Data` folder into your Starfield folder (or install the zip with your
   mod manager).

## Settings

`Data\SFSE\Plugins\CinematicConversationCameraSF.ini`. Put your own changes in
`CinematicConversationCameraSF_user.ini` next to it (same sections and keys) so updates
don't overwrite them. The files are read again before every conversation, so changes
apply without restarting the game.

| Section | What's in it |
|---|---|
| `[General]` | On/off, extra logging |
| `[Director]` | Preset, cutting style, camera distance limits indoors and outdoors, 180-degree rule, camera moves |
| `[Shots]` | Per-shot tweaks: how often each shot is used, its move, amount and duration |
| `[Camera]` | Who gets the camera (shot/reverse or fixed), follow smoothing, third-person exit hold, small-speaker height |
| `[Body]` | Showing your character, building the full body from first person |
| `[Transitions]` | How the first and last frames of a conversation are handled |
| `[PlainCamera]` | The simple over-the-shoulder shots used when the director is off |
| `[Debug]` | Hook and camera-target switches for troubleshooting |
| `[GameSettings]` | Changing the game's own settings while it runs |

Each setting is explained in the INI itself.

## Testing and logs

The log is at
`Documents\My Games\Starfield\SFSE\Logs\CinematicConversationCameraSF.log`. It records
every shot the director chooses and why. Please include it with any report.

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
