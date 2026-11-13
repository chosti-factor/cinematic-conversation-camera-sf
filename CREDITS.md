# Credits

Running record of where Cinematic Conversation Camera SF's code and ideas come from.
Kept up to date with each build; the Nexus credits page will be made from it at beta.

Last updated: 0.4.4 (2026-11-11). Line counts are estimates over this project's own
files (source, INI, build scripts): 4,586 lines in total.

## Code by source

| Source | Licence | Lines | Share | What |
|---|---|---|---|---|
| [Cinematic Conversation Camera](https://github.com/fatalCMD/ccc) by hashhbbrown | GPL-3.0-or-later | ~265 | ~6% | 39-shot framing table, framing-size and composition math, camera-move scales, preset weights and moves (`src/Shots.*`, `src/Director.cpp`), converted to metres |
| [Player Camera During Dialogue](https://github.com/rrrafau1337/player-camera-during-dialogue) by rrrafau | GPL-3.0-or-later | ~10 | <1% | Dialogue camera update location for Starfield 1.16.244; vtable slot of the hide/show function (`src/Plugin.cpp`, `src/Visibility.cpp`) |
| [Cassiopeia Papyrus Extender](https://www.nexusmods.com/starfield/mods/10896) by LarannKiar | - | 0 | 0% | Ideas only (below), plus two Address Library IDs (the game's apply/remove for image-space modifiers) |
| [Starvegas - Cinematic Killcam](https://www.nexusmods.com/starfield/mods/8859) | - | 0 | 0% | Ideas only (below) |
| Written for this project | GPL-3.0-or-later | ~4,310 | ~94% | Everything else |

Dependencies (not counted above): [CommonLibSF](https://github.com/Starfield-Reverse-Engineering/CommonLibSF)
(game structures and hooking), Address Library for SFSE Plugins, SFSE.

## Ideas and inspiration

| Source | What it gave |
|---|---|
| Cinematic Conversation Camera | The whole design: shot vocabulary, director cutting rules (new angle after a few lines, short lines ignored, shortest hold), presets, camera moves, holding a shot's angle once cut, each person keeping their setup across shot/reverse, the 180-degree rule; the roadmap (letterbox, subtitles, reaction shots, NPC-to-NPC filming, stepping away from work) |
| Player Camera During Dialogue | Taking over the dialogue camera; the first-person -> third-person -> dialogue camera cycle and timings for building the player's body (now the fallback); exception handling such as skipping while piloting a ship, and handing the camera back to the game during scripted scenes |
| Cassiopeia Papyrus Extender | Building the player's full body with a direct 3D update request instead of switching cameras; the update flag meanings; the camera state list; finding a spoken line's dialogue record from its subtitle; where the game's apply/remove functions for image-space modifiers are (Address Library IDs 171562 and 105038) |
| Starvegas | Changing field of view through the game's settings (led to finding that the dialogue camera ignores them); snapping a camera onto walkable floor to avoid walls (planned); applying its own depth-of-field image-space modifiers for each shot and removing them afterwards (the lead for focus on the person in shot) |
| Match dialogue camera with character FOV ([Nexus 627](https://nexusmods.com/starfield/mods/627)) | The `fDialogueCameraFailsafeFPFOV` lead; depth-of-field advice |

## Project work

- **Testing and direction (chosti-factor):** tested every build; 60 fps recordings that exposed one- and two-frame glitches; pointed to Starvegas and Cassiopeia (both became fixes); design decisions including the cutting styles and their names, emotion-driven close-ups, interior/exterior camera distances, the INI layout and transition polish.
- **Engineering (Claude):** render-camera-only placement so NPCs keep facing the player; Starfield camera math (metres, axis layout); player body rebuild (camera cycle, then direct refresh); conversation start/exit handling (frame prediction, learned dialogue camera position, clean first-person exit); thread-safety and crash fixes; adapting CCC to Starfield's narrow dialogue lens (distance caps, over-the-shoulder swing-in, interior/exterior limits); cutting styles; small-speaker rule; settings; diagnostics.

## Update log

- 0.4.4: initial record.
- 0.4.14: image-space list search fixed (read the list header in the right order); crew-recruit scenes keep this mod's camera; test switch to apply a game image-space modifier during conversations; scene check logs each scene's quest.
- 0.4.13: merchants on your lines filmed from in front of and beside them (behind them they faded out); the blur test now finds the game's image-space list (it read an empty list in 0.4.12).
- 0.4.12: merchants recognised by their faction's vendor flag and given the plain shot; each line's facial-expression tag read and logged; test switch to turn the depth-of-field blur off on longer shots.
- 0.4.11: focus fix off by default (it shifted angled shots sideways); outdoor shots back to 5 m; seated shot on your lines kept about 1 m from the seated person (they faded out closer); no mid-conversation camera switching when the speaker is in a background scene.
- 0.4.10: seated speakers and merchants get the plain over-the-shoulder shot; over-the-shoulder shots keep the subject clear of the near person; outdoor shots capped at 3 m like indoors; no more NPC shot from your eyes lingering after a late body rebuild; fixed a likely crash in the test-only dialogue-line diagnostic (it could call into memory that wasn't a game record); it now reads one confirmed spot.
- 0.4.9: fixed the camera layout being guessed wrong on some conversations (rotated, swinging shots).
- 0.4.8: depth of field kept on the person in shot by moving the game's camera along the NPC's line of sight; everyday background scenes keep this mod's camera.
- 0.4.6/0.4.7: quest scenes keep the game's own camera (idea from Player Camera During Dialogue's scripted-scene bypass; detection by the speaker's current scene is our own).
