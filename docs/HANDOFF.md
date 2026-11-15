# Handoff notes

For picking this project up in a new Claude session (for example one linked to the gaming PC).
Updated at milestones (each minor version), not every build. No personal folder paths here:
the repository is public.
Read README.md (plan), CHANGELOG.md (what each version did) and CREDITS.md first.

## Working rules

- Ask before any push that changes more than `.md` files: every such push starts a GitHub
  Actions build (Windows, xmake, CommonLibSF). Pushes to `main` also replace the rolling
  "Test build" pre-release; pushes to other branches only build.
- `.md`-only pushes don't build and can go out freely.
- Keep CHANGELOG.md, README.md and CREDITS.md up to date with every change, and
  docs/EMOTIONS.md (the NPC expressions and your face's tables) with every build.
- A pure SFSE DLL, no .esp. Minor versions mark milestones (0.6 obstacle detection, 0.7 the
  interface, 0.8 room shots, the lens and filming people talking, 0.9 polish and
  optimization); 1.0 is the public beta.
- The code can't be compiled locally in the cloud workspace; the GitHub build is the
  compiler. Watch the run with `gh run watch` and read errors with `gh run view --log-failed`.

## Where things stand (2026-11-15, 0.8.10)

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
- Before 1.0: drop every fallback for old INI names (renamed keys read under their old
  names in Settings.cpp, `sPreset=Stardust` for Frontier). Nobody outside testing has an
  old INI, so 1.0 starts clean.

## Open observations from testing

- Many lines are emotion close-ups (0.8.3: every expression but Neutral and Friendly, at
  50%; reactions may interrupt them after their minimum time), which take priority over
  reactions; maybe let a reaction win now and then.
- Large speakers (Vasco) can block you in over-the-shoulder shots: size the near-body check
  from the actor's bounds.
