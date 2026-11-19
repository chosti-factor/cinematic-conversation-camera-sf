# Compatibility

A working list for the Nexus page: which mods work with Cinematic Conversation Camera SF,
which don't, and what still needs a test. Grouped by how sure we are. Game version
1.16.244; CCC SF 0.9.27.

Last updated: 0.9.27 (2026-11-19).

## What CCC SF touches (why some mods matter and most don't)

- **The dialogue camera:** CCC SF drives the camera while you talk, and films people talking
  near you. Any other mod that moves, zooms or replaces the dialogue camera competes with it.
- **The dialogue menu** (`DialogueMenu`): the letterbox bars are added there, and the options
  fade while the NPC speaks. A mod that replaces `dialoguemenu.swf` can work, as long as the
  options list keeps its name (`DialogueList_mc`).
- **The HUD's message layer** (`HUDMessagesMenu`): the subtitle sits in the bottom bar, and the
  testing messages and the preset name show there. A mod that replaces `hudmessagesmenu.gfx`
  can work, as long as the subtitle keeps its place in the menu
  (`BottomCenterGroup_mc.SubtitleText_mc`).
- **Your character's face** while you talk, and the camera's field of view in conversations.

Everything else (textures, quests, NPCs, ships, outposts, the rest of the HUD) is outside what
CCC SF touches.

## Tested compatible

Used alongside CCC SF throughout 0.9.x testing (about 330 mods in the test setup).

| Mod | Area | Notes |
| --- | --- | --- |
| [SFSE Menu Framework](https://www.nexusmods.com/starfield/mods/18201) | Settings page | Optional; gives CCC SF its in-game settings page (F1) |
| [rbtUI2](https://www.nexusmods.com/starfield/mods/17873) | HUD reskin, including the HUD message layer | Letterbox, subtitle in the bar and messages all work |
| PraxisUI Inventory | Inventory menu | |
| Suit Protection Meter; Readable suit protection warnings; Toggle Quest Markers with Scanner and Ship HUD; Clean Vanilla Hitmarker | HUD | |
| No Vignette (60 fps) | Image space | |
| Starvegas - Cinematic Killcam; Starshake - Vizualized Recoil | Camera, in combat | Outside conversations, so no conflict |
| AutoZoom | Camera | Enabled in the test setup; to confirm (see the testing pool) |
| Background Dialogue Unlocked (and its add-ons); Conner's Ordinary Conversations; Companions Conversations Fix; Upgraded Companions; Improved Follower Behavior | Dialogue and scenes | More scenes for the camera to film; no conflict |
| Souls of Cities; NPCs Have Routines and Stores Have Schedules; Better NPCs | NPCs | |
| Starfield Engine Fixes; Address Library for SFSE Plugins; Cassiopeia Papyrus Extender; Console Command Runner; BA2 Limit Fix | Frameworks and fixes | |
| Mod Organizer 2 | Mod manager | The settings page's `_user.ini` lands in MO2's `overwrite` folder, where the game still reads it |

## Likely compatible (untested)

| Mod | Area | Why we expect it to work | Watch for |
| --- | --- | --- | --- |
| [Enhanced Dialogue Interface](https://nexusmods.com/starfield/mods/871) | Dialogue menu | Restyles the menu; the options list keeps its role | The options not fading, the bars missing |
| [Enhanced Subtitles](https://www.nexusmods.com/starfield/mods/1914) | Subtitles | Restyles the subtitle; its "fixed position during dialogue" may disagree with CCC SF's subtitle in the bar | The subtitle in two places, or over the picture |
| [StarUI HUD](https://www.nexusmods.com/starfield/mods/3444) | HUD widgets | Leaves the message layer alone, as far as its page says | |
| [Venworks Customizable HUD and Themes](https://www.nexusmods.com/starfield/mods/17104) | HUD and its message layer | Replaces `hudmessagesmenu.gfx`; works if the subtitle keeps its place in the menu | The subtitle not moving into the bar |
| Better Dialogue Controls | Dialogue menu input | Changes keys and timing, not the layout | |

## Incompatible (tested)

None so far.

## Likely incompatible (untested)

| Mod or setting | Why |
| --- | --- |
| Other dialogue camera mods (for example Player Camera during dialogue, disabled in the test setup) and mods that turn the dialogue camera off or change its zoom | Two mods driving the same camera; use one |
| INI tweaks to the dialogue camera or its field of view | CCC SF sets the lens while it films; the tweak will be overridden or fight it |
| Camera shake or sway mods that act during dialogue | CCC SF has its own sway per preset; the two add up |
| Mods that rename the dialogue options list or move the subtitle out of the HUD message layer | The letterbox, the fading options or the subtitle in the bar won't find them |

## Testing pool (for 1.0)

Five UI mods, one at a time; for each, a conversation from first person and one from third,
and one filming of people talking nearby. Checks: the bars slide in, the subtitle sits in the
bottom bar, the options fade and come back, the testing messages show. Then AutoZoom on once
more. Venworks and rbtUI2 both replace the HUD message layer, so not together.

| Mod | Area | Recent | Popularity | Status |
| --- | --- | --- | --- | --- |
| [rbtUI2](https://www.nexusmods.com/starfield/mods/17873) | HUD and its message layer | Aug 2026, built from 1.16.244 | moderate | Already in the test setup: works |
| [Enhanced Dialogue Interface](https://nexusmods.com/starfield/mods/871) | Dialogue menu | no date shown | high | To test (check 1.16 reports first) |
| [Enhanced Subtitles](https://www.nexusmods.com/starfield/mods/1914) | Subtitles | Oct 2024 (game 1.11.36) | very high (2,888 endorsements) | To test (check 1.16 reports first) |
| [Venworks Customizable HUD and Themes](https://www.nexusmods.com/starfield/mods/17104) | HUD and its message layer | Sep 2026 (beta) | low so far | To test |
| [StarUI HUD](https://www.nexusmods.com/starfield/mods/3444) | HUD widgets | no date shown | the most popular HUD mod | To test |
