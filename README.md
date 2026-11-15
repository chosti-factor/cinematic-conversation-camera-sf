# Cinematic Conversation Camera SF

A cinematic dialogue camera for Starfield. Conversations are filmed like a scene from a
film or TV series: shots and reverse shots that cut to whoever is talking, over-the-
shoulder shots, close-ups and three-quarter angles, camera moves, and a cutting rhythm
you can choose. Works whether you play in first or third person.

A Starfield adaptation of **Cinematic Conversation Camera** (CCC) for Skyrim by
**hashhbbrown** ([source](https://github.com/fatalCMD/ccc)). CCC's shot library, director
and presets are the heart of this mod; everything around them is rebuilt from the ground
up for Starfield's engine, with Starfield-specific features of its own. Taking over
Starfield's dialogue camera builds on **Player Camera During Dialogue** by **rrrafau**
([source](https://github.com/rrrafau1337/player-camera-during-dialogue)), and
**jedijosh920**'s [Player Dialogue Camera](https://www.nexusmods.com/starfield/mods/5229)
first showed it could be done. CCC and Player Camera During Dialogue are
GPL-3.0-or-later, and so is this project. See [CREDITS.md](CREDITS.md) for everything
this mod draws on.

## Status: alpha (0.8.x)

What changed in each version: [CHANGELOG.md](CHANGELOG.md).

### From CCC

- **The shot library and director:** all 39 camera setups with CCC's framing, CCC's three
  presets (Standard, Close, Room), camera moves (push, pull, crane, tilt, drift, orbit,
  slide), and CCC's cutting rules: each person keeps their angle across the
  back-and-forth and gets a new one after a few of their lines; short lines don't count.
- **Close-ups that follow the face:** a close-up turns toward a face turned to its work
  and drops to look up into a lowered one (`bFollowFace`).
- **The 180-degree rule:** each conversation is staged from one side of the line between
  you (the side with more room), and no shot or camera move crosses it (`bEnforceLine`;
  `bTrue180` films everything from that one side).
- **Letterbox:** black bars slide in while the camera films a conversation, with
  Starfield's subtitle sitting in the bottom bar; the dialogue options stay hidden while the
  NPC speaks and fade in when the line ends (`[Interface]`).
- **Reaction shots:** now and then a line is shown on you listening (counted in sentences;
  `iReactionEvery`, `iReactionChance`), with seated people and merchants too. A reaction
  stays on you a couple of seconds, then cuts back to the speaker (`fReactionSeconds`).
- **Holding on the answer** after a persuasion's last round or a skill check: the NPC's
  reply is a close-up, pushing in (`bPersuasionBeat`).
- **Room shots** (Wide, Master, Distant): placed where the room around the two of you has
  space for them, measured with the game's collision check, on the conversation's side of
  the line and with both faces in view (`iRoomShotMinRoom`). In CCC's presets as far back
  as CCC's framing wants; in CCC SF's the furthest of them (Master) up to 9 m indoors and
  12 m outdoors, the others closer in proportion
  (`fRoomShotMaxIndoors`, `fRoomShotMaxOutdoors` to set your own).

### New or distinct in CCC SF

- **Your character in shot from first person:** your full body is built at the start of
  the conversation (about a tenth of a second), including with seated NPCs, and you're
  back in first person when it ends.
- **NPCs keep facing you:** only what's on screen moves; the game still thinks the camera
  is at your eyes.
- **Focus on the person in shot:** the game's conversation blur is refocused on every
  shot onto whoever is in it, so they're sharp and the background stays softly blurred,
  even on longer outdoor shots (`bFocusOnSubject`). Two focus styles (`sFocusStyle`):
  Cinematic (more background blur) and Wide (deeper sharp zone, softer background blur).
  Shots from more than 8 m away have no blur.
- **Obstacle detection:** before each cut, the game's own camera collision test checks the
  line from the person in shot to the camera. A wall behind the camera pulls it in; a
  ladder or pillar in front of the person skips the shot for another angle; when nothing
  is fully clear, the angle with the most room is used (`bObstacleCheck`).
- **Emotion close-ups:** lines the game tags with heightened or emotional expressions
  (Irritated, Surprised, Somber, Nervous, Awed and others, `sCloseUpEmotions`) push in to a
  close-up of the speaker, 40% of them extreme close-ups. CCC does this with lines Skyrim's
  writers marked as intense; Starfield has no such mark, so the expression decides. The
  expression of each line can be shown on screen while testing (`bShowMessages`).
- **Seated people and merchants** get a plain over-the-shoulder shot instead of the shot
  library, whose angles often put the chair, the shop counter or the clutter around it in
  front of them (`bPlainShotsSeated`, `bPlainShotsMerchants`). Merchants are recognised by
  the game's vendor flag on their faction, wherever the shop is.
- **Cutting styles**, calmest to liveliest: Cinematic, Odyssey, SpaceWestern, Dogfight,
  SpaceOpera.
- **Three more presets,** each mirroring one of CCC's with a Starfield flavour: Stardust,
  Starborn and Constellation (see Presets below).
- **Handheld feel:** how much the camera sways with the people in it, from a tripod (0) to
  fully handheld (100) (`iCameraSway`, default 40).
- **Per-preset camera distances**, matched to each preset's widest shot.
- **A wider conversation lens:** Starfield films conversations through a narrow 30 degree
  lens; the mod widens it while it films (`fConversationLens`, 45 by default; 0 keeps the
  game's), so CCC's framings fit from closer. The hybrid of a wider lens and a somewhat
  longer camera distance is the approach for now (see the plan).
- **Your character's face:** the game keeps it neutral and your lines carry no expression
  tags, so the mod sets it: your face answers each NPC line's expression (mirrored or as a
  listener would), options with a skill, background or trait tag ([Persuade], [Space
  Scoundrel], [Wanted]...) and questions show on it, and your background colours neutral
  moments. Close-ups of you when your face shows something (`[Face]`; tables in
  [docs/EMOTIONS.md](docs/EMOTIONS.md)). Uses the expression function found by rrrafau.
- **People talking near you:** NPC-to-NPC conversations are noticed, who's addressing whom
  included, and filmed: a prompt on screen ("Listen in on nearby conversation. Press N", or
  hold R3 on a controller) and/or starting on its own when you sit down or stand still near
  them (`[Overhear]`: `bListenPrompt`, `bAutoListen`; in testing).
- **Bystanders:** someone standing still who'd fill the shot or hide a face is avoided like
  a wall; people walking through stay in.
- **Testing messages:** each line's expression, your face's answer and the shot used, with
  why, in the top bar while a conversation is filmed (`bShowMessages`, `fMessageSeconds`).
- Clean exits in both perspectives (starts can still show a frame or two of the game's
  own camera). Small speakers (robots) and quest scenes you take part in (someone walks
  up and starts talking, several people taking turns) keep the game's own camera.

### Presets

Choose one with `sPreset`. CCC's three are as faithful to CCC as Starfield allows; CCC SF's
three each mirror one of them with a Starfield flavour: room shots kept closer, and a few
shots from the other presets mixed in, moving.

| Preset | From | The look | Room shots go back |
| --- | --- | --- | --- |
| Standard | CCC | Shoulders, close-ups, three-quarters and mediums on both sides, mostly locked off | (none) |
| Close | CCC | Shoulders, tight close-ups and extreme close-ups, pushing and zooming in | Distant only, as far as CCC's framing wants |
| Room | CCC | Wide and full-figure shots: masters, wides, long shots, overheads, with cranes and drifts | As far as CCC's framing wants (12-18 m) |
| Stardust | CCC SF (Standard) | Standard with Room's wider shots mixed in, about 60/40 | Up to 9 m indoors, 12 m outdoors |
| Starborn | CCC SF (Close) | Close, plus the reverse shoulder and slowly drifting three-quarter singles | Distant only, up to 9 m indoors, 12 m outdoors |
| Constellation | CCC SF (Room) | Room, plus close-ups and shoulders now and then (about a fifth) | Up to 9 m indoors, 12 m outdoors |

All six use the same 45 degree conversation lens (`fConversationLens`). Every preset's shots, shares and moves: [docs/PRESETS.md](docs/PRESETS.md).

### Differences from CCC, because of Starfield

- Starfield's conversation lens is narrower (30 degrees) than the ones CCC was made for,
  so its framings want the camera further back. The mod takes a hybrid approach: it widens
  the lens (45 degrees by default) and lets the camera stand somewhat further back; each preset lets the camera go as far as its widest shot needs (Standard: 7 m
  indoors, 10.5 m outdoors), and obstacle detection pulls it in or picks another angle
  where walls are closer.
- Starfield has no emotion strength on its lines (Skyrim's writers set one), so emotion
  close-ups are chosen from the line's expression tag instead.
- There's no in-game settings menu; everything is in the INI.

### Planned, roughly in order

- In testing (0.8): the wider lens, room shots, filming people talking near you, your
  character's face, reaction shots and holding on the answer after a persuasion.
- 0.8.x, filming people talking near you, the rest: groups of three or more with
  occasional group shots; the letterbox and obstacle checks while filming them. (Done:
  the listen key and controller button, the on-screen prompt, starting on its own when
  you sit or stand still, never in combat.)
- Your character's face, later maybe: your traits setting a temperament (Introvert subtler,
  Extrovert fuller, Empath mirrors more), and cautiously more of the text of your lines.
- 0.9.x, polish and optimization:
  - **Distance, lens, or both:** how the shots get CCC's framing through Starfield's
    narrower lens. Pure distance (the game's 30 degree lens, the camera further back:
    Starfield's flatter, telephoto look, but wide shots rarely fit indoors), pure lens
    (CCC's own distances with a lens as wide as CCC's), or a hybrid (a moderately wider lens
    with somewhat longer distances). Hybrid chosen for now (0.8.1: 45 degrees); revisit the
    lens and distances at polish, perhaps per preset, and indoors versus outdoors (room
    shots already have their own caps, `fRoomShotMaxIndoors` and `fRoomShotMaxOutdoors`).
  - Emotion close-ups tuned from testing (which expressions push in, and how often; see
    [docs/EMOTIONS.md](docs/EMOTIONS.md)).
  - Talking to a seated person from the side (about 90 degrees) leaves you out of the
    picture; a seated NPC hidden behind your body in the plain over-the-shoulder shot; a
    switch for reaction shots with seated people and merchants.
  - Large speakers such as Vasco, whose bigger body can block you in over-the-shoulder
    shots.
  - Plants and other objects without collision (which the obstacle check can't see) in
    front of wide outdoor shots, and walls it passes through (beside Commander Tuala at the
    MAST, every check said clear).
  - Smoother conversation starts; quest scenes recognised by their quest rather than the
    scene's name; the default amount of camera sway (`iCameraSway`).
- 1.0, public beta, after testing with popular UI mods (a HUD replacer, a dialogue menu
  replacer, a subtitle mod) and listing compatibility.

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
| `[Director]` | Preset, cutting style, camera distances, 180-degree rule, camera moves, emotion close-ups, close-up distance, room shots, reaction shots, persuasion beat, bystanders |
| `[Shots]` | Per-shot tweaks: how often each shot is used, its move, amount and duration |
| `[Camera]` | The conversation lens, who gets the camera (shot/reverse or fixed), follow smoothing, sway, third-person exit hold, small-speaker height |
| `[Body]` | Showing your character, building the full body from first person |
| `[Transitions]` | How the first and last frames of a conversation are handled |
| `[PlainCamera]` | The simple over-the-shoulder shots used when the director is off |
| `[Interface]` | Letterbox, subtitles in the bars, dialogue options while the NPC speaks |
| `[Overhear]` | Filming people talking near you: on/off, the listen key, how far |
| `[Face]` | Your character's face: on/off, background expressions, questions, close-ups of you |
| `[Debug]` | Testing messages, hook and camera-target switches for troubleshooting |
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

Copyright (C) 2026 chosti-factor and the Cinematic Conversation Camera SF contributors.

This program is free software: you can redistribute it and/or modify it under the terms of
the GNU General Public License as published by the Free Software Foundation, either
version 3 of the License, or (at your option) any later version. It is distributed in the
hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the full text in
[LICENSE](LICENSE).

It includes work from Cinematic Conversation Camera by hashhbbrown and Player Camera During
Dialogue by rrrafau, both GPL-3.0-or-later (details in [CREDITS.md](CREDITS.md)). If you
distribute a build, you must also publish its source.
