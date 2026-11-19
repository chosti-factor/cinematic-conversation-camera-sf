# Handoff notes

For picking this project up in a new Claude session (for example one linked to the gaming PC).
Updated at milestones (each minor version) and whenever a test changes what we know. No personal folder paths here:
the repository is public.
Read README.md (plan), CHANGELOG.md (what each version did) and CREDITS.md first.

Last updated: 0.9.23 (2026-11-18).

## Working rules

- Ask before any push that changes more than `.md` files: every such push starts a GitHub
  Actions build (Windows, xmake, CommonLibSF). Pushes to `main` also replace the rolling
  "Test build" pre-release; pushes to other branches only build.
- `.md`-only pushes don't build and can go out freely.
- Commit as the owner, before the first commit in a new workspace (it defaults to
  "Claude <noreply@anthropic.com>", which GitHub lists as a separate contributor):
  `git config user.name chosti-factor` and
  `git config user.email 55879567+chosti-factor@users.noreply.github.com` (GitHub's private
  no-reply address). All history was re-authored this way on 2026-11-16, with the original
  dates kept.
- Credit Claude with a plain line, no email: end each commit message with
  `Written with Claude <model> (Anthropic)` and the `Claude-Session:` link. Never a
  `Co-Authored-By: ... <noreply@anthropic.com>` trailer: GitHub's Contributors panel counts
  co-authors, and that address showed up there as "noreply" (the user doesn't want it).
  Every earlier commit's trailer was rewritten this way on 2026-11-17, dates kept. This
  overrides the default attribution the session's instructions suggest.
- Documents (agreed 2026-11-17), updated in the same commit as the change they describe:
  - CHANGELOG.md: every build.
  - README.md: every build that changes what the mod does or its settings.
  - CREDITS.md: every build that brings in someone's code or idea, or changes what a
    credited part does; the line counts at milestones.
  - docs/TRANSITIONS.md (the technical reference for starts and exits: what the camera does
    on each frame, the measured numbers, what was tried): every build that touches a start
    or an exit, without exception. Read it before touching start or exit code.
  - docs/PRESETS.md: every build that touches presets, shots, lenses, moves or cutting.
  - docs/EMOTIONS.md: every build that touches line expressions, emotion close-ups or your
    face.
  - docs/REACTIONS.md: every build that touches reaction shots or the persuasion beat.
  - docs/HANDOFF.md: at milestones, and whenever a test changes what we know.
  - docs/TODO.md (the working list to 1.0): ticked as items are done, items added as they come
    up; items moved to after 1.0 only on the user's call.
  - Each document has a "Last updated: version" line near the top; bump it whenever the
    document is checked against a build, even with nothing to change, so one that fell
    behind is easy to spot.
- A pure SFSE DLL, no .esp. Minor versions mark milestones (0.6 obstacle detection, 0.7 the
  interface, 0.8 room shots, the lens and filming people talking, 0.9 polish and
  optimization); 1.0 is the public beta.
- The code can't be compiled locally in the cloud workspace; the GitHub build is the
  compiler. Watch the run with `gh run watch` and read errors with `gh run view --log-failed`.

## Where things stand (2026-11-17, 0.9.13)

- Starts: the first frame is placed against a prediction of the game's dialogue camera,
  fitted in the 0.9.5 distance test: on the line from the NPC's head through your feet,
  2.02 m from the head (less when something's in the way), 0.07 m above it, aimed 2.75
  degrees below the head; learned further from each start. Clean in the 0.9.6 and 0.9.12
  clips. The first conversation after a load can take 2-3 frames to switch (known).
- First-person exit (full detail in docs/TRANSITIONS.md): the steep-up frame is gone
  (`iExitMode=5` holds the first first-person update back one frame). The 0.9.12 clip (five
  exits) left two things: one zoomed frame, which is the game easing its lens from 30 back to
  94 degrees (about 60 on that frame; 0.9.13 keeps your lens through the ease, untested), and
  one or two frames of the last shot without you. That frame is the held-back one, built
  before our hook runs, so it can't be drawn over; 0.9.14 adds `iExitStyle` to compare two
  ways round it (2: cut back to your view 0.4 s after the last line, default; 1: a closing
  shot of the NPC alone). The 0.9.14 test: style 2 "far superior" (the user; the default).
  Its clip: the hand-back is clean; the camera move lands a frame after the body, blur and
  lens changes during the hold, so 0.9.15 stages the cut (`bExitCutStaged`). The 0.9.15
  clips: the order varies by a frame from exit to exit (one exit lens first, another camera
  first), so 0.9.16 makes the cut order-proof (lens with the camera and the game's lens
  update kept off; first person 12 cm ahead easing back, body hidden from frame 3; blur off
  from frame 2).
- The F1 menu mid-conversation (0.9.14 test): the game's camera stops, so the 250 ms rule
  ended the conversation, and a restart on the same dialogue left the options list invisible
  for the session (its alpha stuck at 0 in the persistent dialogue menu). 0.9.15: the rule
  waits while `Menu::IsOpen()`, and the list's alpha is written each time it's shown. The exit lens fix only runs from 0.9.15 (the lens update isn't called
  outside conversations).
- Third-person exit: one in-between frame. 0.9.13's lens fix applies there too; whether
  vanilla has a frame like it waits on the true-vanilla test (mod disabled in MO2).
- In-game settings page (SFSE Menu Framework) and the tally of your lines: 0.9.12, tested.
- The 0.9.17 session: starts and exits clean ("probably good for release", the user; smoother
  transition styles after 1.0). A crash before talking to Vasco was Favorites Banks
  (FavoritesBanks.dll on the game's IO thread), not this mod.
- Cutting styles retired; `iCuttingPace` (-2..+2) nudges the preset's own rhythm.
- Sitting down/standing up on stools: the mod doesn't touch the camera (furniture state).
- Still to do in 0.9.x: the long Sarah test for CCC SF preset tweaks (below), the bar-stool
  and filming tests, Vasco, UI-mod compatibility, and the
  last step (testing messages and debug log off).

## Earlier (2026-11-16, 0.8.17)

- Sway test over: CCC's presets on a tripod (0), Frontier 50, Constellation 70 plus a
  handheld wander at 0.5, Nebula 40, Starborn 0; the sway key is gone.
- Your side's angle now counts your turns (the player is silent; it never counted before).
- Chosen option: read by its highlight or the list's selectedIndex (all options are at full
  opacity, so 0.8.16 always took the first); persuasion panel options read too. The log's
  "HUD: chose ..." line lists every option's state: check it in the 0.8.17 test.
- Persuasion beat on every round's answer (the verdict comes while the panel is open);
  seated people get it in every preset. CCC's presets keep it (CCC ships it on).
- Seated NPC, you standing: the plain shot stands wider and lower behind you.
- Filming others: ends on a move only after you've stopped.
- To test in 0.8.17: tagged choices and persuasion close-ups, seated framing, your angle
  changing in Standard/Starborn/Constellation. Waiting on the user's footage of the Neon
  ranger fading in before any change there.

## Earlier (2026-11-15, 0.8.14)

- Lenses settled: each angle has its own, CCC 1.5.0's numbers as written (CCC's exported
  presets; two Skyrim clips measured: close-ups 26%/23% and extreme close-ups ~59% of the
  picture's height, as CCC's numbers give in Starfield's measure). CCC 1.5.0's shot table,
  read from SceneDirector.dll (a switch of stores at file offset ~0x9e270), matches ours.
- CCC's presets keep CCC's vision (no reactions; emotion close-ups only in Close, strong
  lines); CCC SF's presets rebuilt with film flavours (Frontier space western, Starborn
  Kubrick/Interstellar, Constellation documentary, Nebula trailer): lenses, moves, cutting,
  sway (`iCameraSway=-1`) and Starborn's centred framing per preset.

## Earlier (2026-11-15, 0.8.10)

- 0.8.10 finishes 0.8.x, filming people talking: one scene at a time (nearest first, then
  the next), followed into the game's next scene with the same people, handed back when it
  repeats (two lines in a row already filmed), group shots with three or more, the
  letterbox on HUDMessagesMenu, no prompt with bAutoListen. From first person the game goes
  to third person while it films (the first-person body can't be hidden:
  "Player1stPersonBody" is a label, not a node).
- Conversation-start flashes: "extremely minimal" after 0.8.10 (tester, 2026-11-15); left
  as they are, polished only if there's time before 1.0.
- Next, 0.9.x polish, earlier first in line: conversation-start transitions (flashes). 0.8.10 sets the
  blur focus on the camera thread on the frame of each cut (it lagged a frame or two on the
  main thread, and the opening frame had the game's 1.5 m focus); if flashes remain, get a
  60 fps clip of a start.

## Earlier (2026-11-14, 0.8.0)

- 0.8.0 is the current Test build, not yet tested in game:
  - The conversation lens (`fConversationLens`, 45): the game's camera FOV update
    (Starfield.exe +BCB990, ID 64682) passes `fDialogueCameraFailsafeFPFOV` (30) to
    PlayerCamera's set-FOV (+1E92650, ID 113443; writes PlayerCamera +0x280 and the
    frustum) every frame. THook call-site hooks at its three callers (IDs 64682 +0xF43,
    113316 +0x1FD, 113488 +0x498) swap the value while the mod films. Shots are framed for
    the lens via `Director::LensDegrees()`.
  - Room shots: MeasureRoom (16 bearings, ceiling from the cast's safe point) gives each of
    Wide/Master/Distant an angle on the conversation's side, a distance and a ceiling
    limit; `Shots::Solve` places them from that (`Input::roomAngle/roomDistance/roomRise`).
  - Filming people talking near you, first step (`[Overhear]`): prompt, listen key (N),
    `FilmFrame()` applies the render camera from the first/third-person update hooks.
  - Your face (`src/Face.cpp`): ChangeAnimFaceArchetype (ID 100510); background from the
    perk-rank lookup behind HasPerk (ID 101224); tables in docs/EMOTIONS.md.
- Earlier: the game's high-process actor list is at ProcessLists +0x60, not CommonLibSF's
  +0x58 (reported upstream, libxse/commonlibsf#57, confirmed by another modder; the user
  will offer a PR if it isn't fixed by about 2026-11-28).
- Actor::IsInCombat is vtable slot 0x16B (+0xB58) in 1.16.244, not CommonLibSF's 0x16C
  (Papyrus natives at RVA 0x1F1F690 and 0x1FC6DE0); GetCombatGroup likewise looks one
  slot early (+0xAF8). Don't trust CommonLibSF's Actor virtual indices near there.
- Crash lessons: don't read the lower-process actor lists or the cell's references (0.7.9
  and 0.7.10 crashed on load); only high-process actors are asked anything, guarded.

## Next, with access to the gaming PC

1. Done: the actor list is at ProcessLists +0x60 (count) / +0x68 (handles) in 1.16.244,
   found in Starfield.exe (loads of the singleton, RVA 0x5FD9B88, ID 937584). The exe is on
   the gaming PC (the Steam install folder); the Address Library is in Mod Organizer's mods
   folder ("Address Library for SFSE Plugins"). With pefile
   and capstone (pip through the proxy: NO_PROXY=localhost) code can be read directly.
2. Read test logs straight from `Documents\My Games\Starfield\SFSE\Logs` on the gaming PC, and crash
   logs from Mod Organizer's `overwrite\SFSE\Plugins` folder.
3. Then: NPC-to-NPC filming (prompt / seated / standing still triggers, see README),
   room shots with Starfield framing (CCC's need 18-27 m at this 30 degree lens), your
   character's face (ChangeAnimFaceArchetype at RVA 0x18E0220; credit rrrafau), polish.

## Release plan

- 1.0 public beta on Nexus: moved (2026-11-15) from Friday 2026-11-20 to Friday 2026-11-27
  (Thanksgiving weekend), 9-11 am US Central (15:00-17:00 UTC), or Wednesday 2026-11-25 if
  that serves views better (decided closer to the date). Polish first: conversation-start
  transitions (flashes) are the priority. Features frozen by Tuesday, testing
  Wednesday, Nexus page and a 60-90 s clip Thursday. Reddit (r/starfieldmods) the Saturday
  after, 9 am-1 pm Central. Outreach a few days before (machinima podcast, Starfield mod
  YouTubers, the Starfield Modding Discord). Be stable well before the Steam Winter Sale
  (2026-12-17).
- 0.9.x: an in-game settings page through SFSE Menu Framework (QTR-Modding, Nexus 18201,
  github.com/QTR-Modding/SFSE-Menu-Framework; a port of SKSE Menu Framework, targets
  1.16.244, xmake, C++23). Plugins use its MIT SDK (SFSE-MCP, single header or vcpkg) and
  register pages at kPostLoad; see its example mod repo. Optional: the INI stays the
  source when it isn't installed.
  The user has the latest from Nexus (2026-11-16). SDK HEAD then: ea48af1 (2026-10-29); the
  example pins abeee13. The SDK checks the host's exports at run time
  (GetMenuFrameworkAPIVersion), and its header pulls in clib-utils-qtr's Signing headers
  (Authenticode check of the host DLL): vendor both, MIT, with their notices. Settings
  changed on the page go to the _user.ini (read before every conversation).
  Built in 0.9.12 (untested): `src/Menu.cpp`, a table of about 45 curated keys
  (page, heading, section, key, label, kind, range, help). Kinds: bool, tri (-1 preset's own /
  0 / 1), int, float, own-int and own-float (-1 = preset's own, a checkbox then a slider),
  zero-float (0 = preset's own), choice (sPreset, sFocusStyle). Values come from the _user.ini
  if set there, else the main INI, re-read every 2 s; writes go to the _user.ini at once
  (sliders on release) and Reset deletes the user key. A preset choice clears the preset key's
  override. Registered at SFSE kPostLoad; returns quietly without the framework. The SDK is in
  `extern/` (xmake adds its include dirs and `_SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING`).
  Keep its ranges in step with Settings.cpp's clamps (the lens is 20-70).
- After 1.0, planned (see README "After 1.0"): smoother start and exit styles (the user,
  2026-11-17: 1.0's clean cuts are good for release); large speakers measured from the
  actor's bounds rather than head height (the user, 2026-11-18); Gameplay Options (below); objects the obstacle
  check can't see (plants by nearby objects' bounding boxes along the camera-to-face line,
  like the bystander check; planned by the user, 2026-11-17); more of the text of your lines for your face (high-confidence cues from a
  tally of real options, English only, own setting; moved from "maybe" to planned by the
  user, 2026-11-17; the tally started in 0.9.12: Hud.cpp appends every offered option and
  the pick to `CinematicConversationCameraSF_YourLines.tsv` in the log folder, columns time,
  panel, chosen, how the pick was told apart, text; only while bDebugLog is on). Mulling, not decided: traits setting your face's temperament (trait
  unlocking mods, precedence among several traits, a setting); the game's sitting camera
  (swings round to your face sitting down and standing up, vanilla too, not stopped by
  `bDisableFurnitureCamera`; whether to steady it, outside conversations); walls the sphere cast passes through (Tuala at the MAST) by a navmesh
  check (Starvegas's walkable-floor snap). The navmesh was looked at for 1.0 and not adopted
  for now (user, 2026-11-17): it can't see plants either, rejects good spots over counters
  and tables, and its functions aren't mapped in CommonLibSF.
- After 1.0: most settings in the game's own Settings menu as Gameplay Options (GPOF records,
  added to the game in 2024, shown when in a Gameplay Option Group), from a small (light)
  ESM; the plugin reads what each sets (to find: globals or otherwise). Likely choices, not free
  values, so presets of each setting; the INI and the SFSE Menu Framework page stay.
- Done in 0.9.0: `iCloseUpDistance` removed; old INI names and `sPreset=Stardust` no
  longer read.
- Final polish once feature complete (agreed 2026-11-17), in this order, each followed by a
  build and a test round:
  1. Redundancy pass: remove code no longer in use and remnants of earlier versions
     (finished experiments such as the FOV experiment switch and exit modes 0-4 if 5 holds,
     the exit value search and object dumps, test switches). First, so the later passes
     review less code and dead paths can't hide bugs.
  2. Debugging pass: every INI setting checked against what its comment says (bEnabled and
     iMinLines were both wrong for a while); edge cases: a save loaded or a menu opened
     mid-conversation, ships, scenes, combat, death, fast travel, conversations back to back,
     the game paused; thread safety and lock order; anything that could crash.
  3. Optimization pass: measure first (time spent in each hook per frame, per conversation,
     with the debug log off); then the scans (nearby actors 10 times a second, bystanders 4),
     per-frame string formatting and logging, allocations, obstacle casts per cut.
     After debugging, so only correct code is tuned, and tuned code gets re-tested.
  4. INI and defaults review: last, since the passes above may remove or rename settings;
     comments, defaults, the _user.ini, the settings page matching the INI.
  Then the very last step (testing messages and debug log off), then a full test round on
  the release candidate.
- 0.9.x order agreed 2026-11-16: seated conversations (start jitter, side-on framing, bar
  stool body), large speakers (Vasco), the SFSE Menu Framework page, UI-mod compatibility
  testing. Quest scenes (2026-11-18): 0.9.20 only logs each scene's actions by type and
  whether it directs a camera (Camera or Timeline action; types read from the game's class
  names through the vtable's run-time type information, no address IDs); using that to keep
  this mod's camera on scenes that are just talking moved to after 1.0. The testing messages and the
  detailed log stay ON by default until the very last step before 1.0 (the user's call:
  they keep evaluating with them).

## CCC SF preset review from the 2026-11-18 run (built in 0.9.23)

Data: 0.9.21, the Lodge, about 10 minutes per preset with Muria, Sam, Barrett and Andreja.
Reaction timing is fine in every preset (the user). Room bounds limited almost nothing (every
obstacle check clear; room shots got 97-99% of the distance they wanted). Companions are very
expressive (Frontier: 54 of 74 NPC lines tagged), and runs of emotional lines held one
close-up up to 28 s in every preset.

Decisions (all built in 0.9.23; the NebulaTest preset comes out before 1.0):
- B, all presets: a cap on how long one close-up holds over a run of emotional lines, then a
  cut to a three-quarter or medium of them (or a reaction of you); a value per preset, to be
  settled preset by preset. Frontier 12 s, Starborn 18 s, Constellation 14 s, Nebula 12 s (all confirmed by the user).
- C, Frontier: listening reactions in mediums and three-quarters, not close-ups; close-ups
  of you kept for when your face shows an expression. Other presets: to discuss.
- Starborn, less Kubrick and more Interstellar (the user, all five):
  1. Sway 0 -> 30 (a touch of handheld; Frontier is 50).
  2. Faces off-centre instead of centred (look room halfway, 0.5, unless the user prefers
     full).
  3. Push-ins only on the emotion close-ups (their strength unchanged); the regular close-ups
     of them and of you held still.
  4. More room shots for scale: Distant 10 -> 25, Master 8 -> 20 (about one every few
     minutes instead of one in ten); the crane-up reveal stays on the Master.
  5. Close-up lenses one step wider: close-up of them 45 -> 50, of you 40 -> 45, extreme
     close-ups 40 -> 45 and 35 -> 40. The user isn't sold yet: to judge in the test.
- Bug, all presets (fix approved): 22 shots in the run lasted under 0.2 s (mostly one frame):
  two decisions on the same frame (a fresh angle, then a reaction or an emotion close-up at a
  line's start) cut twice. Settle the final shot before cutting.
- Constellation: the faces share (21% measured, 7% designed) is fine with the user ("dialogue
  is not a real sci-fi documentary"); a little lower at most, never down to 7%. B: 14 s
  proposed (trims the 26 s and 21 s holds; faces about 18%). No extreme close-ups of you in
  Constellation (the user: "not for this preset"; an expression reaction is a regular
  close-up). Room and full-figure share left as it is (38%).
- Your angle held for many turns (Constellation's Distant for 5 of your turns), all presets:
  a room or shared shot (Distant, Master, Wide, overhead, two-shot) stays as your angle for 2
  of your turns at most; the next turn gets another angle (for that turn; the room shot can
  come back later). Over-the-shoulder, close-ups, mediums and three-quarters keep each
  preset's rhythm (the user: room shots only).
- Nebula: no longer "a trailer" but a space opera (Guardians of the Galaxy, The Fifth
  Element, Valerian, the original Star Wars): the variety of angles is its identity, not
  trailer pacing; the user likes its pacing, so cutting stays. B: 12 s proposed (trims the
  17-28 s holds only). Extreme share of close-ups of you stays 40% (the user). Describe it as
  space opera in README and PRESETS.
- Nebula's low angles (low angle on them, low profile, low angle on you): the tilt-up sank the
  person to the bottom of the frame under the subtitles (3 cuts in the run, up to 9.5 s).
  Hybrid approved by the user: a push-in at Nebula's strength plus about a third
  of the old tilt (about 0.13 m instead of 0.4), with a guard that stops the tilt once their
  eyes reach the lower third of the frame. To judge it: a temporary "Nebula test" preset
  (the user chose this over a testing setting or [Shots] weights): Nebula with the three low
  angles making half of the angles picked; sPreset and the U key. Taken out before 1.0.
- Overhead shots: kept in Nebula as a flourish, weight 15 -> 12 each (the user). In every
  preset, the overhead of you is never used for a reaction (the user; old proposal 1).
- Bug, all presets: the close-up of you for a tagged choice ([Commerce] in the run) came one
  turn late, at your next reply (the player is silent, so the NPC answers at once), and only
  as long as you took to pick (2.75 s and 2.05 s), a flick to your face and back. Fix (the
  user, option 1): the close-up of you right when you pick, about 2 s over the start of their
  answer, then their shot; with a skill check, then the persuasion beat.
- After 1.0, Mulling (the user): a new angle of you once you've spent about 10 s choosing a
  reply (CCC has a choosing-reply timer, off by default).
- Scrapped: A (no mild-tier emotion close-ups in Frontier: it would narrow the range of
  emotions) and D (Frontier's two-shot of you for one turn: not seen again).

## CCC SF preset tweaks, proposals only, waiting on the long Sarah test (2026-11-16)

Not to be built until the user has seen the data and decided on each one. 0.9.19 built them
without that decision (a mistake); 0.9.21 took them back out, so the presets are as in 0.9.18.
The proposals stay on the table (the user, 2026-11-18): the data comes from the next test,
long conversations with Sarah, Sam and other quest NPCs through the presets as they are.

Proposed from partial data (0.8.16 Constellation/Nebula extracts, 0.8.17 your-side shots);
the user wants a long conversation with Sarah through the CCC SF presets first, then
confirm or change each:
1. Reactions never on overhead shots of you (Constellation reused PlayerOverhead for three
   reactions in a row: reads as surveillance).
2. Constellation: over-the-shoulder weight 30 -> 15 (a documentary rarely shoots over a
   shoulder; more room for Distant, Master, Long).
3. Extreme share of your reaction close-ups per preset: Constellation 0, Starborn 40,
   Frontier 40, Nebula 50 (now 40 everywhere).
4. Frontier: a two-shot as your angle for one turn only, then back to a single of you
   (it held one for four of your turns; Constellation keeps long holds).
5. Starborn and Nebula: nothing seen to change.
(6, keeping the previous session's log, was done in 0.9.1.)

## Open observations from testing

- Many lines are emotion close-ups (0.8.3: every expression but Neutral and Friendly, at
  50%; reactions may interrupt them after their minimum time), which take priority over
  reactions; maybe let a reaction win now and then.
- Large speakers (Vasco): 0.9.19 keeps the shots of you to 38 degrees or wider when the
  speaker's head is 1.70 m or more above their feet (Vasco 1.75; the 0.9.18 clip: hidden at
  22-27, half at 34, clear at 40). The user (2026-11-18): the height rule stays for 1.0 but
  must be REPLACED after 1.0, since height is only part of the problem: robots are wide and
  bulky. A check from the actor's bounds is planned after 1.0
  (CCC 1.5.0's Keep Subject Visible does it that way: every actor's head and torso as capsules,
  bulkier actors' scaled by their bound against the player's). 0.9.21 test: Starborn still
  hid you, through the plain over-the-shoulder fallback, which skipped the rule; 0.9.22 swings
  it out to the same angle. Settings `fLargeSpeakerHeightNpc` (1.70, 0 off) and
  `fLargeSpeakerAnglePlayer` (38); a "Large speaker" log line names each one it applies to.
