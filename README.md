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

## Status: alpha (0.5.x)

What changed in each version: [CHANGELOG.md](CHANGELOG.md).

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
- **Focus on the person in shot:** the game's conversation blur is refocused on every
  shot onto whoever is in it, so they're sharp and the background stays softly blurred,
  even on longer outdoor shots (`bFocusOnSubject`). Optionally a deeper sharp zone on long
  shots (`bDeepFocusLongShots`).
- **Emotion close-ups:** lines the game tags with heightened or emotional expressions
  (Irritated, Surprised, Somber, Nervous, Awed and others, `sCloseUpEmotions`) push in to a
  close-up of the speaker, 40% of them extreme close-ups, as CCC does with lines Skyrim's
  writers marked as intense. The expression of each line can be shown on screen while
  testing (`bShowEmotions`).
- **Seated people and merchants** get the plain over-the-shoulder shot instead of the
  shot library, whose angles often put the chair, the shop counter or the clutter around
  it in front of them (`bPlainShotsSeated`, `bPlainShotsMerchants`). Merchants are
  recognised by the game's vendor flag on their faction, wherever the shop is.

Differences from CCC, because of Starfield:

- Starfield's conversation lens is fixed and narrower (about 30 degrees) than the ones
  CCC uses, so its framings want the camera further back. Until wall detection exists the
  camera stays within 3 m of the person it frames indoors and 5 m outdoors; shots that
  would need more room come out tighter, and the two-person and room shots are mostly
  unavailable.
- Starfield has no emotion strength on its lines (Skyrim's writers set one), so emotion
  close-ups are chosen from the line's expression tag instead.
- There's no in-game settings menu; everything is in the INI.

Planned, roughly in order:

- Emotion close-ups, tuned from testing (which expressions push in, and how often).
- Wall and obstacle detection, which also unlocks the longer and room shots (now that
  focus works at any distance).
- Close-ups that follow the face, the 180-degree rule tuned for Starfield.
- Black bars, fading the dialogue options during speech, subtitles in the bars.
- Polish for unusual setups: talking to a seated person from the side (about 90 degrees)
  leaves you out of the picture and the shots look poor; from in front, up to about 45
  degrees, they work.
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
