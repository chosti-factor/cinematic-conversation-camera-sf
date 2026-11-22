# Memory: what was tried, measured and removed

Everything the 0.9.30 redundancy pass took out of the code and the INI, kept here with its
technical detail, so an idea, a measurement or a dead end can be looked up again instead of
rediscovered. The working behaviour is described in [TRANSITIONS.md](TRANSITIONS.md) (starts
and exits), [PRESETS.md](PRESETS.md), [REACTIONS.md](REACTIONS.md) and
[EMOTIONS.md](EMOTIONS.md); the full story of each version is in
[CHANGELOG.md](../CHANGELOG.md).

Last updated: 0.9.49 (2026-11-22).

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
