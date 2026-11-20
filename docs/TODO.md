# To 1.0

The working list from now to the 1.0 public beta. Ticked as things are done; anything that
runs out of time moves to "Moved to after 1.0" at the bottom, on the user's call.

Last updated: 0.9.29 (2026-11-19). Dates assume the Friday 27 November release; for
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
- [ ] Redundancy: dead code, finished experiments, old exit modes, spent diagnostics
- [ ] Debugging: every INI setting against its comment; edge cases (loading or a menu
      mid-conversation, ships, scenes, combat, death, fast travel, back-to-back
      conversations); thread safety, crashes
- [ ] Optimization: per-frame and scan costs measured with the log off, then cheaper
- [ ] INI and defaults review: comments, defaults, the `_user.ini`, the settings page (its
      labels, help and ranges against the INI)

## 5. Release candidate (about 25 to 26 Nov)

- [ ] Testing messages and the detailed log off by default
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

- The Ranger fading in at Neon (the user, 2026-11-19: very infrequent; Planned after 1.0)
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
