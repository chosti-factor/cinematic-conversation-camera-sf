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
([source](https://github.com/rrrafau1337/player-camera-during-dialogue)). Shoutout to
**jedijosh920**'s [Player Dialogue Camera](https://www.nexusmods.com/starfield/mods/5229)
who first showed it could be done in Starfield. CCC and Player Camera During Dialogue are
GPL-3.0-or-later, and so is this project. See [CREDITS.md](CREDITS.md) for everything
this mod draws on.

## Status: 1.0, public beta

Last updated: 1.0.3 (2026-11-26).

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
  Starfield's subtitle where the game puts it (where it can overlap the top edge of the bottom
  bar) or centred in the bar (`bSubtitlesInBars`); the dialogue options stay hidden while the
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

New stuff compared to CCC (1.5.0)

- **NPCs keep facing you:** only what's on screen moves; the game still thinks the camera is
  at your eyes.
- **Seated people and merchants** get a plain over-the-shoulder shot instead of the shot
  library, whose angles often put the chair, the shop counter or the clutter around it in
  front of them (`bPlainShotsSeatedNpc`, `bPlainShotsMerchantsNpc`). Merchants are
  recognised by the game's vendor flag on their faction, wherever the shop is. In a quest
  scene you take part in, a merchant is one of several people taking turns and gets the shot
  library like the others (0.9.72).
- **Four more presets, each with a film flavour:** Frontier (a space western), Starborn
  (*Interstellar* with a touch of Kubrick), Constellation (a hard sci-fi documentary) and
  Nebula (a space opera), with their own lenses, moves, cutting and sway (see Presets below).
- **Handheld feel:** how much the camera sways with the people in it, from a tripod (0) to
  fully handheld (100); each preset has its own (CCC's on a tripod as in CCC, Starborn barely
  there, Constellation handheld), or `iCameraSway` sets one. Someone sitting sways the camera
  only so far (8 cm up and down, 4 cm sideways from where their head rests), so leaning on a
  table and back doesn't carry the camera with it (`fSeatedSwayCap`, hidden).
- **How a conversation ends** (`iExitStyle`): a cut back to your own view (the default),
  just before the game hands the camera back from first person and on the frame it does from
  third person; or a closing shot of the NPC alone as
  the dialogue menu closes. Clean in both perspectives, and starts with at most a brief flash.
- **The game's camera where the shots don't fit:** small speakers (robots) keep the game's
  own camera; someone sitting low is never taken for one. Quest scenes you take part in (someone walks up and starts talking, several
  people taking turns) are filmed by this mod too; `bGameCameraInScenes=1` (the settings
  page's "Vanilla camera in quest scenes") leaves them to the game's vanilla camera instead.
- **Preset key:** U switches to the next preset on the fly while a conversation (or people
  talking near you) is filmed (`iPresetKey`).
- **Testing messages** (off by default): each line's expression, your face's answer and the
  shot used, with why, in the top bar while a conversation is filmed, and the preset in use
  in the bottom bar (`bShowMessages`, `fMessageSeconds`).

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
- **Cutting:** CCC's line counts, with each preset's own rhythm. `fCuttingPace` nudges it,
  from -2 (calmer: a new angle after more lines, held longer, fewer fresh angles when the
  turn passes) to +2 (livelier), in steps of 0.5, keeping the preset's character; 0 is the
  preset as designed. Each preset keeps its own (`fCuttingPaceNebula` and so on; the settings
  page sets the one in use). `iMinLines`, `iMaxLines` and `fShortestHold` set exact values
  instead. A room or shared shot (the two of you, the room) stays your angle for 2 of your
  turns at most, and the camera never cuts to the shot already on screen: a wide shot is
  followed by a closer one.
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
  NPCs, and you're back in first person when it ends. Where the game doesn't build it (now and
  then while you sit), you're kept out of frame for that conversation rather than the camera
  switching views to force it (Known issues).
- **Your character's face** (CCC's Responsive Expressions read the English wording of your
  lines; here the expression tags and option tags lead): the game keeps it neutral and your
  lines carry no expression tags, so the mod sets it: your face answers each NPC line's
  expression (mirrored or as a listener would), options with a skill, background or trait
  tag ([Persuade], [Space Scoundrel], [Wanted]...) and questions show on it, and your
  background colours neutral moments. Close-ups of you when your face shows something, and
  (CCC SF's presets) as you choose a tagged option, over the start of their answer: an
  extreme close-up for the confrontational skills (none in Constellation)
  (`bChoiceCloseUpsPlayer`, `bFacePlayer`; tables in
  [docs/EMOTIONS.md](docs/EMOTIONS.md)). Uses the expression function found by rrrafau.
- **People talking near you** (CCC 1.5.0 films them by key, or once you stand still; here
  also as you walk past, with a prompt): NPC-to-NPC conversations are noticed, who's
  addressing whom included, and filmed: a prompt on screen ("Listen in on nearby
  conversation. Press N", or hold B on a controller) and/or, if you turn it on (off by
  default since 0.9.60), starting on its own when a conversation begins near you, even as you
  walk or jog past (sprinting too with `bAutoListenSprinting`), and stops you
  there: your movement is held until you let go of the controls, and moving again ends it;
  looking around is off while it films (`[SceneFilming]`: `bListenPrompt`, `bAutoListen`; no prompt while it starts on its own).
  Only for the game's conversation scenes, not barks; one scene at a time, the nearest
  first, then the next one nearby; followed into the game's next scene with the same people,
  handed back when it only repeats itself. Letterboxed; two people filmed close (close-ups,
  three-quarters, shoulders), with a two-shot or a group shot now and then when three or
  more are talking. From first person, the view is third person while it films, with you
  hidden, and back to first person after. Not in the game's opening, before you've made your
  character (no body of you to film yet; your conversations there keep the game's camera too). The game's HUD (the crosshair, name tags, compass,
  watch, health and weapon) is hidden while it films, and with `bSubtitlesInBars` the subtitle sits in the bottom bar as in your own
  conversations. Over-the-shoulder shots keep the camera at least 1.4 m from the near person,
  outside the distance at which the game hides people close to its camera.
- **Settings:** an in-game page through SFSE Menu Framework, as CCC uses SKSE Menu
  Framework, with changes saved to a `_user.ini` as CCC's go to `SD_user.ini`; optional
  here, and the INI works without it (see Settings below). Its six pages are named after the
  INI's sections (General, Director, Camera, Interface, Scene filming, Debug), and a setting
  changed from its default shows a button naming the default, which puts it back.

### Presets

Choose one with `sPreset` (or switch on the fly with U while testing); **Nebula** is the
default. CCC's three are as
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
| Frontier | CCC SF (Standard) | Standard on slightly longer lenses (40-80): slow dolly-like drifts on the two-shot, three-quarters and mediums, singles with a shoulder edging in, heroic low angles, wide over-the-shoulders, the room now and then, a touch of handheld (sway 45). Emotion close-ups on every tier but rarely extreme (25%); one close-up of them at most 12 s; you listening in mediums and three-quarters | A space western (*Firefly*, *The Expanse*): Akila, Neon, the outposts | Up to 9 m indoors, 12 m outdoors |
| Starborn | CCC SF (Close) | Close-ups and shoulders on long lenses (37.5-60), singles with a shoulder edging in, a close profile, now and then a medium; faces a little off-centre, a barely-there handheld (sway 15), still shots that push in only on emotional lines, slow holds (one close-up at most 18 s), a wide shot for scale every few minutes with a slow crane up | *Interstellar* with a touch of Kubrick: the Unity, the artifacts, the uncanny | Up to 9 m indoors, 12 m outdoors |
| Constellation | CCC SF (Room) | Watched from across the room on longer lenses (40-70), past people's shoulders and side on; small slow drifts everywhere (the master too), the most handheld sway (50, with a slow wander), cranes on the long shots; faces now and then, emotion close-ups on strong lines only (at most 14 s), never an extreme close-up of you | A hard sci-fi documentary (*The Martian*, *Apollo 13*): the Lodge, labs, ships | Up to 9 m indoors, 12 m outdoors |
| Nebula | CCC SF | Every angle in the library and every move CCC has, bold and varied: heroic low angles that push in with a little tilt, high angles, profiles, group shots, now and then an overhead; unhurried cutting, so each angle registers (sway 40; one close-up at most 12 s) | A space opera (*Guardians of the Galaxy*, *The Fifth Element*, *Valerian*, the original *Star Wars*) | Up to 9 m indoors, 12 m outdoors |

In every preset a room or shared shot (the two of you, the room) stays your angle for 2 of
your turns at most, and a reaction is never the overhead shot of you. `iCameraSway` sets the
sway for every preset instead of the preset's own (0 = tripod, 100 = fully handheld);
and each preset's own numbers (`fConversationLens`, one lens for every angle;
`fCloseUpLimitSecondsNpc` and `iExtremeCloseUpChanceNpc`, the close-up limit and the extreme
share; and the rest) are hidden until the preset editor after 1.0: read from `[Advanced]` if
you add them, for testing ([docs/INI_SETTINGS.md](docs/INI_SETTINGS.md)).
Every preset's shots, shares, moves and lenses: [docs/PRESETS.md](docs/PRESETS.md).
How the camera starts and ends a conversation, frame by frame, with the measured numbers:
[docs/TRANSITIONS.md](docs/TRANSITIONS.md). What was tried and removed along the way (test
switches, experiments, dead ends): [docs/MEMORY.md](docs/MEMORY.md).

### Roadmap

What comes after 1.0: test builds numbered 1.0.x on the Test build release here, then 1.1.0,
the next public release, with these and what players report.

High priority:
- Cleaner cuts in conversations with three or more people (the user, 1.0 test: good overall,
  but the cuts at speaker changes should be cleaner).
- More testing inside ships, and your character in frame while you sit from first person
  (the game doesn't always build your full body there; Known issues).
- The rules for filming people talking, looked at as a whole (the user, 2026-11-26): when it
  starts on its own, what ends it, when a scene you walked away from may start again (standing
  still since 1.0.2), sitting or lying in first person, the game's first-person lock, the sprint
  limit, scenes where you wake on a stretcher or table (the Watchtower start, the Ryujin
  questline's operation; since 1.0.3 nothing is filmed before you've made your character).
  From players' reports and a few sessions of play aimed at it.
- Your character looking at the wrong person in conversations with three or more people
  (the user, 2026-11-23): the game turns your head to the person the conversation started
  with, not to whoever is speaking. This mod doesn't set where you look yet; it needs the
  game's head-tracking functions, not yet found for 1.16.244.
- NPCs now and then fading in a moment after a conversation starts (seen once, the Ranger at
  Neon, from first person): rare; to look into with footage. Probably the
  game hiding people close to its camera, the cause of the same fade while filming people
  talking (fixed there in 0.9.29 by keeping the camera further away); a fix that keeps the
  people in shot fully visible would cover both.
- Quest scenes that direct their own camera: this mod films every quest scene since 0.9.76
  (with `bGameCameraInScenes=1` none). Better: the game's camera only while a scene's own
  camera action runs, this mod's otherwise. Built on the scene logs gathered from 0.9.20 on.

Low priority:
- The sprint limit for filming people talking scaled by your speed multiplier (buffs, powers,
  mods that change how fast you move), so a fast jog isn't taken for a sprint. Needs reading
  the game's actor values, not yet confirmed for 1.16.244 (the user, 2026-11-20: after 1.0).
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
  cues only, built from the tally of options offered and picked in play (2,913 options, 676
  picks: docs/research/your-lines-tally.tsv, summed up in docs/EMOTIONS.md) and logged when
  they fire; English only, its own setting.

Mulling (not decided), last:
- Filming people talking while you sit, from first person: since 1.0.2 (a test build; public
  in 1.1.0) it doesn't film then (filming hides you, so the seat showed empty). Building your
  full body for it instead, so it can film with you in it (the user, 2026-11-26).
- The overlapping lines after filming people talking (Known issues): a way to set it off
  reliably, then the cause. A lead: Player Camera During Dialogue keeps camera changes away from an
  NPC's multi-part response until it has fully played, against a dialogue-skip bug of its own.
- The rare soft frame at a cut (the new shot's focus reaching the old shot's last frame): how
  the game's own quest-scene cameras cut cleanly, as a model (the user, 2026-11-22).
- The letterbox kept through a conversation's exit until the cut back to your view (the user,
  2026-11-22). The bars live in the game's dialogue menu, which closes as the last line ends,
  so they go 0.4 s (first person) or about 1.5 s (third person) before the cut. Stand-in bars
  were tried in 0.9.47-0.9.48 and left a one-frame flicker as the menu closed; clean exits
  come first (docs/MEMORY.md).
- A preset editor (the user, 2026-11-21): the per-preset settings (shot choices, close-up
  chances, cutting pace and the like) edited in one place, instead of spread over the settings
  page and the INI. The 13 settings taken off the settings page for it in 0.9.60:
  docs/INI_HISTORY.md.
- Every preset revisited with what 1.0's players report, tweaked further, and maybe new ones
  (the user, 2026-11-20).
- Coming back from a reaction as a turn of its own, so it can bring a fresh angle (0.9.31
  test: Sarah's close-up came back after each of three reactions, five times in all).
- A close-up of you on your own turn when your face shows a strong expression. Today the
  close-ups that follow your face come almost only from reaction shots, which need the NPC to
  talk a while; someone clicking quickly through dialogue mostly sees close-ups of you as
  your turn's angle, not tied to your expression.
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
  in-game settings page. Its text size is the framework's own, for every mod's page: in the
  framework's own Settings (F1, then Settings: UI scale, font size, font), applied at once, or
  `UIScale` and `FontSizeMedium` under `[Fonts]` in `SFSEMenuFramework.ini`

## Install

1. Download the mod from its Nexus Mods page, or `CinematicConversationCameraSF-<version>.zip`
   from the latest release under **Releases** here. (The **Test build** pre-release always has
   the newest build from the main branch, for testing.)
2. Install the zip with your mod manager, or copy its `Data` folder into your Starfield
   folder.

The `.pdb` file next to the DLL is its debug symbols: a crash logger uses it to name the
mod's functions in a crash log, which makes a crash report far more useful. It does nothing
in the game; leave it in place.

## Compatibility

What works with CCC SF, what doesn't, and what's still to be tested:
[docs/COMPATIBILITY.md](docs/COMPATIBILITY.md). In short: other dialogue camera mods don't mix
with it; UI mods work as long as the dialogue options list and the subtitle keep their usual
places in the game's menus. [AutoZoom](https://www.nexusmods.com/starfield/mods/10543) (the one
that switches to first person indoors) is only partially compatible: it caused freezes of your
character's body and of the ship in the 1.0 test.

## Known issues

- **A one-frame glitch at some speaker changes in scenes with three or more people.** When
  someone else takes the line, the game moves its own conversation camera to them; about half
  the time, one frame shows this mod's shot slightly or clearly out of place. On the Roadmap,
  high priority (docs/MEMORY.md has what's known).
- **Your character looking at the wrong person** in conversations with three or more people,
  and **an NPC now and then fading in** a moment after a conversation starts: on the Roadmap, high priority.
- **Sitting yourself, from first person, you're sometimes not in the shots.** The game doesn't
  always build your full body while you sit (a chair in the CF01 brig, a seat aboard your
  ship), so the mod keeps you out of frame for that conversation. On the Roadmap, high priority.
- **AutoZoom** ([Nexus 10543](https://www.nexusmods.com/starfield/mods/10543)): freezes of your
  character's body and of the ship with it on (Compatibility).

- **Overlapping lines after filming people talking (rare).** Now and then, after the mod has
  filmed people talking near you, a later conversation has the speaker's lines playing over
  each other. It hasn't happened without a filming before it, and it isn't reliably
  reproducible: in the tests so far, 2 of 7 conversations after a filming, with the same steps
  each time. Loading a save clears it. If you see it, please send the log with the detailed log
  on, and say what was filmed before and how the filming ended. (Player Camera During Dialogue
  lists the same symptom as a known issue of its own.)

## Settings

`Data\SFSE\Plugins\CinematicConversationCameraSF.ini`. Put your own changes in
`CinematicConversationCameraSF_user.ini` next to it (same sections and keys) so updates
don't overwrite them. The files are read again whenever one of them changes (outside
conversations, within half a second), so changes apply without restarting the game.

| Section | What's in it |
|---|---|
| `[General]` | On/off |
| `[Director]` | Preset, cutting pace, 180-degree rule, camera moves, emotion close-ups on or off, persuasion beat, reaction shots of you, close-ups of you on tagged choices, your character's face on or off |
| `[Camera]` | Camera sway, the game's camera in quest scenes, the focus style |
| `[Interface]` | Letterbox and its height, subtitles in the bars, dialogue options hidden while the NPC speaks |
| `[SceneFilming]` | Filming people talking near you (the settings page's Scene filming page): on/off, starting on its own, the prompt, the listen key and button, how far |
| `[Debug]` | The detailed log, testing messages, the preset key, and checks to turn off for troubleshooting (obstacles, bystanders, close-ups following the face, focus on the person in shot, showing your character, the shot library, the full body from first person) |
| `[Advanced]` | Technical settings, rarely worth changing, in four groups: shots (never across the eyeline, who gets the camera, plain shots for seated people and merchants, small and large speakers), close-ups (the expressions' tiers, the persuasion tags), exits and screen (the exit style, the letterbox's slide, the options' fade), filming people talking (hand-back timings, stop distances) |

Each preset's own numbers (lens, distances, close-up and reaction chances and timing, your
face's background and question looks) and changes to single shots (`[Shots]`) are hidden
until the preset editor after 1.0 (0.9.70, the user): not in the INI, but read if you add
them, for testing. All of them, explained: [docs/INI_SETTINGS.md](docs/INI_SETTINGS.md).

Each setting is explained in the INI itself. Every setting, its section, default, settings
page and whether it's hidden: [docs/INI_SETTINGS.md](docs/INI_SETTINGS.md); how the layout came
about: [docs/INI_HISTORY.md](docs/INI_HISTORY.md).

## Testing and logs

The log is at
`Documents\My Games\Starfield\SFSE\Logs\CinematicConversationCameraSF.log`. It records
every shot the director chooses and why. Please include it with any report, with the detailed
log on (`bDebugLog=1`, or **Detailed log** on the settings page's Debug page).

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

Additional terms, under section 7 of the GPL (for the parts written for this project):

- Modified versions must be marked as different from the original (section 7c): they must use
  a different name, and must not present themselves as Cinematic Conversation Camera SF or
  CCC SF, or as made by chosti-factor.
- No rights are granted to use the name Cinematic Conversation Camera SF, CCC SF, or the
  author's name to promote a modified version (section 7e), beyond the credits the GPL
  requires.

Not required, but kindly asked: link to the Nexus or GitHub page rather than re-uploading, so
players get updates and fixes; send fixes back (a pull request or a message); say so if you
used AI tools for your changes; and please don't use this mod to train AI models.
