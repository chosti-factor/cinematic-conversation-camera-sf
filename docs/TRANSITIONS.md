# Starts and exits: how the camera is handled

Technical reference for how a conversation's camera starts and ends, with the numbers
measured in testing. Kept up to date with every build (last updated: 0.9.46, 2026-11-22).
Version notes in brackets say when something was found or changed; the full story is in
[CHANGELOG.md](../CHANGELOG.md).

0.9.30 (the redundancy pass): the test switches named below are gone from the INI and the
code, each fixed at the value this document calls the default: `iStartMode` and
`iStartModeThirdPerson` (1), `iExitMode` (5) with `iExitSkipFrames` (1),
`iExitModeThirdPerson` (1), `bExitLens` and `bExitCutStaged` (on), `iBodyRebuildMethodPlayer`
(1, the direct refresh, with the camera cycle as its automatic fallback),
`iBodyUpdateFlagsPlayer` (63). `iExitStyle` keeps 1 and 2 (0, the last shot to the end, is
gone; seated in third person still ends that way). What the other values did, and why they
were dropped: [MEMORY.md](MEMORY.md).

## The pieces

### Camera modes (the game's camera states, as logged)

| Mode | State |
|---|---|
| 0 | First person (also iron sights) |
| 20 | Third person |
| 17 | Dialogue (the game's conversation camera) |
| 6 | Furniture (sitting) |
| -1, -2 | Loading, menus |

### Hooks

- **Dialogue camera update:** the mod's shots are placed here, after the game's own update
  each frame. This hook drives the whole conversation.
- **First-person and third-person camera updates:** used only around exits, and for filming
  people talking. Everything else in them passes straight through.
- **Lens:** the three places that hand the field of view to the player camera's set-FOV
  function (Address Library IDs 64682 +0xF43, 113316 +0x1FD and 113488 +0x498; set-FOV is
  113443, which writes PlayerCamera +280 and the render frustum). While a conversation is
  filmed, the value is swapped for the shot's lens. The game asks for 29.6 degrees
  (`fDialogueCameraFailsafeFPFOV` = 30); gameplay is 94 degrees. Outside conversations the
  hook remembers the lens the game asks for in first person (mode 0) and third person
  (mode 20), for the exits (0.9.13).

### How a shot is drawn

- **Render camera only.** The mod moves the render camera relative to the camera root (the
  game's dialogue camera node). The root stays where the game puts it, so NPCs keep facing
  you.
- **Composition:** render camera world = local × parent (checked on the first frame of every
  conversation).
- **Lens on the render frustum:** right = tan(lens / 2), top = right / aspect.
- **Frame timing (0.8.x–0.9.x):** the picture for a frame is largely set during the game's
  own camera update. A write after that update may not reach that frame. So a cover drawn
  after an update to hide that update's bad frame never works (0.8.18, 0.9.0, 0.9.7 all
  failed). What does work: changing what the update starts from, or not running the update.
- **Cuts (0.9.51-0.9.52):** the camera, its lens and its focus each reach the screen on their
  own time. The focus (the conversation blur records) is read when the game draws: written on
  the cut's frame it showed on the old shot's last frame now and then (0.9.50 clips), written a
  frame later on the new shot's first (0.9.51 clips). Neither timing is fixed, so a cut's
  frame gets **no blur** (right for both shots) and the new focus a frame later (0.9.52); not
  when the old focus does for both. The lens through the game's lens update or its set-FOV
  came a frame late now and then (the new shot's first frame zoomed about the centre); since
  0.9.52 it's also written on the render camera beside the camera's position, scaled to the
  game's maths for a lens (learned from a steady shot). The main thread writes neither the
  lens nor the focus while the mod films your conversation.

## Starts

### Frame 1: the game still reports its gameplay camera

- **The switch:** on the conversation's first frame (rarely the first two or three), the game
  still reports the gameplay camera's position, while the picture is drawn from where its
  dialogue camera is about to be.
- **How long:** one frame normally. Two or three on the first conversation after loading a
  save (known: one frame from a different angle).
- **The prediction:** the mod places the shot against where the dialogue camera will be.
  Fitted from ten starts at 0.75–3.6 m, first and third person, seated, tall and raised NPCs
  (0.9.5 test):
  - **Position:** on the line from the NPC's head through your feet, **2.02 m from the NPC's
    head**, and **0.07 m above it**. Your distance to them doesn't matter.
  - **Shorter when blocked:** 1.76–1.84 m when something is in the way (a tall NPC, a
    stool).
  - **Aim:** **2.75 degrees below** the line from that spot to the NPC's head (measured
    −2.7 to −2.8 in every start), and 0 degrees across.
- **Learning from each start:**
  - **Distance and height:** a quarter weight, only when within 0.12 m and 0.08 m of the
    current value (a camera pulled in by something says nothing about the next start).
  - **Aim:** a half weight, only within 20 degrees.
- **Accuracy:** 0.9.6 test, every start within 4 cm and 0.1 degrees.
- **Before 0.9.6:** an offset from your eyes, up to 1.9 m off when you stood close or far.
- **First frames through the shot's lens (0.9.4):** while the game's frustum is still the
  gameplay one, the render frustum is set from the shot's lens.
- **Settings:** `iStartMode` (first person), `iStartModeThirdPerson`, both 1: the first shot,
  placed against the prediction. 0 = your view first; 2 = your view, placed the same way.

### From first person: your body

- **Why:** the game shows only the first-person body (arms, no head), so the mod asks for a
  model refresh while the dialogue camera is up, and the game builds the full body.
  - The call: `Set3DUpdateFlag(63)`, `iBodyUpdateFlagsPlayer`.
  - `iBodyRebuildMethodPlayer=1`, the direct method.
- **When it counts as built:** sampled every 12 ms, three steady samples (head moved under
  5 cm), and:
  - head ≥ **1.58 m** above your feet (standing); or
  - **seated:** the head started below 1.33 m (bar stool: 1.09 m first-person) and rose
    more than 0.2 m (full body 1.55 m); or
  - **picked up again (0.9.10):** within 5 s of your body last being built, at the same
    height ± 0.08 m. For a conversation the mod ended and restarted, such as buying a drink,
    which paused the game's dialogue camera for about a second.
- **Timing:** usually built **109–145 ms** after the request. Asks again at 1.2 s, 1.5 s and
  2.5 s if not. Falls back to the camera cycle at 2 s (4 s in a background scene).
- **Meanwhile:**
  - You're hidden.
  - The opening shot is a clean single of the NPC: three-quarter, medium or medium profile;
    in Close and Starborn, close-up or three-quarter, 50/50.
  - You're shown only from the first cut after your body is built, never mid-shot (0.9.4).
- **The camera cycle** (`iBodyRebuildMethodPlayer=0`, fallback): first person → third person
  → dialogue camera. It shows on screen (0.9.7 test): two frames of a close-up of the NPC's
  torso, then about 0.3 s of your own view. Not used by default.
- **The rebuild's side effect at the exit:** see the first-person exit below.

### From third person

- **The opening:** placed against the prediction from frame 1 (`iStartModeThirdPerson=1`,
  0.9.4).
- **No close-ups** in the opening, except in Close and Starborn.

### Shot choice at the start (all cases)

- **Seated NPCs** (head under `fSeatedHeadHeight` = 1.35 m above their feet) and
  **merchants** (vendor faction) get a plain over-the-shoulder shot instead of the shot
  library.
- **Seated NPC, you standing:** the over-the-shoulder stands 0.35 m behind, 0.6 m to the
  side, 0.25 m lower (`fOverSeated*`).
- **Starts are otherwise the same for standing, seated, merchant and bar-stool
  conversations.**

## During: the obstacle and bystander checks

- **Obstacles:** the game's own sphere cast, 0.05 m wide (as the game's dialogue camera).
  Plants and some props have no collision, so the cast can't see them (post-1.0).
- **Bystanders:** people who haven't moved 0.25 m in 1.5 s (`fBystanderStill`), within 15 m.
  One blocks a shot when either:
  - their body (from 0.3 m above their feet to 0.2 m above their head) comes within 0.32 m
    of the line from the subject to the camera; or
  - they fill the picture: their middle within 1.7 m of the camera, or (0.9.10) their head
    within 1.2 m, and within 0.30 rad (17 degrees) of the view axis. The head test is for
    seated people, whose middle is out of the picture while their head is right in front of
    the lens.
- **When:** checked when a shot is chosen, not while it's held.

## Exits

### When the conversation ends

- **The hold:** after the last line ("Speaker gone"), the game keeps its dialogue camera for
  about **1.5 s** while the dialogue menu closes (letterbox and subtitles go away): measured
  1.50-1.56 s in five exits and 0.75 s in one (0.9.12 logs). The mod holds the last shot
  through it, up to 3 s, until the game leaves its dialogue camera (`fExitHoldSeconds`, a minimum of 0.5 s, had no effect in practice and is gone since 0.9.35).
- **The exit style (0.9.14, `iExitStyle`), during that hold:**
  - **2 (default; the user's pick after the 0.9.14 comparison): cut back to your view** `fExitCutSeconds` (0.4 s) after the last line:
    your saved first- or third-person view (`ShowGameplayView`), your gameplay lens (the
    lens hook's waiting value: the game's lens at the conversation's first frame), the blur
    off (shot distance 0). From first person your body is hidden first (the game's own
    first-person conversation view has none). When the game takes over, the picture already
    matches it. Logged: "Exit style 2: cut to your ... view".
  - **1: closing shot of the NPC alone**: if the shot on screen shows
    you (or is the plain shot), the director solves medium, close-up or three-quarter on the
    NPC from the last frame it saw (locked, obstacle-checked, pulled in), with its own lens
    and focus. Logged: "Exit style 1: closing shot ...". The blur still goes off on its last
    frame (0.9.14 clip, every exit).
    - **When (0.9.15):** as the dialogue menu closes, once it has been open in this
      conversation. The menu (and the letterbox, which lives in it) closes a couple of frames
      before the game lets go of the speaker: 0.9.14 cut at "Speaker gone", and the clip
      showed the last shot two frames without its bars first. The shot is held from then
      on.
    - **Staged (0.9.15, `bExitCutStaged=1`):** the 0.9.14 clip showed, during the hold,
      the camera move landing one frame after the body, blur and lens changes: the frame
      before the cut was the last shot with you hidden (first person) or through your 94
      degree lens (third person), no blur; and in third person the first frame of your view
      came through the narrower frustum saved at the start, then 94. So on the cut's first
      frame only the camera moves (`ApplyRender`, frustum untouched); from the next, your
      body is hidden, the blur goes off, the lens hook passes your lens and the frustum is
      set from it (`SetRenderLens`, not the saved frustum). The hand-back itself was clean
      in that clip (first person: your view straight through, the HUD back, no jump).
    - **The 0.9.15 clips** (staged as above): still one frame of the last shot with you
      hidden and no blur (first person, every exit); in third person one exit had the last
      shot a frame through 94, another your view two frames through the narrow lens. **The
      order in which these reach the screen varies by a frame** (the hook runs on a
      different worker thread each frame), so no fixed staging can line them up.
    - **Order-proof (0.9.16, `bExitCutStaged=1`):** the lens is set on the render camera
      with the move (0.9.16: `SetRenderLens`; **0.9.17: the game's own set-FOV**, called
      right after `ApplyRender`, since `SetRenderLens(94)` came out about 1.8 times narrower
      than the game's 94 and widened at the hand-back), and the game's lens update
      is kept off the camera meanwhile (the lens hooks skip set-FOV while `g_cutOwnsLens`),
      so camera and lens arrive together. From first person the camera lands **12 cm ahead
      of your eyes** and eases back over **0.25 s**; your body is hidden from the cut's
      **third frame**: while it's still there the camera is in front of your face, and the
      hiding can't show on the last shot. The blur goes off from the **second frame**.
      `End()` and the exit lens fix hand the lens back to the game.
    - **Heading (0.9.40):** from first person the view cut to is turned to **your body's
      heading** (`GetAngle().z`), pitch and position kept. In first person your view can sit
      a few degrees off your body's heading, and the game takes over looking along your body:
      the NPC shifted sideways at the hand-back (0.9.38-0.9.39 logs: 0-9.4 degrees, always
      the difference between the view saved before the conversation and your body's heading;
      your body doesn't turn during one). 0.9.39 turned by your body's turn instead (none).
    - **The 0.9.16 clip:** clean at the cut in all five standing exits, first and third
      person (no teleport or flicker); starts clean. The only flaw: the zoomed view during
      the hold and its widening at the hand-back (fixed in 0.9.17 as above).
    - **Lens fallback (0.9.16):** a conversation picked up again after a menu (the bar's
      drink) never sees your gameplay lens at its start; the cut and the exit lens fix use
      the last one seen at a conversation start (0.9.15 clip: your view half a second
      through the conversation lens, then a jump at the hand-back).
  - **Seated in third person (0.9.18):** no cut to your view, and nothing drawn on the
    switch frame: the last shot stays and the game's camera takes over (with your lens put
    back). The 0.9.17 test at the bar: your saved view (from before the conversation) and
    the game's seated camera after it differ, so the cut back was followed by a second cut
    (lower, closer) a moment after the hand-back. Seated is read from your process's
    occupied furniture at the start. Seated in first person keeps the cut (clean there).
  - **0:** the last shot to the end (up to 0.9.13).
  - If a speaker comes back during the hold, the next shot resets all of it (body shown
    again by the usual rule).
- **The mod also ends a conversation when:**
  - the game leaves its dialogue camera, after 150 ms of grace for the first-person exit to
    finish; or
  - the dialogue camera stops updating for **250 ms** (a menu, such as the bar's drink).
    The conversation that picks up again counts your body as built if your head is at its
    last built height, within a minute (0.9.17; 5 s before, and 6 s in the drink menu left
    you hidden, with the exit on the older path showing your back for a few frames).
    When the camera comes back, a new conversation starts. Not while the SFSE Menu Framework
    menu (F1) is open, or for 0.5 s after it closes (0.9.15): the camera stops then too. The
    same for the menus a conversation opens over itself (0.9.36): trade, container, crew, ship
    builder.

### First person

What happens on the switch:
1. **The first first-person update after a conversation sets the camera's pitch to −68.13
   degrees** (steeply up), but only when your full body was built for the conversation:
   every exit with the direct rebuild, none with it off or with the camera cycle, none in
   vanilla (0.9.7 tests).
   - **The pitch fields:** FirstPersonState +C8 and +CC, in degrees (negative is up). The
     update writes −68.13 there (−68.1265, a near-constant).
   - **The entry flag** (+E0 = 1, +68 1.0 → 30) isn't the cause: clearing it changed nothing
     (0.9.4).
   - **Not stored anywhere:** the value wasn't found in the player camera, the camera states
     or the player (0.9.7 search). Most likely it's computed from the third-person body's
     pose.
   - **The next update** puts your pitch back.
2. **`iExitMode=5` (0.9.8, default):** that update is held back for one frame
   (`iExitSkipFrames=1`; more doesn't help, since the conversation ends during that frame).
   The update that's drawn then has your first-person body back. Measured: no steep-up frame
   in any exit since.
3. **On the held-back frame:**
   - The conversation lens goes off (0.9.10).
   - Your first-person view is drawn through the gameplay frustum (`ShowGameplayView`).
4. **The game's dialogue camera update ends the conversation on that same frame.** Since
   0.9.11, ending it keeps your view drawn and releases the render camera just before the
   first-person camera's next update.
   - **Before 0.9.11:** ending it restored the render camera, which drew one frame of the
     game's dialogue camera from your eyes through the conversation lens.
5. **The frame after: the game's lens ease (0.9.12 clip, all five exits).** The first frame
   once the game has its camera back showed your view zoomed in: the right place and aim, but
   magnified about **1.85 times** about the centre, a lens of about **60 degrees**, the same
   after every last shot. Two possible causes: the game easing its lens back from the
   conversation's (60 is about halfway from 29.6 to 94), or the camera keeping our last lens
   until the game sets it again. The vanilla clip (0.9.14 test, mod disabled) argues for the
   second: its first-person exit goes from the 30 degree close-up straight to your 94 degree
   view, with no in-between frame at 60 fps. (It isn't the held-back frame: that one already
   uses the gameplay frustum saved at the conversation's start.)
   - **Since 0.9.13 (`bExitLens=1`):** at the exit your gameplay lens is put on the camera at
     once through set-FOV (which also writes PlayerCamera +280, which the ease may start
     from), and for up to 6 lens updates or 0.25 s, the game's in-between values are swapped
     for it at all three callers, until the game asks for it itself. That covers both
     causes; the log lists each swapped value ("Exit lens: ..."), which tells them apart.
   - **0.9.13 and 0.9.14: it never ran.** It took the gameplay lens from the lens update
     outside conversations, and that update isn't called there (no "Exit lens" line in the
     0.9.14 log; the zoomed frame at every exit). From 0.9.15 the lens is the game's on the
     conversation's first frame (`startFov`) and the camera is the one the lens update sees
     during the conversation; "Exit lens: not put back" is logged if either is missing.
6. **One or two frames of the last shot with you already gone from it** (and its background
   blur gone), just before your view (your body is swapped back at the switch). This is the
   held-back frame itself: the view drawn on it never reaches the screen (built before our
   hook runs; the frame-timing rule above). So the exit styles (0.9.14) avoid having
   anything left to change on screen at the switch: style 2 is already on your view, style 1
   on a shot without you.

Older modes:
- **3:** the entry flag cleared, and the pitch and root put back after the update. That
  doesn't reach the screen.
- **4:** 3 plus your view drawn over the frame. Doesn't either.
- **1:** nothing.
- **0, 2:** older covers.

### Third person

- **The bars at the exit:** the letterbox lives in the dialogue menu, which the game closes as
  the last line ends: the bars go then, 0.4 s before the first-person cut and about 1.5 s
  before the third-person one. Stand-in bars (0.9.47-0.9.48, on the HUD messages menu until
  the cut) left a one-frame flicker as the menu closed; taken out in 0.9.49 (the user: clean
  exits over bars to the cut; post-1.0 Mulling; docs/MEMORY.md).
- **Softer during the hold (0.9.46 clip):** your view from the cut to the hand-back is softer
  and a little darker than the game's own, and sharpens on the hand-back frame (sharpness
  about 1,530 then 2,550): the game's conversation image effect stays on until it takes over;
  the mod's blur off only zeroes its depth of field. Since 0.9.48 the third-person cut is on that frame.
- **When (0.9.48, the standard after the 0.9.47 trial as `iExitStyle=3`):** from third
  person there's no cut during the hold; the last shot stays until the game
  takes over, and your view is drawn on that switch frame. The early cut left the camera still
  for about a second before the hand-back, where the 0.9.46 clip showed your character's pose
  change over three frames and the view sharpen (the game's conversation image effect going);
  vanilla moves its camera on that frame. The 0.9.47 clip: the cut frame sharp at once and
  nothing moving after it. First person keeps the early cut (pristine there).
- **Heading (0.9.46):** the style-2 cut turns your saved third-person view **round your feet**
  to behind your body, looking `g_thirdPersonAim` off its heading (-1.14 degrees in the 0.9.45
  log; measured again on the game's first third-person frame after each exit, kept if within
  6 degrees). Before it, your view from before the conversation sat orbited 20-24 degrees off
  your body's heading, and the game takes over behind your body: a 20-degree, 0.6 m swing at
  the hand-back, all on frame 1 (no ease), your body not turning or moving.
- **The switch frame:** your saved third-person view is drawn through the gameplay frustum
  (`iExitModeThirdPerson=1`, 0.9.4). The render camera goes back to the game just before the
  third-person camera's own first update.
- **Known: one frame** of the third-person camera close behind you (or to the side) as it
  eases out to its distance.
  - **The 0.9.12 clips (two exits from a bar stool) show it's the lens ease:** the in-between
    frame is your third-person view, already in place, magnified, like the first-person
    one. 0.9.13 keeps your third-person lens through the ease (`bExitLens`); untested.
  - Just before it, one frame of the last shot with its background blur gone.
  - **Vanilla has its own in-between frame** (0.9.14 test, mod disabled in Mod Organizer,
    two exits): one frame without the HUD from a wider spot across the room, then the
    third-person camera behind you. A different frame from ours, but not clean either.
  - `iExitModeThirdPerson=2` (camera root put on your view first) didn't change it (0.9.8).

## Sitting down and standing up (furniture, mode 6)

- **The mod doesn't touch it:** while sitting down (6.45 s) and standing up (3.25 s) the log
  has nothing from the mod but the nearby-people scan (0.9.12 bar test, twice each; the same
  durations each time, the game's animations). No game setting is changed (`[GameSettings]`
  is empty by default).
- **What it does:** the game's furniture camera swings round to your face from the front and
  drifts while it settles; standing up starts with one black frame (the camera inside
  something). **Vanilla does the same** (0.9.14 test, mod disabled, twice): the swing to your
  face sitting down and standing up, and standing up starts with one bright frame and about
  ten dark ones. Its settings: `bDisableFurnitureCamera:Camera` (false), `fFurnitureCameraAngle`
  (0.39 rad), `fFurnitureCameraZoom` (3.57).
- **The game's own switch,** `bDisableFurnitureCamera:Camera=1`, doesn't stop it (0.9.15 test:
  applied, and the camera still swung round to your face, from first person too).

## Off switch

- **`bEnabled=0` (0.9.11):** the whole mod stands down: shots, filming, scans, dialogue
  options and letterbox, face, blur, lens. The hooks stay installed but pass through. The INI
  is still checked twice a second and read once it changes (0.9.41; every 2 s before).
- **Before 0.9.11:** it only stopped the shots.
- **For a strict vanilla comparison,** disable the mod in the mod manager.

## How exits and starts are checked

- **Recording:** 60 fps clips.
- **Finding the switches:** each frame's average brightness, looking for jumps.
- **Checking:** strips of every frame around each start and exit, matched to the log by the
  time between conversations.
- **What to look for:** a frame that doesn't belong to either side of the cut.
