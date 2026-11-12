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
| [Cassiopeia Papyrus Extender](https://www.nexusmods.com/starfield/mods/10896) by LarannKiar | - | 0 | 0% | Ideas only (below) |
| Starvegas | - | 0 | 0% | Ideas only (below) |
| Written for this project | GPL-3.0-or-later | ~4,310 | ~94% | Everything else |

Dependencies (not counted above): [CommonLibSF](https://github.com/Starfield-Reverse-Engineering/CommonLibSF)
(game structures and hooking), Address Library for SFSE Plugins, SFSE.

## Ideas and inspiration

| Source | What it gave |
|---|---|
| Cinematic Conversation Camera | The whole design: shot vocabulary, director cutting rules (new angle after a few lines, short lines ignored, shortest hold), presets, camera moves, holding a shot's angle once cut, each person keeping their setup across shot/reverse, the 180-degree rule; the roadmap (letterbox, subtitles, reaction shots, NPC-to-NPC filming, stepping away from work) |
| Player Camera During Dialogue | Taking over the dialogue camera; the first-person -> third-person -> dialogue camera cycle and timings for building the player's body (now the fallback); exception handling such as skipping while piloting a ship, and handing the camera back to the game during scripted scenes |
| Cassiopeia Papyrus Extender | Building the player's full body with a direct 3D update request instead of switching cameras; the update flag meanings; the camera state list; finding a spoken line's dialogue record from its subtitle |
| Starvegas | Changing field of view through the game's settings (led to finding that the dialogue camera ignores them); snapping a camera onto walkable floor to avoid walls (planned) |
| Match dialogue camera with character FOV ([Nexus 627](https://nexusmods.com/starfield/mods/627)) | The `fDialogueCameraFailsafeFPFOV` lead; depth-of-field advice |

## Project work

- **Testing and direction (chosti-factor):** tested every build; 60 fps recordings that exposed one- and two-frame glitches; pointed to Starvegas and Cassiopeia (both became fixes); design decisions including the cutting styles and their names, emotion-driven close-ups, interior/exterior camera distances, the INI layout and transition polish.
- **Engineering (Claude):** render-camera-only placement so NPCs keep facing the player; Starfield camera math (metres, axis layout); player body rebuild (camera cycle, then direct refresh); conversation start/exit handling (frame prediction, learned dialogue camera position, clean first-person exit); thread-safety and crash fixes; adapting CCC to Starfield's narrow dialogue lens (distance caps, over-the-shoulder swing-in, interior/exterior limits); cutting styles; small-speaker rule; settings; diagnostics.

## Update log

- 0.4.4: initial record.
- 0.4.10: seated speakers and merchants get the plain over-the-shoulder shot; fixed a likely crash in the test-only dialogue-line diagnostic (it could call into memory that wasn't a game record); it now reads one confirmed spot.
- 0.4.9: fixed the camera layout being guessed wrong on some conversations (rotated, swinging shots).
- 0.4.8: depth of field kept on the person in shot by moving the game's camera along the NPC's line of sight; everyday background scenes keep this mod's camera.
- 0.4.6/0.4.7: quest scenes keep the game's own camera (idea from Player Camera During Dialogue's scripted-scene bypass; detection by the speaker's current scene is our own).
