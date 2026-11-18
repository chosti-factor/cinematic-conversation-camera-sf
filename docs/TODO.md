# To 1.0

The working list from now to the 1.0 public beta. Ticked as things are done; anything that
runs out of time moves to "Moved to after 1.0" at the bottom, on the user's call.

Last updated: 0.9.16 (2026-11-17). Dates assume the Friday 27 November release; for
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
- [ ] Exit style 2's cut (0.9.16, `bExitCutStaged`): no odd frame before the cut in first or
      third person, no narrow frames, the hand-back still clean; and after the bar's drink
      (0.9.15 staging still had one odd frame; the order varies)
- [x] Settings page tested (0.9.14): works; found the options bug below
- [x] F1 mid-conversation (0.9.15): the conversation waits and carries on, options stay
- [x] First person on a bar stool with a drink (0.9.15): your body there after the drink, the
      person next to you didn't block you
- [ ] Seen once (0.9.14 clip): a cut to over-the-shoulder showed the NPC without hair for
      one frame (Muria); watch for it
- [ ] Third-person exit: the one frame of the camera easing in (if vanilla is clean: keep your
      view drawn until the camera has settled)
- [ ] Furniture camera swing when sitting and standing: vanilla does it too; the game's
      `bDisableFurnitureCamera` doesn't stop it (0.9.15 test). Leave it, or look for a fix after
      1.0 (the user's call)
- [x] Seated next to someone at a bar, third person (0.9.12, twice): their head didn't block
      you; the tally of your lines works
- [ ] Check the 0.9.10 fix in first person: the bar stool with the drink (your body after the
      conversation picks up again)
- [ ] Optional: the extra frame on the first conversation after loading a save

## 2. Remaining features and tests (about 18 to 21 Nov)

- [ ] Filming people talking (still untested): two NPCs talking, once walking up with momentum
- [ ] The long Sarah conversation through the CCC SF presets, then the preset tweaks in
      HANDOFF.md (reactions on overhead, Constellation OTS 30 to 15, extreme close-up share per
      preset, Frontier two-shot one turn)
- [ ] Large speakers (Vasco) blocking you in over-the-shoulder shots
- [ ] SFSE Menu Framework settings page (built in 0.9.12; test: each page opens, a change
      lands in the `_user.ini` and applies next conversation, Reset, a preset change, the
      game without the framework)
- [x] The tally of your lines started (0.9.12; while the detailed log is on)
- [ ] Compatibility with UI mods (HUD replacer, dialogue menu replacer, subtitle mod), and
      auto-zoom back on as one check
- [ ] The Ranger fading in at Neon (waiting on footage)
- [ ] If time allows: quest scenes recognised by their quest
- [ ] Optional: `iCuttingPace` +2/-2 feel test; seated over-the-shoulder side 0.6 to 0.75

## 3. Feature freeze (about 21 to 22 Nov)

- [ ] Features frozen

## 4. Final polish passes (about 22 to 25 Nov), a build and a test round after each

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

(Nothing yet.)
