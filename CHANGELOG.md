# Changelog

All notable changes to Cinematic Conversation Camera SF, newest first. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/). 1.0.0 is the first public release
(a beta); every 0.x version was an alpha test build. "Diagnostic" items only add information
to the log for testing.

## 1.0.1 - unreleased

### Changed (the user's new game)
- While the game keeps you in first person and won't let you change view (as at the start of a
  new game, until you've made your character, and probably in the CF01 brig), people talking
  near you aren't filmed. Filming switches the view to third person and hides you, so where the
  game has you lying or sitting somewhere, its shots showed the place empty: in the user's new
  game, with "start on its own" on, it filmed Lin and Heller in the mine several times before
  the character editor, the stretcher you lie on empty behind them (the Watchtower creation
  reuses that scene). Your own conversations keep this mod's camera (the user's choice).
  Read from the game's input switches (changing view switched off for a second or more of
  gameplay); the log says when the game switches changing view off and on.
- Nor while you sit or lie on something in first person (a chair, a bench, a bar stool, the
  stretcher): filming hides you, so its shots showed the place empty. The user's second test:
  the mine's view lock read as above only covered a moment of the opening, and the stretcher
  scene before the character editor (the game's furniture camera) was still filmed. From third
  person, sitting, filming goes on as before (you're in its shots).

## 1.0.0 - 2026-11-25 (public beta)

The first public release: 0.9.77, the release-candidate test's changes below, and the release
itself.

### Changed (the release-candidate test, the user)
- The overhead shots are accents, as the extreme close-ups are: never a person's usual angle.
  As Agent No. 1's, the camera came back to hers after a reaction of you with its crane down
  again, 13 s after the first, and held it for 22 s over four of her lines.
- The overhead shots keep the head clear of the top letterbox bar (headroom 0.16 of the
  picture's height, was 0.00, CCC's): the bar nearly covered Agent No. 1's head.
- Nebula's overhead shots back to a weight of 15 each (from 12, 0.9.23), now that they can't
  take over: about 1 in 55 new angles for each, 1 in 28 for either.
- Someone sitting sways with the camera only so far: the framed head stays within 8 cm of where
  it rests up and down and 4 cm sideways (the side-to-side sway the more noticeable: a head
  moving sideways also turns the line between the two people, which every shot is set from),
  and the rest point follows the head over a few seconds (hidden `fSeatedSwayCap`, [Advanced],
  the up-and-down cap, sideways half of it; 0 = no cap). Leaning on a table and back, as your
  character does at the Wilcox table, went past the 4 cm dead band, so the camera followed the
  whole lean. Standing people and every preset's sway are as before.

### Fixed (the release-candidate test)
- Talking to someone while this mod films people talking (pressing A on one of them, or anyone
  nearby): the conversation was ended on its first frame and played through the game's
  first-person view. The game hadn't switched to its dialogue camera yet (filming had it in
  third person), and that was taken for the conversation having ended. Such a conversation now
  gets a second and a half for the switch (Noel at the Lodge, Inevitable at the Trackers
  Alliance HQ).
- Two crashes in the game's UI engine as a conversation started (Tank at the Trackers Alliance
  HQ; Nyx at Neon, the first conversation after starting the game). The dialogue menu is built
  as the conversation starts, partly on another of the game's threads, and this mod's letterbox
  bars and option features went into it 5-35 ms after it opened, while it could still be being
  built. A new menu is now left alone for its first half second; the letterbox bars still come
  in at once, drawn on the HUD messages menu (as when filming people talking) until the
  dialogue menu's take over. A failed call into a menu waits half a second too (was the next
  frame).
- A cut to a new speaker sitting or standing close to the last one (under 2 m) slid from the last
  speaker's head to the new one's for about a second, and the spot predicted for the game's
  camera at the change was worked out from that sliding point (0.5 m off). The head was
  followed smoothly and only snapped for 2 m or more; it's now taken afresh at every speaker
  change (Emma Wilcox and Sarah at one table: a messy cut to Sarah).
- Talking to someone while you sit, from first person, where the game didn't build your full body
  (Sarah aboard the Cryoknox): the camera cycle that builds it switched the game's camera between
  first and third person four times and gave up, as it takes a body as built only at standing
  height. Sitting, it isn't tried: you stay out of frame for that conversation (the user: "the
  camera bounced around like crazy").
- Someone sitting low (Karl Fielding, his head 0.76 m above his feet) was taken for a small
  robot and left to the game's camera. Someone sitting on something is never a small speaker
  now; a low head is logged with the speaker's race, to learn from.

### Changed (the settings page)
- "Game's camera in quest scenes" is now "Vanilla camera in quest scenes" (the user).
- The pages are named after the INI sections, so a setting has the same home in both (the
  user): General, Director, Camera, Interface, Scene filming, Debug (were General, Camera,
  Close-ups and reactions, Screen, Filming people talking), with friendly headings within. The
  preset, cutting pace, camera moves and the 180-degree rule are on Director with the close-ups.
- The headings within each page are white and bold (were grey), a switch's label is greyed while
  it's off, and the notes (changes apply from the next conversation, where they're saved, the
  preset key, and the filming heads-up) are together at the end of each page, one bullet each
  (the note on when changes apply was at the top of every page) (the user).
- "Reset" is now a button naming the default ("Default: Wide"), shown exactly while a setting
  isn't at its default; clicking it, or setting the default by hand, takes the line out of your
  `_user.ini` (the user: Reset was hard to reach, came and went, and didn't say what it reset
  to). Lists and sliders are a third as wide. "Them" is "NPCs" ("Close-ups of NPCs", "Hold on
  the NPC's answer", "Hide while NPCs speak").
- `bDebugLog` (the detailed log) is in [Debug] now, beside `bShowMessages` (testing messages):
  both on the new Debug page. [General] holds only `bEnabled`, the General page only the mod's
  switch. Under [General] `bDebugLog` is no longer read: move it in your `_user.ini`.

### Compatibility
- [AutoZoom](https://www.nexusmods.com/starfield/mods/10543) (Nexus 10543, the plugin that
  switches you to first person indoors) is partially compatible: in the 1.0 test it caused
  freezes of your character's body and of the ship (docs/COMPATIBILITY.md).

### Release
- The version is 1.0.0. The README's status and install sections describe the release (Nexus
  Mods, or the versioned releases here) instead of the alpha test build; its known issues add
  the one-frame glitch at some speaker changes in scenes with three or more people, your
  character looking at the wrong person in those scenes, an NPC now and then fading in, you
  out of frame now and then while you sit from first person, and AutoZoom.
- The build makes a versioned release from a `v` tag (`CinematicConversationCameraSF-1.0.0.zip`,
  checked against the version); the rolling Test build stays, its notes no longer saying alpha
  or that it writes detailed logs (off by default since 0.9.63).

## 0.9.77 - 2026-11-24

### Changed (the pre-1.0 code review)
- `bSpeakerChangeRoot` places the shot against the game's camera's predicted spot on the
  speaker change's own frame only (it was up to three frames while the move went unreported).
  In all 29 late reports in the tests (three runs) the move was reported on the next frame, so nothing
  changes there; a speaker change where the game's camera doesn't move at all would now cost
  one misplaced frame at most, not three.

## 0.9.76 - 2026-11-24

### Changed (the user)
- `bGameCameraInScenes` is off by default: this mod films quest scenes you take part in too,
  with tonight's pacing and speaker-change fixes (0.9.67-0.9.75). 1 (or the settings page)
  leaves them to the game's own camera, as before.

## 0.9.75 - 2026-11-24

### Removed
- The `iSpeakerChangeOldRoot` test (0.9.74). Its guess was wrong: in the test (Benjamin
  Bayu's scene, twice, clips frame by frame), with it on every speaker change where the game
  reports its camera's move at once had two or three wrong frames instead of one small jump.
  The jump stays, after 1.0 (docs/MEMORY.md).

### Docs (the user)
- docs/INI_SETTINGS.md is now only the current settings: every one with its section, default,
  settings page and whether it's hidden, and the counts. The old one, the reviews and the
  reasons behind the layout, is docs/INI_HISTORY.md (with the preset numbers' INI text).

### Test results (0.9.74)
- The learned camera spot settled at 1.97-2.01 m from the speaker's head; the prediction was
  1-6 cm off (2-8 before). The nudge before the cut, where the game reports its move a frame
  late, looks about the same in the clips: kept, it does no harm.

## 0.9.74 - 2026-11-24

### Changed (the jumps left at speaker changes; the user)
- The spot the game's camera moves to at a speaker change is learned from each change (as at a
  conversation's start), so the prediction 0.9.73 places the shot against, 2-8 cm off in the
  test, closes in over a scene: a smaller nudge before the cut where the game reports its move
  a frame late. The detailed log notes each spot learned.

### Test (hidden, off)
- `iSpeakerChangeOldRoot=2` under `[Advanced]`: where the game reports its camera's move on the
  speaker change's own frame, the shot placed against where its camera was, for that many frames
  (0-3). The guess: those frames are still drawn from there, the cause of the two frames of the
  new speaker in a slightly wrong framing just after the cut.

## 0.9.73 - 2026-11-24

### Fixed (scenes with three or more people: the flash at a speaker change)
- `bSpeakerChangeRoot` is on by default (still hidden in `[Advanced]`, 0 turns it off). The
  0.9.72 test, two runs of Ularu Chen's and Benjamin Bayu's scenes (clips and log, frame by
  frame): at every speaker change the game moves its own conversation camera to the new
  speaker (0.6 to 5 m), to within 2-8 cm of the spot predicted for it. In 9 of 16 changes it
  reports the move a frame late, and the picture of that frame was this mod's shot dragged by
  the move: a view from behind Imogene's head, two black frames inside a wall, a wrong shot of
  Bayu. With the shot placed against the predicted spot (run B), all 9 were gone; left is a
  slight nudge of the held shot two frames before the cut.
- Still there (after 1.0 unless it's easy): in the other 7 changes, where the game reports the
  move at once, the new speaker shows for two frames in a framing close to, but not quite, this
  mod's (likely the game's own shot of them), then this mod's: a small jump just after the cut.

## 0.9.72 - 2026-11-24

### Changed (the user)
- Merchants in a quest scene you take part in are filmed with the shot library, like the
  others there, not the plain shots (`bPlainShotsMerchantsNpc` still applies everywhere else,
  shopping included). In Benjamin Bayu's scene, Yannick Legrande behind his counter switched
  the camera between the plain shots and the shot library at every turn, the cause of 0.9.71's
  fixes. Seated people keep the plain shots (a chair in front of them is worse than a counter).

### Diagnostic and test (the 0.9.69 Ularu Chen clips, frame by frame)
- One flash left in the Ularu scene, as Imogene Salzo took the line: a frame of a frontal
  close-up of her that isn't one of this mod's shots, between Ularu's close-up and the reaction
  of you that followed. The likely cause, as at a conversation's start: the game moves its own
  conversation camera to the new speaker and draws from there a frame before it reports it, so
  this mod's shot is placed against the old spot for that frame. The detailed log now records
  where the game's camera is reported on each of the six frames after a speaker change, and
  `bSpeakerChangeRoot=1` (hidden, `[Advanced]`, off) places the shot against the spot the game's
  camera is about to move to, as the start does, for a comparison run.

## 0.9.71 - 2026-11-24

### Fixed (rough cuts in scenes with three or more people; the 0.9.69 clips of Benjamin Bayu and
Yannick Legrande, frame by frame, and the log)
The Ularu Chen scene (no merchant in it) was clean; the rough cuts came where a merchant
(Yannick, behind his counter, filmed with the plain shots) and someone filmed with the shot
library took turns.
- A merchant's or seated person's emotion close-up stays through the gap between two lines
  (half a second). It went back to the plain shot as the line ended, and the next line came
  70 ms later: four frames of the plain shot between his close-up and the next speaker's.
- When another NPC takes the line while you're on screen, the camera stays on you until their
  line is decided (half a second at most), then makes one cut. It cut to them at once, and
  their emotion close-up followed a frame or three later.
- A shot held at a speaker change keeps its focus and its lens. The focus was measured to the
  new speaker, so the held shot sharpened or softened three frames before the cut; the lens
  could follow the shot behind it.
- A merchant's or seated person's emotion close-up counts as a cut for these holds and for the
  shortest hold.

## 0.9.70 - 2026-11-23

### Changed (the settings page, the user)
- A General page, first: the mod's switch (Cinematic conversations) and the detailed log (for
  bug reports). They were on the Camera and Screen pages. The testing messages stay on Screen.

- Each preset's own numbers and single-shot changes are hidden until the preset editor after
  1.0 (the user): `[PresetTuning]` (20 settings) and `[Shots]` are out of the INI. The 20 are
  read from `[Advanced]` if added, `[Shots]` as before; each explained in docs/INI_SETTINGS.md
  (an appendix with the INI's text). 54 settings in the INI now, 50 hidden.
- `[GameSettings]` is hidden: out of the INI (it had one commented example, never tried in
  play), still read if you add it (docs/INI_SETTINGS.md, docs/MEMORY.md).

### Docs
- README, after 1.0: your character looking at the wrong person in conversations with three
  or more people (the user); NPCs fading in now and then was already there.

## 0.9.69 - 2026-11-23

### Fixed (0.9.67's pacing in scenes with three or more people; the user: flickers)
From the 0.9.68 test log (Ularu Chen and Imogene Salzo, Benjamin Bayu and Yannick Legrande).
- A reaction of you that would come before the shot on them has been up the preset's shortest
  hold is left to a later line, not waited for. Waiting (0.9.67) cut to you late in the line:
  reactions of 0.4 s and 1.0 s, cut to just before the line ended.
- Another NPC taking the line no longer counts as a cut for their emotion close-up or a
  reaction: both come at the line's start again, as before 0.9.67, not 0.7 s or 2.5 s in. It
  still holds off a new angle or a wider shot.
- With someone on screen when another NPC takes the line, the shot stays on the last speaker
  until the new line is decided (half a second at most), so a reaction or an emotion close-up
  comes straight from it, not after a moment of the shot turned to the new speaker.
- The detailed log notes each of these (a reaction left to a later line, a shot held at a
  speaker change).

## 0.9.68 - 2026-11-23

### Fixed
- Leaving a quest scene the game's own camera films (`bGameCameraInScenes`) by walking away:
  the scene's shot widened, out of focus and at an odd angle, for a moment before the game's
  exit. The game kept its conversation camera on for 0.8 s after the conversation ended, and
  this mod's exit (your view drawn for a frame, your gameplay lens forced) ran on top of it.
  Conversations the game's camera films now end with the game's own exit, untouched. (From
  before 0.9.67; seen in its test, leaving Ularu Chen's scene, twice.)

### Changed (the settings page, the user)
- Settings that depend on another are indented four times as far (3.4 times the font size,
  136 px at the framework's 40 px; it was 34), so it keeps its proportion at any UI scale.
- The "Heads up" note for "Start on its own" sits at the end of the page, not between that
  setting and the next one.

## 0.9.67 - 2026-11-23

### Changed (quest scenes, the user: shots cycling too fast, some too short)
From the 0.9.60 log of quest scenes filmed by this mod (Ularu Chen and Imogene Salzo, Nyx,
Sophia Grace, Trevor Petyarre; Nebula), every shot under 2.5 s sorted by cause. Each rule
below waits for the preset's own shortest hold (paced), so each preset keeps its rhythm.
- No cut back to you, as their line ends, before the shot on them has been up the shortest
  hold. Their close-up was cut to as a short line ended and away again 0.8 s later, for the
  options; a wider shot of a persuasion's long answer lasted 1.0 s.
- When another NPC takes the line while you're on screen, your shot stays as it was until the
  cut to them. It was turned towards the new speaker as their subtitle came up, and cut away
  a third of a second later, as their voice started: four 0.3 s shots in one scene.
- When another NPC takes the line while one is on screen (the shot now framing the new
  speaker), the next cut waits as after any other: a wider shot came 0.1 s after one.
- A reaction of you waits until the shot on them has been up the shortest hold (one came
  1.6 s after a cut).
- The close-up of you on a tagged choice or a persuasion option lasts at least the shortest
  hold (`fChoiceCloseUpSecondsPlayer`, 2 s, was the length whatever the preset): five in one
  persuasion, each cut to and away from in 2 s.

### Docs
- README: the settings page's text size is SFSE Menu Framework's own, for every mod's page:
  its own Settings page (UI scale, font size, font, applied at once) or its INI.

## 0.9.66 - 2026-11-23

### Fixed (the pre-1.0 code review)
- The settings page: the camera sway slider had no tooltip (only its "the preset's own"
  box did).
- The settings page: resetting the preset to the INI's value didn't show while a preset
  chosen with the preset key was in force; the reset now clears that choice, as a change does.

### Changed (the user's call)
- The preset key (U) works only while a conversation or people talking near you are filmed.
  Elsewhere a U typed into a name (a ship, an outpost) switched the preset unseen.
- Camera sway's "the preset's own" is a switch like the other on/off settings, and the sway
  slider stays on the page, greyed out while it's on.

### Removed
- The settings page's unused widget kinds (whole numbers, decimals with "the preset's own",
  and the 0.05-step sliders), left over since settings moved off the page in 0.9.60.

### Docs
- README: the `.pdb` that ships with the DLL, and what it's for (crash logs).

## 0.9.65 - 2026-11-23

### Changed (the user's call)
- `[Face]` merged into `[Director]`: `bFacePlayer` (your expressions) sits after the reaction
  and tagged-choice switches, as on the settings page.
- The testing messages' and the log's reason for an emotion close-up says so: "emotion
  close-up", "turn, emotion close-up", "emotion close-up, after the extreme close-up" (it was
  "intense line", a word used nowhere else; the user).
- `[Overhear]` renamed `[SceneFilming]` (filming people talking near you). No old name is read
  (nothing released): a `_user.ini` with `[Overhear]` lines needs them under the new name.

## 0.9.64 - 2026-11-23

### Changed (the settings page, the user's asks)
- Every setting has a tooltip: added for the game's camera in quest scenes, the letterbox,
  filming people talking and its range; fuller ones for emotion close-ups (what the preset's
  own is), holding on their answer (skill checks too), reaction shots and the skill-choice
  close-up (the preset's own), the bar height (0.12 is about 2.39:1) and the testing messages
  (off normally).
- On/off settings are switches, as on a phone (green when on, grey when off), instead of
  checkboxes; clicking the label flips them too.
- Settings regrouped so like sits with like: Camera (the mod; look: preset, pace, focus style;
  movement: camera moves, sway; staging: 180-degree rule, quest scenes), Close-ups and reactions
  (them; you), Screen (letterbox, dialogue options, testing), Filming people talking (people
  talking near you: film them, range; how it starts: the prompt, on its own, while sprinting).
- Each group's name sits on a greyed line with a gap above it; a setting that needs another
  is indented almost twice as far (34 px a step), joined to it by a greyed corner line.

## 0.9.63 - 2026-11-23

### Changed (the INI, [Advanced] thinned, the user's call)
- Hidden (out of the INI, still read from `[Advanced]` if added; listed with their defaults in
  docs/INI_SETTINGS.md): `iMinLines`, `iMaxLines`, `fShortestHold` (the cutting pace covers
  them), `fReactionLineMinSecondsPlayer`, `fReactionLineMaxSecondsPlayer`,
  `fChoiceCloseUpSecondsPlayer`, `iRoomShotMinRoom`, `fFollowSmoothing`, `fWideFocusBeyond`,
  `fListenHold`.
- To `[Debug]`: `bDirector` (the shot library), `bFixFirstPersonBodyPlayer` (the step most
  likely to clash with a body or first-person mod: off to rule that out) and `fMessageSeconds`.
- To `[PresetTuning]`: `fReactionLongLineSecondsPlayer` (how long a reaction of you lasts on a
  long line, a preset's feel).
- `[Advanced]` keeps 19, in four groups: shots, close-ups, exits and screen, filming people
  talking. `[PresetTuning]` (20) in five: lens and distances, their close-ups, reactions of
  you, close-ups of you, your face.

## 0.9.62 - 2026-11-23

### Changed (the INI, section by section: [Overhear] and [Face], the user's call)
- `[Overhear]` unchanged.
- `[Face]` keeps your expressions on or off; how often your background shows on your face and
  the questioning look on questions (`iSignatureChancePlayer`, `iQuestionChancePlayer`) to
  `[PresetTuning]` (the same for every preset for now).
- The settings page: four pages. "Your expressions" moved to "Close-ups and reactions", with
  the close-ups and reactions of you; the "You" page is gone.

## 0.9.61 - 2026-11-23

### Changed (the INI, section by section: [General] and [Director], the user's call)
- A new `[PresetTuning]` section, before `[Shots]` and `[Advanced]`: each preset's own
  numbers, meant for the preset editor after 1.0. From `[Director]`: the camera's and the room
  shots' furthest distances, their emotion close-up chances by tier, the extreme share and the
  longest close-up, the reaction shots' timing and chance, listening reactions in close-up, and
  the close-ups of you by tier. `[Shots]` moved next to it.
- `fChoiceCloseUpSecondsPlayer` to `[Advanced]`.
- `bObstacleCheck`, `bAvoidBystandersNpc`, `bFollowFace` and `bFocusOnSubject` to `[Debug]`,
  for troubleshooting (the user: no reason to turn them off otherwise), and off the settings
  page. Without focus on the person in shot, the game's blur focuses about 2 m from the
  camera and anyone further away is soft.
- `fConversationLens` from `[Camera]` to `[PresetTuning]`. `[Camera]` keeps the sway (back on
  the settings page: the user wants that override at hand; a preset editor would set each
  preset's own, and this overrides them all), the game's camera in quest scenes and the focus
  style.
- `[Body]` is gone: showing your character (`bRevealPlayer`) to `[Debug]` (off, the shots of you
  show where you'd be). The letterbox's slide time (`fLetterboxFade`) and the options' fade
  (`fOptionsFade`) to `[Advanced]`. All three off the settings page: 24 entries.
- `[Director]` keeps 10 settings, all choices: the preset, the pace, the 180-degree rule,
  camera moves, and emotion close-ups, the persuasion beat, reaction shots and tagged-choice
  close-ups on or off.

## 0.9.60 - 2026-11-22

### Changed (the INI review, first pass: the user took the proposal in docs/INI_SETTINGS.md)
- The settings page has the main choices only: 30 entries instead of 55. Off it, still in the
  INI and read as before:
  - a preset's own numbers, for a preset editor after 1.0: camera sway, lens, the furthest
    distances indoors and outdoors, their close-up chances by tier, the extreme close-up
    share, the longest close-up, the reaction chance, listening reactions in close-up, the
    skill-choice close-up's length, extreme close-ups of you;
  - technical tuning: never across the eyeline, the plain shot for seated people and
    merchants, the three large-speaker settings, full body from first person, your
    background's expression and asking a question (%), the three "ending it by moving"
    settings.
  A line at the page's foot says the rest is in the INI, each explained.
- Defaults: the detailed log off (`bDebugLog=0`), the testing messages off
  (`bShowMessages=0`), and filming people talking no longer starts on its own
  (`bAutoListen=0`; the listen prompt stays on). Settings you've changed on the page or in
  your `_user.ini` keep your value.
- "Emotion close-ups" help text: close-ups come on every tier of expression (always on strong
  ones, now and then on milder ones), not only strong lines.
- The INI's comments: the header says how the files are read again (when they change, not
  before every conversation, since 0.9.41) and that the settings page writes to the
  `_user.ini`; version history taken out of the comments (the CHANGELOG keeps it); the exit
  style's comment shortened; "GroundLevel isn't available yet" gone.
- The INI's technical settings (31) moved to a new `[Advanced]` section at its end (the
  user's pick of the table in docs/INI_SETTINGS.md, section 6); `[Transitions]` and
  `[PlainCamera]` are gone with it, and the shot library's switch is `bDirector` there (it was
  `[Director] bEnabled`). 20 fine-tuning settings found by testing are out of the INI, still
  read from `[Advanced]` if added (listed with their defaults in the README). No old names
  or sections are read: nothing had been released (the user).

## 0.9.59 - 2026-11-22

### Removed (before the INI review)
- The cut timing trace (0.9.56, detailed log only): it found the zoom at cuts (fixed 0.9.57).
- `bFilmTestStopAfterLine` (0.9.58): with it off, two filmings stopped mid-line were followed by
  clean conversations too, so it told nothing; the overlapping lines are a known issue.
- What each did and found: docs/MEMORY.md.

## 0.9.58 - 2026-11-22

### Known issue (the user's call, 2026-11-22)
- Overlapping lines in a conversation after filming people talking: rare and intermittent
  (the tests: 2 of 7 conversations after a filming, same steps; never without a filming, with
  filming off or with the mod off). Listed in the README; the hunt goes on after 1.0. The
  `bFilmTestStopAfterLine` test proved nothing (with it off, two mid-line stops were clean
  too) and comes out in the next build.

### Test (out before 1.0)
- `bFilmTestStopAfterLine` ([Debug], on in this build): moving while someone in a filmed scene
  talks gives you your controls back at once, but the view stays on the filmed shot until that
  line ends (10 s at most), then the filming stops as usual. The user's tests (0.9.57): a
  filming of Walter and Vasco stopped by moving mid-line left the next conversation, with
  Cora, with lines overlapping (the 0.9.43 bug); left to end on its own, Cora was clean; with
  filming off or the mod off, clean. This tells whether the view snapping back mid-line is the
  trigger. Logged: "you moved while ... talks" and "the line you moved during is over".

## 0.9.57 - 2026-11-22

### Fixed (found with 0.9.56's cut timing trace)
- **The zoom at some cuts.** A cut away from a shot with a lens over 60 degrees
  (MediumPlayer's 62, the two-shots and other wide ones) didn't put the new lens on with the
  camera: the check for a conversation's first frames, when the game still draws through
  your gameplay lens, took any lens over 60 degrees for that. The new shot was drawn through
  the old lens until the game's own lens update, which runs only every other frame: one or
  two frames zoomed (the trace: all three cuts from MediumPlayer; every other cut had its lens
  on the camera on the cut's own frame). The check now leaves out the lens this mod put on.
  This was most likely behind the zooms seen since the lens was first set per shot.
- Checked (the user's 0.9.57 clip, Nebula, DLSS on): 15 of 15 cuts clean, one frame each: no
  zoom (every cut's lens on the camera on its own frame, trace and clip agree, including cuts
  from 62, 65, 72 and 80 degree shots) and no blurred frame before or after a cut.
- Checked again (the Lodge, indoors, starts from first person; Cora, Sarah and Vasco; four
  clips, about 45 cuts): no zoom and no two-step cut anywhere; one mild softening of the old
  shot just before a cut (CloseUp to OverNpcShoulderLow). Starts and exits as before.
- Seen in that run (the user): Cora's conversation had lines overlapping, the bug fixed in
  0.9.43, after a filming of Matteo and Walter that was stopped by moving 2 minutes earlier;
  Sarah's and Vasco's conversations after it were fine. Not yet known whether it's the mod.

## 0.9.56 - 2026-11-22

### Diagnostic (detailed log only; to come out before the RC)
- The cut timing trace: for 120 ms from each cut, "Cut timing +ms [thread] ..." lines for the
  mod's camera updates, the game's lens updates (all three callers: what the game asked, what
  was set, the render camera's lens before and after) and the focus writes. To find why a
  cut's lens lands a frame late and its focus a frame early now and then.

### Found (the user's DLSS-off test, 0.9.55)
- Not DLSS: with the upscaler and RenoDX's neural rendering off, 21 cuts, 12 clean; 4 zooms
  (the new shot's first frame through the old lens: 1.22, 0.83, 1.56 and 1.04 times), 6 with
  the old shot's last frame drawn with the new focus (3 heavy), the worst cuts with both. Frame
  generation wasn't on either (the user). The camera update runs on a different worker thread
  each frame; the game seems to read the camera, its lens and the blur at three different
  moments, and a cut's three writes can straddle them.
- The user: the game's own quest-scene cameras cut between you and the NPC cleanly; how they
  apply the lens and the blur is the next lead.

## 0.9.55 - 2026-11-22

### Changed (the user's pick after the 0.9.54 comparison)
- The focus at a cut stays on the cut's own frame, as through 0.9.50; `bCutFocusBridge` is
  gone. Measured: off, 13 of 15 cuts clean, one blurred frame before a cut; on, its stretched
  sharp zone made those frames blurrier (4 heavy, 4 moderate of 10), and the run's three
  zooms (1.04, 1.26 and the 1.67 the user saw, Wide to Profile) were all on its cuts.
  The four tries and what each showed: docs/MEMORY.md.

### Found
- One cut (CloseHigh to TwoShot) showed two odd frames before the TwoShot: the first from the
  right place but with bodies and the hedges missing (heads floating), the second with the
  hedges back and both people missing. The user plays with DLSS frame generation; these and
  the other one-frame oddities at cuts may be generated frames between two shots. To test:
  the same conversation with frame generation off.

## 0.9.54 - 2026-11-22

### Changed (back, the user's call)
- The focus at a cut goes on with the camera again, on the cut's own frame (0.8.10-0.9.50).
  0.9.52's no-blur frame and 0.9.53's eased blur stopped the blurred frames but made the
  blur's change itself distracting (the user, 0.9.53 test). The lens written with the
  camera (0.9.52) stays: no flicker seen at the cuts since.

### Added (a comparison test, replacing `bCutFocusEase`)
- `bCutFocusBridge` ([Debug]; **Focus on both at a cut**, Screen > Testing). Off: the new
  shot's focus on the cut's frame, as through 0.9.50 (now and then drawn on the old shot's
  last frame). On: on the cut's frame the sharp zone reaches from the nearer person to the
  farther, the background blurred as usual, so either shot drawn on that frame has its person
  sharp; the new shot's own focus from the next frame. With the detailed log: "Focus at the
  cut: ...". Not yet known: how the game's sharp-zone depth spreads round the focus.

### How we got here (CCC)
- CCC never refocuses between shots: its depth of field (off by default) is one fixed vanilla
  blur, Skyrim's unused VATS one at a set strength, for the whole conversation. Here the focus
  follows each shot (0.5.0), because Starfield's conversation blur, unfocused, left people
  soft on longer shots.

## 0.9.53 - 2026-11-22

### Added (a comparison test, the user's ask)
- `bCutFocusEase` ([Debug]; **Ease the blur back after a cut**, Screen > Testing): after a
  cut's sharp frame (0.9.52), 0 brings the new shot's blur back on the next frame, 1 eases it
  back over a quarter of a second (a quick focus pull). Off by default; one stays for 1.0.

### Changed
- The passer-by in the 0.9.51 clip is fine (the user: people walking close past the camera
  add life to a scene); off the TODO.

## 0.9.52 - 2026-11-22

### Fixed (the 0.9.51 clips: 31 cuts, 20 clean)
- **A blurred frame at a cut, from either side.** 0.9.51 moved the focus a frame later; the
  blurred frame before the cut was gone, but 7 cuts now had a blurred frame just after it,
  the new shot drawn with the old focus (3 heavy: into a master, a two-shot and a medium
  shot from close ones). So the game reads the focus when it draws, and a write lands on
  the frame before the camera's or on the camera's own, whichever comes first. Now the blur
  is off on a cut's frame (right for both shots) and the new focus goes on a frame later, when
  only the new shot can be drawn. Not for cuts where the old focus does for both (within a
  quarter, or both shots past 8 m). The main thread no longer writes the focus once the
  camera does (its writes land on their own time too).
- **The zoom just after a cut** (4 of the 31: the new shot's first frame through the old lens,
  1.6 and 1.1 times about the centre, measured). 0.9.51's call to the game's set-FOV didn't
  stop it. The lens is now also written on the render camera itself with the camera's
  position, scaled to the game's own maths for a lens, learned from a steady shot (with the
  detailed log: "Lens: the game draws ... degrees ... times the plain maths").

### Found, not changed
- In clip 1 a passer-by walked right past the camera, their head and hair filling the
  screen for a moment (the bystander check looks at people standing still near the
  conversation, not people walking through); listed in TODO.

## 0.9.51 - 2026-11-22

### Fixed (the messy cuts in the 0.9.50 clips)
- **The blurred flash before a cut.** The focus on the person in shot was written on the
  cut's own frame (0.8.10), and reached the screen a frame before the camera: the last frame
  of the old shot was drawn with the new shot's focus. From a distant, master or two-shot into
  a close-up, that's the far shot fully out of focus for a frame; milder into far shots. The focus now goes on a frame after the camera, from the
  camera thread and the main thread alike.
- **The "teleport" (a zoom-pop) just after a cut.** A shot's lens went on through the game's
  lens update, which came a frame after the camera; the main thread could also put the old
  lens back for a frame more (two-step cuts). Now the lens goes on with the camera, through
  the game's own set-FOV, as the exit cut has done since 0.9.17, and the camera thread alone
  keeps it while the mod films your conversation.
- A conversation starts with no blur until its first shot's focus is due (the last
  conversation's distance could be read for a frame).

## 0.9.50 - 2026-11-22

### Removed (before the INI review, the user's call)
- The measuring code of the optimization pass: the cost lines (per conversation and per
  minute), the HUD's and the main-thread task's parts.
- Finished diagnostics: the exit frame comparisons and logs (first and third person), the
  start frame logs and the dialogue camera's return log, the menu layout dump and search
  (`bDumpMenus`, gone from the INI; also the 4 ms hitch at the first conversation), your
  line's record, the raw scene action dumps, the per-frame task's tick count.
- The tally of your lines (`CinematicConversationCameraSF_YourLines.tsv`): its data, 2,913
  options and 676 picks, is kept in docs/research/your-lines-tally.tsv and summed up in
  docs/EMOTIONS.md, for the post-1.0 feature reading your lines' wording.
- Everything removed, what it measured and what it found: docs/MEMORY.md.

### Changed
- Kept with the detailed log only (for bug reports): each line's emotion, the first-person
  exit's skipped entry step. Logged always: the INI read again after a change on disk.

## 0.9.49 - 2026-11-22

### Changed (back)
- The stand-in bars through the exit are gone (the user: clean exits over bars kept to the
  cut; post-1.0 Mulling). They were this mod's addition in 0.9.47 and left a flicker: one frame
  without bars as the game closed its dialogue menu, then the stand-ins, then the cut (every
  exit in the 0.9.47-0.9.48 clips; before them the bars simply went with the menu). The bars
  go with the dialogue menu again, as up to 0.9.46: 0.4 s before the first-person cut, about
  1.5 s before the third-person one. With them went the menu-event listener and the 0.9.48
  delay of a tick (which also left the bars a frame over the first-person cut). The
  third-person cut on the hand-back frame (0.9.48) stays, as does the turn of your view when
  no cut came first. What was tried, and what's left to try: docs/MEMORY.md.

## 0.9.48 - 2026-11-22

### Changed
- Third-person exits cut to your view on the frame the game takes over (the user's pick after
  comparing the 0.9.47 trial `iExitStyle=3`: cleaner transitions, a clean exit). The last shot
  holds, with its bars, until then; your character's pose change and the view sharpening as
  the game's conversation effect goes now happen in the cut instead of on a still camera.
  First-person exits keep the early cut (`fExitCutSeconds`). `iExitStyle=3` is gone again
  (read as 2).

### Fixed
- The bars through the exit, to the frame (the 0.9.47 clips, measured frame by frame): at
  every exit one frame went without bars as the game closed its dialogue menu (the stand-in
  bars came on the main task's next tick), and in one style-3 exit of two the bars went a
  frame before the cut. The game's own menu event now puts them up the moment the menu closes
  (on the main thread; elsewhere the next tick does), and they go a tick after the cut, not on
  the tick that sees it (the camera runs on worker threads, so that tick sometimes came before
  the cut was drawn): at worst a frame after the cut, never before it.
- From the review of 0.9.44-0.9.48 for regressions (the user's request):
  - The menu event asks the game's UI nothing: which menus are open and the HUD messages
    menu itself are kept from the main task's last tick (asking the UI from inside its own
    menu event could lock the game if it holds its menu list meanwhile). The menu kept in hand
    is never freed, so quitting mid-conversation can't release it after the UI has gone.
  - A style-2 third-person exit whose hold ended before `fExitCutSeconds` (no cut) drew your
    view un-turned at the hand-back, the 20-degree swing of before 0.9.46: the view is turned
    on that frame whenever no cut came first.
  - The dialogue menu coming back while the stand-in bars are up takes their height at once
    (no slide in again).

### Found
- `iExitStyle=3` (0.9.47 clip, two third-person exits): the cut came on the game's hand-back
  frame, sharp at once (no soft first frame, the game's own state), and nothing moved after
  it: your character's pose change and the sharpening went into the cut.
- One exit's hold (style 2, Sarah's hair close in frame) shimmered sharp and soft every other
  frame once the dialogue menu closed: the game's hair on a still camera; not the mod's.
- Two cuts within a conversation (of 6 in the style-2 clip; none of 12 in the other clips)
  showed the old shot a frame with the new shot's focus (sharp background) before the camera
  moved. The focus and the camera are written together at a cut (0.8.10); which the game picks
  up first varies with its threads. Not from these builds and not from the exit style.

## 0.9.47 - 2026-11-22

### Added (on trial)
- `iExitStyle=3`: as 2 from first person; from third person the last shot holds and the cut
  to your view comes on the frame the game takes over. The 0.9.46 clip (the user: a small but
  noticeable jolt of your character left): the camera now stays put at the hand-back (the
  background still to 0.15 px), but your character's pose changes over three frames as the
  game ends the conversation (arm and head), with the camera already still since the early
  cut. In vanilla the game's camera moves on that frame and hides it; style 3 puts our cut
  there. To compare with 2 in one session (the INI is read again within half a second).

### Fixed
- Exits: the letterbox no longer vanishes 0.4 s before the cut back to your view. It lives in
  the game's dialogue menu, which the game closes as the last line ends; the 0.9.46 clip had
  the last shot jump to full frame, then the cut 25 frames later (two changes where one should
  be). The bars on the HUD messages menu (filming's) now stand in, at the height they had,
  from the menu closing to the cut (or, without a cut, until the game takes over), and go
  with the cut. Not while a trade or similar menu, or the settings page, is open over the
  conversation.

### Found, not changed (the shot-to-shot review of the 0.9.46 clip)
- Cuts within conversations (8) and starts (2): clean, one frame each. During the exit hold
  your view is softer and a little darker than the game's own, and sharpens on the hand-back
  frame (the game's conversation image effect stays on until then; the mod's "blur off"
  only takes out its depth of field): `iExitStyle=3` hides that in its cut too. The exit
  cut's first frame is soft by design (blur off from frame 2, 0.9.16).

## 0.9.46 - 2026-11-22

### Fixed
- Third-person exits: no more swing round you as the game takes over (the user: a shift and
  turn as the camera settled, your body seeming to jolt as if hit). Before a conversation your
  third-person camera sat orbited off your body's heading (the 0.9.45 log: 20-24 degrees in
  all four exits); the game takes over with its camera straight behind your body, so the cut
  back to your view was followed by a 20-degree, 0.6 m swing round you in one frame (exactly
  that turn about your feet at 1.7 m). Your body never turned or moved (0 in every exit).
  The cut now turns your view round your feet to where the game's camera will be: behind your
  body, looking 1.1 degrees left of its heading as the game's does (measured again at every
  exit, in case another mod sets its own camera).

### Optimization pass: done
- With the detailed log off (0.9.44-0.9.45 runs): the main-thread task 16-27 us a frame
  outside conversations, 50-310 in them (mostly the HUD: the subtitle, bars and options);
  the conversation camera about 20 us and the filming camera 8 on the camera thread. The
  0.9.44 run's three 15 ms reads of the INI were edits on the settings page (none in the
  0.9.45 run, no spike over 5 ms).

## 0.9.45 - 2026-11-21

### Diagnostic
- Third-person exits (the user: a noticeable shift and turn as the camera settles back, and
  sometimes your body moves as if it was hit): the log compares the game's view with the one
  drawn over the second after the hand-back (every frame of the first three, then at 5, 10,
  15, 20, 30, 45 and 60), with how far your body turned and moved meanwhile, and at the
  hand-back how far your body turned during the conversation and how far the view drawn
  (your view from before it) looks off your body's heading. Logged with the detailed log off
  too.
- A line each time the INI is read again after a change on disk, with how long it took: the
  0.9.44 run (detailed log off) had three 15 ms reads a minute apart, the only spikes left.

## 0.9.44 - 2026-11-21

### Changed
- The settings page (the user's request): a setting that only works while another is on is
  indented under it and greyed out with "(off)" while that one is off, its tooltip saying
  which ("Start on its own while sprinting" under "Start on its own", everything of filming
  under "Film them", the letterbox's height, fade and subtitles under "Letterbox bars", your
  expressions' chances under "Your expressions", the close-up chances under "Emotion
  close-ups", the reaction settings under "Reaction shots of you", the focus style under
  "Focus on the person in shot", the large speaker's angles under "Large speaker from").
  "Skill-choice close-up (s)" now comes after its switch. The "Start on its own" heads-up
  only shows while filming is on.
- `bDebugLog` is on the settings page too: **Detailed log**, on the Screen page's Testing
  group (for bug reports).

## 0.9.43 - 2026-11-21

### Fixed
- Conversations after filming people talking no longer have lines overlapping and mouths out
  of step (Tuala's at the MAST, the pre-release item). Filming held the game's camera root
  where it started, every frame (since 0.9.18, against a shake while you walked on); the game
  takes that root as where its camera is, and something in its dialogue timing stayed broken
  until a save was loaded. The 0.9.41-0.9.42 test switches found it: with only that hold left
  out, the same run was clean. The hold is gone; the view still moves as before. Filming
  started with the listen key while you walk may shake a little until you stop (filming that
  starts on its own holds you still).
- The test switches (`bFilmTest*`) are gone again (docs/MEMORY.md).

### Diagnostic
- The main-thread task's other parts are timed too, in the cost lines (the cost log itself,
  the camera mode log, waiting for the camera thread's lock, bystanders, filming people
  talking, the preset key, the INI check, lines and face, focus, the conversation's upkeep,
  your visibility): the 0.9.41 log still had a 16-22 ms worst now and then, with the HUD and
  the scan under 1 ms.

## 0.9.42 - 2026-11-21

### Diagnostic
- The lip-sync hunt, narrowed: 0.9.41's test with the camera left out (`bFilmTestNoCamera=1`)
  was clean, everything else of filming still on (third person and your hiding, the input
  hold, the HUD hiding, the lens, the bars). The camera's two parts get a switch each:
  `bFilmTestNoRootPin=1` leaves out holding the game's camera root where filming started (only
  filming does that; the view still moves). The obstacle checks are out of it: in the 0.9.40
  run they couldn't run yet (no conversation of yours before the filming), and it still broke.

## 0.9.41 - 2026-11-21

### Changed (the optimization pass)
- The INI is read only after it has changed (its write time, checked twice a second outside
  conversations), not in full every 2 s, and no longer at each conversation's start (on the
  camera thread, while the main thread could be reading the settings). Edits, from the
  settings page too, still apply within half a second outside conversations. The 0.9.37-0.9.40
  logs had main-thread stalls of 16-24 ms in every minute, with the HUD and the scans cheap
  and the log flushed every 3 s.
- The HUD's idle work stops once there's nothing on screen: the filming bars, the subtitle
  and the listen prompt, once off screen and not wanted, are left alone. The 0.9.40 log, by
  part: after the first filming, the filming bars cost 50 us a frame, the subtitle 45 us
  and the prompt (once shown) 75-145 us, for good, with nothing on screen.

### Diagnostic
- The lip-sync hunt: `[Debug]` switches that each leave one part of filming people talking
  out (`bFilmTestNoHud`, `bFilmTestNoHold`, `bFilmTestNoLens`, `bFilmTestNoCamera`; with
  third-person play and `bLetterbox=0`, every part can be left out). The 0.9.40 test: the
  Vanguard ceremony and a city-life scene filmed to their ends, no interruption, then
  Tuala's conversation unfilmed: lines overlapping again. Temporary: gone before 1.0.

## 0.9.40 - 2026-11-21

### Fixed
- First-person exits: the NPC no longer shifts sideways as the game takes over (0.9.39's fix
  missed it). In first person your view can sit a few degrees off your body's heading, and the
  game takes over looking straight along your body; the cut went back to your view. The 0.9.39
  log matched the game's view to your body's heading on all five exits, with your view 0-5
  degrees off it (the shift seen) and your body not turned at all. The cut now goes to your
  body's heading.

### Diagnostic
- The HUD's cost is broken down by part in the log's cost lines (the ready check, the HUD
  hiding, the prompt, testing messages, the bars, subtitles, dialogue options): 0.9.39 still
  had 0.15-0.3 ms a frame for it with the testing messages off, and main-thread stalls of
  15-27 ms with the log written every 3 s.

## 0.9.39 - 2026-11-21

### Fixed
- First-person exits: the NPC no longer shifts sideways as the game takes over. The game turns
  you during a conversation and takes over with that heading, while the cut went back to your
  view from before it (0.9.38's log: 9.4, -3.4 and -2.0 degrees across). Your view is now
  turned to where you face as the cut begins.
- The listen prompt (`bListenPrompt`, with `bAutoListen` off) no longer shows as each of your own
  conversations ends. The people you'd just talked to still counted as a conversation nearby
  for a few seconds: they were seen talking throughout, and the last line they'd been overheard
  saying, before yours started, was to someone else. Only lines that start after your own
  conversation count now. (Tested 2026-11-21: no prompt at the end of conversations.)

### Added
- `bAutoListenSprinting` (off by default; on the settings page): filming people talking also
  starts on its own while you sprint past.

### Changed (the optimization pass)
- The log is written to disk every 3 s instead of on every line (warnings and errors still at
  once). The 0.9.38 measurement still had main-thread stalls of 16-29 ms now and then with the
  INI re-read gone; writing each line to disk, in a folder that may be synced, is the likely
  cause.

## 0.9.38 - 2026-11-21

### Fixed
- A conversation of your own starting while filming people talking had put the game in third
  person (filming from first person does) now ends in first person; it came back in third
  (the 0.9.37 test: Tuala at the MAST, a quest scene). And a quest scene's first frame is no
  longer drawn through filming's lens (the log: 50 degrees for the game's 29.6).

### Changed
- Nebula's high angle of you cranes down half as far (26 to 13): the 0.9.37 test, Sarah's
  head ended half under the top bar.

### Diagnostic
- First-person exits: the game's first three frames after the hand-back, against the view
  drawn for it (how far across and up, in degrees), for the small shift to the right seen
  as the game takes over.

### Changed (the optimization pass)
- The INI is re-read only when it has changed (checked twice a second, outside
  conversations), not read in full every 2 s. The 0.9.37 measurement: each full read took up
  to 20 ms on the main thread, a hitch of a frame or more every 2 s, and the read at every
  conversation's start (on the camera thread, while the main thread could be reading the
  settings) made that frame take 16-20 ms too. That read is gone; edits still apply within
  half a second outside conversations.
- The HUD skips its work when there's nothing to do: the letterbox bars, once off screen and
  not wanted, no longer look up their menus every frame (both menus, every frame, were most
  of the HUD's 0.25-0.35 ms a frame outside conversations), and the subtitle is left alone
  until there's a reason to move it.
- (Correction, 2026-11-21: the two items above were written up but their code never reached
  the build; the INI was still read in full every 2 s and the HUD idle work still ran,
  through 0.9.40. Both are in 0.9.41.)

## 0.9.37 - 2026-11-21

The optimization pass (the third of the four before 1.0), first step: measuring.

### Diagnostic
- What the mod costs: the time spent in its own code, per part (the main-thread task, the
  HUD, the nearby-people scan, the conversation camera, the filming camera), logged at the
  end of every conversation and once a minute outside them: average and worst per call, and
  in all. Logged whatever bDebugLog says, so it can be measured with the detailed log off.

## 0.9.36 - 2026-11-21

The debugging pass, second part.

### Fixed
- Trading with a merchant mid-conversation: the trade menu stops the game's conversation
  camera, so the conversation ended a quarter of a second in, and when the trade closed the
  game's camera came back for a moment before ending it: about 0.8 s of a new opening shot,
  then the cut back to your view (the 0.9.35 test, twice with Anya Griffon). Now the
  conversation waits while the trade menu is open, as it does for the settings page, and
  ends normally after. The same for the container, crew and ship builder menus a
  conversation can open.

## 0.9.35 - 2026-11-20

The debugging pass (the second of the four before 1.0), first part.

### Fixed
- The build's warnings in the mod's own code: an unused setting reference, signed and
  unsigned comparisons of form types, two names hiding others.
- "The dialogue menu has no Menu_mc.DialogueList_mc" was logged as a false alarm on a
  conversation's first frame, when the menu isn't built yet; now only once it's been missing
  for a second.
- INI comments: Nebula's rhythm (3-5 lines and 40% since 0.9.31) and which presets use room
  shots (now pointing to docs/PRESETS.md; the list was out of date).
- Filming people talking ends at a loading screen (fast travel, a door, a save loaded), so you
  aren't left held still, unable to look around and without your HUD on the other side.
- Settings page: "Longest close-up" between 0 and 4 snaps to 0 (no limit) or 4, instead of
  quietly becoming 4; "Large speaker from" goes up to 3 m, as the INI does.

### Removed
- `fExitHoldSeconds`: the last shot is held until the game takes over anyway, and exit style 2
  cuts at `fExitCutSeconds`, so it had no effect. In docs/MEMORY.md.

## 0.9.34 - 2026-11-20

### Changed
- Filming people talking: the game's whole HUD is hidden while it films (the 0.9.33 test: the
  watch, health and weapon stayed at the bottom of the screen), not only the crosshair and the
  name tags and prompts; the compass and quest markers go too. The subtitles, the letterbox
  and the testing messages live in a menu of their own and stay. All back as it ends.

## 0.9.33 - 2026-11-20

### Fixed
- Filming people talking: looking around (the mouse, or the right stick) is switched off for
  as long as it films (0.9.32 test: with mouse and keyboard, moving the mouse turned the
  camera's angles mid-scene). It's back as filming ends.

### Diagnostic (the 0.9.32 test)
- Holding you still works: you went 0.56 to 0.60 m in the 0.8-0.9 s you held the stick
  after jogging in at 4.6-5.0 m/s, the glide to a stop, then nothing. The slide seen in the
  0.9.31 test was most likely the aim following someone, not you. A sprint measured 7.7 m/s
  (jogging 4.5-5.0), so the 6.5 m/s limit sits between them. The distance went while held
  is no longer logged (0.9.32's diagnostic, answered).

### Added
- A heads-up under "Start on its own" on the settings page while it's on, and in the INI:
  the camera is taken over by these scenes, and you're stopped where you are until you let go
  of the movement controls (the user's request).

### Removed
- Leftovers the redundancy pass missed: conversation and filming state nothing read any more,
  three unused constants, and comments and an INI note still naming removed settings (the INI
  said an old `iCuttingPace` line was still read; it isn't since 0.9.30). In docs/MEMORY.md.

## 0.9.32 - 2026-11-20

### Diagnostic
- Filming that starts on its own: how far you went while held still, logged as you let go
  (the 0.9.31 test: the picture slid while the stick was held, and stopped the moment it was
  let go, which looks like you still moving, not the camera).

## 0.9.31 - 2026-11-20

### Fixed
- Filming people talking: the subtitle now moves into the bottom bar while filming too, with
  "Subtitles in the bottom bar" on (it only followed your own conversations' bars).
- Filming people talking starts on its own while you jog past again. Since 0.8.18 it waited
  until you moved slower than 3 m/s, under Starfield's usual jog, so in practice only once
  you'd slowed down (the 0.9.30 test). Now only sprinting holds it back (6.5 m/s).

### Changed
- Filming that starts on its own stops you where you are (the user's rule): your movement
  input is switched off (an input layer of the mod's own, as SFSE Menu Framework uses) until
  you let go of every movement input (W, A, S, D, the arrow keys, the left stick); moving again
  then gives you your controls back and ends the filming. So walking or jogging into a scene
  no longer ends it early. Remapped movement keys aren't seen; moving then ends it by
  distance, as before (`fFilmStopSeconds`, `fFilmStopDistance`). Started with the listen key,
  nothing changes: walking on `fFilmWalkOnDistance` still ends it.
- Nebula a touch more varied (0.9.30 test, a conversation with Sarah: her low angle came back
  on five of her turns): what a cutting pace of +0.5 gave, now its own. A fresh angle on 40%
  of turns (was 30%), a new angle every 3-5 lines (was 3-6), held 2.5 s at least (was 2.9).
  The pace setting still goes both ways from here.

- With both ways to start filming off (`bListenPrompt=0`, `bAutoListen=0`), the listen key
  no longer starts it either: nothing does (the user's call). With either on, the key works as
  before.

### Diagnostic
- Your speed is logged when filming starts on its own, and when sprinting held it back, to set
  that limit from real numbers.

## 0.9.30 - 2026-11-19

The redundancy pass (the first of the four before 1.0): code no longer in use and finished
experiments taken out. Nothing should look or behave differently; what was removed, with its
technical detail, is in [docs/MEMORY.md](docs/MEMORY.md).

### Removed
- Test switches, each kept at the value in use: `[Transitions]` `iStartMode`,
  `iStartModeThirdPerson`, `iExitMode`, `iExitSkipFrames`, `iExitModeThirdPerson`, `bExitLens`,
  `bExitCutStaged`; `[Body]` `iBodyRebuildMethodPlayer` (the direct refresh, with the camera
  cycle still its automatic fallback) and `iBodyUpdateFlagsPlayer`; `[Debug]` `bStateHooks`
  and `iCameraTarget`. `iExitStyle` keeps 1 and 2 (0, the last shot to the end, is gone).
- Old setting names no longer read: `iCuttingPace` (now `fCuttingPace`), `fDeepFocusBeyond`
  (now `fWideFocusBeyond`), `bDeepFocusLongShots` (now `sFocusStyle`).
- Settings that were read but did nothing: `fAutoListenAfter`, `iReactionSentencesPlayer`,
  `fReactionSecondsPlayer`, `fReactionMinSecondsPlayer`, and unused fields from 0.3.x (blend
  time, the old field-of-view value and method).
- Experiments fixed at their outcome: the field-of-view experiment, the ways of writing the
  camera, the plain shot style, the late body rebuild, the exit covers.
- Diagnostic only: the memory dumps, the field-of-view and position scans, the game settings
  listing, the exit value search, the head-motion and head-distance measurements, the start
  facts, and four unused helpers.

## 0.9.29 - 2026-11-19

### Fixed
- Filming people talking near you: someone vanishing in the over-the-shoulder shots and
  fading back in after the cut (0.9.29 test: Walter, missing in the shot over his shoulder
  and fading in at the start of the close-up on him). The game's third-person camera hides
  anyone close to it, and the camera stood about 0.85 m behind the near person. It now
  stands at least 1.4 m from them, so their shoulder shows in the corner of the frame as
  intended; the speaker is a touch smaller in frame when the two stand close. Your own
  conversations are unchanged. The log gives the distance for each over-the-shoulder shot.

### Changed
- Filming people talking near you: the crosshair (the dot in the middle of the screen) is
  hidden while it films, as the name tags and prompts already were, and back when it ends.
  The game already hides it in your own conversations. A HUD mod that moves the crosshair
  elsewhere keeps its own.
- "Subtitles in the bottom bar" (`bSubtitlesInBars`) explained on the settings page and in the
  INI: off, the subtitle stays where the game puts it (vanilla positioning, kept for mods that
  place subtitles themselves), and there it can overlap the top edge of the bottom bar.

## 0.9.28 - 2026-11-19

### Changed
- Testing messages: when your face shows your background's expression in a quiet moment, the
  background is named beside it ("You: Suspicious (Bounty Hunter)"), as the log already did.
- "Hide while they speak" (`bHideOptionsWhileSpeaking`) explained on the settings page and in
  the INI: the game hides your options during most lines but brings them back before some
  have ended; on, they stay hidden for the whole line (not during the greeting).

### Fixed
- A cut from a shot to the same shot. Room and shared shots frame you both, so both sides can
  hold the same one as their angle, and the turn passing cut from it to itself (0.9.27 test:
  four distant-to-distant cuts in Constellation; one in 0.9.24). Now the new side takes another
  angle, and not a room shot, so a wide shot is followed by a closer one. Every preset.

### Changed
- Constellation keeps its room share but frames people larger (0.9.27 test: too far, people
  too small, less rhythm): the distant shot 95 to 65, the master 85 to 95, the long shots 65
  to 80. Wide and full-figure shots 56% of its weights; faces unchanged.
- Nebula is calmer by default: what a cutting pace of -1.5 gave it, which the user kept
  choosing (a fresh angle on 30% of the turns, was 60%; a new angle every 3 to 6 lines, was 2
  to 4; held 2.9 s at least, was 2). Its space opera is in the variety of angles, not in the
  speed of the cuts. The pace setting still goes both ways from here.

## 0.9.27 - 2026-11-19

### Changed
- Camera sway: Constellation 60 to 50 (with its handheld wander it was a touch strong at
  times), Frontier 50 to 45. Constellation, with the wander, is still the most handheld.
- Constellation leans back towards the room it watches from across (the tests measured 18%
  wide indoors, 32% outdoors, against about half designed): the distant shot 75 to 95, the
  master 70 to 85, the long shots 55 to 65, the three-quarters 40 to 25 and the mediums 35 to
  20. Wide and full-figure shots 47% to 55% of its weights; faces unchanged.

### Fixed
- Settings page: the cutting pace couldn't be moved with a controller (only with the mouse).
  Its half steps were made by rounding a decimal slider, and a controller's nudges (0.1) were
  rounded straight back; it's a slider of half steps now. The three finest sliders (the large
  speaker height, the filming stop distance and time) move in 0.05 steps instead of 0.01, so a
  controller crosses them in a few dozen presses, not a couple of hundred.
- The cutting pace is each preset's own (the user): `fCuttingPaceNebula`, `fCuttingPaceStarborn`
  and so on; the settings page shows and sets the pace of the preset in use, named beside it,
  and switching presets picks up that preset's pace. `fCuttingPace` still applies to any preset
  without its own. (The log printed a pace of 0 as "-0.0" at times; fixed.)
- Settings page: when changes apply ("from the next conversation; made during one, from the
  one after") is now at the top of every page, not only at the bottom.

## 0.9.26 - 2026-11-19

### Changed
- Nebula is the default preset (was Standard; the user's favourite after the tests), and its
  pace a little calmer: a fresh angle on 60% of the turns (75%), a new angle every 2 to 4
  lines (2 to 3), each shot held 2 s at least (1.6 s).
- The cutting pace goes in half steps: `fCuttingPace`, -2 to +2 in steps of 0.5 (was
  `iCuttingPace`, whole steps; an `iCuttingPace` line in your `_user.ini` is still read, and
  `fCuttingPace` wins if both are there). On the settings page the slider moves in 0.5 steps.

### Fixed
- Starborn never showed its wide shots, and CCC's Close never showed its distant shot. Every
  shot is left out when its framing wants the camera more than 2.5 times the preset's distance
  cap away, and that check also caught the room shots, which have their own limit: Starborn's
  and Close's cap is 5 m (a 12.5 m line), and their distant shot on a 40-degree lens wants
  16.5 m, Starborn's master 12.7 m. Room shots now skip that check and keep their own (room on
  the conversation's side, both faces in view, enough of the distance they want). The other
  presets' room shots want 14.6 m at most against a 17.5 m line, so nothing changes for them.
  (0.9.24-0.9.25 tests: no Starborn wide shot in 11 fresh angles, where about 2 were due.)

## 0.9.25 - 2026-11-19

### Changed
- Constellation's master shot drifts instead of craning up: the documentary operator is on
  the floor with the people (the user). Its long shots still crane up.
- From the 0.9.24 test (the user):
  - Starborn overshot a little towards *Interstellar*: sway 30 to 15, faces a quarter
    off-centre (half before), and its close-up lenses half of 0.9.23's step wider than the
    original (their close-up 47.5 degrees, yours 42.5, the extreme ones 42.5 and 37.5).
  - The low angle on them (the lowest camera of the three) tilts a sixth of the old tilt, not
    a third; it was still a bit much next to the others.
  - Frontier's extreme close-ups of them are rarer: 25% of its emotion close-ups (4 of 9 in the
    test). `iExtremeCloseUpChanceNpc` is now -1, the preset's own (Frontier 25, the others 40).
  - With a large speaker, the plain over-the-shoulder shot of you swings round to 45 degrees
    (38 still showed part of Vasco when you stood close); `fLargeSpeakerPlainAnglePlayer`, in
    the INI and on the settings page.

### Removed
- The temporary NebulaTest preset: the low angles' new move is judged (the user: "they all
  looked great"), so it's done its job.

### Fixed
- The last one-frame flashes (3 in the 0.9.24 test, down from 22): an emotion close-up in the
  middle of their turn, then a reaction on the next frame. It now waits for the line's
  reaction to be decided too.
- Settings page: the longest close-up can be set to 0 (no limit); the slider showed 0 as "the
  preset's own".

### Documentation
- README: New in CCC SF now comes before Changed from CCC; the Presets section rewritten for
  where the CCC SF presets stand after tonight's tuning (each one's sway, lenses, extreme
  share, close-up limit and new shots); the settings overview lists the new settings.

## 0.9.24 - 2026-11-18

### Changed
- More of the shot library in three of CCC SF's presets, at low weights, each in its film's
  style (the user's call; anything that doesn't work out in testing comes out again):
  - Frontier: singles with a shoulder edging in (*Firefly*, *The Expanse*), the western's
    heroic low angle on them and on you (with the push in and little tilt tested on Nebula),
    and the wide over-the-shoulders. All wider than a close-up: faces 22% to 19% of its
    weights.
  - Starborn: *Interstellar*'s cockpit singles with a shoulder in the foreground, a close
    profile of them, and now and then a medium of each of you; all held still.
  - Constellation: singles with a shoulder edging in (the observer shooting past people),
    the profile of you both and their medium profile, drifting. Its medium profile can also
    open a conversation from first person.
- Constellation's camera sway 70 to 60 (with its handheld wander as before).

## 0.9.23 - 2026-11-18

From the review of the 2026-11-18 run (0.9.21, the Lodge, about ten minutes per CCC SF
preset with Muria, Sam, Barrett and Andreja). Nothing changes in CCC's presets except the
two fixes.

### Changed
- One close-up of them holds 12 to 18 s at most in CCC SF's presets (Frontier and Nebula 12,
  Constellation 14, Starborn 18), then a three-quarter or medium of them at the next line's
  start; runs of emotional lines held one for 20 to 28 s (Andreja's four Nervous lines: 28 s).
  `fCloseUpLimitSecondsNpc`, in the INI and on the settings page; CCC's presets none.
- A room or shared shot (the two of you, the room, the overhead of you) stays your angle for
  2 of your turns at most, in every preset (Constellation held its distant shot for 5).
- Reactions are never the overhead shot of you, in every preset (it reads as someone
  watching you).
- Frontier: you listening is a medium or three-quarter; close-ups of you are kept for your
  expressions (`bListeningCloseUpsPlayer`, in the INI and on the settings page).
- Starborn, less Kubrick and more *Interstellar*: a light touch of handheld (sway 30), faces
  half off-centre, regular close-ups held still with the push-ins kept for the emotional
  lines, close-up lenses a step wider (50, 45, 45, 40), and the distant and master shots
  more often (25 and 20), so a wide shot for scale comes every few minutes.
- Constellation: no extreme close-ups of you (a documentary doesn't go that close).
- Nebula is a space opera now, not a trailer (the variety of angles is the point; its pacing
  stays). Its overhead shots a little rarer (15 to 12). Its low angles push in with a third
  of their old tilt, and the face never sinks past the middle of the frame (the tilt alone
  sank it under the subtitles).

### Fixed
- One-frame flashes of a shot: two decisions on the same frame (a new angle or an emotion
  close-up, then a reaction or a second cut) cut twice; 22 in the run, in every preset. A cut
  at a line's start now waits for the line's reaction to be decided.
- The close-up of you for a tagged option came a turn late, at your next reply, for only as
  long as you took to pick ([Commerce]: a flick to your face and back). It now comes as you
  pick, over the start of their answer, for `fChoiceCloseUpSecondsPlayer` (2 s), then their
  shot or the persuasion beat.

### Added
- A temporary eighth preset for testing, `NebulaTest`: Nebula with its low angles as half of
  the angles picked, to judge their new move. It comes out before 1.0.

## 0.9.22 - 2026-11-18

### Fixed
- Large speakers (Vasco) still hid you in Starborn (0.9.21 test): none of Starborn's shots of
  you is wide enough with a large speaker, so the mod fell back on its plain over-the-shoulder
  shot, which the 0.9.19 rule didn't cover; his back filled the frame. With a large speaker
  that shot now swings round you to the same angle as the rest (38 degrees off the line), at
  the same distance and with the preset's lens.

### Added
- Settings for large speakers, in the INI (`[Camera]`) and on the F1 page (Camera, Staging):
  `fLargeSpeakerHeightNpc` (1.70 m head above feet; Vasco 1.75, people 1.58-1.63 in the logs;
  0 = off) and `fLargeSpeakerAnglePlayer` (38 degrees). The log names each speaker it applies
  to ("Large speaker: head ... m above their feet").

## 0.9.21 - 2026-11-18

### Changed
- The CCC SF preset tweaks of 0.9.19 are taken back out: they were proposals waiting on a long
  conversation through the presets as they were, and the decision on each is the user's.
  Back as in 0.9.18: reactions may use the overhead shot of you; Constellation's
  over-the-shoulder weight 30; `iExtremeCloseUpChancePlayer=40` for every preset (no
  "preset's own" value); Frontier's two-shot of you held as long as any angle. The large
  speaker fix of 0.9.19 (Vasco) and the 0.9.20 scene logging stay.

## 0.9.20 - 2026-11-18

### Added
- Diagnostic: the "Scene check" line lists what a quest scene is made of, its actions counted
  by type (Dialogue, PlayerDialogue, Camera, Timeline, Move and so on), and says whether it
  "directs a camera" (has a Camera or Timeline action) or has "no camera actions". The first
  Camera and Timeline action of each scene is also dumped once. Nothing changes on screen:
  every quest scene you take part in still goes to the game's camera. The aim is to learn,
  from testing and the beta, which quest scenes really direct their own shots, so that the
  ones that are just people talking can keep this mod's camera after 1.0. The action types
  are read from the game's own class names, with no new addresses and every read guarded.

### Documentation
- README: the features are now in three lists, From CCC, Changed from CCC (what CCC also
  does, done differently, with Starfield's reasons in place) and New in CCC SF, checked
  against CCC 1.5.0's own changelog and usage notes. CCC already had obstacle checks (Keep
  Subject Visible, off by default), avoiding bystanders, filming other people's
  conversations, a depth-of-field blur, player expressions and a settings page, so those
  moved to Changed. The separate "because of Starfield" list is merged in; its "no in-game
  settings menu" line was out of date since 0.9.12.

## 0.9.19 - 2026-11-18

### Changed
- CCC SF preset tweaks (proposed after the 0.8.16-0.8.17 tests; to confirm with a long
  conversation):
  - Reactions never use the overhead shot of you (Constellation used it for three reactions
    in a row: it read as surveillance).
  - Constellation: over-the-shoulder weight 30 to 15 (a documentary rarely shoots over a
    shoulder; more of the distant, master and long shots).
  - The extreme share of the close-ups of you is the preset's own by default
    (`iExtremeCloseUpChancePlayer=-1`): Constellation none, Nebula 50%, the others 40% as
    before. A confrontational choice is still always an extreme close-up.
  - Frontier: a two-shot as your angle lasts one turn, then a single of you (it held one for
    four of your turns).

### Fixed
- Large speakers (Vasco) hid you in the narrower shots of you (0.9.18 test: in close-ups at
  22-27 degrees off the line his back filled the picture; at 34 you were half behind him; at
  40 clear). With a speaker whose head is 1.70 m or more above their feet (people are about
  1.62, Vasco 1.75), the shots of you keep to 38 degrees or wider. The obstacle check leaves
  the person nearest the camera out on purpose, since their shoulder belongs in an
  over-the-shoulder shot.

## 0.9.18 - 2026-11-17

### Changed
- Filming people talking: no more shaking when it starts while you're still walking (0.9.17
  test, on a controller: the shots shook until you let go of the stick). The shots are placed
  against the game's camera, which follows you, and the mod's placement reaches the screen a
  frame behind it; the game camera is now held where it was when filming started. It still
  starts while you walk (not while you sprint), as before.
- Once you've stopped, moving ends filming more readily: after standing still
  `fFilmStopSeconds` (0.3 s; 0.4 before), moving `fFilmStopDistance` (0.25 m; 0.5 before)
  ends it. Walking up is unchanged: walking on `fFilmWalkOnDistance` (3 m) before you've
  stopped. All three are now in the INI (`[Overhear]`) and on the settings page.
- Seated in third person, a conversation now ends on its last shot and the game's camera
  takes over in one cut. Your view from before the conversation isn't where the game's
  seated camera comes back to, so the cut back to it was followed a moment later by a second
  cut (0.9.17 test at the bar).

## 0.9.17 - 2026-11-17

### Fixed
- Exit style 2: your view came up zoomed in (about 1.8 times) and widened at the hand-back
  (0.9.16 clip, every first-person exit): the cut set its lens with the mod's own frustum
  maths, which doesn't match the game's idea of the same angle. It now goes through the
  game's own lens function, still with the camera move.
- After a vendor or bartender menu (0.9.16 test, 6 s buying a drink): your body wasn't taken
  as built when the conversation picked up again, so you stayed hidden and the exit took the
  older path, showing your back for a few frames. A conversation picking up within a minute
  (5 s until now) with your last built height counts as built; that older exit path also
  puts your lens back now.

## 0.9.16 - 2026-11-17

### Fixed
- Exit style 2, after the bar's drink menu (0.9.15 clip): your view came up through the
  narrow conversation lens for about half a second, then jumped at the hand-back. A
  conversation picked up again after a menu never sees your gameplay lens at its start; it
  now uses the last one seen.
- The exit lens fix only ran on the first exit of a session (a counter that nothing counts
  down outside conversations).
- Exit style 2's cut: the 0.9.15 clips still had a frame of the last shot with you hidden
  (first person) or through the wrong lens (third person). The order in which the camera
  move, the lens, the blur and your body's hiding reach the screen varies by a frame from
  exit to exit, so the cut no longer depends on it: the lens is set with the camera (and the
  game's lens update kept off it meanwhile); from first person the camera lands 12 cm ahead
  of your eyes and eases back over a quarter of a second, with your body hidden from the
  third frame, so neither order shows; the blur goes off from the second frame.

### Known
- `bDisableFurnitureCamera:Camera=1` (the game's own switch) doesn't stop the sitting
  camera's swing, from first or third person (0.9.15 test).

## 0.9.15 - 2026-11-17

### Changed
- `iExitStyle=2` (the cut back to your view) stays the default: the user's pick after the
  0.9.14 comparison.
- Style 1's closing shot now comes as the dialogue menu closes: the game closes it (and the
  letterbox, which lives in it) a couple of frames before it lets go of the speaker, and the
  0.9.14 clip showed the last shot two frames without its bars before the cut.

### Fixed
- Dialogue options gone for the rest of the session (0.9.14 test, after opening the F1
  menu mid-conversation; the cutting pace was a coincidence): the options list belongs to
  the game's dialogue menu, which outlives a conversation, and if the game hid it while it was
  faded out, it came back invisible every time after. Its visibility is now written each time
  the game shows it.
- Opening the F1 settings menu mid-conversation ended the conversation (the game's camera
  stops while the menu is open) and started a new one on the same dialogue when it closed.
  The conversation now waits.
- Exit style 2: the frame before the cut to your view showed the last shot with you hidden
  (first person) or through your wide lens (third person), and no blur; in third person the
  first frame of your view came through a narrower lens. During the hold the camera move
  lands a frame after those changes, so the cut is now staged: the camera on its first frame,
  your body, the blur and your lens on the next (`bExitCutStaged`).
- 0.9.13's exit lens fix never ran: the game's lens update isn't called outside
  conversations, so the gameplay lens was never remembered (0.9.14 clips: the zoomed frame
  at every exit, both styles). It now uses the game's lens at the conversation's first frame
  and the camera seen during the conversation, and logs if it can't.

## 0.9.14 - 2026-11-17

### Added
- `iExitStyle` in `[Transitions]`, to compare two ways of ending a conversation. The 0.9.12
  clips showed that the frame or two with you gone from the last shot can't be drawn over
  (the game builds it before the mod's hook runs), so these make sure nothing visible
  changes when the game takes the camera back:
  - 2 (default): 0.4 s after the last line (`fExitCutSeconds`), a cut back to your own
    view, first or third person, through your gameplay lens and without blur; from first
    person your body is hidden first, as in the game's own conversation view.
  - 1: as the last line ends, a closing shot of the NPC alone (medium, close-up or
    three-quarter, the first that fits), if the shot on screen shows you.
  - 0: the last shot to the end, as before.

## 0.9.13 - 2026-11-17

### Fixed
- First-person exits (0.9.12 clip, all five exits): one frame of your view zoomed in, after
  the clean held-back frame. The camera was right; the lens wasn't: the game eases its lens
  back from the conversation's 30 degrees to your 94 over a few frames, and its first
  in-between frame (about 60 degrees) came straight after ours. Now your gameplay lens goes
  on the camera at the exit and is kept through the game's ease (a quarter of a second at
  most). Third-person exits get the same: the 0.9.12 bar clips show their in-between frame
  is the same zoomed view. `bExitLens` in `[Transitions]` turns it off, to compare.

### Known
- First-person exits: one or two frames of the last shot with you gone from it, just before
  your view.

## 0.9.12 - 2026-11-17

### Added
- An in-game settings page through
  [SFSE Menu Framework](https://www.nexusmods.com/starfield/mods/18201) (optional): Mod Menu
  (F1) → Cinematic Conversation Camera SF, five pages (Camera, Close-ups and reactions, You,
  Screen, Filming people talking) with about 45 of the INI's settings, each with its
  explanation on hover. It mirrors the INI: a setting shows your `_user.ini`'s value if you've
  changed it, the main INI's otherwise; a change is written to
  `CinematicConversationCameraSF_user.ini` and applies from the next conversation; Reset
  removes your change. Choosing a preset there replaces one picked with the preset key.
  Without the framework installed nothing changes. Test switches, transitions, per-shot
  tweaks and the plain camera stay INI-only.
- The tally of your lines: while the detailed log is on, every dialogue and persuasion option
  the game offers you, and which one you pick, is appended to
  `CinematicConversationCameraSF_YourLines.tsv` next to the log, across sessions. It's the
  material for reading your character's lines by their wording after 1.0.

### Credits
- The SFSE Menu Framework SDK (MIT) and the signing headers it uses from CLibUtilsQTR (MIT),
  both by QTR-Modding, vendored in `extern/`.

## 0.9.11 - 2026-11-17

### Fixed
- First-person exits (0.9.10 clips, all four exits): the frame after the held-back one still
  showed the game's dialogue camera from your eyes through the conversation lens (it looked
  like your view zoomed in). The game's dialogue camera update ends the conversation on the
  held-back frame, and ending it put the render camera back to the game's dialogue camera.
  Your view now stays drawn until the first-person camera's next update, and the lens goes off
  before it's drawn.
- `bEnabled=0` now turns the whole mod off, not only the conversation shots: filming people
  talking, the nearby-people scans, the dialogue options and letterbox, your face, the blur
  settings and the lens all stand down. The camera hooks stay installed (they can't be removed
  while the game runs) but pass straight through. The INI is still read every couple of
  seconds, so `bEnabled=1` brings it back. For a strict vanilla comparison, disable the mod in
  the mod manager.

### Known
- First-person exits: one frame of the last shot with you already gone from it, just before
  your view (your body is swapped back to the first-person one at the switch).
- Third-person exits: one frame of the third-person camera close behind you as it eases out.

## 0.9.10 - 2026-11-17

### Fixed
- Someone sitting next to you could fill a shot with their head (0.9.8 test, the Viewport bar:
  the person on the next stool's hair covered the opening shot of the bartender and the
  shot of you over her shoulder). The check for bystanders in front of the camera measured
  to the middle of their body, low and out of the picture for someone seated; it now also
  measures to their head, and allows for hair above it.
- A conversation that picks up again a moment after the last one (buying a drink paused the
  game's conversation camera for a second, so the mod ended it and started a new one) began
  with your full body already built, seated at 1.55 m: it was never taken as built, so you
  were hidden and the game was asked to rebuild it twice more. Your last built height,
  within the last few seconds, counts as built now.

### Changed
- Third-person exits: `iExitModeThirdPerson` back to 1. 2 (0.9.8) didn't remove the one frame
  of the third-person camera easing in (it came from behind instead of the side); vanilla
  has the same frame.

### Confirmed (0.9.8 clips)
- First-person exits: the 0.9.8 clips with seated people show the same two things 0.9.9
  fixes (a frame of the last shot without you, then a frame through the conversation lens).
- Seated people from first and third person: framing fine; sitting down and standing up
  untouched by the mod. Shots of you through a plant at the bar: plants have no collision,
  so the obstacle check can't see them (post-1.0).

## 0.9.9 - 2026-11-17

### Fixed
- First-person exits: 0.9.8's held-back update worked, with no steep-up frame in any of the
  seven exits checked (one held-back frame or two, same result). Two
  smaller things showed instead:
  - The held-back frame kept the last shot, and you were already gone from it (by then your
    body is the first-person one, which is why the next frame is clean): one or two frames of
    the shot without you. Your own view is drawn on that frame now, through your own lens.
  - The first first-person frame came through the conversation lens (your view, zoomed in),
    when the game's update ran before the mod's main-thread tick turned the lens off. The
    lens goes off the moment the conversation ends now.
- `iExitSkipFrames=2` held back only one frame: the conversation ends during that frame. One
  is all it needs, so it stays at 1.

## 0.9.8 - 2026-11-17

### Found
- The first-person exit's one frame looking steeply up (sky) comes from building your full
  body for the conversation. 0.9.7 tests, three conversations each, every exit checked frame
  by frame: vanilla (`bEnabled=0`) clean; the mod with the director off, and with defaults,
  the frame in every exit; with `bFixFirstPersonBodyPlayer=0`, or with the older camera-cycle
  body method (`iBodyRebuildMethodPlayer=0`), no frame (and the log shows the camera's pitch
  never set to -68.13). The camera-cycle method isn't the answer: its starts flash (two frames
  of a close torso, then your own view for 0.3 s).
- Third-person exits: vanilla has the same thing as the mod, one in-between frame (from your
  eyes) before the third-person camera settles; `iExitModeThirdPerson` 0 and 1 look the same.
- The 0.9.7 cover (`iExitMode=4`) didn't reach the screen.

### Changed
- Cutting: the named cutting styles (`sCuttingStyle`: Cinematic, Odyssey, SpaceWestern,
  Dogfight, SpaceOpera) are gone. Since each preset got its own rhythm they mostly repeated
  that range, and a style replaced the preset's rhythm (Starborn with SpaceOpera cut on every
  turn). `iCuttingPace` (-2 to +2, 0 = the preset as designed) nudges the preset's own rhythm
  instead: each step makes the lines between new angles and the shortest hold about a
  quarter longer or shorter (x1.3 and x1.6 calmer, x0.75 and x0.55 livelier) and a fresh
  angle on the turn 20 points less or more likely. An `sCuttingStyle` left in an INI is
  ignored.
- `iMinLines`, `iMaxLines` and `fShortestHold` now override the preset's rhythm too (with the
  old `sCuttingStyle=Preset` default they were ignored); 0 or left out = the preset's own.

### Changed (to confirm)
- First-person exits, `iExitMode=5` (new default): the first-person camera's first update after
  the conversation is held back (`iExitSkipFrames`, default 1) with the last shot kept on screen,
  so the update that's drawn comes once the game has your first-person body back.
- Third-person exits, `iExitModeThirdPerson=2` (new default): the third-person camera eases out
  from where the camera stood on its first update (vanilla: your eyes; here the game's
  conversation camera, so one frame close and from the side). The camera is put on your
  third-person view just before that update, so the ease starts where it ends.

## 0.9.7 - 2026-11-17

### Fixed (to confirm)
- First-person exits: the 0.9.6 clip still showed one frame looking steeply up (sky, or a
  tilted view of the ground and a wall) in all three exits, though the log showed the pitch
  and the camera root put back right after the game's update: that frame's picture is
  already taken during the update. `iExitMode=4` (the new default) also draws your view over
  that frame, now on a camera root that's right (the 0.9.0 cover was drawn against a root
  aimed at the floor). 3 is 0.9.6's behaviour.

### Diagnostic
- Where the exit's -68.13 comes from: it isn't saved-up mouse movement (the 0.9.6 tests used
  only a controller), and only the two pitch fields change in the first-person camera during
  that update. Just before it, copies are kept of the player camera, the dialogue,
  first-person and third-person camera states, the player, and what the first-person state
  points at; right after it, they're searched for the new value (degrees, radians, sine,
  cosine) and every value that changed is listed ("Exit search" in the log).

### Known
- Third-person exits (0.9.6 clip): one frame of the last shot without its background blur,
  then the third-person camera swings in from a closer side angle over two or three frames.

## 0.9.6 - 2026-11-16

### Fixed
- Starts, first frame: the 0.9.5 guess at where the game's dialogue camera would be was
  still off by up to 1.9 m when you stood close or far (0.9.5 distance test, ten starts).
  The test showed the rule the game uses: the camera sits on the line from the NPC's head
  through you, 2.02 m from the NPC's head (a little less when something is in the way) and
  0.07 m above it, whatever the distance between you; it looks 2.75 degrees below the line
  to the head. The first frame is now placed with that rule, from first and third person,
  for seated, tall and raised NPCs too, and the values keep adjusting from each start.

### Diagnostic
- The first-person exit in the 0.9.5 log: right after the camera's first update its pitch
  fields were put back (from -68.13 to where you were looking) and the camera root reported
  your view on that frame. To confirm in a clip.

## 0.9.5 - 2026-11-16

### Fixed
- First-person exits, still one frame of floor in 0.9.4: skipping the camera's entry step
  didn't change it (0.9.4 log: its first update still set +C8/+CC to -68.13, a constant in
  every exit, and pointed the camera root at the floor). With `iExitMode=3`, right after
  that update those two values are put back, and the camera root is put on your view, in
  case the frame is taken from them after the update. The log says which held. (The 0.9.4
  clip shows that frame looking up at the sky, not at the floor: -68 is upwards.)
- Starts, first frame (0.9.4 clip): the first frame of the opening shot was tilted and
  shifted a little, then settled on the next frame. While the game still reports its
  gameplay camera, the shot is placed against where its dialogue camera will be, and that
  guess aimed straight at the NPC's head and 0.18 m to one side. The game's camera looks
  about 5 degrees lower and sits almost straight behind your eyes (0.9.4 log), so the guess
  now does too, and keeps learning both from each start.

### Known
- Third person, first conversation after loading a save: the game took three frames instead
  of one to switch to its dialogue camera, and two frames showed the opening shot from 1.6 m
  further back (from behind you). Later conversations switched in one frame.

## 0.9.4 - 2026-11-16

### Fixed
- The frame of floor on the way back to first person, found in the 0.9.3 log: the
  first-person camera comes back with a "just entered" flag, and on that first update it
  sets its pitch to -68 degrees whatever you were looking at (-13 to -20 in the test), then
  puts it back the next frame. That first frame is the one drawn. Now the flag is cleared
  just before that update, so it's a normal one, looking where you were looking
  (`iExitMode=3`, the new default; 1 brings the old behaviour back).
- Third-person starts (0.9.3 clip): your view held for a few frames, then two frames of it
  moved to where the game's camera would be, then the first shot. They now open straight
  on the first shot, as first-person starts do (`iStartModeThirdPerson=1`, was 2).
- Third-person exits (0.9.3 clip): one frame of the game's own conversation camera between
  the last shot and your view. That frame is drawn from your third-person view now, through
  the gameplay lens (`iExitModeThirdPerson=1`; 0 = the game's).
- Starts (0.9.3 clips): the first frame or two were still drawn through the gameplay lens
  (94 degrees), so the first shot appeared wide and then zoomed to its own lens. Those
  frames are drawn through the shot's lens now.
- From first person, you popped in at the edge of the opening shot of a seated person a few
  frames in (0.9.3 clip), when your body was ready. You're shown from the first cut after
  your body is built now, so you never appear mid-shot.
- Starts in Close and Starborn: the opening three-quarter (an angle those presets don't
  list) was replaced by a close-up a frame later, a second cut at the start. It stays while
  it can be placed.

## 0.9.3 - 2026-11-16

### Fixed
- Sitting on a bar stool in first person, you were hidden the whole conversation (0.9.2
  test, the Viewport): your full body's head sat at 1.55 m, short of standing height, so it
  was never taken as built (then the older method failed the same way). A body that starts
  seated is taken as built once its head has risen from the first-person body's.

### Diagnostic
- Exits to first person: 0.9.2's log ruled out your pitch (unchanged, and the camera aimed
  right just before the first first-person update, at the floor just after it). The
  first-person camera's own state is dumped before and after that update and the next, to
  find what it starts from.

## 0.9.2 - 2026-11-16

### Changed
- Conversations started in first person open straight on the preset's first shot of the
  NPC, as from third person, through its own lens: a three-quarter or medium shot of them
  with no one else in the picture (close-ups are kept for emphasis; in Close and Starborn,
  which have neither, their close-up or a three-quarter through the lens of their nearest
  angle, half the time each; with merchants and seated people, held for their turn), so
  your body is built off screen and never pops in. The openings, preset by preset:
  docs/PRESETS.md.
- A conversation's first shot, from third person too, is never a close-up: they're kept for
  emphasis, and opening on one every time wore them out. Close and Starborn excepted (their
  close-ups are their main shot); a strong first line's emotion close-up still can.
  0.9.1's opening from just in front of your eyes started cleanly but looked like the
  game's own camera and set the wrong tone (0.9.1 test).

### Fixed
- First-person exits, a third try at the frame of floor: the picture of the first
  first-person frame is taken during the game's own update, before anything this mod does
  after it, so covering it afterwards (0.8.18, 0.9.0) never reached the screen. Now, just
  before that update, your pitch is put back to where you were looking when the
  conversation started, if it changed meanwhile.

### Diagnostic
- The exit logs your angles and the camera's aim just before that first update.

## 0.9.1 - 2026-11-16

### Changed
- Conversations started in first person open on the NPC from just in front of your eyes,
  as the game's own camera does, with you out of frame while your body is built; the
  director's shots start with their next line or your turn. 0.8.15 to 0.9.0 held your own
  view for that moment instead, and each clip showed a new flash from it (a zoom, frames
  from inside your head, a frame of the game's camera slipping through, you popping in);
  0.8.14 opened over your shoulder, with you popping in after a tenth of a second.

- Every shot is drawn through a lens from the preset, also shots the preset doesn't list:
  the lens of the preset's nearest angle of the same person. The opening shot from your
  eyes takes the preset's close-up lens (Standard 50, Close 50, Room 72, Frontier 45,
  Starborn 45, Constellation 45, Nebula 50); the plain shots for seated people and merchants
  their over-the-shoulder or three-quarter lens; a close-up in a preset without one, its
  nearest single. Until now these fell back to one general lens per preset (45 or 37.5).
- Close-ups over the plain shots (seated people, merchants) are drawn through the lens
  they're framed for (they used the general lens, a slightly different size).

### Added
- The previous game session's log is kept as `CinematicConversationCameraSF.prev.log`
  (same folder) when the game starts, so a long test's log isn't lost to the next start.

### Fixed
- First-person exits: the cover tried in 0.8.18 and 0.9.0 never reached the screen (two
  frames of ceiling, then the game's floor frame unchanged); taken out. The one frame of
  floor remains, as before 0.8.18, to be solved another way.

## 0.9.0 - 2026-11-16

The start of 0.9.x, polish. Testing messages and the detailed log stay on until the last
step before 1.0.

### Fixed
- First-person exits: 0.8.18's cover of the game's first frame came out as two frames of
  ceiling (it was placed against the camera as it was on that frame, still pointing away,
  and the game then moved it). It's now placed against your view, held for the first
  updates, at your own field of view.
- First-person starts: 0.8.18 showed you a frame too late (the first shot without you,
  then your head popped in). You're shown one frame sooner.

### Removed
- Old INI names no longer read: the keys from before the Npc/Player renames (bReactionShots,
  bEmotionCloseUps, bPersuasionBeat, iExtremeCloseUpChance, bPlainShotsSeated, bFacePlayer's
  bPlayerFace, and the rest), bShowEmotions/bShowShots, and `sPreset=Stardust` (Frontier).
- `iCloseUpDistance`: every angle has its own lens, so close-ups stand at CCC's distance.
- `iCloseUpChancePlayer`, unused since 0.8.5 (the Mild/Medium/Strong chances replaced it).

### Changed
- `fConversationLens`: -1 (the default) is each angle's own lens, as with every "the
  preset's own" setting; 0 no longer means the game's lens (30 does); one lens for
  everything is 20-70.
- `bFadeOptions` merged into `fOptionsFade`: 0 = the options appear and go at once (the
  default, as before), a number = they fade over that many seconds.
- `sBeatTags`: Negotiation added ("Negotiate" didn't match the game's [Negotiation]).
- The INI's comments brought up to date and tidied: each preset's cutting listed, the
  persuasion hold's two behaviours, starting filming not while running, the controller
  button, the real names of your close-up chances; development notes removed.

## 0.8.18 - 2026-11-16

### Fixed
- Filming people talking started as you ran past a conversation, flipped to third person
  and back 0.4 s later (0.8.17 test, Neon Security). It no longer starts on its own while
  you're running (over 3 m/s); walking up to one still starts it.
- Flashes at conversation starts from first person (0.8.17 clips, seated and standing):
  your own view, shown while your body is rebuilt (about 0.1 s), was drawn through the
  conversation lens, a sudden zoom; it keeps your own field of view now. Then two frames
  from inside your head (your hat's brim, your eyebrows) as the first shot came in: you're
  shown only once that shot has been on screen for a few frames.
- A one-frame flash on the way back to first person: the game's first frame looked at the
  floor (in all ten exits). That frame is covered with your view.
- After a won persuasion, the whole reply stayed on the NPC and you were barely seen (23 s
  on Sieghart). With reaction shots on (CCC SF's presets), the hold covers their answer and
  the verdict; the next line is you, reacting. CCC's presets hold the whole reply, as CCC.

## 0.8.17 - 2026-11-16

### Fixed
- Your angle never changed on its own. The player doesn't speak, so your side's line count
  (the preset's "new angle every N lines") never moved: in the 0.8.16 test Constellation
  held a two-shot of you for nine turns in a row, and in Standard, Close, Room and Starborn
  (no fresh-angle roll) your angle only changed when it stopped fitting. Each of your turns
  now counts as one of your lines.
- Filming people talking ended on momentum: walking up to a conversation, it started and
  your last steps ended it 1.3 s later (0.8.16 test, TerraBrew). Moving ends it only once
  you've come to a stop (0.4 s still); walking on 3 m without stopping still ends it.
- The option you chose was misread: every option shows at full opacity, so the first one
  was taken for whatever you picked (the 0.8.16 test logged 'Let's trade gear.' for a
  Sarah line, and [Pay ...] for [Persuade]), so tagged choices never got their close-up.
  Now the option with its highlight showing, or the list's selected index; and the log says
  how it was told apart.
- The persuasion panel's options weren't read at all. Each is a skill check, so each now
  gets the close-up of you too (an extreme one for the confrontational skills).
- With a seated NPC (plain shots), the close-up for your choice waited for an NPC line, and
  you don't speak, so it never showed.
- Seated NPCs: your body hid most of them. The plain shot looked down over your shoulder
  onto their lower head; it now stands wider and lower behind you
  (`fOverSeatedBehind`/`Side`/`Height`), so the line to them passes clear of you.
- The persuasion beat landed on the wrong line: the NPC answers each round and gives the
  verdict ("You got yourself a deal.") while the panel is still open, but the beat waited
  for the panel to close and held the line after ("Well then. Goodbye."). Every round's
  answer now gets it, the verdict included.
- Seated people never got the persuasion beat in Standard and Room (no emotion close-ups
  there); they get it now, in every preset, over their plain shot.
- Camera sway in CCC's presets (Standard, Close, Room): CCC's camera doesn't sway, so
  they're on a tripod now (were 40).

### Changed
- The sway test is over (0.8.16): the presets keep their sway, and Constellation now has a
  slow handheld wander of its own, at half the strength tested (`iCameraSway=0` turns it
  off). Frontier and Nebula have none.

### Removed
- The sway key (`iSwayKey`, L) and the sway part of the testing message's preset label.

## 0.8.16 - 2026-11-16

### Added
- A close-up of you when you choose a tagged option, for the line you say (CCC SF's
  presets; `bChoiceCloseUpsPlayer`, -1 the preset's own): every tag, skills, backgrounds and
  traits ([Persuade], [Chef], [Ronin], [Wanted]...), but not [Pay ...]. An extreme close-up
  for the confrontational skills (Intimidate, Threaten, Manipulate, Deceive, Lie, Attack);
  the others extreme `iExtremeCloseUpChancePlayer` (40%) of the time, as the NPCs' are. With
  your face already showing the tag's expression, and a persuasion's answer held on them,
  it plays as a beat: your face makes the play, theirs answers.

## 0.8.15 - 2026-11-16

### Fixed
- Extreme close-ups became someone's usual angle: in Close the camera came back to an
  extreme close-up of you on ten turns in a row (0.8.14 test), and the NPC's emotion
  extreme close-up came back on later turns too. As in CCC, they're only for the lines that
  call for one (the emotion close-ups): never picked as a regular angle, never kept as a
  person's angle.
- Filming people talking started again for a second after a scene had ended (0.8.14 test,
  Amoli and Gerhard: a talking flag flickering), a flip to third person and back. It only
  starts on its own on a line that began after it last stopped.
- Starting a conversation from first person: the first shot began with you missing and you
  appeared a moment later (0.8.14 test, the Ranger at Neon), while your full body was being
  built. Your own view holds until it's ready (about 0.15 s, at most 0.6 s).
- Filming people talking used whatever lens your last conversation ended on. Each shot has
  its own now (Standard's, as CCC: close-up 50, shoulder 55, three-quarter 58, medium 62,
  two-shot and group 72).

### Added
- The sway key (testing, `iSwayKey`, L): cycles how the camera sways, shown in the bottom
  bar: as set (the preset's `iCameraSway`, the share of the people's own head movement the
  camera follows; 0.8.14 test: subtle, as standing people's heads move only 1-3 cm), 150%
  wherever the preset has any, or as set plus a handheld wander (a slow, smooth drift of up
  to 2 cm and 0.35 degrees, from a few sine waves a frame: Constellation full, Frontier and
  Nebula half, Starborn and CCC's presets none). Sway now goes up to 150.

### Changed
- Emotion close-ups and reactions fitted to each CCC SF preset (0.8.14 test: reactions took
  16-36% of the time, emotion close-ups 4-16%, so up to half of Frontier was the extras
  rather than its own angles): reactions after 10 s of their talking in Frontier, 9 s in
  Starborn, 12 s in Constellation, 7 s in Nebula (`fReactionAfterSecondsPlayer=-1`, the
  preset's own); Constellation's emotion close-ups on strong lines only, the documentary
  keeping its distance.
- Filming people talking: closer for two people (close-ups, three-quarters and shoulders
  most, a medium now and then, no two-shot); the two-shot and the group shot stay for three
  or more.

## 0.8.14 - 2026-11-15

### Changed
- CCC SF's presets rebuilt on CCC's lenses, each with a film flavour (Starfield's
  "NASA-punk" influences), its own lenses, movement, cutting and camera sway:
  - **Frontier**, a space western (*Firefly*, *The Expanse*), built on Standard: slightly
    longer lenses (40-80), slow dolly-like drifts on the two-shot, three-quarters and
    mediums, the room now and then, a touch of handheld (sway 50); a new angle every 2-4
    lines and on a quarter of the turns.
  - **Starborn**, Kubrick and *Interstellar*, built on Close: long, compressed lenses
    (35-45), faces centred in the frame, very slow push-ins (12 s) held long, a locked-off
    camera (sway 0), one slow crane up on the master as a reveal; a new angle every 4-7
    lines, held 3.5 s at least.
  - **Constellation**, a hard sci-fi documentary (*The Martian*, *Apollo 13*), built on
    Room: watched from across the room on longer lenses (40-70; Room's go to 95), small
    slow drifts everywhere, the most sway (70), cranes on the master and the long shots,
    faces about a fifth of the time.
  - **Nebula**, a trailer: every angle, every move bigger and quicker, lenses 40-95, a new
    angle every 2-3 lines and on most turns, held 1.6 s at least.
- Camera sway is each preset's own (`iCameraSway=-1`, the new default: 40 in CCC's and
  Nebula); a number still sets one for all.
- Close-ups stand at CCC's distance in every preset (`iCloseUpDistance=-1`), now that every
  preset has its own lenses.
- CCC's lenses are used as CCC wrote them. Measured in two Skyrim clips with CCC on Close
  (Faralda, Tolfdir): close-ups, close-ups of you and extreme close-ups fill 26%, 23% and
  53-65% of the picture's height, as CCC's numbers give when Skyrim's lens is measured the
  way Starfield's is; 0.8.12's reading of it as measured across a 4:3 frame gave 20%, 17%
  and 44%. CCC 1.5.0's shot table (fills, angles, heights), read from its DLL, is the same
  as this port's.

### Removed
- The lens key (L) and its three ways of setting the lens: CCC's lenses as written is the
  answer.

## 0.8.13 - 2026-11-15

### Changed
- CCC's presets keep CCC's vision: emotion close-ups and reaction shots are each preset's
  own now (`bEmotionCloseUpsNpc` and `bReactionShotsPlayer`, -1, the new default).
  - Standard and Room: no emotion close-ups (CCC pushes in for emotion only with the
    extreme close-up, an angle only Close has; Room is about the room) and no reactions
    (off as CCC ships).
  - Close: emotion close-ups on strong lines only, the nearest thing to CCC's "full
    intensity"; no reactions.
  - CCC SF's presets: every tier, and reactions, with close-ups of you in them: what sets
    them apart.
  - 0 or 1 still sets either for every preset.

### Fixed
- The lens key changed the lens only at the next cut (0.8.12 test): the shot on screen kept
  the lens its cut had worked out. It changes at once now.

## 0.8.12 - 2026-11-15

### Changed
- CCC's presets made faithful to CCC 1.5.0, from its own exported presets:
  - **A lens for every angle**, as CCC has had since 1.3 (Close 35-50 degrees, Standard
    50-72, Room 45-95, in CCC's numbers). They were all 45 degrees here, which put the
    camera further back than CCC, most of all on close-ups. Each angle stands where CCC's
    numbers put it and is drawn as Skyrim shows it: Skyrim seems to measure its lens across
    a 4:3 frame, so a 16:9 screen shows more (CCC's 50 is about 64 of Starfield's).
  - **Each preset's own cutting rhythm**: Standard a new angle every 2-3 lines, Close every
    3-6, Room every 3-5, never just because the turn passed; held 2.4 s at least, Room 3 s
    (`sCuttingStyle=Preset`, the new default; a named style still sets one rhythm for all).
  - Room's medium, three-quarter and overhead shots are still, as in CCC 1.5.0.
  - Close-ups in CCC's presets at CCC's distance (`iCloseUpDistance=-1`, the preset's own:
    100% for CCC's, 115% for CCC SF's narrower lens).
- Room shots are measured at their own lens.

### Added
- The lens key (testing, `iLensKey`, L): during a conversation, CCC's lenses as Skyrim
  shows them (the default), as CCC wrote them, or one lens for every angle (as before),
  with the lens in use in the bottom bar.

## 0.8.11 - 2026-11-15

### Fixed
- Filming people talking started 1.5 s late on Helena's scene at TerraBrew (0.8.10 test): a
  customer's "Excuse me." nearby, not part of any scene, was picked as the nearest
  conversation until it ended. Conversation scenes come first now; anything else only for
  the listen key.

## 0.8.10 - 2026-11-15

### Fixed
- After a long line's reaction (on you for its first 4 s, then back to them), the time on
  them wasn't started again, so the next lines were all reactions too (0.8.9 test, Sarah's
  monologue: four in a row, the count reaching 36 s). A reaction on screen during a line
  now starts the time again, however long it stayed.
- Filming people talking: two scenes side by side were filmed as one, cutting between them
  (0.8.9 test, TerraBrew: Helena and her manager, and the customers in line). One scene at a
  time now: the nearest first, only its lines while it lasts, then on to the next one
  nearby if there is one, without leaving the filming camera.
- Conversation starts: the first frames of the first shot could be blurred (the game's own
  focus, 1.5 m, on a shot 4 m away), a flash. The focus was set from the main thread a
  frame or two after the camera moved; it's now set on the frame of the cut, for every cut,
  and there's no blur at all before the first shot.

### Added
- Filming people talking, the rest of 0.8:
  - The letterbox bars (`bLetterbox`), as in your own conversations.
  - Group shots: in a scene with three or more people, about one new line in four is a
    shot of the whole group, from outside it across from the speaker; only when clear.
  - A scene that leads into the game's next scene with the same people is followed into it,
    without leaving the camera (as CCC does).
  - A scene that's only repeating itself (two lines in a row already filmed) is handed
    back, and isn't started again on its own for two minutes (as CCC does).
  - No listen prompt while `bAutoListen` is on (it starts on its own anyway).

## 0.8.9 - 2026-11-15

### Fixed
- Filming people talking started 6 s late at Sarah and Nyssa (0.8.7 test): Nyssa's opening
  line came out "to you" (she was facing you), and filming waited for Sarah's answer. A
  scene's opening line now goes to the nearest other person in the same scene.

### Changed
- An extreme close-up holds its own line only (0.8.7 test: 4 of them ran 13 s on average,
  through the emotion lines after them, 18% of the time on screen). Their next line goes to
  a regular close-up if it's an emotion line too, otherwise back to their usual angle (or a
  new one); never two extreme close-ups in a row. Coming back from a reaction to an extreme
  close-up is a regular one.
- Emotion close-ups' tier chances lowered: mild 25% (was 30), medium 40% (was 50), strong
  still 100%; the same for close-ups of you in reactions (`iCloseUpChance...Npc`,
  `iCloseUpChance...Player`).

## 0.8.8 - 2026-11-15

### Fixed
- Filming people talking from first person: your first-person armour still showed, T-posed
  (0.8.6 test). It can't be found and hidden: "Player1stPersonBody" turned out to be a label
  the game passes around, not a name in the scene (0.8.7 log: not found). Now the game's
  view goes to third person while it films, with you hidden, and back to first person when
  it stops.

### Changed
- Two emotion lines in a row held one close-up for 17 s, the due reaction waiting at the
  second (0.8.7 test, Sarah). An emotion line that carries on the close-up already on
  screen (a regular one, on at least 2 s, no cut since) now takes a due reaction like any
  line. One that brings a new close-up still waits, so it isn't cut away from at once.
- A line guessed at 7.4 s lasted 6.0, and its split went you, then her for 2 s, then you
  for your turn (0.8.7 test). Lines up to a second over `fReactionLineMaxSecondsPlayer`
  (7 to 8 s) are shown on you whole; only longer ones are split.

## 0.8.7 - 2026-11-15

### Fixed
- A crash when filming people talking started (0.8.6 test, Nyssa Marcano and a UC Vanguard
  captain): looking for the first-person body beside your 3D read a list of the scene graph
  that isn't laid out the way the library describes. It's now looked for with the game's own
  search, and what's hidden is held so it can't be freed under the camera.
- A reaction still due when your turn came was taken on the first line of their reply, a cut
  to you 0.6 s after the cut to them (0.8.6 test, after choosing an option with Sarah). Your
  turn drops it now, as it starts the time on them again.

### Changed
- Long lines take reactions too (0.8.6 test: Sarah's lines run 8 to 12 s, none was short
  enough, and she talked for 49 s without one). A line over `fReactionLineMaxSecondsPlayer`
  (7 s) is shown on you for its first `fReactionLongLineSecondsPlayer` (4 s, new), then
  back to them for the rest. 0 waits for a line that suits, as before.
- The Stardust preset is now Frontier (too close to Starborn). `sPreset=Stardust` still
  works and gives Frontier.

## 0.8.6 - 2026-11-15

### Fixed
- Filming people talking with nothing clear used the first shot as it was, the camera
  beyond the room's walls (0.8.5 test, Neon Security's headquarters). Now the shot with the
  most room, pulled in short of the wall.
- Filming people talking from first person: your body was hidden but your armour stayed,
  T-posed (0.8.5 test). The game's first-person body ("Player1stPersonBody") is looked for
  and hidden too; the log says where it was found.
- Pressing A or clicking to skip a companion's greeting chose your first option instead
  (Sam and Vasco: "Let's trade gear", into their inventory; 0.8.4 and 0.8.5 tests). The
  game shows the options during greetings, where A chooses rather than skips, and this mod
  hid them while the NPC spoke, so they were chosen unseen. They stay visible during the
  NPC's first line now, and are hidden while they speak after that, as before.

### Changed
- Emotion close-ups come in three tiers, each with its own chance: mild expressions
  (Amused, Confident, Thinking, Grateful, Question, Impressed, Tired, Concentrating,
  Relieved) 30%, medium (Concerned, Serious, Smirk, Happy, Confused, Puzzled, Apologetic,
  Awed) 50%, strong (the rest but Neutral and Friendly) 100%: `sCloseUpEmotionsMildNpc`,
  `iCloseUpChanceMildNpc`, and the same for Medium and Strong. About one NPC line in five
  (was one in four at 50% for all), with the strong ones always. `sCloseUpEmotionsNpc` and
  `iCloseUpChanceNpc` are gone.
- Reaction shots are inserts: the angle of you is picked for the reaction only (your own
  angle for your turns stays as it was, room shots included), and afterwards the camera
  goes back to exactly the speaker's shot from before, with no new-angle roll and their
  line count going on. Before, each reaction counted as two turns, so it changed the
  speaker's angle more often than the preset's rhythm and narrowed your turns' shots.
- Close-ups of you in a reaction go by your expression's tier, the same lists as theirs:
  mild 30%, medium 50%, strong 100% (`iCloseUpChanceMildPlayer`, `...MediumPlayer`,
  `...StrongPlayer`; was 50% for any expression, `iCloseUpChancePlayer`, now gone). About a
  quarter of reactions are close-ups of you (was about 40%), and every strong reaction is.
- Filming people talking starts on its own as soon as a conversation scene begins near
  you, even while you're walking (scenes are few, and moving ends it; 0.8.5 test): no more
  standing still first (`fAutoListenAfter` is gone). It gives you a second to stop walking
  before moving counts, and a conversation you've ended by moving or with the listen key
  doesn't start again on its own for two minutes.
- Filming people talking is only offered, or started on its own, for conversations: the
  game's scenes of people talking (in the 0.8.5 log every real NPC conversation was one,
  and every bark or one-liner, like "Need something?", wasn't). The listen key still films
  anyone talking.
- A reaction could cut away from an emotion close-up 0.4 s after it began, when the 7 s on
  them were reached just then (0.8.5 log, Sam Coe, three times). Within the close-up's
  minimum time on screen (`fShortestHold`, 2 s) the reaction waits for the next line.

## 0.8.5 - 2026-11-15

### Fixed
- Filming people talking from first person showed your character in a T-pose (the game
  doesn't animate your full body in first person; 0.8.4 test, The Rock in Akila). You're
  hidden while it films from first person.
- Your head went missing in a conversation started from first person (0.8.4 test, Vasco
  at the Lodge): the body rebuild was asked for again at 0.7 s and the first answer came at
  0.83 s. It asks again at 1.2 s now.
- The log's "loaded" line named 0.8.1 whatever the version.

### Changed
- Filming people talking ends sooner after the conversation: 0.5 s after the last word
  (`fFilmEndSecondsNpc`), up to 4 s while they're still in one of the game's scenes
  (`fFilmEndSecondsSceneNpc`; was 3 and 10).
- Reaction shots reworked to break the monotony of long stretches (0.8.4 test: some were
  too abrupt): after `fReactionAfterSecondsPlayer` (7 s) of their talking without a cut to
  you, the next line of theirs that suits it (2.5 to 7 s, not an emotion close-up) is shown
  on you from its start to its end, and again after every 7 s in a long speech. Short
  exchanges never get one. The cuts fall at the edges of their lines, never in the middle.
  The sentence count, `iReactionSentencesPlayer`, `fReactionSecondsPlayer` and
  `fReactionMinSecondsPlayer` are gone. Their emotion close-ups are part of the stretch:
  when the 7 s are reached during a regular one, the camera cuts to you there, to the end
  of the line (no cut back to them mid-line any more); never during an extreme one.
  `iReactionChancePlayer` is now the chance once due (100). New:
  `fReactionLineMinSecondsPlayer`, `fReactionLineMaxSecondsPlayer`.
- The persuasion beat holds their whole reply, as CCC (was the first line only): a
  close-up, pushing in; in a long answer (over 10 s), once the push-in is done, one cut to
  a medium or three-quarter shot of them. Never a cut away from them or a reaction shot,
  and the answer doesn't count towards the next reaction.
- The listen prompt's fonts are assigned rather than learned: the words in the bold main
  font, light (the game's own prompt words are dark on a panel this prompt doesn't have),
  the button as its icon.
- README: Close "feels like" a film-noir conversation.

## 0.8.4 - 2026-11-15

### Fixed
- The listen prompt took the look of the interaction prompt's button icon
  ('$Controller_Buttons_inverted', size 42: 0.8.3 log) instead of its words. The icon's
  font is skipped now.
- Lines that follow each other without a pause (one dialogue record with several
  responses) were taken as one long line: 74 characters "lasted" 38 s in the 0.8.3 log.
  A new subtitle while they keep talking now starts a new line, for emotion close-ups,
  reactions, your face and the camera's line count alike. This also makes the
  line-length log (and the 14-characters-a-second guess it checks) meaningful: single
  lines ran 13-17 characters a second.

### Changed
- Filming people talking near you (0.8.3 test):
  - It stops soon after the conversation ends: 3 s after the last word, or up to 10 s
    while they're still playing their part in one of the game's scenes (which pause
    between lines). Before, always 10 s, and the camera lingered until you moved.
  - It starts sooner on its own: standing still 1 s (`fAutoListenAfter`, was 2), sitting
    1 s (was 1.5).
  - The prompt shows the B button as its icon, in the game's button font ("Hold [B]").

## 0.8.3 - 2026-11-15

### Added
- The preset key (`[Debug] iPresetKey`, U): switches to the next preset on the fly, also
  mid-conversation (the next lines of each take angles from the new one, and the room is
  measured again for its room shot limits), until the game is restarted. U because
  Starfield doesn't use it (P is its Skills key).
- With the testing messages, the preset in use stays on screen in the bottom bar's
  right-hand side while a conversation is filmed ("PRESET: STARDUST").
- Nebula preset, CCC SF's own: every shot in the library. The everyday ones come up most
  often, and CCC's rarer angles now and then: low, high and dirty singles, profiles and the
  shoulder variants. No preset used those before. The ground-level shot waits for room
  measurement.

### Changed
- Each preset has its own lens (`fConversationLens=-1`, the new default): 45 degrees for
  CCC's presets, 37.5 for CCC SF's (Stardust, Starborn, Constellation, Nebula), a little
  closer to Starfield's own look. 0 is still the game's 30, and 20-70 your own for every
  preset.
- Emotion close-ups: every expression but Neutral and Friendly is now on the list
  (`sCloseUpEmotions`, was 29 of them), with a chance (`iCloseUpChance`, 50%). A listed
  line that rolls no is an ordinary line. About a quarter of NPC lines get an emotion
  close-up (estimate from the expression tally in docs/EMOTIONS.md).
- Reaction shots: the time on you counts from the cut to you, not from the moment it was
  decided (up to 0.8 s earlier: 0.8.2 test, some felt too short), and is never less than
  `fReactionMinSeconds` (2 s). A reaction on a short line followed at once by another
  carries into it until then.
- Reaction shots work as in CCC (0.8.2 review): lines of 40 characters or more count their
  sentences; when the count is reached, one roll; a yes owes a reaction to the next line
  that can take it, a no starts the count again (was a roll on every line once due, so
  reactions could come back to back). Short lines no longer count. A quick skip takes the
  roll back too.
- Close-ups of you are a kind of reaction now, not an extra roll on every line: when a
  reaction is taken and your face shows an expression, `iCloseUpChancePlayer` (50%) of the
  time it's a close-up of you (`iExtremeCloseUpChancePlayer`, 40%, extreme). Before, any
  long line with an expression on your face had its own 50% chance, so the camera was on
  you very often.
- A reaction never interrupts an extreme close-up (the strongest beat); whether an emotion
  close-up is extreme is now decided with the line.
- A listening reaction uses your regular shots only, never the room, the two-shot or the
  profile of you both (as CCC).
- The log notes each line's real length against the 14-characters-a-second guess used to
  plan reactions ("Line length: ...").
- INI keys renamed to say whose shot they're about (old names still read):
  `bReactionShots` -> `bReactionShotsPlayer`, `iReactionEvery` ->
  `iReactionSentencesPlayer`, `iReactionChance` -> `iReactionChancePlayer`,
  `fReactionSeconds` -> `fReactionSecondsPlayer`, `fReactionMinSeconds` ->
  `fReactionMinSecondsPlayer`, `[Face] iPlayerCloseUpChance` -> `[Director]
  iCloseUpChancePlayer`, new `iExtremeCloseUpChancePlayer`, `bEmotionCloseUps` ->
  `bEmotionCloseUpsNpc`, `sCloseUpEmotions` -> `sCloseUpEmotionsNpc`, `iCloseUpChance` ->
  `iCloseUpChanceNpc`, `iExtremeCloseUpChance` -> `iExtremeCloseUpChanceNpc`,
  `bPersuasionBeat` -> `bPersuasionBeatNpc`, `iSignatureChance` -> `iSignatureChancePlayer`,
  `iQuestionChance` -> `iQuestionChancePlayer`, `bPlayerFace` -> `bFacePlayer`,
  `bAvoidBystanders` -> `bAvoidBystandersNpc`, `fBystanderStill` -> `fBystanderStillNpc`,
  `bPlainShotsSeated` -> `bPlainShotsSeatedNpc`, `bPlainShotsMerchants` ->
  `bPlainShotsMerchantsNpc`, `fMinSpeakerHeight` -> `fMinSpeakerHeightNpc`, `fVendorSide`
  -> `fVendorSideNpc`, `fVendorHeight` -> `fVendorHeightNpc`, `bFixFirstPersonBody` ->
  `bFixFirstPersonBodyPlayer`, `iBodyRebuildMethod` -> `iBodyRebuildMethodPlayer`,
  `iBodyUpdateFlags` -> `iBodyUpdateFlagsPlayer`, `bFilmConversations` ->
  `bFilmConversationsNpc`. The rule, in the INI's header: Npc = about the people you talk to
  or overhear, Player = about you, no ending = the camera or both of you.
- docs/REACTIONS.md: the steps, rules and chances for emotion close-ups, reaction shots and
  close-ups of you.
- A reaction shot can interrupt an emotion close-up: their close-up first, for its minimum
  time on screen (`fShortestHold`, 2 s), then you for `fReactionSeconds`, then back to the
  same close-up, pushing in. Only on lines long enough for all three (judged from the
  subtitle's length). Before, an emotion close-up line was never a reaction.
- Stardust is closer to Standard: exactly Standard's shots, weights and moves, with a few of
  Room's wider shots added (about a fifth, was 40%).
- Filming people talking near you:
  - Each line's shot is chosen from the ones the obstacle check finds clear (walls, things,
    and people standing still, now watched while filming too), so wide shots stay where
    they fit and are skipped where something's in the way (0.8.2 test: blocked at
    TerraBrew, good with Keeper Aquilus's group).
  - The camera stays put for the line, and its aim follows the person in shot through a
    small dead band, slowly: no more horizontal jitter (0.8.2 test).
  - The game's name tags and interaction prompt are hidden while it films (0.8.2 test:
    the camera's aim brought them up).
  - The listen prompt takes the look and size of the game's own interaction prompt (learned
    the first time one is on screen), more readable than the notifications' look.
  - Standing still 2 seconds starts it (`fAutoListenAfter`, was 4).
  - The controller's listen button is B, held (`iListenButton=8192`, was the right stick
    click).

## 0.8.2 - 2026-11-15

### Added
- Two ways to start filming people talking near you (`[Overhear]`):
  - `bListenPrompt`: a prompt on screen below the crosshair, "Listen in on nearby
    conversation. Press N" ("Hold R3" when you last used a controller), in the
    notifications' look. It replaces the testing message that offered it.
  - `bAutoListen`: starts on its own when you sit down near a conversation (after 1.5 s)
    or stand still near one for `fAutoListenAfter` seconds (4). Stopping it with the
    listen key keeps it from starting on its own for a minute.
  Neither shows or starts while a menu is open (pause, inventory, map, scanner...).
- Never in combat: no prompt, no starting, and filming stops. Actor's IsInCombat is
  virtual slot 0x16B in 1.16.244, not CommonLibSF's 0x16C (found from the game's Papyrus
  natives Actor.IsInCombat and Actor.GetCombatState); called by its real slot.

### Changed
- docs/PRESETS.md: every preset's shots, shares, moves and distances in tables.
- The plan: filming people talking is finished in 0.8.x; 0.9.x is polish and optimization;
  1.0 the public beta.

### Fixed
- Room shots under a distance limit came out alike (0.8.1 test: Wide and Distant both 12 m
  back on the line outdoors, all three 9 m on the line indoors). The limit now applies to
  the furthest of them (Master) and the others keep their distance from it in proportion
  (outdoors in CCC SF's presets about 12, 10 and 8 m for Master, Distant and Wide), and
  each keeps further away from where the others stand.

## 0.8.1 - 2026-11-15

### Fixed
- The conversation lens: the game hands its set-FOV the 29.6 degrees it draws, not the
  setting's 30 (0.8.0 log), so the shots were framed for 44.4 degrees instead of the 45 set.
- Room shots outdoors all stood at the same place (0.8.0 test: Wide, Master and Distant at
  158 degrees off the line). Where the room allows, each now prefers CCC's own angle for
  it (Wide and Distant on the line, Master 24 degrees off it) and keeps away from where
  the others stand.
- The far room shots focused on things near the camera, leaving the people soft (0.8.0
  test, the Lodge). Shots more than 8 m from the person in shot now have no blur at all.
- Filming people talking stopped during long lines and pauses (Keeper Aquilus's group) and
  had to be started again: the 10 seconds of quiet that end it are now counted from when
  they last spoke, not from when the line began.

### Changed
- CCC's presets (Standard, Close, Room) stay as faithful to CCC as possible: Room's room
  shots go as far back as CCC's framing wants (12 to 18 m at 45 degrees). CCC SF's presets
  keep them to 9 m indoors and 12 m outdoors (0.8.0 test: beautiful, but some felt too far
  away). `fRoomShotMaxIndoors` and `fRoomShotMaxOutdoors` set your own limit (-1 = the
  preset's own, 0 = as far as CCC's framing wants).
- The README records the hybrid approach (a wider lens and somewhat longer distances) as
  the one for now.
- Reaction shots stay on you about 2.5 seconds, then cut back to the speaker, instead of
  the whole line (`fReactionSeconds`, 0 = the whole line). A line with less than about
  1.5 seconds left stays on you to the end.

### Added
- Two presets, CCC SF's own, each mirroring one of CCC's with a Starfield flavour:
  Starborn (Close, plus the reverse shoulder and drifting three-quarter singles) and
  Constellation (Room, plus close-ups and shoulders now and then). With Stardust (Standard
  with Room's wider shots), each of CCC's presets now has a CCC SF counterpart.
- A presets table in the README.
- A controller button for filming people talking near you: hold the right stick click for
  half a second (`[Overhear]` `iListenButton`, `fListenHold`), so a tap still does what the
  game uses it for. Read straight from XInput (Xbox controllers, and most others through
  Steam Input). The prompt names both ("press N or hold the right stick to listen").

## 0.8.0 - 2026-11-14

### Fixed
- Your face for a choice and for the NPC's answer: your character isn't voiced, so the
  answer starts as you choose, and the choice (noticed a moment later) replaced the answer's
  expression for the whole line (0.7.12 test: Sam Coe's awed answer met a questioning look
  throughout). The choice's expression now shows for 1.5 s, then your answer to the line.
- Reaction shots on a blank face: a line with no expression on it leaves your face to the
  game, so a reaction shot on it showed you neutral (0.7.12 test). Your face now takes a
  listening look for the reaction (your background's expression, or Concentrating), and the
  shot message names your expression ("reaction: you listening, Suspicious", or "no
  expression").

### Changed
- Close-ups of you (`iPlayerCloseUpChance`) 50% of eligible lines instead of 20%: in testing
  they hardly came up, since most expressive lines are the NPC's own emotion close-ups.

### Added
- The conversation lens (`[Camera]` `fConversationLens`, 45 in the INI, 0 = the game's 30):
  Starfield films conversations through a 30 degree lens (`fDialogueCameraFailsafeFPFOV`),
  put in place every frame by the camera's field-of-view update (found in Starfield.exe
  1.16.244 at +BCB990; it hands the value to the player camera's set-FOV function at
  +1E92650, which writes the value 0.3.x tried to change, and the next frame put 30 back).
  The value is now changed on its way into that call, at all three of its callers, while
  this mod films a conversation. Shots are framed for the lens in use.
- Room shots (Room and Stardust presets: Wide, Master, Distant), from CCC: each stands where
  the room measurement found the most space on the conversation's side of the line, with
  both faces in clear view and under the ceiling, as far back as CCC's framing wants or the
  room allows; left out where the room gives less than `iRoomShotMinRoom` (60%) of that.
  The room is measured whenever the director films (before, only for the log).
- Filming people talking near you, first step (`[Overhear]`): when two people within 12 m
  talk to each other, a notification offers it ("Sam Coe and Cora Coe are talking: press N
  to listen"); the listen key (N, `iListenKey`) starts and stops it. The camera films whoever
  is speaking against whoever they're addressing, through the gameplay camera's own update,
  a new shot for each line (over the listener's shoulder, three-quarter, medium, close-up or
  the two of them). Moving, a conversation of your own, or 10 s of silence hand the camera
  back. No letterbox or obstacle check yet; keyboard only.

## 0.7.12 - 2026-11-14

### Added
- Your face for more skill checks: science options ([Astrophysics], [Geology], [Botany],
  [Zoology]...) look thoughtful, technical ones ([Security], [Engineering], [Piloting]...)
  focused, [Leadership] confident, [Stealth] suspicious.
- Questions: an option without a tag that asks a question has a 40% chance of a questioning
  look (`[Face]` `iQuestionChance`), the first use of your line's text.

### Changed
- Your face answers a Confident line with Confident (it was treated as neutral; Confident
  is about one NPC line in ten).

### Fixed
- Close-ups of you (0.7.11 test): the one that came up followed your face from the line
  before (the camera decided before your face was set for the new line), and the camera
  stayed on the three-quarter shot of you it was already on. It now waits for your face (at
  most 0.8 s), and cuts to the close-up even when the camera is already on you.
- Overheard conversations: someone in a scene answers whoever in the same scene spoke last
  (0.7.11 test: Andreja, answering Matteo while facing away, came out as talking "to no one",
  so her lines never showed on screen; the same for Cora Coe answering Sam).

## 0.7.11 - 2026-11-14

### Added
- Your background on your face (`[Face]` `iSignatureChance`, 30%): at moments your face would
  otherwise be neutral (an NPC line without an expression, an option without a skill tag),
  your background's expression shows instead now and then: Space Scoundrel smirks, Gangster
  looks smug, Bounty Hunter suspicious, Diplomat friendly, Professor thoughtful... (all 20
  backgrounds: see the INI or `docs/EMOTIONS.md`). The background is read from your perks with the game's own
  check behind Papyrus' HasPerk (found in 1.16.244, Address Library ID 101224).
- Reaction shots with seated people and merchants too: their conversations keep the plain
  over-the-shoulder shot, and a reaction shows the plain shot of you that's already used
  when you speak (or your emotion close-up). Their lines now count towards reactions.
- Background and trait options: choosing an option tagged with a background ("[Space
  Scoundrel]") or a trait ("[Wanted]", "[Empath]") sets an expression for it, like the
  skill tags do: a background's own expression, and one per trait (see `docs/EMOTIONS.md`).
- `docs/EMOTIONS.md` now has your face's tables (answers to each NPC expression, skill tags,
  backgrounds, background and trait options), kept up to date with every build.
- Your emotion close-ups (`[Face]` `iPlayerCloseUpChance`, 20%): when your face answers an
  NPC's line with an expression, that line may be shown on you as a close-up, pushing in
  (an extreme close-up as often as the NPC's, `iExtremeCloseUpChance`). It counts as a
  reaction shot; the message says "reaction: your expression". Close-ups of you were rare.
- Close-up distance (`[Director]` `iCloseUpDistance`, 115%): close-ups and extreme close-ups
  stand further back than CCC's framing; through Starfield's narrow lens they felt too
  tight.
- Your expression in the testing messages: a line's message ends with your face's answer
  ("Sarah Morgan: Worried (#2) | You: Concerned | Camera: CloseUp (...)"), and choosing a
  skill-tagged option shows "You: Confident ([Persuade])".

### Diagnostic
- Each overheard NPC-to-NPC message is noted in the log when it's sent (0.7.7 test:
  Andreja's answers to Matteo didn't seem to show, only his lines).

## 0.7.10 - 2026-11-14

### Fixed
- Another crash while a save loaded (0.7.9): with the game's actor list empty during
  loading, the mod went through your cell's references instead, whose layout isn't confirmed
  for this game version. That route is gone, and so is reading the other three actor lists
  (only their sizes are logged now). Nearby people come only from the game's high-process
  list, which is confirmed, and nobody is looked up while the game is loading.

### Diagnostic
- 0.7.9's log answered the crowd question in part: in a busy spot, up to 100 people within
  20 m were in the game's low-process list, none of them talking as far as the game says.
  Crowd citizens' chatter may not go through the dialogue system at all.

## 0.7.9 - 2026-11-14

### Fixed
- A crash shortly after loading a save (0.7.8): listening to the game's lower-process actor
  lists asked an actor that wasn't fully set up for its name. Only actors with their full
  process are now asked anything beyond where they stand (the others are still counted in
  the log), and those questions are guarded.

## 0.7.8 - 2026-11-14

### Added
- Your character's face, first step (`[Face]` `bPlayerFace`): the game keeps your face
  neutral, and your lines carry no expression tags. Now, while the NPC speaks, your face
  answers their line's expression, mirrored where it's natural (a smile for a smile) or as a
  listener would (concern for worry, attention for a question); angry lines get a guarded
  look or anger back, picked at random each line;
  an untagged line gives your face back to the game. Choosing an option with a skill tag
  ([Persuade], [Intimidate], [Lie]...) sets an expression for it. Expressions are the game's
  own facial archetypes, set with the function behind Papyrus' ChangeAnimFaceArchetype
  (found by rrrafau for PointCameraAtPlayer; confirmed in 1.16.244 as Address Library ID
  100510). The log notes each change ("Face: you look Concerned (listening to Sarah Morgan:
  Worried)").

### Diagnostic
- Overheard lines: the game's other three actor lists (low, middle-high and middle-low
  process) are listened to as well, and every 10 s the log lists who's within 20 m and
  which list held how many. 0.7.7's test heard named NPCs but none of the crowd citizens
  talking around you, who may not be in the high-process list.

## 0.7.7 - 2026-11-14

### Fixed
- Bystanders and overheard NPC lines: the game's list of active actors that both read was
  always empty on Starfield 1.16.244 (0.7.6 test: "0 actors nearby" all session), so neither
  ever saw anyone; standing-still people weren't avoided after all. CommonLibSF has the list
  8 bytes off for this game version: Starfield.exe 1.16.244 itself reads it at ProcessLists
  +60 (count) and +68 (the handles), which is now read directly. If it ever comes back
  empty, the actors in your cell are used instead.

### Diagnostic
- The reaction count says why a line that could have been a reaction wasn't: an emotion
  close-up line, under 40 characters, or the chance roll.

## 0.7.6 - 2026-11-14

### Added
- Testing messages in the top bar: while a conversation is filmed, `bShowMessages`' line and
  shot messages show as one line of text in the top letterbox bar, staying until the next
  one or `fMessageSeconds` (6) pass. They look like the game's own notifications (same font,
  size, colour and effects, learned from the first message of the session, which also goes
  out as a notification) and are drawn by the same HUD messages menu. The notifications went
  by too fast for a line and its shot together; outside conversations (overheard NPC lines)
  they're still used.

### Changed
- Leaving a conversation in third person: the last shot is held until the game hands back
  its camera, then cuts straight to your third-person view, as in first person. 0.7.5 showed
  the third-person view from before the conversation meanwhile, resting on the NPC's middle
  for about a second.
- Reaction shots: lines are counted as sentences (each ending in a full stop, ! or ?, or a
  pause, "..."), so a reply of three sentences counts three; any line counts, short ones too.
  A line you skip after half a second on screen now counts (0.7.5: lines skipped within
  1.2 s didn't). Only lines of 40 characters or more are shown as reactions themselves. The
  log now notes the running count.

### Fixed
- Room measurement: every place with a roof measured a 1.6 m ceiling (the cast straight up
  reported a hit at once), which also stopped the measurements higher up. The ceiling is now
  read from the point the cast reports back, cross-checked from higher up and against the
  floor, with the raw readings in the log.

### Diagnostic
- NPC-to-NPC conversations: 0.7.5 noticed no lines at all. A line is now also noticed when a
  new dialogue record or subtitle turns up on someone (the game may only mark NPCs as talking
  when they speak to you), and every 10 s the log sums up how many people are nearby, how many
  have a line and who's talking.

## 0.7.5 - 2026-11-14

### Added
- Shot messages: each shot the camera uses is shown on screen by its `[Shots]` name and why
  ("Camera: ThreeQuarterNpc (turn, new angle)", "Camera: ClosePlayer (reaction: you
  listening)", "Camera: plain over-the-shoulder shot (seated)", ", pulled in" for an
  obstacle). A guide to building presets.
- A line's expression and the cut it brings now show as one message ("Sarah Morgan:
  Surprised (#3) | Camera: CloseUp (emotion close-up)", or "| same shot" when the camera
  holds). One setting, `[Debug]` `bShowMessages`, turns all of them on or off; it replaces
  `bShowEmotions` and `bShowShots`, which an older INI can still use.
- Bystanders (`bAvoidBystanders`, `fBystanderStill`): someone standing still (1.5 s) whose body
  would cover the face in shot, or who'd fill most of the picture from close to the camera,
  now pulls the camera in past them or makes it pick another angle, like a wall. The game's
  collision check doesn't see people (0.7.4: Sarah standing still covered most of a shot in
  the Viewport). People walking through a shot are left in.

- Groundwork for filming NPC-to-NPC conversations (nothing is filmed yet): each line an NPC
  speaks within 20 m of you is logged with its words, expression, who the speaker is facing
  (the person they're addressing within 5 m, or you, or no one) and their group (people
  within 5 m of one another who've spoken in the last 20 s), and whether it's part of a quest
  or random scene. With `bShowMessages`, NPC-to-NPC lines also show on screen ("Overheard:
  Name -> Name: Happy (group of 3, 6 m)").
- Groundwork for CCC's room shots (Wide, Master, Distant; nothing is filmed differently yet):
  each conversation's room is measured with the collision check from between the two people
  (the ceiling, and how far it's open in 16 directions at eye height and higher up), again
  when they move more than 1.5 m. The log gives, for each room shot, how far back it wants to
  be, the best place it could stand, how big the people would be from there and whether both
  faces are in clear view; with `bShowMessages`, a summary shows on screen ("Room: 8 x 6 m,
  ceiling 3.0 m | Wide 3.8/18 m, Master 3.8/27 m, Distant 3.8/23 m").

### Changed
- Reaction shots: counted over the whole conversation, not per reply. Once `iReactionEvery`
  (now 2) of the NPC's lines have been heard, each further line has an `iReactionChance` (50%)
  chance of being a reaction; a reaction starts the count again. CCC's per-reply count with
  Starfield's mostly two-line replies gave no reactions at all in 0.7.4.
- Lines you click past within 1.2 s don't count towards a reaction.
- The persuasion log calls the last round "over" rather than won or lost (a won last round can
  still lose the persuasion).

## 0.7.4 - 2026-11-14

### Changed
- Emotion close-ups: Stern, Defiant, Smug, Sarcastic, Flirting, Pleading and Disbelief join the
  expressions that push in (`sCloseUpEmotions`). `docs/EMOTIONS.md` lists all 43 of the game's
  expressions, seen or not.
- Six guessed names on that list that aren't real expressions (Angry, Annoyed, Fearful,
  Scared, Disgusted, Sad) are replaced with the game's own: VeryAngry, Rage, Afraid,
  Depressed and Shocked.

## 0.7.3 - 2026-11-14

### Changed
- Reaction shots: every 3rd line of a reply may be one (`iReactionEvery=3`, CCC's value; was 2).

### Fixed
- Persuasion beat on every round of the mini-game: each round plays its own success or
  failure animation. Only the last round (no turns left) gets the beat now; an earlier win
  isn't recognised yet, and the meter's reading is logged to find it.
- One failed Scaleform call (as a conversation started, probably the menu being rebuilt)
  turned the bars and the options handling off for the rest of the session. A failure now
  skips that frame; only three in a session turn them off, and the log names the call.

### Diagnostic
- Your choice's dialogue record: your own voice data never showed your lines (0.7.2), so
  when you choose an option the dialogue menu's topic manager is searched for dialogue
  records and each is logged with its expression tag ("Your choice").

## 0.7.2 - 2026-11-14

### Diagnostic
- Your own lines' expression tags: the game never reports the player as talking, so 0.7.1's
  check logged nothing. The player's voice data is now watched directly; each new line of
  yours (a new dialogue record or subtitle) is logged with its tag ("Your line emotion").

### Changed
- The dialogue options appear at once by default (`fOptionsFade=0`, was 0.6 s); any value up
  to 1.5 s fades them in.

### Fixed
- Reaction shots never happened: the subtitle (used to skip short lines) isn't there yet
  when a line starts, so every line looked empty. The decision now waits up to half a second
  for it. The roll is also on the iReactionEvery-th line itself instead of the one after,
  since Starfield's replies are mostly two or three lines.
- Persuasion beat: the mini-game's panel closing after its success animation counted as a
  second outcome; an outcome at the end of one conversation carried into the next.
- "Pay" is no longer a beat tag by default: "[Pay 14000 Credits] You're hired." is how crew
  are hired, not a skill check.

## 0.7.1 - 2026-11-14

### Added
- Reaction shots (CCC's "Show You Listening", `bReactionShots`, `iReactionEvery`,
  `iReactionChance`): now and then one of the NPC's lines is shown on you, listening, then the
  camera cuts back. After every 2 of their lines in a reply, a 50% chance the next one is a
  reaction. Short lines and emotion close-up lines are skipped.
- Persuasion beat (CCC's "Hold On Their Answer", `bPersuasionBeat`): the NPC's answer is a
  close-up of them, held for the whole line while the camera pushes in. For the persuasion
  mini-game only the final answer (the panel closing or its success or failure animation);
  also for options with a skill-check tag ([Intimidate], [Manipulation], [Pay 500 Credits]...,
  `sBeatTags`), read from the option you chose.

### Diagnostic
- Your own lines' expression tags are logged ("Your line emotion"), to see whether player
  dialogue carries them too. Persuasion panel, outcome and chosen tagged options are logged.

## 0.7.0 - 2026-11-14

Milestone: the conversation's on-screen dressing from CCC is in place. Black bars slide in
while the camera films (0.6.3), the dialogue options stay hidden while the NPC speaks and
fade in when the line ends (0.6.4-0.6.5), and Starfield's own subtitle sits in the bottom
bar (optionally centred in it).

### Changed
- `fOptionsFade` is limited to 0.1-1.5 s. The fade applies when the options appear; when
  they go, the game hides them itself within 0.2 s, and the mod leaves that alone.

## 0.6.5 - 2026-11-13

### Added
- Subtitles centred in the bottom bar, optional and off by default (`[Interface]`
  `bSubtitlesInBars`): Starfield's own subtitle position already falls at the top edge of the
  bar. When on, the subtitle (the HUD messages menu's `SubtitleText_mc`, found by 0.6.4's
  search) slides down with the bars until its text is centred in the bottom bar, and back when
  they go, since the same element shows ambient lines outside conversations.

### Changed
- The dialogue options fade over 0.6 s (was 0.35 s, too subtle to tell from a pop).

### Diagnostic
- The log says each time the game shows or hides the options, when they start fading out
  for an NPC line, and warns once if the game resets their transparency (which would cancel
  the fade).

## 0.6.4 - 2026-11-13

### Added
- The dialogue options fade out while the NPC speaks and back in when the line ends (CCC's
  option; `[Interface]` `bHideOptionsWhileSpeaking`). The game already hides them during most
  lines, but not during the greeting, where the subtitle ran across the last option. They
  also fade in when the game shows them, instead of popping (`bFadeOptions`, `fOptionsFade`).

### Diagnostic
- The HUD search: 0.6.3's dump of the HUD stopped at 400 elements before reaching the
  subtitles. The HUD and the HUD messages menu are now searched for anything named like a
  subtitle, speaker or caption, and the option list is dumped once when it first appears.

## 0.6.3 - 2026-11-13

### Added
- Letterbox (CCC's black bars, `[Interface]` `bLetterbox`, `fLetterboxHeight`,
  `fLetterboxFade`): black bars slide in at the top and bottom of the screen while the camera
  films a conversation. They're added inside the dialogue menu, under its own content, so its
  subtitles and options stay on top (the first step towards subtitles in the bars).

### Diagnostic
- `bDumpMenus`: the dialogue menu's and the HUD's layout (every element's name, position, size
  and text) is written to the log once per game session, to find the subtitle and the dialogue
  options for the next steps.

### Fixed
- Player missing from a conversation (Violet), and some conversations starting with the
  camera-switching fallback, which flickers (Dirk, a merchant): the game sometimes takes
  1.5-3 s to build the player's full body. The request is now repeated, the camera stays on
  shots of the NPC from the player's eyes meanwhile, and the fallback waits 2 s (4 s when the
  speaker is in a background scene, where it isn't used at all) instead of 0.7 s.
- Mild horizontal jitter on wide shots (the two-shot at 10 m): the camera's angles are
  measured off where the two people stand, and those positions twitched by a centimetre or
  so as the characters settled on the ground. They're now steadied (3 cm dead band, eased).
- `iCameraSway` barely did anything: the percentage was applied after the 4 cm head dead band
  had already removed most of the motion it was meant to scale. It now scales the whole small
  head motion, the camera's angle and its framing point alike: 0 = steady, 100 = exactly
  0.6.0's sway, linear in between.

## 0.6.2 - 2026-11-13

### Changed
- Emotion close-ups: Happy, Smirk, Puzzled, Suspicious, Relieved and Concentrating join the
  expressions that push in (`sCloseUpEmotions`). About 19% of lines now trigger a close-up
  (was 15%); see `docs/EMOTIONS.md`.

## 0.6.1 - 2026-11-13

### Added
- Close-ups that follow the face (CCC's `bFollowFace`): the camera turns toward where a
  face points (half its bearing off the eyeline, at most 35 degrees) and drops to look up
  into a lowered face, decided at each cut and held. The face direction comes from the head
  bone; its forward axis is chosen once per character model and logged.
- The 180-degree rule: each conversation is staged from the side of the line between you
  with more room (measured with the game's collision check; near-equal room is a coin toss).
  `bEnforceLine` (on) keeps every shot, camera move and face turn at least 8 degrees off the
  line (34 for shots of the pair). `bTrue180` puts every shot on that one side. The plain
  shots follow the same side.
- `fFaceDeadband` (0.04 m): the camera ignores head movement smaller than this, so idle sway
  and the bob of someone working at a bench stay off the screen.

### Fixed
- Sway: every shot's angle was measured off the line between the two heads, so when a head
  swayed a few centimetres the camera orbited with it, by more the further out it stood (most
  visible from 0.6.0, with the camera up to 7-10 m away). `iCameraSway` (0-100%, default 40)
  sets how much of that sway the camera follows: 0 measures the line from where the two
  stand, as CCC does between steady points; 100 is 0.6.0's full sway.

## 0.6.0 - 2026-11-13

Milestone: the two things that kept most of CCC's shots out of Starfield are solved. The
person in shot is in focus at any distance (0.5.0), and shots that would put a wall or object
in the way are skipped or pulled in (0.5.3-0.5.4).

### Added
- Stardust preset (`sPreset=Stardust`), CCC SF's own: Standard's shoulders and close-ups
  (locked off, close-ups pushing in) with Room's wider shots mixed in, weighted about 60/40.
  The wider shots move: cranes on the full-figure, overhead and master shots, pull-outs on
  the wide and distant ones, drifts on the two-shot and profile. Caps as Room (7 m / 13 m).

### Changed
- Each preset has its own camera distance caps, as far as its widest shot needs now that
  obstacle detection keeps the camera out of walls: Close 5 m / 5 m, Standard 7 m indoors
  and 10.5 m outdoors (turns on the two-shot outdoors), Room 7 m / 13 m.
  `fMaxCameraDistance` and `fMaxCameraDistanceExterior` now default to 0 (the preset's own);
  any other value is the player's own limit for every preset.

## 0.5.4 - 2026-11-13

### Changed
- Obstacle detection: when no shot in the preset is fully clear, the camera takes the blocked
  shot with the most room left, pulled in (at least 1.5 m and 40% of its distance), before
  falling back to the plain shot. Next to Barrett's ladder this gives the side-on two-shot
  instead of a view from his eyes through the ladder.

### Fixed
- When nothing fitted a turn, 0.5.3 kept re-checking every frame (and filling the log); the
  plain shot now holds until the turn changes.

## 0.5.3 - 2026-11-13

### Added
- Obstacle detection, step 3: before the camera cuts to a shot, the game's own collision check
  tests the line from the person in shot to where the camera would stand (and to where its
  move ends).
  - A wall or object close behind the camera pulls the camera in to just in front of it, when
    that keeps at least 70% of the distance (and, over a shoulder, keeps the camera behind
    that shoulder).
  - Anything closer to the person in shot (a ladder in front of their face) skips the shot;
    the Director picks another. Emotion close-ups are checked the same way.
  - The near person's own body in an over-the-shoulder shot doesn't count: the check carries
    on past them.
  - If the plain over-the-shoulder shot is blocked too, the camera shoots from just in front
    of the other person's eyes instead.
  - `bObstacleCheck` (on) and `fObstacleRadius` (0.05 m, as the game's own dialogue camera)
    in `[Director]`. The log lists skipped and pulled-in shots.

### Fixed
- The 0.5.2 obstacle log measured the wrong lengths: the game's cast moves the end point it's
  given back to the safe spot, so it now gets copies.

### Removed
- The 0.5.1 search for the game's camera-collision code (done).

## 0.5.2 - 2026-11-13

### Changed
- Focus style Wide (deeper sharp zone, softer background blur) is the default.

### Diagnostic
- Obstacle detection, step 2: once per shot, the game's own dialogue-camera sphere cast (found
  in 0.5.1) checks the line from the person in shot to the camera, and the log says whether
  it's clear or blocked, how far along and by what. Logging only; the camera isn't changed.

## 0.5.1 - 2026-11-13

### Changed
- Focus styles (`sFocusStyle`): Cinematic (more background blur) and Wide (deeper sharp zone,
  softer background blur), replacing `bDeepFocusLongShots` (still read: 1 = Wide).
  `fDeepFocusBeyond` is now `fWideFocusBeyond` (old name still read).
- Outdoor shots may stand up to 7 m from the person in shot (was 5 m), closer to CCC's
  framings now that focus works at any distance.

### Added
- `docs/EMOTIONS.md`: running tally of the expression tags seen in testing.

### Diagnostic
- Obstacle detection, step 1: finds the game's own camera-collision code (through the settings
  it reads) and writes it to the log for study. Read-only, once per game session, in the
  background.

## 0.5.0 - 2026-11-13

The focus milestone: the person in shot is sharp at any camera distance.

### Added
- **Focus on the person in shot** (`bFocusOnSubject`, on by default). The game's conversation
  blur reads its settings live from its Dialogue_Close, Medium and Long_DOF records; on every
  shot their focus distance is set to the distance from the camera to whoever is in shot, and
  put back afterwards. Not used in quest scenes, where the game's own camera runs.
- Optional deeper sharp zone on long shots (`bDeepFocusLongShots`, `fDeepFocusBeyond`), for a
  softer background blur.

### Changed
- 40% of emotion close-ups are extreme close-ups (was 50%).
- Steadier following while the speaker walks (from 0.4.20-0.4.21) and the emotion messages in
  `[Debug]` are part of this release.

### Removed
- The experiments that led here: moving the game's camera for focus (`iFocusMethod`), the
  image-space blur experiment (`iBlurExperiment`), applying modifiers by hand
  (`sTestModifier`), the conversation blur test (`iDialogueBlurTest`, now the feature), the
  searches for the live blur and the record dumps. Their INI lines are gone; old INIs that
  still have them are harmless.

## 0.4.22 - 2026-11-13

### Added
- Conversation blur test (`iDialogueBlurTest`): changes the game's own Dialogue_Close, Medium
  and Long_DOF records while a conversation runs (put back afterwards): 1 = no blur, 2 = focus
  at the shot's distance, 3 = a deeper sharp zone on long shots. Their blur strength, focus
  distance and sharp-zone depth were found in the 0.4.21 dumps.

## 0.4.21 - 2026-11-13

### Changed
- Steadier following while the speaker moves starts at 0.25 m/s (`fBusyHeadSpeed`), so it
  reacts to walking rather than ordinary talking.

### Diagnostic
- The game's dialogue blur modifiers, Starvegas's and Chargen_DoF dumped in full (4 KB each), to
  find their focus distance, sharp-zone depth and blur strength.
- How often the steadier following switched on, per conversation.
- Finding: moving the game's camera never moves the conversation focus (methods 1-6, and the
  game doesn't put its camera back after the hook). That route is closed.

## 0.4.20 - 2026-11-13

### Fixed
- The picture jittered while the speaker walked (their head bobs with each step). While
  their head moves faster than `fBusyHeadSpeed` (0.2 m/s), the camera follows with
  `fBusyFollowSmoothing` (1.0 s) of smoothing instead of 0.35 s.

### Added
- Focus test methods 5 and 6 (`iFocusMethod`): move the game's camera later in the frame, on
  the main thread, in case the game works out its focus before the hook's move.

### Diagnostic
- Timing check: whether the game camera is still where the hook left it later in the frame.
- The game's dialogue blur modifiers, Starvegas's own and a few other depth-of-field ones are
  dumped with the data they point to, to find where their focus distances are kept.

## 0.4.19 - 2026-11-12

### Diagnostic
- The live conversation blur search now also walks out from the image-space manager, the
  dialogue camera, the player camera and the render camera, and reports any reference to the
  Dialogue_*_DOF records. Read-only, no INI changes needed.
- Per-shot measurement of how much the speaker's head moves, and how far the smoothed head
  the camera follows lags behind it (for the jitter with NPCs posing or at crafting tables).

## 0.4.18 - 2026-11-12

### Changed
- The image-space blur experiment is off by default (`iBlurExperiment=0`): it only reached
  the distant-haze settings, not the conversation blur. `iFocusMethod` stays off by
  default: none of its four ways of moving the game's camera moved the focus.

### Diagnostic
- Finds the game's live conversation blur (the running copy of Dialogue_Close, Medium or
  Long_DOF) and logs what it holds, a few times per conversation. Read-only.

## 0.4.17 - 2026-11-12

### Added
- Focus test (`iFocusMethod`): four ways of moving the game's camera so its depth of field
  lands on the person in shot, to find one that doesn't shift the picture as 0.4.8 did.

### Fixed
- Crash when `sTestModifier` was set (0.4.16): the direct call to the game's apply function
  is disabled. The game's dialogue blur modifiers focus on a target, so applying them by
  hand isn't the way forward.

## 0.4.16 - 2026-11-12

### Fixed
- `sTestModifier` never took effect: reading the `_user.ini` afterwards switched it off.
- Long text settings were cut at 128 characters, so the end of `sCloseUpEmotions`
  (Serious, Confused) was ignored.

### Changed
- The on-screen emotion messages (`bShowEmotions`) moved to the INI's `[Debug]` section
  (`[General]` is still read for older INIs).

### Diagnostic
- Each conversation start logs the test settings in effect.

## 0.4.15 - 2026-11-12

### Changed
- Emotion close-ups are more distinct: half are extreme close-ups (`iExtremeCloseUpChance`),
  and the regular one pushes in slowly and strongly, as in CCC's Close preset.
- Seated people and merchants get emotion close-ups too, from in front of them, then go back
  to the plain shot when the line ends.
- Disgust, Serious and Confused added to the close-up emotions.
- The on-screen emotion message says what the camera did: close-up, extreme close-up or no cut.

## 0.4.14 - 2026-11-12

### Added
- Emotion close-ups: lines tagged with heightened or emotional expressions (Irritated,
  Surprised, Somber, Nervous, Awed and others, set in `sCloseUpEmotions`) push in to a
  close-up of the speaker, after CCC's close-ups on intense lines.
- On-screen message with each spoken line's expression tag (`bShowEmotions`), to help choose
  the close-up emotions.
- Test switch to apply one of the game's image-space modifiers during conversations
  (`sTestModifier`), the way the console's `imod` does.

### Changed
- Crew recruits get this mod's camera instead of being treated as a quest scene.

### Fixed
- The search for the game's image spaces read the list header in the wrong order and found
  nothing.

### Diagnostic
- The scene check logs the quest each scene belongs to.

## 0.4.13 - 2026-11-12

### Fixed
- Merchants no longer fade in when the camera comes back to them: on your lines the camera
  stands in front of and beside them instead of behind them.

### Diagnostic
- Find the game's image spaces and image-space modifiers by scanning, and list them.

## 0.4.12 - 2026-11-12

### Added
- Merchants are recognised by the vendor flag on their faction, wherever the shop is, and
  get the plain over-the-shoulder shot.
- Blur experiment (`iBlurExperiment`): switch off the game's depth of field on the longer shots.

### Diagnostic
- Each line's expression tag (Friendly, Irritated...) written to the log.

## 0.4.11 - 2026-11-11

### Changed
- The focus fix from 0.4.8 is off by default: it shifted angled shots sideways, leaving
  people at the edge of the frame.
- Outdoor shots may stand up to 5 m away again.
- On your lines, the seated shot keeps about 1 m from the seated person, who no longer fades
  out and back in.

### Fixed
- When the speaker is in a background scene, the camera is no longer switched to first
  person and back mid-conversation (one cause of stacked lines).

### Diagnostic
- Searches for the game's focus distance, the line's response data and faction data.

## 0.4.10 - 2026-11-11

### Added
- Seated people get the plain over-the-shoulder shot instead of the shot library, whose
  angles could put the chair in front of them.

### Fixed
- Over-the-shoulder shots no longer swing in so close to the line between the two of you
  that the near person's shoulder hides the speaker.
- The NPC shot from your eyes no longer lingers after a late body rebuild.
- A likely crash in the test-only line diagnostic (seen with Ularu).

### Changed
- Outdoor shots limited to 3 m (undone in 0.4.11).

## 0.4.9 - 2026-11-11

### Fixed
- About one conversation in five had rotated or swinging shots: the camera layout was
  guessed per conversation and sometimes guessed wrong. It's now fixed.

## 0.4.8 - 2026-11-11

### Added
- Focus fix: move the game's camera so its depth of field lands on the person in shot
  (turned off in 0.4.11).

### Changed
- Everyday background scenes (people chatting, a receptionist's greeting) keep this mod's camera.

## 0.4.7 - 2026-11-11

### Added
- Quest scenes you take part in keep the game's own camera, decided on the first frame.
  The idea comes from Player Camera During Dialogue.

### Fixed
- Merchants were wrongly treated as scenes in 0.4.6.

## 0.4.6 - 2026-11-11

### Added
- First attempt at handing scripted scenes back to the game's camera (replaced in 0.4.7).

## 0.4.5 - 2026-11-11

### Diagnostic
- Look for the spoken line's dialogue record; depth-of-field test settings.

## 0.4.4 - 2026-11-11

### Added
- CREDITS.md and an updated README.

### Diagnostic
- The game's depth-of-field settings and the line's response data (emotions).

## 0.4.3 - 2026-11-11

### Changed
- Cutting styles renamed: Cinematic, Odyssey, SpaceWestern, Dogfight, SpaceOpera (old names
  still accepted).

## 0.4.2 - 2026-11-11

### Added
- Cutting styles: how often the camera takes a fresh angle when the turn passes, how many
  lines per angle, and the shortest hold.

## 0.4.1 - 2026-11-11

### Changed
- Each shot holds its angle and framing; each person keeps their angle across the
  back-and-forth; the camera may stand further back outdoors.

## 0.4.0 - 2026-11-10

### Added
- The CCC port begins: all 39 camera setups, the director with its presets (Standard, Close,
  Room) and cutting rules, camera moves.
- Small speakers (robots) keep the game's own camera.

### Changed
- Over-the-shoulder shots swing in until the near person's shoulder is in frame through
  Starfield's narrow lens. The INI was reorganised.

## 0.3.36 - 2026-11-10

### Changed
- Conversation start behaviour chosen separately for first and third person; no exit cover
  by default.

### Fixed
- Camera stuck behind the head after switching view.

## 0.3.35 - 2026-11-10

### Added
- Transition test settings: start-frame prediction, exit cover modes, game settings changed
  from the INI while the game runs.

## 0.3.34 - 2026-11-10

### Changed
- From first person: open over the shoulder, hold the last shot to the end and cut straight
  back to your view.

## 0.3.33 - 2026-11-10

### Fixed
- Exit glitches: the passing third-person frame and player visibility on the way back.
- Your character is hidden while the body rebuilds.

## 0.3.32 - 2026-11-10

### Fixed
- The first frames no longer flash a wide view; the opening shot holds until the first cut.

## 0.3.29 to 0.3.31 - 2026-11-10

### Changed
- Your full body is built with a direct refresh of your character, without switching
  cameras (idea from Cassiopeia Papyrus Extender); the camera switch is kept as a fallback.

## 0.3.27 to 0.3.28 - 2026-11-10

### Added
- Field-of-view and body-refresh test settings.

## 0.3.12 to 0.3.26 - 2026-11-09 to 2026-11-10

### Changed
- Many rounds of first-person body rebuild work: waiting for a quiet moment, retries with
  back-off, predicting where the game's camera will be, holding the view steady during the
  rebuild.
- Your character is revealed on the same frame as the first shot of them.

### Diagnostic
- Camera modes, field of view, camera positions and the game's settings.

## 0.3.10 to 0.3.11 - 2026-11-09

### Fixed
- A crash during a second body rebuild (camera changes now only happen on the main thread).

## 0.3.5 to 0.3.9 - 2026-11-09

### Added
- Seated shot: when the person nearest the camera is sitting, the camera stands in front of
  them instead of behind the chair.
- An opening grace period and a shortest time between cuts.

### Changed
- The body rebuild checks your head is at standing height before showing you, and keeps the
  camera off you when the full body can't be built.

## 0.3.0 to 0.3.4 - 2026-11-09

### Added
- From first person, your full body is rebuilt so you look right from outside.
- The last shot holds while the game's dialogue camera winds down; smooth head following.

### Removed
- A fast exit that left controls locked.

## 0.2.0 to 0.2.3 - 2026-11-09

### Added
- Only the picture on screen moves, so NPCs keep facing you; your character is shown.

### Fixed
- A freeze on the first conversation frame, and the camera's orientation in Starfield.

## 0.1.0 to 0.1.4 - 2026-11-09

### Added
- First version: the dialogue camera replaced by over-the-shoulder shots that follow whoever
  is speaking.

### Fixed
- Distances in metres, and Starfield's camera axis layout.
