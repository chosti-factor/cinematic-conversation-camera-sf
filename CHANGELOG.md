# Changelog

All notable changes to Cinematic Conversation Camera SF, newest first. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/). Every 0.x version is an alpha
test build; "diagnostic" items only add information to the log for testing.

## 0.9.25 - 2026-11-18

### Changed
- Constellation's master shot drifts instead of craning up: the documentary operator is on
  the floor with the people (the user). Its long shots still crane up.

- From the 0.9.24 test (the user):
  - Starborn overshot a little towards *Interstellar*: sway 30 to 15, faces a quarter
    off-centre (half before), and its close-up lenses half of 0.9.23's step wider than the
    original (their close-up 47.5 degrees, yours 42.5, the extreme ones 42.5 and 37.5).
  - The low angle on them (the lowest camera of the three) tilts a sixth of the old tilt, not
    a third; it was still a bit much next to the others.
  - Frontier's extreme close-ups of them are rarer: 20% of its emotion close-ups (4 of 9 in the
    test). `iExtremeCloseUpChanceNpc` is now -1, the preset's own (Frontier 20, the others 40).
  - With a large speaker, the plain over-the-shoulder shot of you swings round to 45 degrees
    (38 still showed part of Vasco when you stood close); `fLargeSpeakerPlainAnglePlayer`, in
    the INI and on the settings page.

### Fixed
- The last one-frame flashes (3 in the 0.9.24 test, down from 22): an emotion close-up in the
  middle of their turn, then a reaction on the next frame. It now waits for the line's
  reaction to be decided too.

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
