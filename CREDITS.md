# Credits

Running record of where Cinematic Conversation Camera SF's code and ideas come from.
Kept up to date with each build; the Nexus credits page will be made from it at beta.

Last updated: 0.6.0 (2026-11-13). Line counts are estimates over this project's own
files (source, INI, build scripts): about 5,900 lines in total.

## Code by source

| Source | Licence | Lines | Share | What |
|---|---|---|---|---|
| [Cinematic Conversation Camera](https://github.com/fatalCMD/ccc) by hashhbbrown | GPL-3.0-or-later | ~265 | ~5% | 39-shot framing table, framing-size and composition math, camera-move scales, preset weights and moves (`src/Shots.*`, `src/Director.cpp`), converted to metres |
| [Player Camera During Dialogue](https://github.com/rrrafau1337/player-camera-during-dialogue) by rrrafau | GPL-3.0-or-later | ~10 | <1% | Dialogue camera update location for Starfield 1.16.244; vtable slot of the hide/show function (`src/Plugin.cpp`, `src/Visibility.cpp`) |
| [Cassiopeia Papyrus Extender](https://www.nexusmods.com/starfield/mods/10896) by LarannKiar | - | 0 | 0% | Ideas only (below) |
| [Starvegas - Cinematic Killcam](https://www.nexusmods.com/starfield/mods/8859) | - | 0 | 0% | Ideas only (below) |
| Written for this project | GPL-3.0-or-later | ~5,625 | ~95% | Everything else |

## Built on

These aren't part of this project's code, but nothing here would work without them:

- [SFSE (Starfield Script Extender)](https://www.nexusmods.com/starfield/mods/106) by the SFSE
  team (ianpatt and contributors): loads the plugin into the game.
- [Address Library for SFSE Plugins](https://www.nexusmods.com/starfield/mods/3256) by meh321:
  where the game's functions and data are in each version (every game address this mod uses
  goes through it).
- [CommonLibSF](https://github.com/Starfield-Reverse-Engineering/CommonLibSF) by the
  Starfield Reverse Engineering team: the game's structures, hooking and the Address Library
  lookups.

## Ideas and inspiration

| Source | What it gave |
|---|---|
| [Player Dialogue Camera](https://www.nexusmods.com/starfield/mods/5229) by jedijosh920 | The proof of concept: showed that Starfield's own dialogue camera can be taken over to show the player character in conversations, as in Fallout 4, which is what made a port of CCC look possible |
| Cinematic Conversation Camera | The whole design: shot vocabulary, director cutting rules (new angle after a few lines, short lines ignored, shortest hold), presets, camera moves, holding a shot's angle once cut, each person keeping their setup across shot/reverse, the 180-degree rule; the roadmap (letterbox, subtitles, reaction shots, NPC-to-NPC filming, stepping away from work) |
| Player Camera During Dialogue | Taking over the dialogue camera; the first-person -> third-person -> dialogue camera cycle and timings for building the player's body (now the fallback); exception handling such as skipping while piloting a ship, and handing the camera back to the game during scripted scenes |
| Cassiopeia Papyrus Extender | Building the player's full body with a direct 3D update request instead of switching cameras; the update flag meanings; the camera state list; finding a spoken line's dialogue record from its subtitle; the game's apply/remove functions for image-space modifiers (used in test builds 0.4.14-0.4.16, not in the mod) |
| Starvegas | Changing field of view through the game's settings (led to finding that the dialogue camera ignores them); snapping a camera onto walkable floor to avoid walls (planned); applying its own depth-of-field image-space modifiers for each shot: comparing them with the game's own conversation blur records showed where focus distance, sharp-zone depth and blur strength are kept, which became focus on the person in shot |
| Match dialogue camera with character FOV ([Nexus 627](https://nexusmods.com/starfield/mods/627)) | The `fDialogueCameraFailsafeFPFOV` lead; depth-of-field advice |

## Project work

- **Testing and direction (chosti-factor):** tested every build; 60 fps recordings that exposed one- and two-frame glitches; pointed to Starvegas and Cassiopeia (both became fixes); kept the depth-of-field hunt going (the Starvegas lead, the console tests, the idea of controlling the sharp zone and blur radius); design decisions including the cutting styles and their names, emotion-driven close-ups and the on-screen emotion messages, which emotions push in, interior/exterior camera distances, per-preset camera distances, the Stardust preset, the INI layout and transition polish; credited jedijosh920's Player Dialogue Camera as the proof of concept.
- **Engineering (Claude):** render-camera-only placement so NPCs keep facing the player; Starfield camera math (metres, axis layout); player body rebuild (camera cycle, then direct refresh); conversation start/exit handling (frame prediction, learned dialogue camera position, clean first-person exit); thread-safety and crash fixes; finding the conversation blur's live settings (focus on the person in shot); reading each line's expression tag; merchant detection from the faction vendor flag; adapting CCC to Starfield's narrow dialogue lens (distance caps, over-the-shoulder swing-in, interior/exterior limits); cutting styles; small-speaker rule; obstacle detection (finding and calling the game's own dialogue-camera sphere cast, skipping or pulling in blocked shots, best-fit fallback); settings; diagnostics.

## Update log

The version history is in [CHANGELOG.md](CHANGELOG.md).
