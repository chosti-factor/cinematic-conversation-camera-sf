# Compatibility

A working list for the Nexus page: which mods work with Cinematic Conversation Camera SF,
which don't, and what still needs a test. Grouped by how sure we are. Game version
1.16.244; CCC SF 0.9.27 to 1.0.0.

Last updated: 1.0.0 (2026-11-25).

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
- **The game's HUD** (`HUDMenu`): hidden as a whole while CCC SF films people talking near you,
  and put back after. A HUD mod that draws into it is hidden with it, which is intended.
- **Your character's face** while you talk, and the camera's field of view in conversations.

Everything else (textures, quests, NPCs, ships, outposts, the rest of the HUD) is outside what
CCC SF touches.

## Tested compatible

Active alongside CCC SF through every 0.9.x test and the 1.0 test, which makes them the most tested of all (the
test setup runs about 330 mods with 257 active plugins; the ones that matter to CCC SF are
listed here).

| Mod | Area | Notes |
| --- | --- | --- |
| [SFSE Menu Framework](https://www.nexusmods.com/starfield/mods/18201) | Settings page | Optional; gives CCC SF its in-game settings page (F1). One crash in its own code in the 1.0 test (its window-blur build, in its window handling; none of CCC SF on the stack), not seen again |
| [rbtUI2](https://www.nexusmods.com/starfield/mods/17873) | HUD reskin, including the HUD message layer | Letterbox, subtitle in the bar and messages all work |
| [Enhanced Subtitles](https://www.nexusmods.com/starfield/mods/1914) | Subtitles | Tested (0.9.28): CCC SF moves its taller, higher subtitle into the bottom bar. No cost to the frame rate (59.5 fps in the same conversation as without it, once other apps were closed) |
| [Venworks Customizable HUD and Themes](https://www.nexusmods.com/starfield/mods/17104) with [Venworks Canvas](https://www.nexusmods.com/starfield/mods/18327) | HUD | Tested (0.9.28; HUD 2.1.0, Canvas 1.0.5, Core 2.1.8): the bars, the subtitle in the bar, the options and the testing messages all work |
| [StarUI HUD](https://www.nexusmods.com/starfield/mods/3444) | HUD widgets | Tested (0.9.28): the bars, the subtitle in the bar, the options and the testing messages all work. Again with 0.9.34, together with Enhanced Dialogue Interface and Enhanced Subtitles: its HUD hides and comes back with the game's while filming people talking |
| [Enhanced Dialogue Interface](https://nexusmods.com/starfield/mods/871) | Dialogue menu | Tested on its own (0.9.27): the bars, the options hidden while the NPC speaks, the subtitle in the bar and your choices all work. It highlights every option, so CCC SF reads your choice from the list's own selection instead |
| PraxisUI Inventory | Inventory menu | |
| Suit Protection Meter; Readable suit protection warnings; Toggle Quest Markers with Scanner and Ship HUD; Clean Vanilla Hitmarker | HUD | |
| No Vignette (60 fps) | Image space | |
| Starvegas - Cinematic Killcam; Starshake - Vizualized Recoil | Camera, in combat | Outside conversations, so no conflict |
| Background Dialogue Unlocked (and its add-ons); Conner's Ordinary Conversations; Companions Conversations Fix; Upgraded Companions; Improved Follower Behavior | Dialogue and scenes | More scenes for the camera to film; no conflict |
| Souls of Cities; NPCs Have Routines and Stores Have Schedules; Better NPCs | NPCs | |
| Starfield Engine Fixes; Address Library for SFSE Plugins; Cassiopeia Papyrus Extender; Console Command Runner; BA2 Limit Fix | Frameworks and fixes | |
| Mod Organizer 2 | Mod manager | The settings page's `_user.ini` lands in MO2's `overwrite` folder, where the game still reads it |

## Partially compatible (tested)

| Mod | Area | What happens |
| --- | --- | --- |
| [AutoZoom](https://www.nexusmods.com/starfield/mods/10543) (Nexus mod 10543: the plugin `AutoZoom.esm` that switches you to first person when you load into an interior and back to third person outside; not the other zoom mods of similar names) | Camera, first and third person | Conversations and filming people talking work (0.9.29), but in the 1.0 test it caused unexpected freezes of your character's body and of the ship. Use it with that in mind, or leave it off |

## Likely compatible (untested)

| Mod | Area | Why we expect it to work | Watch for |
| --- | --- | --- | --- |
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
(done in 0.9.29: compatible; again in the 1.0 test: partially compatible, freezes of the body
and the ship).

| Mod | Area | Recent | Popularity | Status |
| --- | --- | --- | --- | --- |
| [rbtUI2](https://www.nexusmods.com/starfield/mods/17873) | HUD and its message layer | Aug 2026, built from 1.16.244 | moderate | Already in the test setup: works |
| [Enhanced Dialogue Interface](https://nexusmods.com/starfield/mods/871) | Dialogue menu | no date shown | high | Tested: works (0.9.27) |
| [Enhanced Subtitles](https://www.nexusmods.com/starfield/mods/1914) | Subtitles | Oct 2024 (game 1.11.36) | very high (2,888 endorsements) | Tested: works (0.9.28) |
| [Venworks Customizable HUD and Themes](https://www.nexusmods.com/starfield/mods/17104) | HUD and its message layer | Sep 2026 (beta) | low so far | Tested: works (0.9.28) |
| [StarUI HUD](https://www.nexusmods.com/starfield/mods/3444) | HUD widgets | no date shown | the most popular HUD mod | Tested: works (0.9.28) |
