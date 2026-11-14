# Handoff notes

For picking this project up in a new Claude session (for example one linked to the gaming PC).
Read README.md (plan), CHANGELOG.md (what each version did) and CREDITS.md first.

## Working rules

- Ask before any push that changes more than `.md` files: every such push starts a GitHub
  Actions build (Windows, xmake, CommonLibSF). Pushes to `main` also replace the rolling
  "Test build" pre-release; pushes to other branches only build.
- `.md`-only pushes don't build and can go out freely.
- Keep CHANGELOG.md, README.md and CREDITS.md up to date with every change.
- A pure SFSE DLL, no .esp. Minor versions mark milestones (0.6 obstacle detection, 0.7 the
  interface); 1.0 is the public beta.
- The code can't be compiled locally in the cloud workspace; the GitHub build is the
  compiler. Watch the run with `gh run watch` and read errors with `gh run view --log-failed`.

## Where things stand (2026-11-14)

- 0.7.6 is the current Test build: testing messages in the top bar (the notifications' own
  look: `$MAIN_Font_Bold`, 18, white, learned from `HUDMessagesMenu.Menu_mc.Messages_mc...
  Text_tf`), third-person exit holds the last shot to the end, reactions counted in
  sentences, ceiling measured from the cast's safe point (its fraction is 0 going straight
  up; the safe point is right).
- 0.7.7 (committed, held, see below): the game's actor list. CommonLibSF's
  `ProcessLists::highActorHandles` (+0x58) is always empty on Starfield 1.16.244, so
  bystander avoidance (0.7.5) and overheard NPC lines never saw anyone. 0.7.7 reads the
  list where the game does, +0x60 (BSTArray: size u32, capacity u32, data pointer at +8;
  32-bit handles), and falls back to the player's cell references.
  The reaction log also says why a due line wasn't a reaction.

## Next, with access to the gaming PC

1. Done: the actor list is at ProcessLists +0x60 (count) / +0x68 (handles) in 1.16.244,
   found in Starfield.exe (loads of the singleton, RVA 0x5FD9B88, ID 937584). The exe is on
   the gaming PC (F:\SteamLibrary\steamapps\common\Starfield); the Address Library is in
   Mod Organizer's mods folder (F:\Ad Astra\mods\Address Library for SFSE Plugins). With pefile
   and capstone (pip through the proxy: NO_PROXY=localhost) code can be read directly.
2. Read test logs straight from `C:\Users\ricar\OneDrive\Documents\My Games\Starfield\SFSE\Logs`.
3. Then: NPC-to-NPC filming (prompt / seated / standing still triggers, see README),
   room shots with Starfield framing (CCC's need 18-27 m at this 30 degree lens), your
   character's face (ChangeAnimFaceArchetype at RVA 0x18E0220; credit rrrafau), polish.

## Open observations from testing

- Many lines are emotion close-ups (29 expressions listed), which take priority over
  reactions; maybe let a reaction win now and then.
- Large speakers (Vasco) can block you in over-the-shoulder shots: size the near-body check
  from the actor's bounds.
