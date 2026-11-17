# To 1.0

The working list from now to the 1.0 public beta. Ticked as things are done; anything that
runs out of time moves to "Moved to after 1.0" at the bottom, on the user's call.

Last updated: 0.9.12 (2026-11-17). Dates assume the Friday 27 November release; for
Wednesday the 25th, about two days earlier.

## 1. Starts and exits (now to about 19 Nov)

- [x] Start prediction from the game's own dialogue camera rule (0.9.6)
- [x] First-person exit: no steep-up frame (0.9.8)
- [x] `bEnabled=0` turns the whole mod off (0.9.11)
- [ ] Tests on 0.9.11: three first-person exits; true vanilla with the mod disabled in Mod
      Organizer (one third-person and one first-person conversation, sit and stand once)
- [ ] First-person exit: the last frame of the shot without you (if it's all that's left: end
      first-person conversations on a shot without you; the user's style call)
- [ ] Third-person exit: the one frame of the camera easing in (if vanilla is clean: keep your
      view drawn until the camera has settled)
- [ ] Furniture camera swing when sitting and standing (only if vanilla doesn't do it)
- [ ] Check the 0.9.10 fixes in first person: the bar stool with the drink (your body after the
      conversation picks up again), someone seated next to you (their head)
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
