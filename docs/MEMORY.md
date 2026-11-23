# Memory: what was tried, measured and removed

Everything the 0.9.30 redundancy pass took out of the code and the INI, kept here with its
technical detail, so an idea, a measurement or a dead end can be looked up again instead of
rediscovered. The working behaviour is described in [TRANSITIONS.md](TRANSITIONS.md) (starts
and exits), [PRESETS.md](PRESETS.md), [REACTIONS.md](REACTIONS.md) and
[EMOTIONS.md](EMOTIONS.md); the full story of each version is in
[CHANGELOG.md](../CHANGELOG.md).

Last updated: 0.9.59 (2026-11-22).

**The code as it was before the pass:** commit `04abb04` (0.9.29). Any removed piece can be
read back with `git show 04abb04:src/Plugin.cpp` (or `Settings.h`, `Settings.cpp`,
`Camera.cpp`, `Camera.h`, the INI). Function and variable names below are as they were there.

All offsets are for Starfield 1.16.244. Address Library IDs are given where the code used
them.

## Lenses and field of view

The conversation lens works since 0.8 by changing the value on its way into the player
camera's set-FOV function (ID 113443, Starfield.exe +1E92650; writes PlayerCamera +280 and the
render frustum) at all three of its callers: the camera's field-of-view update (ID 64682,
+BCB990) and two others (`LensHook2`, and `LensHook3` at ID 113488 +0x498). That stays. What led
there, removed:

- **`fDialogueFOV` / `dialogueFov`** (0.3.27 to 0.3.30): a fixed field of view for the
  conversation shots, 0 = the game's (about 30, a tight close-up). Replaced by the lens hook
  and `fConversationLens`; the field was unused since.
- **`iFovMethod` / `fovMethod`** (0.3.30), how `fDialogueFOV` was written:
  1 = just before the game's dialogue camera update (the game then worked from it);
  2 = just after it, field of view and view cone (made the picture shake);
  3 = before the update, plus the view cone after it.
  None held: the game's update writes 30 every frame. Unused since.
- **`iFovExperiment` / `fFovExperimentValue`** (0.3.25 to 0.3.28): set some of the game's own
  settings to the value for the length of a conversation, put back after, to find which one
  the dialogue camera listens to. Re-read every conversation.
  - 1 = `fTPWorldFOV:Camera` + `fFPWorldFOV:Camera`
  - 2 = `fDialogueCameraFailsafeFPFOV:Dialogue` (30): **no visible effect** (0.3.28 test),
    though it's the value the game draws conversations at. The lens is decided elsewhere and
    handed to set-FOV each frame.
  - 3 = `fDefault1stPersonFOV:Camera` + `fFPGeometryFOV:Camera`
  - 4 = a float inside the dialogue camera state object, **+0x2A8, which holds 90.0 during
    conversations** (seen in the 0.3.25 dumps), written while live, put back after (only if
    it held 10 to 170 degrees).
  Settings were found by walking `INISettingCollection` and `INIPrefSettingCollection` for a
  float with that key (`FindSetting`).
- **The conversation field-of-view keeper** (main-thread task, every 240 ticks): looked up
  `fDialogueCameraFailsafeFPFOV` and wrote its own original value back (the 0.3.28 finding
  left it at the game's value), logging "Conversation field of view: ... set to 30.0". A
  no-op since.
- **The render camera's own frustum** (the tangents of its half angles), found in the 0.3.28
  dumps at render camera +0x1C0, a different place from the library's layout
  (`kRenderCameraFrustumOffset`). Unused since the lens hook; the frustum is reached through
  the library's `viewFrustum` (`SetRenderLens`).
- **The FOV scan** (`LogFovCandidates`, `ScanFloats`): on each camera mode change, the first
  three times each for first person (0), dialogue (17) and third person (20), every 4-byte
  float that looked like an angle (15 to 150 degrees, or 0.25 to 2.7 radians, not 1.0) in
  PlayerCamera +0x20 to +0x188 and the current camera state +0x8 to +0x340. This is how
  PlayerCamera +280 (the field of view, `kPlayerCameraFovOffset`, still used) was found.
- **The exit lens switch `bExitLens`** (0.9.13): see Exits.

## Writing the camera

- **`iCameraTarget` / `cameraTarget`** (from 0.2.0): 1 = move only the rendered view (the
  NiCamera under the camera root, `ApplyRender`), so the game still thinks the camera is at
  your eyes and NPCs keep facing you; 0 = move the game's own camera root (`Apply`), and
  NPCs turn to face it (0.1.x: they did). Now always 1. `Apply` stays as the automatic
  fallback when the render camera can't be found.
- **`poseMode`** (0.1.x test switch, not in the INI since): how `Apply` wrote the root.
  0 = world + local + a node update (kept), 1 = world + local only, 2 = world only.
- **`fBlendSeconds` / `blendSeconds`**: seconds to blend between shots, 0 = hard cut. Nothing
  read it any more: CCC cuts, and so does this mod.
- **`RenderDrift`** (Camera.cpp, with `g_lastWritten`): how far the picture the game last drew
  was from the shot placed for it, in metres and degrees (from the trace of the two
  rotations, 1 + 2 cos angle). Nonzero when something moved the game's camera after our
  update (scripted scenes, NPCs who start a conversation while walking). No caller left.
- **`RenderCameraObject`**: the render camera's address, only for the dumps below.

## Starts

The first frames are placed against a prediction of where the game's dialogue camera will be
(TRANSITIONS.md, "Frame 1"): on the line from the NPC's head through your feet, 2.02 m from
the head, 0.07 m above it, aimed 2.75 degrees below; learned further from each start. Kept.
Removed:

- **`iStartMode` (first person) / `iStartModeThirdPerson`**, what the first frames show while
  the game still reports its gameplay camera (1 to 3 frames, 0.3.33/0.3.34 logs and clips: the
  game draws from its dialogue camera meanwhile):
  - 0 = your view from just before the conversation (`waitingView`, with your own field of
    view through `g_waitingLens`; from first person until your body was ready, at most
    0.6 s; 0.8.14 test, the Ranger at Neon: the first shot began with you missing and you
    popped in a moment later).
  - 1 = the first shot, placed against the predicted dialogue camera (kept, both).
  - 2 = your view, placed that way. Third person was 2 until 0.9.4: its clips showed your
    view, then two frames of it moved to where the game's camera would be, then the first
    shot.
  - 3 (first person, gone earlier) = the 0.3.33 opening: a shot from your eyes until the
    first cut.
  `framesSinceWaiting` (frames on a shot since your own view ended) went with it; it was no
  longer read.
- **"Start facts"** (first frame of the first 12 conversations): both people's feet and heads,
  and the NPC's head height above its feet. The prediction's numbers came from these and
  from "Start frame" (kept).

## Exits

### First person

The cause, found in 0.9.7: **the first first-person update after a conversation sets the
camera's pitch to −68.13 degrees** (FirstPersonState +C8 and +CC, degrees, negative up;
−68.1265, a near-constant), but only when your full body was rebuilt for it (every exit with
the direct rebuild, none with it off, none with the camera cycle, none in vanilla). The next
update puts your pitch back. Fixed by `iExitMode=5` (0.9.8): that update is held back one
frame with the last shot on screen, so the update drawn comes once your first-person body is
back. Kept, now fixed (`kExitSkipFrames = 1`). The entry-step skip and the pitch put back
after the update (from mode 3) still run under it. Removed:

- **`iExitMode` 0 to 4:**
  - 0 = cover the game's first first-person frame by drawing your saved view over it from the
    first-person hook (`g_exitMasking`, `g_exitCoverFrames` = 1). 0.8.18: two frames of
    ceiling; 0.9.0: the floor frame unchanged.
  - 1 = nothing (logged only).
  - 2 = the same cover for three frames.
  - 3 (0.9.4/0.9.5) = before the update, mark the first-person camera as already entered
    (+E0 1 → 0, +E2 0 → 1, +68 1.0 → 30: its "just entered" flag), and after it, put +C8/+CC
    back and the camera root on your view. The flag isn't the cause; the frame didn't change.
  - 4 (0.9.7) = 3 plus your view drawn over the frame. Didn't reach the screen either: the
    picture is taken during the game's own update, before any hook after it runs.
  - The cover was also released if you switched to third person with it in place (0.3.35:
    the view stayed behind the head until the next conversation; `ExitMaskRelease`).
  - `g_exitMaskUntil`, a one-second window after a first-person exit, was set but no longer
    read.
- **`iExitSkipFrames`** (0.9.8): updates held back under mode 5, 1 to 4. More than 1 doesn't
  help: the conversation ends during that frame.
- **The exit search** (0.9.7, `SnapExitObjects`, `SearchExitObjects`): before the first
  first-person update, copies of the first-person state (0x200 bytes), the player camera
  (0x400), the dialogue and third-person states (0x400 each), the player (0x1000) and
  whatever the first-person state points to at +88, +90, +B0 and +B8 (0x400 each); after the
  update, each copy searched for the new pitch as degrees, radians, sine, cosine or tangent,
  either sign (tolerances 0.02 degrees, 0.0004, 0.002 for the tangent), and the first 40
  changed values listed. **Result: −68.13 isn't stored anywhere**, before or after; most
  likely computed from the third-person body's pose. That led to mode 5.
- **The exit dumps** (0.9.3): the first-person state (0x200) before and after the game's update
  on the exit's first two first-person frames, for the first 12 conversations. Found the
  entry flag and the pitch fields.

### Third person

- **`iExitModeThirdPerson`:** 1 (0.9.4, kept) = your saved third-person view drawn on the frame
  the game switches back (otherwise a frame of the game's own conversation camera); the
  render camera goes back to the game just before the third-person camera's first update.
  0 = the game's. 2 (0.9.8) = also put the camera root on your view before the third-person
  camera's first update, so its ease out starts where it ends: **didn't change the one frame
  of the camera easing in** (from behind instead of from the side). The 0.9.12 clips showed
  that frame is the lens ease (your view in place, magnified); vanilla has its own
  in-between frame (0.9.14 test). `g_exitPendingThird` went with it.

### The lens at the exit

- **`bExitLens`** (0.9.13): your gameplay lens put back at the exit through set-FOV (which also
  writes PlayerCamera +280, which the game's ease may start from), and for up to 6 lens
  updates or 0.25 s the game's in-between values swapped for it at all three callers. Covers
  the frame zoomed about 1.85 times (a lens of about 60 degrees, halfway from 29.6 to 94) seen
  at every exit in the 0.9.12 clips. Off = that frame. Now always on.

### Exit styles

- **`iExitStyle` 0** (up to 0.9.13): the last shot stays to the end. When the game hands the
  camera back it swaps your body back and turns the blur off, and the last shot showed you
  gone from it for one or two frames (0.9.12 clips; built before our hook runs, so it can't
  be drawn over). Styles 1 (a closing shot of the NPC alone) and 2 (a cut back to your view,
  the default) stay. Seated in third person still ends on the last shot (0.9.18: your view
  from before isn't where the game's seated camera comes back).
- **`bExitCutStaged` off** (0.9.15/0.9.16): style 2's cut with everything at once: your body
  hidden, `g_waitingLens` set to your lens, the blur off, `ShowGameplayView` on your view, all
  on the same frame. The 0.9.15 clips showed the order these reach the screen varies by a
  frame from exit to exit (one exit lens first, another camera first). The staged cut (kept)
  sets the lens with the camera through the game's own set-FOV (0.9.17; our own frustum maths
  gave a lens about 1.8 times narrower than the game's 94), lands 12 cm ahead of your eyes and
  eases back over 0.25 s from first person, hides your body from the third frame and turns the
  blur off from the second.

## Your body from first person

Kept: the direct refresh (0.3.29 to 0.3.31; idea from Cassiopeia Papyrus Extender):
`Set3DUpdateFlag` on the player while the dialogue camera is active builds the full body
with no camera switching, on the conversation's first frames; the camera cycle as its
automatic fallback when the body hasn't appeared after a moment. Removed:

- **`iBodyRebuildMethodPlayer`:** 0 = the camera cycle only (first person → third person →
  first person, many rounds of work in 0.3.12 to 0.3.26: waiting for a quiet moment, retries
  with back-off, predicting where the game's camera would be, holding the view steady). With
  the cycle, conversations opened on the NPC from just in front of your eyes (`povNpcShot`,
  `Camera::Shot(player, npc, -0.30, 0, 0)`) until the first cut, so nothing half-built showed.
  The fallback still does.
- **`iBodyUpdateFlagsPlayer`:** the refresh's parts (1 model, 2 skin, 4 head, 8 face, 16
  scale, 32 skeleton). 63, all of them, was best; **33 left the head missing**. Now
  `kBodyUpdateFlags = 63`.
- **`bEarlyBodyRebuild` off:** the rebuild a quarter of a second in (`kRepairStartDelay`)
  instead of on the first frames, which visibly zoomed the picture out and back.
- **`bStateHooks`:** the hooks on the first- and third-person cameras' updates (vtable index
  `kDialogueUpdateVtableIndex` on FirstPersonState and ThirdPersonState). Off only for early
  testing; the exits and the body cycle's masking need them. Now always on.
- **"Body rebuild refused" dumps:** the player camera (0x188) and the dialogue state (0x340)
  when the game refused to leave its dialogue camera (seated NPCs, every time).

- **One camera cycle only** (`kRepairMaxCycles = 1`, and a 16 ms retry, `kRepairRetry`): 0.3.9
  crashed inside the game while switching camera modes on a second cycle (Linden, Ryujin
  HQ), so a failed rebuild kept the camera on the other person for that conversation. The
  constants had no reader left; the attempts are limited by `kRepairMaxAttempts` (4) now.

## Shots

- **`iShotStyle` / `shotStyle`** (0.3.x, not in the INI since 0.4): 1 = the plain shots always
  from in front of and beside the near person (the seated shot's geometry, `fSeatedForward`,
  `fSeatedSide`, `fSeatedHeight`), keeping the camera in the open space between two people
  instead of behind someone backed against a wall or chair. Seated people and merchants still
  get that shot; everyone else gets the director's shots, with the obstacle check.
- **The head-motion measurement** (0.4.19, `LogHeadMotion`): per shot, the speaker's head path,
  average and peak speed (steps over 0.5 m ignored), and how far the smoothed head the camera
  follows lagged behind the raw one (average and most), logged when the shot changed or the
  conversation ended (at least 20 frames and 0.3 s). For the jitter with NPCs posing or
  working at a crafting table; it led to `fBusyFollowSmoothing` / `fBusyHeadSpeed` and the
  dead band (`fFaceDeadband`, 0.6.1).
- **"Distance between heads"** on the first frame (game units, with the camera state).

## Focus

- **`bDeepFocusLongShots`** (0.5.0): 1 = a deeper sharp zone on long shots. Replaced by
  `sFocusStyle=Wide` (0.5.1); the old key was still read as Wide until 0.9.30.
- **`fDeepFocusBeyond`:** the old name of `fWideFocusBeyond` (3.5 m), read until 0.9.30.
- The settings scan (below) listed the game's `dof`, `focus`, `depth`, `bokeh` and `blur`
  settings while the focus was looked for; the answer was the Dialogue_Close/Medium/Long_DOF
  image-space records (+0x38C strength, +0x398 focus distance, +0x3A4 sharp-zone depth), which
  the game reads live (kept, 0.5.0).
- **The focus at a cut, 0.9.51-0.9.54 (all dropped; 0.9.55 is 0.8.10's way, the focus on the
  cut's own frame).** Measured per cut in the user's clips (sharpness inside the bars, frame by
  frame). On the cut's frame (0.8.10-0.9.50): now and then the old shot's last frame drawn with
  the new focus, a blurred frame (the 0.9.54 clip: 1 of 15 cuts). A frame later (0.9.51): the
  blurred frame moved to the new shot's first (7 of 31). No blur on the cut's frame, the new
  focus a frame later (0.9.52), and that blur eased back over 0.25 s (`bCutFocusEase`, 0.9.53):
  no blurred frames, but the blur's change itself was distracting (the user). The sharp zone
  (+0x3A4) stretched from the nearer person past the farther on the cut's frame
  (`bCutFocusBridge`, 0.9.54): those frames came out blurrier, not sharper (a TwoShot's first
  frame at a tenth of its sharpness), so a larger sharp-zone value doesn't widen what's sharp
  the way it was read; all three 1.04-1.67x lens zooms of that run were on its cuts too.
- **Not frame generation, not DLSS (2026-11-22):** frame generation was suspected and wasn't
  on (the user: DLSS Performance, RenoDX HDR and RenoDX's DLSS neural rendering). With DLSS and
  neural rendering off (0.9.55 clip, 21 cuts) the same oddities: 4 lens zooms (1.22, 0.83, 1.56,
  1.04 times), 6 early-focus frames, the worst cuts with both. Next: 0.9.56's cut timing trace.

- **The cut timing trace** (0.9.56-0.9.58, detailed log only): for 120 ms from each cut,
  "Cut timing +ms [thread]" lines for the mod's camera updates (the render camera's lens at
  their start, the shot's lens), the lens written with the camera, the game's lens updates at
  all three callers (asked, set, the camera's lens before and after) and the focus writes.
  Found: the game's lens update (64682) runs only every other camera update, on the same
  worker thread just before it, and never put an old lens back; at three cuts, all from
  MediumPlayer (62 degrees), the mod hadn't written the new lens with the camera, because the
  first-frames check (`wideFrame`: the game's FOV over 60 degrees taken for your gameplay lens
  still on) took the mod's own wide lens for it. Fixed in 0.9.57 (the check leaves out the lens
  the mod put on): no zoom in the ~60 cuts measured since. The focus writes on a cut's own frame
  happen before the trace started, so they weren't in it.

## Reactions and filming

- **`iReactionSentencesPlayer`** (2), **`fReactionSecondsPlayer`** (2.5 s on you, then back to
  the speaker; 0 = the whole line), **`fReactionMinSecondsPlayer`** (2 s): the 0.8.1 to 0.8.4
  reaction timing, counting the speaker's lines as sentences (`CountSentences`: each run of
  `.`, `!`, `?` or `…` followed by a space or the end, at least one per line). Replaced in
  0.8.5/0.8.6 by whole lines (REACTIONS.md); the settings were still read, unused, until
  0.9.30.
- **`fAutoListenAfter`**: seconds standing still near a conversation before filming started on
  its own (4, then 2 in 0.8.2, 1 in 0.8.4). Gone from the INI in 0.8.6 (it starts as the scene
  begins, even while you walk); still read, unused, until 0.9.30.

- **Filming's camera root hold** (`g_film.rootPin`, 0.9.18 to 0.9.42): every filming frame,
  the gameplay camera's root (`PlayerCamera::cameraRoot`) was written back to its pose from
  filming's first frame (`Camera::SetRootPose`: world and local, translate and rotate), before
  `ApplyRender` placed the render camera against it. Why: the shot is placed relative to the
  root, which follows you; walking on while it filmed (the 0.9.17 test), the root moved again
  after our hook, and the shot landed a frame's walk off, shaking until you stopped. The game's
  third-person update sets the root afresh each frame, so the hold didn't build up.
  **It set off the lip-sync bug** (found 2026-11-21): after filming, conversations had lines
  overlapping (several playing at once) and mouths out of step, Tuala's at the MAST most
  often, until a save was loaded; filming quest or ambient scenes alike, interrupted or played
  to the end. The game evidently reads the camera root as where the camera (the listener) is,
  and something in its dialogue timing kept the bad state. Without the hold, moving the view
  (render camera only, as in your own conversations) is harmless. The shake it fixed is mostly
  gone anyway: auto-started filming holds you still (0.9.31); a listen-key start while
  walking may shake until you stop. The exit path `ExitToFirstPerson` also writes the root
  once (`SetRootPose` on your view); it hasn't run in any recent log.
- **The lip-sync hunt** (0.9.39 to 0.9.42, the user's runs on one save: the Vanguard recruits'
  ceremony and city-life scenes at the MAST filmed, auto filming off, then Tuala's MQ102
  conversation): filming off from load, clean; filmed to the end, no interruption, overlap
  (so not the interruption, nor a reload mid-scene). Temporary `[Debug]` switches, one part
  of filming each: `bFilmTestNoHud` (the HUD kept), `bFilmTestNoHold` (no input layer hold),
  `bFilmTestNoLens` (the game's lens), `bFilmTestNoCamera` (`FilmFrame` skipped: no view
  move, no root hold, no obstacle casts), `bFilmTestNoRootPin` (only the root hold left out).
  `bFilmTestNoCamera=1` with first person (so the forced third person and your hiding still
  on, and the HUD hiding, the hold, the lens, the bars): clean. `bFilmTestNoRootPin=1`, all
  else on: clean. The obstacle casts were out already: they use the dialogue camera's
  collision query (`g_dialogueStateObject + 0x48`), only known after a conversation of yours,
  and the 0.9.40 run broke with filming before any. Switches removed in 0.9.43.

- **`bFilmTestStopAfterLine`** (0.9.58, removed 0.9.59): the overlapping lines came back at the
  Lodge (0.9.57: Cora, twice, each after a filming of Walter with Matteo or Vasco stopped by
  moving while one of them talked; clean with filming off, with the mod off, and after a filming
  left to end on its own). The test: moving mid-line gave your controls back at once but held
  the view on the filmed shot until that line ended. On, clean; but off, two filmings stopped
  mid-line were followed by clean conversations with Cora too. 2 broken of 7 conversations
  after a filming, same steps: intermittent; a known issue for 1.0 (README). Player Camera During
  Dialogue lists the same symptom and keeps its camera changes away from an NPC's multi-part
  response until it has played (its "dialogue-skip bug"); Cora's responses are multi-part.

## Exits, later

- **Stand-in bars through the exit** (0.9.47-0.9.48, taken out in 0.9.49; post-1.0 Mulling).
  The letterbox is two shapes added to the dialogue menu's movie; the game closes that menu
  as the last line ends ("Speaker gone" comes a few frames later), so the bars go 0.4 s before
  the first-person cut (`fExitCutSeconds`) and about 1.5 s before the third-person one (on the
  hand-back frame since 0.9.48). The stand-in: the HUD messages menu's bars (filming's,
  `g_filmBars`) shown from the menu closing to the cut, at the height the dialogue bars had
  (`g_lastDialogueEase`), no slide, dropped on the cut.
  - 0.9.47: driven from the main task (`Hud::SetExitBars`; wanted while this conversation's
    dialogue menu had been open and was now closed, no trade/container/crew/ship-builder menu
    or settings page over it, no cut yet). The clips, measured frame by frame (the top 20 px'
    brightness: 0 = bar): one frame without bars at every exit as the menu closed (the task's
    next tick put them up after that frame was drawn), and in one third-person exit of two
    the bars went a frame before the cut.
  - 0.9.48: `BSTEventSink<MenuOpenCloseEvent>` registered on `RE::UI` (`RegisterSink`), the
    bars put up inside the event when it came on the main task's thread; nothing asking the
    UI inside it (`IsMenuOpen`/`GetMenuMovie` could deadlock if the UI holds its menu list
    while it sends the event): the HUD messages menu's movie and "a menu over it" kept from the
    main task's last tick (the movie handle on a never-freed holder, so quitting
    mid-conversation can't release it after the UI has gone). The drop a tick after the cut
    (the camera runs on worker threads: the tick that saw the cut sometimes came before it was
    drawn). The 0.9.48 clips: still the frame without bars as the menu closed (all four exits;
    the event's thread never logged, so unknown whether it came on another thread or after the
    frame's UI was drawn); third-person cut and bars on one frame; first person, the bars a
    frame after the cut (the tick's delay there; it was only needed at the hand-back).
  - Ways still open: the event's thread and timing first (a log line per exit was written for
    0.9.49 and dropped with the rest); bars put in place hidden before the menu closes and
    only switched on (needs a way to know the end is coming); bars kept on the HUD messages
    menu all through a conversation (it draws over the dialogue menu: they'd cover its EXIT
    button hint in the bottom bar).

- **`fExitHoldSeconds`** (`[Camera]`, 0.5, 0-5; removed 0.9.35): after the last line, hold the
  final shot at least this long before your view comes back. Since 0.7.6 the last shot is held
  anyway until the game leaves its dialogue camera (up to 3 s), and exit style 2 cuts to your
  view at `fExitCutSeconds` (0.4 s), so it never changed anything.

## Leftovers removed in 0.9.33

- Conversation state never read: `lastAppliedFrame` (written only), `driftBits` and
  `driftLogged` (the game moving its camera after ours, from `RenderDrift`), `lastRoot`,
  `lastNpcFeet`, `lastPlayerFeet`. `g_fovLogged` (reset each conversation, never read).
- `stillAt` and `stillSince` in the filming state: the standing-still timer behind
  `fAutoListenAfter` (gone in 0.8.6).

## Measuring code and diagnostics removed in 0.9.50

Taken out once the optimization pass was done (the user, 2026-11-22: before the INI review).
What each measured or logged, and what it found:

- **The cost meters** (0.9.37-0.9.49, Plugin.cpp `CostMeter`/`CostScope`/`LogCosts`): time in
  the mod's own code, per call, around the main-thread task, the HUD, the nearby-people scan,
  the conversation camera and the filming camera; logged at the end of each conversation and
  once a minute outside them (average and worst per call, total). Found, in order: a full
  INI read every 2 s and at every conversation's start (up to 20 ms each; read only after a
  change since 0.9.41), the HUD's idle work (letterbox, subtitle and prompt lookups every
  frame once used, 50-150 us; skipped since 0.9.41), the log flushed on every line (every
  3 s since 0.9.39). Final figures, detailed log off (0.9.44-0.9.49): the main-thread task
  16-35 us a frame outside conversations, 50-310 in them (mostly the HUD: subtitle 45-120,
  bars 10-45, dialogue options 15-50), the scan 60-90 us at 10 a second, the conversation
  camera about 20-24 us, the filming camera 8; no spike over 5 ms but the INI read after an
  edit (15-20 ms, once).
- **The HUD's parts** (0.9.40, Hud.cpp `Part`/`PartScope`/`CostReport`): Update timed by part
  (ready check, HUD hiding, prompt, testing messages, conversation bars, filming bars,
  subtitles, dialogue options). Found the idle work above.
- **The main-thread task's parts** (0.9.43): the cost log, the camera mode log and ship check,
  waiting for the camera thread's lock, bystanders, filming, the preset key, the INI check,
  lines and face, focus, the conversation's upkeep, your visibility. Found the last spikes
  were the INI read after a settings-page edit; everything else under 3 ms worst.
- **Exit frame comparisons** (0.9.38, third person 0.9.45; `CompareExitFrame`): the game's
  view on its first frames after the hand-back (three in first person, a sample over the
  second in third) against the view drawn, in degrees across and up and metres, with your
  body's turn and movement. Found the first-person exit shift (your view 0-9 degrees off your
  body's heading, fixed 0.9.40), the third-person swing (your view orbited 20-24 degrees off,
  0.6 m, fixed 0.9.46) and the third-person camera's aim (-1.14 degrees, still learned at each
  exit), and that your body never turns or moves at an exit.
- **First-person exit frame logs** (0.9.2-0.9.5, `LogExitFrame`/`LogExitFrames`, and
  `BeforeExitUpdate`'s angle and root lines): the reported camera, aim and lens on the exit's
  first frames against your view. Found the -68.13 degree "just entered" pitch (the entry
  step now skipped; that line stays with the detailed log).
- **Start frame logs** (0.9.5-0.9.6): "Start frame N (mode)" (the reported root against your
  view on each of the first six frames of the first twelve conversations), "Frame N (mode,
  rebuild stage)" with the camera root's description (`Camera::DescribeRoot`), the predicted
  dialogue camera ("placing against the predicted dialogue camera", "now reports its dialogue
  camera: m from the NPC's head...", "its aim is ... degrees"), "Dialogue root facts" and
  "Dialogue return +N ms" (the body rebuild's return to the dialogue camera,
  `LogRootAgainstPrediction`). Found the prediction (2.02 m from the NPC's head, 0.07 m above,
  aim 2.75 degrees below), still learned at each start, silently now.
- **The menu layout dump and search** (`bDumpMenus`, 0.6.3; `Dump`, `Search`, `DumpTree`,
  `SearchTree`, the options list and persuasion meter dumps): the dialogue menu's and the
  HUD's display lists once per session, searched for subtitle and option clips. Found every
  element the HUD code uses (Menu_mc.BottomCenterGroup_mc.SubtitleText_mc, DialogueList_mc,
  PersuasionData_mc.ScoreMeter_mc, RolloverWidget_mc); cost a 4 ms hitch at the first
  conversation.
- **Your line's record** (0.7.1-0.7.3): your voice data's dialogue record and subtitle at each
  of your lines, looking for expression tags on your lines. Found none: the game never tags
  your lines (your face is the mod's).
- **The tally of your lines** (0.9.12, Hud.cpp `TallyChoice`): every option offered and the
  pick, appended to `CinematicConversationCameraSF_YourLines.tsv` with the detailed log on.
  The data is kept: docs/research/your-lines-tally.tsv, summed up in docs/EMOTIONS.md.
- **Scene action dumps** (0.9.20, `DumpObject`): the raw first 0x80 bytes of the first Camera
  and Timeline action of each scene. The action counts per scene stay in the scene check's
  log line (for the post-1.0 per-scene camera rule).
- **The tick counter** ("Per-frame task: N ticks on the main thread, M on other threads"):
  where SFSE runs the per-frame task (0.3.9's crash on a job thread). The main-thread check
  stays.
- Kept with the detailed log, for bug reports: shot choices and reasons, speaker changes,
  line emotions, scene checks, filming decisions, obstacle checks, camera mode changes, the
  nearby-people summary every 10 s. The INI read after a change is logged always.

## Old setting names

- **`iCuttingPace`** (0.9.8 to 0.9.25, whole steps −2 to +2): `fCuttingPace` since 0.9.26 (half
  steps) and `fCuttingPace<Preset>` since 0.9.27. Read as a fallback until 0.9.30.

## Measurements and dumps (general)

- **`DumpObject`** (kept, for the scene actions only): raw contents of a game object, 16
  values a line, guarded. Removed uses:
  - Gameplay first and third person, once each: the player camera (0x188 and a second part
    0x400 from +0x188), the current camera state (0x340), the render camera (0x400).
  - The first 12 conversations, frame 1 and one steady frame (90 or later, after any
    rebuild): the player camera (both parts), the dialogue state (0x340), the render camera.
    The 0.3.25 dumps found the 90.0 at dialogue state +0x2A8.
- **The position scan** (`ScanPositions`, `LogPositionCandidates`; no caller left by 0.9.29):
  every 12-byte float triple within 2.5 m of a point (up to 24), to find where the dialogue
  camera keeps its own position.
- **The settings scan** (`LogCameraSettings`, once after the first hook call): every game
  setting in `INISettingCollection` and `INIPrefSettingCollection` whose name contains `fov`,
  `dialog`, `camera`, `zoom`, `dof`, `focus`, `depth`, `bokeh` or `blur`, with its value
  (the dialogue camera's `fDialogueCamera...` settings among them; `[GameSettings]` in the
  INI can still change any of them while the game runs).
- **`FormLabel`**: "form type T id X 'EditorID'" for any address that is a registered form.
  No caller left.
