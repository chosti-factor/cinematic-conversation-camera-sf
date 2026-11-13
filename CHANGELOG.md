# Changelog

All notable changes to Cinematic Conversation Camera SF, newest first. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/). Every 0.x version is an alpha
test build; "diagnostic" items only add information to the log for testing.

## 0.5.4 - 2026-11-13

### Changed
- Obstacle detection: when no shot in the preset is fully clear, the camera takes the blocked
  shot with the most room left, pulled in (at least 1.5 m and 40% of its distance), before
  falling back to the plain shot. Next to Barrett's ladder this gives the side-on two-shot
  instead of a view from his eyes through the ladder.

### Fixed
- When nothing fitted a turn, 0.5.3 kept re-checking every frame (and filling the log); the
  plain shot now holds until the turn changes.

## 0.5.3 - 2026-11-13

### Added
- Obstacle detection, step 3: before the camera cuts to a shot, the game's own collision check
  tests the line from the person in shot to where the camera would stand (and to where its
  move ends).
  - A wall or object close behind the camera pulls the camera in to just in front of it, when
    that keeps at least 70% of the distance (and, over a shoulder, keeps the camera behind
    that shoulder).
  - Anything closer to the person in shot (a ladder in front of their face) skips the shot;
    the Director picks another. Emotion close-ups are checked the same way.
  - The near person's own body in an over-the-shoulder shot doesn't count: the check carries
    on past them.
  - If the plain over-the-shoulder shot is blocked too, the camera shoots from just in front
    of the other person's eyes instead.
  - `bObstacleCheck` (on) and `fObstacleRadius` (0.05 m, as the game's own dialogue camera)
    in `[Director]`. The log lists skipped and pulled-in shots.

### Fixed
- The 0.5.2 obstacle log measured the wrong lengths: the game's cast moves the end point it's
  given back to the safe spot, so it now gets copies.

### Removed
- The 0.5.1 search for the game's camera-collision code (done).

## 0.5.2 - 2026-11-13

### Changed
- Focus style Wide (deeper sharp zone, softer background blur) is the default.

### Diagnostic
- Obstacle detection, step 2: once per shot, the game's own dialogue-camera sphere cast (found
  in 0.5.1) checks the line from the person in shot to the camera, and the log says whether
  it's clear or blocked, how far along and by what. Logging only; the camera isn't changed.

## 0.5.1 - 2026-11-13

### Changed
- Focus styles (`sFocusStyle`): Cinematic (more background blur) and Wide (deeper sharp zone,
  softer background blur), replacing `bDeepFocusLongShots` (still read: 1 = Wide).
  `fDeepFocusBeyond` is now `fWideFocusBeyond` (old name still read).
- Outdoor shots may stand up to 7 m from the person in shot (was 5 m), closer to CCC's
  framings now that focus works at any distance.

### Added
- `docs/EMOTIONS.md`: running tally of the expression tags seen in testing.

### Diagnostic
- Obstacle detection, step 1: finds the game's own camera-collision code (through the settings
  it reads) and writes it to the log for study. Read-only, once per game session, in the
  background.

## 0.5.0 - 2026-11-13

The focus milestone: the person in shot is sharp at any camera distance.

### Added
- **Focus on the person in shot** (`bFocusOnSubject`, on by default). The game's conversation
  blur reads its settings live from its Dialogue_Close, Medium and Long_DOF records; on every
  shot their focus distance is set to the distance from the camera to whoever is in shot, and
  put back afterwards. Not used in quest scenes, where the game's own camera runs.
- Optional deeper sharp zone on long shots (`bDeepFocusLongShots`, `fDeepFocusBeyond`), for a
  softer background blur.

### Changed
- 40% of emotion close-ups are extreme close-ups (was 50%).
- Steadier following while the speaker walks (from 0.4.20-0.4.21) and the emotion messages in
  `[Debug]` are part of this release.

### Removed
- The experiments that led here: moving the game's camera for focus (`iFocusMethod`), the
  image-space blur experiment (`iBlurExperiment`), applying modifiers by hand
  (`sTestModifier`), the conversation blur test (`iDialogueBlurTest`, now the feature), the
  searches for the live blur and the record dumps. Their INI lines are gone; old INIs that
  still have them are harmless.

## 0.4.22 - 2026-11-13

### Added
- Conversation blur test (`iDialogueBlurTest`): changes the game's own Dialogue_Close, Medium
  and Long_DOF records while a conversation runs (put back afterwards): 1 = no blur, 2 = focus
  at the shot's distance, 3 = a deeper sharp zone on long shots. Their blur strength, focus
  distance and sharp-zone depth were found in the 0.4.21 dumps.

## 0.4.21 - 2026-11-13

### Changed
- Steadier following while the speaker moves starts at 0.25 m/s (`fBusyHeadSpeed`), so it
  reacts to walking rather than ordinary talking.

### Diagnostic
- The game's dialogue blur modifiers, Starvegas's and Chargen_DoF dumped in full (4 KB each), to
  find their focus distance, sharp-zone depth and blur strength.
- How often the steadier following switched on, per conversation.
- Finding: moving the game's camera never moves the conversation focus (methods 1-6, and the
  game doesn't put its camera back after the hook). That route is closed.

## 0.4.20 - 2026-11-13

### Fixed
- The picture jittered while the speaker walked (their head bobs with each step). While
  their head moves faster than `fBusyHeadSpeed` (0.2 m/s), the camera follows with
  `fBusyFollowSmoothing` (1.0 s) of smoothing instead of 0.35 s.

### Added
- Focus test methods 5 and 6 (`iFocusMethod`): move the game's camera later in the frame, on
  the main thread, in case the game works out its focus before the hook's move.

### Diagnostic
- Timing check: whether the game camera is still where the hook left it later in the frame.
- The game's dialogue blur modifiers, Starvegas's own and a few other depth-of-field ones are
  dumped with the data they point to, to find where their focus distances are kept.

## 0.4.19 - 2026-11-12

### Diagnostic
- The live conversation blur search now also walks out from the image-space manager, the
  dialogue camera, the player camera and the render camera, and reports any reference to the
  Dialogue_*_DOF records. Read-only, no INI changes needed.
- Per-shot measurement of how much the speaker's head moves, and how far the smoothed head
  the camera follows lags behind it (for the jitter with NPCs posing or at crafting tables).

## 0.4.18 - 2026-11-12

### Changed
- The image-space blur experiment is off by default (`iBlurExperiment=0`): it only reached
  the distant-haze settings, not the conversation blur. `iFocusMethod` stays off by
  default: none of its four ways of moving the game's camera moved the focus.

### Diagnostic
- Finds the game's live conversation blur (the running copy of Dialogue_Close, Medium or
  Long_DOF) and logs what it holds, a few times per conversation. Read-only.

## 0.4.17 - 2026-11-12

### Added
- Focus test (`iFocusMethod`): four ways of moving the game's camera so its depth of field
  lands on the person in shot, to find one that doesn't shift the picture as 0.4.8 did.

### Fixed
- Crash when `sTestModifier` was set (0.4.16): the direct call to the game's apply function
  is disabled. The game's dialogue blur modifiers focus on a target, so applying them by
  hand isn't the way forward.

## 0.4.16 - 2026-11-12

### Fixed
- `sTestModifier` never took effect: reading the `_user.ini` afterwards switched it off.
- Long text settings were cut at 128 characters, so the end of `sCloseUpEmotions`
  (Serious, Confused) was ignored.

### Changed
- The on-screen emotion messages (`bShowEmotions`) moved to the INI's `[Debug]` section
  (`[General]` is still read for older INIs).

### Diagnostic
- Each conversation start logs the test settings in effect.

## 0.4.15 - 2026-11-12

### Changed
- Emotion close-ups are more distinct: half are extreme close-ups (`iExtremeCloseUpChance`),
  and the regular one pushes in slowly and strongly, as in CCC's Close preset.
- Seated people and merchants get emotion close-ups too, from in front of them, then go back
  to the plain shot when the line ends.
- Disgust, Serious and Confused added to the close-up emotions.
- The on-screen emotion message says what the camera did: close-up, extreme close-up or no cut.

## 0.4.14 - 2026-11-12

### Added
- Emotion close-ups: lines tagged with heightened or emotional expressions (Irritated,
  Surprised, Somber, Nervous, Awed and others, set in `sCloseUpEmotions`) push in to a
  close-up of the speaker, after CCC's close-ups on intense lines.
- On-screen message with each spoken line's expression tag (`bShowEmotions`), to help choose
  the close-up emotions.
- Test switch to apply one of the game's image-space modifiers during conversations
  (`sTestModifier`), the way the console's `imod` does.

### Changed
- Crew recruits get this mod's camera instead of being treated as a quest scene.

### Fixed
- The search for the game's image spaces read the list header in the wrong order and found
  nothing.

### Diagnostic
- The scene check logs the quest each scene belongs to.

## 0.4.13 - 2026-11-12

### Fixed
- Merchants no longer fade in when the camera comes back to them: on your lines the camera
  stands in front of and beside them instead of behind them.

### Diagnostic
- Find the game's image spaces and image-space modifiers by scanning, and list them.

## 0.4.12 - 2026-11-12

### Added
- Merchants are recognised by the vendor flag on their faction, wherever the shop is, and
  get the plain over-the-shoulder shot.
- Blur experiment (`iBlurExperiment`): switch off the game's depth of field on the longer shots.

### Diagnostic
- Each line's expression tag (Friendly, Irritated...) written to the log.

## 0.4.11 - 2026-11-11

### Changed
- The focus fix from 0.4.8 is off by default: it shifted angled shots sideways, leaving
  people at the edge of the frame.
- Outdoor shots may stand up to 5 m away again.
- On your lines, the seated shot keeps about 1 m from the seated person, who no longer fades
  out and back in.

### Fixed
- When the speaker is in a background scene, the camera is no longer switched to first
  person and back mid-conversation (one cause of stacked lines).

### Diagnostic
- Searches for the game's focus distance, the line's response data and faction data.

## 0.4.10 - 2026-11-11

### Added
- Seated people get the plain over-the-shoulder shot instead of the shot library, whose
  angles could put the chair in front of them.

### Fixed
- Over-the-shoulder shots no longer swing in so close to the line between the two of you
  that the near person's shoulder hides the speaker.
- The NPC shot from your eyes no longer lingers after a late body rebuild.
- A likely crash in the test-only line diagnostic (seen with Ularu).

### Changed
- Outdoor shots limited to 3 m (undone in 0.4.11).

## 0.4.9 - 2026-11-11

### Fixed
- About one conversation in five had rotated or swinging shots: the camera layout was
  guessed per conversation and sometimes guessed wrong. It's now fixed.

## 0.4.8 - 2026-11-11

### Added
- Focus fix: move the game's camera so its depth of field lands on the person in shot
  (turned off in 0.4.11).

### Changed
- Everyday background scenes (people chatting, a receptionist's greeting) keep this mod's camera.

## 0.4.7 - 2026-11-11

### Added
- Quest scenes you take part in keep the game's own camera, decided on the first frame.
  The idea comes from Player Camera During Dialogue.

### Fixed
- Merchants were wrongly treated as scenes in 0.4.6.

## 0.4.6 - 2026-11-11

### Added
- First attempt at handing scripted scenes back to the game's camera (replaced in 0.4.7).

## 0.4.5 - 2026-11-11

### Diagnostic
- Look for the spoken line's dialogue record; depth-of-field test settings.

## 0.4.4 - 2026-11-11

### Added
- CREDITS.md and an updated README.

### Diagnostic
- The game's depth-of-field settings and the line's response data (emotions).

## 0.4.3 - 2026-11-11

### Changed
- Cutting styles renamed: Cinematic, Odyssey, SpaceWestern, Dogfight, SpaceOpera (old names
  still accepted).

## 0.4.2 - 2026-11-11

### Added
- Cutting styles: how often the camera takes a fresh angle when the turn passes, how many
  lines per angle, and the shortest hold.

## 0.4.1 - 2026-11-11

### Changed
- Each shot holds its angle and framing; each person keeps their angle across the
  back-and-forth; the camera may stand further back outdoors.

## 0.4.0 - 2026-11-10

### Added
- The CCC port begins: all 39 camera setups, the director with its presets (Standard, Close,
  Room) and cutting rules, camera moves.
- Small speakers (robots) keep the game's own camera.

### Changed
- Over-the-shoulder shots swing in until the near person's shoulder is in frame through
  Starfield's narrow lens. The INI was reorganised.

## 0.3.36 - 2026-11-10

### Changed
- Conversation start behaviour chosen separately for first and third person; no exit cover
  by default.

### Fixed
- Camera stuck behind the head after switching view.

## 0.3.35 - 2026-11-10

### Added
- Transition test settings: start-frame prediction, exit cover modes, game settings changed
  from the INI while the game runs.

## 0.3.34 - 2026-11-10

### Changed
- From first person: open over the shoulder, hold the last shot to the end and cut straight
  back to your view.

## 0.3.33 - 2026-11-10

### Fixed
- Exit glitches: the passing third-person frame and player visibility on the way back.
- Your character is hidden while the body rebuilds.

## 0.3.32 - 2026-11-10

### Fixed
- The first frames no longer flash a wide view; the opening shot holds until the first cut.

## 0.3.29 to 0.3.31 - 2026-11-10

### Changed
- Your full body is built with a direct refresh of your character, without switching
  cameras (idea from Cassiopeia Papyrus Extender); the camera switch is kept as a fallback.

## 0.3.27 to 0.3.28 - 2026-11-10

### Added
- Field-of-view and body-refresh test settings.

## 0.3.12 to 0.3.26 - 2026-11-09 to 2026-11-10

### Changed
- Many rounds of first-person body rebuild work: waiting for a quiet moment, retries with
  back-off, predicting where the game's camera will be, holding the view steady during the
  rebuild.
- Your character is revealed on the same frame as the first shot of them.

### Diagnostic
- Camera modes, field of view, camera positions and the game's settings.

## 0.3.10 to 0.3.11 - 2026-11-09

### Fixed
- A crash during a second body rebuild (camera changes now only happen on the main thread).

## 0.3.5 to 0.3.9 - 2026-11-09

### Added
- Seated shot: when the person nearest the camera is sitting, the camera stands in front of
  them instead of behind the chair.
- An opening grace period and a shortest time between cuts.

### Changed
- The body rebuild checks your head is at standing height before showing you, and keeps the
  camera off you when the full body can't be built.

## 0.3.0 to 0.3.4 - 2026-11-09

### Added
- From first person, your full body is rebuilt so you look right from outside.
- The last shot holds while the game's dialogue camera winds down; smooth head following.

### Removed
- A fast exit that left controls locked.

## 0.2.0 to 0.2.3 - 2026-11-09

### Added
- Only the picture on screen moves, so NPCs keep facing you; your character is shown.

### Fixed
- A freeze on the first conversation frame, and the camera's orientation in Starfield.

## 0.1.0 to 0.1.4 - 2026-11-09

### Added
- First version: the dialogue camera replaced by over-the-shoulder shots that follow whoever
  is speaking.

### Fixed
- Distances in metres, and Starfield's camera axis layout.
