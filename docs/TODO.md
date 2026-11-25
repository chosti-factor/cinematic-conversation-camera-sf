# To 1.0

The working list from now to the 1.0 public beta. Ticked as things are done; anything that
runs out of time moves to "Moved to after 1.0" at the bottom, on the user's call.

Last updated: 0.9.66 (2026-11-23). Dates assume the Friday 27 November release; for
Wednesday the 25th, about two days earlier.

## 1. Starts and exits (now to about 19 Nov)

- [x] Start prediction from the game's own dialogue camera rule (0.9.6)
- [x] First-person exit: no steep-up frame (0.9.8)
- [x] `bEnabled=0` turns the whole mod off (0.9.11)
- [x] First-person exits on 0.9.12 (five, one clip): no steep-up frame, starts clean; one
      zoomed frame, and one or two frames of the last shot without you
- [ ] The zoomed frame gone (0.9.13's fix didn't run; 0.9.15)
- [x] True vanilla with the mod disabled in Mod Organizer: the sitting camera's swing is the
      game's; vanilla third-person exits have their own in-between frame; the first-person
      exit is clean
- [x] Exit style compared (0.9.14 clips): 2, the cut back to your view, is "far superior"
      (the user); the default
- [x] Exit style 2's cut (0.9.16 clip): clean at the cut in first and third person, starts
      clean; "probably good for release" (the user). Smoother transition styles: after 1.0
- [x] 0.9.17: no zoom at the hand-back; after a bartender's menu (8 s) your body there and the
      exit on the normal path; seated third person: a second cut from the game's seated camera
      (0.9.18: the game's camera takes those exits)
- [x] 0.9.18 (test): a seated third-person exit in one cut; filming people talking started
      while you walk, no shaking; once stopped, moving ends it more readily (all three in the
      INI and on the settings page)
- [x] Settings page tested (0.9.14): works; found the options bug below
- [x] F1 mid-conversation (0.9.15): the conversation waits and carries on, options stay
- [x] First person on a bar stool with a drink (0.9.15): your body there after the drink, the
      person next to you didn't block you
- [ ] Seen once (0.9.14 clip): a cut to over-the-shoulder showed the NPC without hair for
      one frame (Muria); watch for it
- [ ] Third-person exit: the one frame of the camera easing in (if vanilla is clean: keep your
      view drawn until the camera has settled)
- [x] Third-person exits (the user, 2026-11-21, nice before release): a noticeable shift and
      turn as the camera settles back, sometimes your body moving as if hit. 0.9.45 logs the
      game's view against the one drawn over the second after the hand-back, and your body's
      turn and movement. 0.9.45 log: your view from before sat 20-24 degrees off your body's
      heading, the game's camera behind your body, the swing on frame 1; your body still.
      0.9.46 turns the cut round your feet to behind you. Tested 2026-11-22 (three exits,
      log and clip): the game's view within 0.04 degrees of the one drawn, the clip clean
      frame by frame at both hand-backs. The user: still a small but noticeable jolt of your
      character. Measured in the clip: the camera still (background under 0.15 px), your
      character's pose changing over three frames as the game ends the conversation. 0.9.47
      puts a trial `iExitStyle=3` (the cut on the hand-back frame, third person) to compare.
      0.9.47 test (the user): style 3 cleaner, a clean exit, the bars flickering (a frame
      without them as the menu closed, a frame early once); 0.9.48 makes the hand-back cut
      standard for third person and fixes the bars to the frame. 0.9.48 clips: third person
      clean (bars and cut on one frame, the cut sharp); the frame without bars as the menu
      closed still there, and first person a frame of bars over its cut. The user
      (2026-11-22): clean exits over bars kept to the cut; 0.9.49 takes the stand-in bars out
      (post-1.0 Mulling). Confirmed 2026-11-22 (the user: exits clean; 0.9.49 clip, two
      first-person and two third-person exits: the bars go once with the menu, then one cut).
- [x] Furniture camera swing when sitting and standing: vanilla does it too and the game's
      `bDisableFurnitureCamera` doesn't stop it; left for after 1.0, under Mulling (the
      user, 2026-11-17)
- [x] Seated next to someone at a bar, third person (0.9.12, twice): their head didn't block
      you; the tally of your lines works
- [ ] Check the 0.9.10 fix in first person: the bar stool with the drink (your body after the
      conversation picks up again)
- [ ] Optional: the extra frame on the first conversation after loading a save

## 2. Remaining features and tests (about 18 to 21 Nov)

- [x] Filming people talking (0.9.17 test): starts on its own, cuts between them, skips
      blocked angles, stops when they stop or you move; shook when started on momentum (0.9.18: the game camera's
      root held while filming; starting while you walk stays, the user's call)
- [x] The CCC SF presets reviewed from the 2026-11-18 run (Muria, Sam, Barrett, Andreja; the
      user's decisions in HANDOFF.md), built in 0.9.23
- [ ] Test 0.9.23 (Barrett, then Andreja): close-up limits, room shots as your angle,
      Frontier's listening reactions, Starborn towards Interstellar, Nebula's low angles
      (NebulaTest), the tagged option's close-up, no one-frame flashes; 0.9.24's added shots
      in Frontier, Starborn and Constellation (keep or take out)
- [x] Large speakers (Vasco): his back hid you in the narrower shots of you (0.9.18 clip);
      0.9.19 keeps those to 38 degrees or wider with a large speaker. 0.9.21 test: still hidden
      in Starborn, by the plain over-the-shoulder shot it fell back on (no Starborn shot of you
      was wide enough); 0.9.22 swings that one out too, settings in the INI and F1. To check
- [x] SFSE Menu Framework settings page (0.9.12; tested 0.9.14-0.9.18: pages, writes to the
      `_user.ini` (with Mod Organizer, in its overwrite folder), Reset, F1 mid-conversation)
- [x] The tally of your lines started (0.9.12; while the detailed log is on)
- [x] Compatibility with UI mods: the testing pool of five in docs/COMPATIBILITY.md (rbtUI2,
      Enhanced Dialogue Interface, Enhanced Subtitles, Venworks HUD and StarUI HUD all work),
      and AutoZoom with its plugin on (compatible, 0.9.29)
- [x] Quest scenes: what each one is made of is logged (0.9.20: its actions by type, and
      whether it directs a camera). Using it to keep this mod's camera on quest scenes that
      are just people talking moved to after 1.0
- [ ] Optional: `fCuttingPace` +2/-2 feel test (half steps since 0.9.26); seated over-the-shoulder side 0.6 to 0.75

## 3. Feature freeze (about 21 to 22 Nov)

- [x] Features frozen (2026-11-19, after the 0.9.29 test; two days early)

## 4. Final polish passes (about 22 to 25 Nov), a build and a test round after each

- [x] Take out the temporary "Nebula test" preset (done in 0.9.25)
- [x] Redundancy: dead code, finished experiments, old exit modes, spent diagnostics (0.9.30; tested with 0.9.31, 2026-11-20: all good; removed parts in docs/MEMORY.md)
- [x] Debugging: every INI setting against its comment; edge cases (loading or a menu
      mid-conversation, ships, scenes, combat, death, fast travel, back-to-back
      conversations); thread safety, crashes (0.9.35-0.9.36, tested 2026-11-21: build warnings
      gone, filming ends at a loading screen, the conversation waits through a trade)
- [x] Optimization: per-frame and scan costs measured with the log off, then cheaper (0.9.37:
      the cost meters; 0.9.39: the log written every 3 s, stalls of 15-27 ms still there;
      0.9.40: the HUD's parts timed; 0.9.41: the INI read only after a change and the HUD's
      idle work skipped, both written up in 0.9.38 but never in the build; 0.9.43 log: the
      main-thread task 16-40 us a frame outside conversations, 100-260 in them (the HUD's
      subtitle 65-120 of it); one spike, 14.8 ms, the INI re-read after an edit on the
      settings page; with the log off (0.9.44-0.9.45): 16-27 us outside conversations, 50-310
      in them, no spike over 5 ms but the INI read after an edit. Done 2026-11-22)
- [ ] Before the INI review (the user, 2026-11-22): the shot-to-shot cuts that aren't clean
      (some flicker or "teleport"; most are instant). Lead: in the 0.9.47 clip 2 of 6 cuts
      showed the old shot a frame with the new shot's focus before the camera moved. Next: a
      session with the detailed log on, longer conversations (Nebula cuts most), clips; every
      cut measured frame by frame and sorted by cause. (0.9.50 clips: two causes, the focus
      a frame early and the lens one or two late. 0.9.51 sets the lens with the camera and
      the focus a frame after: 20 of 31 cuts clean, 7 with the blurred frame now after the
      cut, 4 zooms. 0.9.52: no blur on a cut's frame, the lens written on the render camera
      with its position; to check with new clips. A passer-by close past the camera, seen
      in the clip, is fine: the user, they add life to a scene)
- [x] Compare `bCutFocusBridge` 0 and 1: 0 kept, the switch gone (0.9.55)
- [ ] Before 1.0: the odd frames at some cuts (a frame with bodies or hedges missing, the old
      lens for a frame, the new focus a frame early). Not frame generation (it was off) and not
      DLSS (0.9.55 test with it off: the same). 0.9.56: the cut timing trace (detailed log):
      the zooms were cuts from shots over 60 degrees, taken for the start's gameplay lens
      (fixed 0.9.57). 0.9.57 clips: 15 of 15 cuts clean outdoors, then about 45 at the Lodge
      with no zoom and one mild early softening. Done; the trace came out in 0.9.59. Post-1.0
      Mulling if the early focus ever bothers: how the game's quest-scene cameras cut
      cleanly (the user)
- [x] Cora's lines overlapping: listed as a known issue for 1.0 (README; the user, 2026-11-22),
      the hunt post-1.0 (Mulling). Notes kept below.
- Cora's lines overlapping at the Lodge (0.9.57 runs), the 0.9.43 bug back. The user's
      tests: filming off, Cora clean; filming Walter and Vasco (stopped by moving), Cora
      overlapping; mod off, clean. Both broken runs had the filming stopped mid-scene by moving;
      0.9.43's tests let scenes play out. 2026-11-22 22:48: the filming of Walter and Vasco
      left to end on its own ("they've stopped talking"), then Cora: clean. So the trigger is
      stopping a filming mid-line (moving while someone in the scene talks): the view and
      first person snap back while a line plays. 0.9.58: `bFilmTestStopAfterLine` holds the
      view to the line's end (controls back at once); test it the same way. 0.9.58 tests: on,
      clean; off, mid-line stops twice, clean both times. So intermittent: 2 broken of 7
      conversations after a filming, same steps. Clues for later: Player Camera During
      Dialogue lists "rare dialogue voice-line overlap" as a known issue, and its code holds
      off any camera change from a dialogue choice until the NPC's whole response (its voice
      fragments, with up to 1.5 s gaps) has played, against what it calls the dialogue-skip
      bug: the game's sequencing of a multi-part response seems fragile to camera-state or
      dialogue-target changes. Cora's responses are multi-part; filming switches the game's
      camera state (first to third person and back). CCC (Skyrim) has nothing on it
- [x] INI and defaults review, first pass (0.9.60; docs/INI_HISTORY.md): the page trimmed to
      the main choices, defaults (log, messages, start on its own off), comment fixes
- [x] With it, a review of the settings page's layout and options (the user, 2026-11-21): the
      design isn't clear enough for people; which options to keep on the page, which to hide
      (INI only), and which belong in a future preset editor (post-1.0, Mulling). First pass in
      0.9.60; a second look after the user has used the new page

## 5. Release candidate (about 25 to 26 Nov)

- [x] SFSE Menu Framework: a global way to set the F1 menu's font and its size or scaling (the
      user plays at 4K, where it can be too small). Found on the user's PC (framework 1.2.0):
      `SFSEMenuFramework.ini` [Fonts] has `PrimaryFont`, `FontWeight`, `FontSizeMedium` (40)
      and `UIScale` (1), for every mod's page; in the README (0.9.67)
- [x] Quest-scene pacing (the user, 2026-11-22): 0.9.67-0.9.75 (holds, speaker changes, merchants
      in scenes, the flash at speaker changes), tested on Ularu Chen's and Benjamin Bayu's
      scenes; `bGameCameraInScenes` off by default (0.9.76, the user)
- [x] Pre-1.0 code review (0.9.66, 2026-11-23): no warnings, no leftovers of the experiments,
      INI, code and page agree, defaults match the INI. Fixed: the sway slider's tooltip, the
      preset reset under a preset key choice; the preset key only while filming; sway's
      "preset's own" a switch; the page's unused widget kinds out. The `.pdb` already ships
      (crash logs), now in the README

- [x] Before release (the user, 2026-11-21): lip sync out of step in the conversation that
      follows filming people talking. Twice in MQ102 at the MAST: filming Tuala and Sarah,
      then Tuala's conversation with you (a quest scene, the game's camera), with his lines out
      of lip sync, and you came back in third person after it (0.9.38 fixes that one, and the
      filming lens on its first frame). Suspected: the hand-over from filming to the game's
      camera, the game in third person with you hidden as the conversation starts. First an
      A/B test from a save before it: auto filming off, then on. 0.9.39 test (auto filming off
      before loading, the Vanguard ceremony and the MAST scene nearby, not filmed): no overlap.
      So filming starts it. The 0.9.38 run after its reload filmed the ceremony to its end, no
      interruption, and Tuala's (unfilmed) conversation still had it; loading another save
      cleared it. Next test: film the ceremony to its end only, then Tuala with auto filming off.
      0.9.40: overlapping again. 0.9.41 (a switch per part of filming), the camera left out:
      clean, so not third person, your hiding, the hold, the HUD, the lens or the bars. 0.9.42
      splits the camera: the view moved, or the game's camera root held. 0.9.42, the root
      hold left out: clean. Found: filming's hold on the game's camera root; gone in 0.9.43
      (docs/MEMORY.md). 0.9.43 test (the user, 2026-11-21): clean. To confirm once more on the release candidate. The user's rule
      (2026-11-21): story-quest
      scenes stay filmed (the Constellation members' conversation at the Lodge "amazing to
      see"); only a scene or quest shown to conflict is left to the game, never quest scenes
      as a group.
- [x] Nice before release, else after 1.0 (the user, 2026-11-21): first-person exits, a small
      shift of the NPC to the right as the game takes over. 0.9.38's log: the game's heading
      differs from the view drawn (9.4, -3.4, -2.0 degrees across); 0.9.39 turns the view to
      your current facing. 0.9.39 test: still there (two Sarah exits, one shifted); your body
      hadn't turned, but your first-person view sat 0-5 degrees off your body's heading, and
      the game takes over along your body (matched to 0.05 degrees on all five exits). 0.9.40
      cuts to your body's heading. Tested 2026-11-21: pristine.

- [x] Testing messages and the detailed log off by default (0.9.60)
- [x] Take out the measuring code once the optimization pass is done (the user, 2026-11-21; done in 0.9.50, before the INI review, with the finished diagnostics):
      the cost meters and their log lines (0.9.37), the HUD's parts (0.9.40), the main-thread
      task's parts (0.9.43); and the diagnostics that only fed finished investigations (the
      exit frame comparison, 0.9.38). Into docs/MEMORY.md with what they measured.
- [x] Take out the lip-sync test switches (`bFilmTest*`, 0.9.41) once it's found; into docs/MEMORY.md (0.9.43)
- [ ] Full test round on the release candidate (starts, exits, every preset, seated,
      merchants, filming, the settings page)
- [ ] Nexus page and its credits section (from CREDITS.md)
- [ ] 60-90 s clip
- [ ] Outreach a few days before (machinima podcast, Starfield mod YouTubers, the Starfield
      Modding Discord)

## 6. Release

- [ ] 1.0 public beta on Nexus: Friday 27 Nov, 9-11 am Central (or Wednesday the 25th)
- [ ] Reddit (r/starfieldmods) the Saturday after, 9 am-1 pm Central

## Moved to after 1.0

- Inside ships: more testing (the user, 1.0 test: the Cryoknox, Sarah, the camera bouncing; fixed
  for 1.0 by not cycling the camera while you sit). Why the full body isn't built there from first
  person, seated: find out, so you're in frame.
- The SFSE Menu Framework crashed once in the 1.0 test (21:49, its own code: MenuOwnership
  SetLayerDisabled, in the window's message handling; none of this mod on the stack). Its build is
  the window-blur one; worth reporting to its author if it comes back.

- Cleaner transitions in conversations with three or more people (the user, 1.0 release-candidate
  test: good overall, but the cuts at speaker changes should be cleaner).
- Speaker changes with you seated: the spot predicted for the game's camera 0.2-0.4 m off at the
  Wilcox table (1-4 cm standing, Bayu and Yannick); the game seems to place its camera from
  something else when you sit. Log your head and feet at the change to find out.

- The Ranger fading in at Neon (the user, 2026-11-19: very infrequent; Planned after 1.0)
- Watch for: crashes in the game's UI engine (Scaleform's ActionScript, on another thread,
  DialogueMenu on the stack) as a conversation starts. Two in the release-candidate test
  (2026-11-24): Tank at the Trackers Alliance HQ (02:43, BSJobs, 50 ms after the letterbox bars
  went into the new dialogue menu) and Nyx at Neon (15:31, IOManager, the first conversation
  after starting the game; this mod's first call into the new dialogue menu faulted at 30 ms and
  the game froze). Fixed in 1.0.0 as far as we can tell: a new menu movie is left alone for its
  first half second (Hud.cpp, kSettle), the HUD messages menu's bars stand in meanwhile. If it
  comes back: a longer wait, or the calls moved onto the menu's own update.
- Speaker changes in scenes of three or more: where the game reports its camera's move at once,
  two frames of the new speaker in a slightly wrong framing just after the cut (0.9.75; the
  0.9.74 guess was wrong; docs/MEMORY.md)
- Your character looking at the wrong person in conversations with three or more people (the
  user, 2026-11-23: after 1.0 unless easy; it isn't: needs the game's head tracking; README Planned)
- [x] Filming people talking: the near person in over-the-shoulder shots hidden by the game's
  camera and fading back in after the cut (Walter, 0.9.29 test). The camera kept 1.4 m from
  them (0.9.29); to check in the next test, with the distance in the log
- Replace the large-speaker height rule (decided 2026-11-18; it stays until then) with a
  check of the speaker's actual body: width and bulk, not only height (robots like Vasco are
  wide and bulky). The line from the camera to your face tested against the speaker's body
  shapes, as CCC 1.5.0's Keep Subject Visible does (head and torso capsules, sized from
  each actor's bound against the player's).
- Quest scenes that are just people talking keep this mod's camera (decided 2026-11-18):
  first per scene (a scene with no Camera or Timeline action keeps ours), later per phase
  (the game's camera only while a camera action runs). Needs the 0.9.20 logs from many
  quest scenes first, to confirm the Camera action is what takes the camera over.
  First data (0.9.29 test, MQ101/MQ102 at the Lodge): the first Camera action seen,
  'MQ101_035_SarahMorganScene' (1 Camera among 14 actions); 'MQ101_040_SarahFinalScene' and
  'MQ102_001_PlayerScene' had none, so the per-scene rule would have kept this mod's camera
  on them. Everyday 'Dialogue...' scenes (Noel's greeting) keep it already, by name.
