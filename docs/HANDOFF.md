# Handoff notes

For picking this project up in a new Claude session (for example one linked to the gaming PC).
Updated at milestones (each minor version) and whenever a test changes what we know. No personal folder paths here:
the repository is public.
Read README.md (plan), CHANGELOG.md (what each version did) and CREDITS.md first.

Last updated: 0.9.12 (2026-11-17).

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

## Where things stand (2026-11-17, 0.9.8)

- Starts: the first frame is placed against a prediction of the game's dialogue camera,
  fitted in the 0.9.5 distance test: on the line from the NPC's head through your feet,
  2.02 m from the head (less when something's in the way), 0.07 m above it, aimed 2.75
  degrees below the head; learned further from each start. Clean in the 0.9.6 clip. The first
  conversation after a load can take 2-3 frames to switch (known).
- First-person exit, one frame looking steeply up: caused by the direct body rebuild (the
  first first-person update runs with your third-person body and sets pitch -68.13). Vanilla,
  rebuild off and the camera-cycle method are clean; the camera cycle flashes at starts.
  0.9.8 `iExitMode=5` holds that update back one frame (`iExitSkipFrames`): to confirm.
- Third-person exit: one in-between frame, as vanilla. 0.9.8 `iExitModeThirdPerson=2` puts the
  camera root on your view before the third-person camera's first update: to confirm.
- Cutting styles retired; `iCuttingPace` (-2..+2) nudges the preset's own rhythm.
- Sitting down/standing up on stools: the mod doesn't touch the camera (furniture state).
- Still to do in 0.9.x: the long Sarah test for CCC SF preset tweaks (below), the bar-stool
  and filming tests, Vasco, the SFSE Menu Framework page, UI-mod compatibility, and the
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
- After 1.0, planned (see README "After 1.0"): Gameplay Options (below); objects the obstacle
  check can't see; more of the text of your lines for your face (high-confidence cues from a
  tally of real options, English only, own setting; moved from "maybe" to planned by the
  user, 2026-11-17; the tally started in 0.9.12: Hud.cpp appends every offered option and
  the pick to `CinematicConversationCameraSF_YourLines.tsv` in the log folder, columns time,
  panel, chosen, how the pick was told apart, text; only while bDebugLog is on). Mulling, not decided: traits setting your face's temperament (trait
  unlocking mods, precedence among several traits, a setting).
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
  testing, then if time allows quest-scene recognition. The testing messages and the
  detailed log stay ON by default until the very last step before 1.0 (the user's call:
  they keep evaluating with them).

## CCC SF preset tweaks, to confirm with a long Sarah test (2026-11-16)

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
- Large speakers (Vasco) can block you in over-the-shoulder shots: size the near-body check
  from the actor's bounds.
