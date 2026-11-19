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

## Status: alpha (0.9.x, polish)

Last updated: 0.9.26 (2026-11-19).

What changed in each version: [CHANGELOG.md](CHANGELOG.md).

### From CCC

- **The shot library and director:** all 39 camera setups with CCC's framing, CCC 1.5.0's
  three presets (Standard, Close, Room) with each angle's own lens, camera moves (push,
  pull, crane, tilt, drift), and CCC's cutting rules: each person keeps their angle across
  the back-and-forth and gets a new one after a few of their lines, at each preset's own
  rhythm; short lines don't count.
- **Close-ups that follow the face:** a close-up turns toward a face turned to its work
  and drops to look up into a lowered one (`bFollowFace`).
- **The 180-degree rule:** each conversation is staged from one side of the line between
  you (the side with more room), and no shot or camera move crosses it (`bEnforceLine`;
  `bTrue180` films everything from that one side).
- **Letterbox:** black bars slide in while the camera films a conversation, with
  Starfield's subtitle sitting in the bottom bar; the dialogue options stay hidden while the
  NPC speaks (except during their first line, the greeting, where the game lets A choose
  one) and fade in when the line ends (`[Interface]`).
- **Holding on the answer** after each persuasion round (the verdict included) or a skill
  check, seated people too: a close-up of the NPC, pushing in. In CCC's presets their whole
  reply stays on them, as in CCC; in CCC SF's, after the answer and the verdict, a shot of
  you reacting (`bPersuasionBeatNpc`).
- **Room shots** (Wide, Master, Distant): placed where the room around the two of you has
  space for them, measured with the game's collision check, on the conversation's side of
  the line and with both faces in view (`iRoomShotMinRoom`). In CCC's presets as far back
  as CCC's framing wants; in CCC SF's the furthest of them (Master) up to 9 m indoors and
  12 m outdoors, the others closer in proportion
  (`fRoomShotMaxIndoors`, `fRoomShotMaxOutdoors` to set your own).

### New in CCC SF

What CCC doesn't do.

- **NPCs keep facing you:** only what's on screen moves; the game still thinks the camera is
  at your eyes.
- **Seated people and merchants** get a plain over-the-shoulder shot instead of the shot
  library, whose angles often put the chair, the shop counter or the clutter around it in
  front of them (`bPlainShotsSeatedNpc`, `bPlainShotsMerchantsNpc`). Merchants are
  recognised by the game's vendor flag on their faction, wherever the shop is.
- **Four more presets, each with a film flavour:** Frontier (a space western), Starborn
  (*Interstellar* with a touch of Kubrick), Constellation (a hard sci-fi documentary) and
  Nebula (a space opera), with their own lenses, moves, cutting and sway (see Presets below).
- **Handheld feel:** how much the camera sways with the people in it, from a tripod (0) to
  fully handheld (100); each preset has its own (CCC's on a tripod as in CCC, Starborn barely
  there, Constellation handheld), or `iCameraSway` sets one.
- **How a conversation ends** (`iExitStyle`): a cut back to your own view just before the
  game hands the camera back (the default), a closing shot of the NPC alone as the dialogue
  menu closes, or the last shot to the end. Clean in both perspectives, and starts with at
  most a brief flash.
- **The game's camera where the shots don't fit:** small speakers (robots) and quest scenes
  you take part in (someone walks up and starts talking, several people taking turns) keep
  the game's own camera.
- **Preset key:** U switches to the next preset on the fly, even mid-conversation
  (`iPresetKey`).
- **Testing messages:** each line's expression, your face's answer and the shot used, with
  why, in the top bar while a conversation is filmed, and the preset in use in the bottom
  bar (`bShowMessages`, `fMessageSeconds`).

### Changed from CCC

What CCC also does, done differently here; where Starfield is the reason, the bullet says so.

- **A lens per angle, as CCC,** with a Starfield difference: Starfield films conversations
  through a narrow 30 degree lens; the mod sets its own while it films. CCC's presets use
  CCC 1.5.0's own lens for every angle (Close 35-50 degrees, Standard 50-72, Room 45-95), as
  CCC wrote them (checked against Skyrim clips); CCC SF's presets have their own (Starborn's
  long and compressed, Nebula's wide and long). `fConversationLens` sets one lens for
  everything (-1, the default, keeps each angle's own; 30 is the game's).
- **Emotion close-ups** (every tier in Frontier, Starborn and Nebula; strong lines only in
  Constellation and, of CCC's, Close): lines the game tags with an expression push in to a
  close-up of the speaker, 40% of them extreme close-ups (Frontier 25%; each for its own line
  only). The
  expressions come in three tiers with their own chances: mild (Amused, Confident,
  Thinking...) 25%, medium (Concerned, Serious, Smirk...) 40%, strong (Angry, Afraid,
  Shocked, Somber...) always; Neutral and Friendly never (`sCloseUpEmotions...Npc`,
  `iCloseUpChance...Npc`). About one line in five, more with emotional characters. CCC does
  this with lines Skyrim's writers marked as intense; because Starfield's lines carry no
  such mark, the expression decides. In CCC SF's presets one close-up of them holds 12 to 18 s
  at most (each preset its own), then a wider shot of them, so a run of emotional lines
  doesn't stay on one face for half a minute (`fCloseUpLimitSecondsNpc`). Each line's
  expression can be shown on screen while testing (`bShowMessages`). Tables:
  [docs/EMOTIONS.md](docs/EMOTIONS.md).
- **Reaction shots, reworked** (CCC SF's presets; CCC's have them off, as CCC ships, and CCC
  rolls a chance after every few lines instead): to break the monotony of a long stretch,
  after 7 to 12 s of their talking without a cut to you (each preset its own), their next
  line of 2.5 to 8 s is shown on you from its start to its end; a longer line, its first 4 s
  (`fReactionAfterSecondsPlayer`). Their emotion close-ups count towards the 7 s, and a
  regular one can be cut away from when they're reached. A reaction is an insert: the camera
  goes back to exactly the speaker's shot. It's you listening, or a close-up of your
  expression, by its tier like theirs: mild 25%, medium 40%, strong always
  (`iCloseUpChance...Player`). Never the overhead shot of you; in Frontier you listening is
  a medium or three-quarter, with close-ups of you kept for your expressions
  (`bListeningCloseUpsPlayer`). Details: [docs/REACTIONS.md](docs/REACTIONS.md).
- **Cutting:** CCC's line counts, with each preset's own rhythm. `iCuttingPace` nudges it,
  from -2 (calmer: a new angle after more lines, held longer, fewer fresh angles when the
  turn passes) to +2 (livelier), keeping the preset's character; 0 is the preset as
  designed. `iMinLines`, `iMaxLines` and `fShortestHold` set exact values instead. A room or
  shared shot (the two of you, the room) stays your angle for 2 of your turns at most.
- **Per-preset camera distances,** matched to each preset's widest shot: the camera stands
  where CCC's numbers put it, and each preset lets it go as far as its widest shot needs
  (Standard: 7 m indoors, 10.5 m outdoors); obstacle detection pulls it in or picks another
  angle where walls are closer.
- **Obstacle detection,** on by default (CCC's Keep Subject Visible is off by default):
  before each cut, the game's own camera collision test checks the line from the person in
  shot to the camera. A wall behind the camera pulls it in; a ladder or pillar in front of
  the person skips the shot for another angle; when nothing is fully clear, the angle with
  the most room is used (`bObstacleCheck`).
- **Bystanders** (as CCC's Avoid Framing Bystanders): someone standing still who'd fill the
  shot or hide a face is avoided like a wall; people walking through stay in.
- **Focus on the person in shot:** CCC has an optional depth-of-field blur over the picture;
  here the game's conversation blur is refocused on every shot onto whoever is in it, so
  they're sharp and the background stays softly blurred, even on longer outdoor shots
  (`bFocusOnSubject`). Two focus styles (`sFocusStyle`): Cinematic (more background blur)
  and Wide (deeper sharp zone, softer background blur). Shots from more than 8 m away have
  no blur.
- **Your character in shot from first person** (because of Starfield: its first-person view
  has no full body to film, where CCC switches Skyrim to third person): your full body is
  built at the start of the conversation (about a tenth of a second), including with seated
  NPCs, and you're back in first person when it ends.
- **Your character's face** (CCC's Responsive Expressions read the English wording of your
  lines; here the expression tags and option tags lead): the game keeps it neutral and your
  lines carry no expression tags, so the mod sets it: your face answers each NPC line's
  expression (mirrored or as a listener would), options with a skill, background or trait
  tag ([Persuade], [Space Scoundrel], [Wanted]...) and questions show on it, and your
  background colours neutral moments. Close-ups of you when your face shows something, and
  (CCC SF's presets) as you choose a tagged option, over the start of their answer: an
  extreme close-up for the confrontational skills (none in Constellation)
  (`bChoiceCloseUpsPlayer`, `fChoiceCloseUpSecondsPlayer`, `[Face]`; tables in
  [docs/EMOTIONS.md](docs/EMOTIONS.md)). Uses the expression function found by rrrafau.
- **People talking near you** (CCC 1.5.0 films them by key, or once you stand still; here
  also as you walk past, with a prompt): NPC-to-NPC conversations are noticed, who's
  addressing whom included, and filmed: a prompt on screen ("Listen in on nearby
  conversation. Press N", or hold B on a controller) and/or starting on its own when a
  conversation begins near you, even as you walk past; moving ends it once you've come to a
  stop (`[Overhear]`: `bListenPrompt`, `bAutoListen`; no prompt while it starts on its own).
  Only for the game's conversation scenes, not barks; one scene at a time, the nearest
  first, then the next one nearby; followed into the game's next scene with the same people,
  handed back when it only repeats itself. Letterboxed; two people filmed close (close-ups,
  three-quarters, shoulders), with a two-shot or a group shot now and then when three or
  more are talking. From first person, the view is third person while it films, with you
  hidden, and back to first person after.
- **Settings:** an in-game page through SFSE Menu Framework, as CCC uses SKSE Menu
  Framework, with changes saved to a `_user.ini` as CCC's go to `SD_user.ini`; optional
  here, and the INI works without it (see Settings below).

### Presets

Choose one with `sPreset` (or switch on the fly with U while testing). CCC's three are as
faithful to CCC 1.5.0 as Starfield allows: its angles, frequencies, moves, each angle's own
lens and each preset's cutting, from CCC's own exported presets, without this mod's additions
(no reaction shots; emotion close-ups only in Close, on strong lines; no close-up limit).
CCC SF's four each have a film flavour drawn from Starfield's "NASA-punk" influences, their
own lenses, moves, cutting and camera sway, and everything this mod adds: emotion close-ups,
reaction shots of you, close-ups of your expressions and choices, and a limit on how long
one close-up of them holds. They were last tuned from long test conversations with Sarah,
Sam, Barrett, Andreja and Muria (0.9.23-0.9.25).

| Preset | From | The look | Feels like | Room shots go back |
| --- | --- | --- | --- | --- |
| Standard | CCC | Shoulders, close-ups, three-quarters and mediums on both sides, mostly still (lenses 50-72) | A TV drama: classic shot / reverse shot, calm and readable | (none) |
| Close | CCC | Shoulders, tight close-ups and extreme close-ups, pushing and zooming in (lenses 35-50) | A film-noir conversation: faces fill the screen, every flicker of an expression counts | Distant only, as far as CCC's framing wants |
| Room | CCC | Wide and full-figure shots: masters, wides, long shots, overheads, with cranes on the wide ones (lenses 45-95) | A classic western or a stage play: people small in their surroundings, the place matters | As far as CCC's framing wants |
| Frontier | CCC SF (Standard) | Standard on slightly longer lenses (40-80): slow dolly-like drifts on the two-shot, three-quarters and mediums, singles with a shoulder edging in, heroic low angles, wide over-the-shoulders, the room now and then, a touch of handheld (sway 50). Emotion close-ups on every tier but rarely extreme (25%); one close-up of them at most 12 s; you listening in mediums and three-quarters | A space western (*Firefly*, *The Expanse*): Akila, Neon, the outposts | Up to 9 m indoors, 12 m outdoors |
| Starborn | CCC SF (Close) | Close-ups and shoulders on long lenses (37.5-60), singles with a shoulder edging in, a close profile, now and then a medium; faces a little off-centre, a barely-there handheld (sway 15), still shots that push in only on emotional lines, slow holds (one close-up at most 18 s), a wide shot for scale every few minutes with a slow crane up | *Interstellar* with a touch of Kubrick: the Unity, the artifacts, the uncanny | Up to 9 m indoors, 12 m outdoors |
| Constellation | CCC SF (Room) | Watched from across the room on longer lenses (40-70), past people's shoulders and side on; small slow drifts everywhere (the master too), the most handheld sway (60, with a slow wander), cranes on the long shots; faces now and then, emotion close-ups on strong lines only (at most 14 s), never an extreme close-up of you | A hard sci-fi documentary (*The Martian*, *Apollo 13*): the Lodge, labs, ships | Up to 9 m indoors, 12 m outdoors |
| Nebula | CCC SF | Every angle in the library and every move CCC has, bold and varied: heroic low angles that push in with a little tilt, high angles, profiles, group shots, now and then an overhead; quick cutting (sway 40; one close-up at most 12 s) | A space opera (*Guardians of the Galaxy*, *The Fifth Element*, *Valerian*, the original *Star Wars*) | Up to 9 m indoors, 12 m outdoors |

In every preset a room or shared shot (the two of you, the room) stays your angle for 2 of
your turns at most, and a reaction is never the overhead shot of you. `iCameraSway` sets the
sway for every preset instead of the preset's own (0 = tripod, 100 = fully handheld);
`fConversationLens` sets one lens for every angle; `fCloseUpLimitSecondsNpc` and
`iExtremeCloseUpChanceNpc` set the close-up limit and the extreme share for every preset.
Every preset's shots, shares, moves and lenses: [docs/PRESETS.md](docs/PRESETS.md).
How the camera starts and ends a conversation, frame by frame, with the measured numbers:
[docs/TRANSITIONS.md](docs/TRANSITIONS.md).

### Planned, roughly in order

- Done in 0.8.x: CCC's presets made faithful to CCC 1.5.0 (each angle's own lens, its
  cutting), CCC SF's presets rebuilt with film flavours, filming people talking near you
  (prompt and auto-start, scenes one at a time, followed into the next, group shots,
  letterbox), your character's face, reworked reaction shots, holding on the answer after a
  persuasion, emotion close-ups by tier.
- 0.9.x, polish and optimization, in this order:
  - Done in 0.9.0: the old INI names (from before the Npc/Player renames, and Stardust) no
    longer read; `iCloseUpDistance` removed (every preset has its own lenses now).
  - Seated conversations: strong vertical jitter at the start (0.8.5 test, The Rock); in
    first person on a bar stool your full body isn't built, so you're kept out of frame;
    talking to a seated person from the side (about 90 degrees) leaves you out of the
    picture (0.8.17 moved the plain shot so a seated NPC isn't hidden behind your body).
  - Built in 0.9.12, testing: an in-game settings page through
    [SFSE Menu Framework](https://www.nexusmods.com/starfield/mods/18201) (optional; F1):
    preset, reaction shots, emotion close-ups, sway and the rest, with the INI still working
    without it.
  - Done in 0.9.19 and 0.9.22: large speakers such as Vasco, whose bigger body blocked you in
    the narrower shots of you (those keep to wider angles now, the plain fallback shot too;
    `fLargeSpeakerHeightNpc`, `fLargeSpeakerAnglePlayer`).
  - Emotion close-ups tuned from testing (see [docs/EMOTIONS.md](docs/EMOTIONS.md)).
  - Compatibility testing with popular UI mods.
  - Quest scenes: logged in 0.9.20, what each one is made of (its actions, and whether it
    directs a camera of its own), to prepare the change planned after 1.0.
  - Once feature complete, four passes, in this order:
    1. Redundancy: code no longer in use and remnants of earlier versions taken out
       (finished experiments, old exit covers, diagnostics that have served their purpose).
    2. Debugging: looking for hidden bugs, such as a setting that doesn't do what the INI
       says (`bEnabled=0` didn't turn the whole mod off until 0.9.11; `iMinLines` was
       ignored until 0.9.8), and edge cases (loading or a menu mid-conversation, ships,
       scenes, combat, quick conversations in a row).
    3. Optimization: the cost of each frame and each scan measured, then made cheaper.
    4. The INI and its defaults reviewed last, once the settings are final.
  - Last, just before 1.0: the log's testing diagnostics and the testing messages off by
    default (they stay on until then, for testing), then a full test round on the release
    candidate.
- 1.0, public beta (late November 2026), after testing with popular UI mods (a HUD
  replacer, a dialogue menu replacer, a subtitle mod) and listing compatibility.

### After 1.0

Planned:
- Quest scenes that are just people talking filmed by this mod: today every quest scene you
  take part in goes to the game's camera, the safe choice, because some direct their own
  shots. Scenes with no camera actions of their own would keep this mod's camera, and later
  the game's camera would take over only while a scene's camera action runs. Built on the
  scene logs gathered from 0.9.20 on.
- Most settings also in the game's own Settings menu, as Gameplay Options from a small ESM.
- Large speakers measured by their actual body, replacing the height rule (height is only
  part of it: robots like Vasco are wide and bulky too). The line from the camera to your
  face is checked against the speaker's body, its width and bulk as well as its height (as
  CCC's Keep Subject Visible does, with each body as shapes sized from its bounds), so the
  shots of you clear any bulky speaker by exactly as much as they need to, and no more.
- Smoother starts and exits, as a matter of style: easing into the first shot and out of
  the last (a short push-in or settle, a fade, a match cut to your view) instead of the
  clean cuts used in 1.0.
- Objects the obstacle check can't see: plants and other things without collision in front of
  shots (the Viewport bar's plant), found by checking the bounding boxes of nearby objects
  along the line from the camera to the face, as is already done for people standing in the
  way; and walls it passes through (beside Commander Tuala at the MAST, every check said
  clear).
- More of the text of your lines for your face (and close-ups of you on strong ones): beyond
  the skill, background and trait tags and a question mark, clear cues in the wording, such
  as thanks, apologies, refusals, threats, jokes, condolences and goodbyes. High-confidence
  cues only, built from a tally of the options actually picked in play (the log records each
  one; since 0.9.12 the options offered and picked also go to their own file) and logged
  when they fire; English only, its own setting.

Mulling (not decided):
- A new angle of you once you've spent a while choosing your reply (about 10 s), instead of
  one shot held for as long as the options are open (CCC has a timer for this, off by default).
- Your traits setting your face's temperament (Introvert subtler, Extrovert fuller, Empath
  mirroring more). Open questions: mods that unlock every trait (many traits at once), which
  trait wins when several apply, and whether it should work only with the game's usual trait
  limits; it would need its own setting.
- The game's sitting camera: sitting down and standing up, it swings round to your face and
  drifts while it settles (vanilla too; the game's `bDisableFurnitureCamera` doesn't stop it).
  Whether the mod should steady it, since it's outside conversations.
- For walls the collision test passes through: a second check that the camera stands on walkable
  floor (the navmesh, Starvegas's approach) connected to where the two of you stand. Looked at
  for 1.0 and not adopted for now: the collision test covers nearly every case, the floor
  check would turn down good spots (above a counter or a table), and reading the navmesh needs
  game functions not yet found.

## Requirements

- Starfield **1.16.244** (other versions: the plugin loads but stays off)
- [SFSE](https://www.nexusmods.com/starfield/mods/106) for 1.16.244
- [Address Library for SFSE Plugins](https://www.nexusmods.com/starfield/mods/3256)
- Keep Starfield's own dialogue camera setting **on** (this mod hooks into it)
- Disable other dialogue camera mods
- Optional: [SFSE Menu Framework](https://www.nexusmods.com/starfield/mods/18201) for the
  in-game settings page

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
| `[Director]` | Preset, cutting pace, camera distances, 180-degree rule, camera moves, emotion close-ups (their tiers, the extreme share, the longest one close-up holds), room shots, reaction shots (listening close-ups or not), close-ups of you on tagged options (and how long), persuasion beat, bystanders |
| `[Shots]` | Per-shot tweaks: how often each shot is used, its move, amount and duration |
| `[Camera]` | One lens for every angle (instead of each angle's own), who gets the camera (shot/reverse or fixed), follow smoothing, sway, third-person exit hold, small-speaker height, large speakers (from what height, how far round the shots of you stay, the plain shot's angle) |
| `[Body]` | Showing your character, building the full body from first person |
| `[Transitions]` | How the first and last frames of a conversation are handled |
| `[PlainCamera]` | The simple over-the-shoulder shots used when the director is off |
| `[Interface]` | Letterbox, subtitles in the bars, dialogue options while the NPC speaks |
| `[Overhear]` | Filming people talking near you: on/off, the listen key, how far |
| `[Face]` | Your character's face: on/off, background expressions, questions, close-ups of you |
| `[Debug]` | Testing messages, the preset key, hook and camera-target switches for troubleshooting |
| `[GameSettings]` | Changing the game's own settings while it runs |

Each setting is explained in the INI itself.

With [SFSE Menu Framework](https://www.nexusmods.com/starfield/mods/18201) installed, most
of them are also in the game: Mod Menu (F1) → **Cinematic Conversation Camera SF**, on five
pages (Camera, Close-ups and reactions, You, Screen, Filming people talking). The page shows
your `_user.ini`'s value where you've changed a setting and the main INI's otherwise, and
writes changes to the `_user.ini`; **Reset** beside a setting removes your change. Changes
apply from the next conversation. If there's no `_user.ini` yet, the page creates one; with
Mod Organizer it lands in Mod Organizer's `overwrite` folder (`overwrite\SFSE\Plugins`),
where the game still reads it. Test switches, transitions, per-shot tweaks and the plain
camera are in the INI only.

## Testing and logs

The log is at
`Documents\My Games\Starfield\SFSE\Logs\CinematicConversationCameraSF.log`. It records
every shot the director chooses and why. Please include it with any report.

While the detailed log is on (`bDebugLog=1`), the dialogue options you're offered and the one
you pick are also added to `CinematicConversationCameraSF_YourLines.tsv` in the same folder,
kept across sessions (delete it any time). It helps build the wording cues planned after 1.0.

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
