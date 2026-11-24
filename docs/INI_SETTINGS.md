# INI settings

Every setting the mod reads, where it goes, its default, and whether it's on the in-game
settings page or hidden. The INI explains each one in full; this is the reference. How the
layout came about (the reviews, what moved where and why): [INI_HISTORY.md](INI_HISTORY.md).

Last updated: 1.0.0 (2026-11-24).

- **The INI:** `Data\SFSE\Plugins\CinematicConversationCameraSF.ini`. Your own changes go in
  `CinematicConversationCameraSF_user.ini` (same folder, same section names), which updates
  don't overwrite; the settings page writes there too. Both files are read again whenever one
  changes (outside conversations, within half a second).
- **Settings page:** SFSE Menu Framework (F1), **Cinematic Conversation Camera SF**, five pages:
  General, Camera, Close-ups and reactions, Screen, Filming people talking. Its text size is the
  framework's own (its Settings: UI scale, font size).
- **Hidden:** not in the shipped INI, but read if you add them to your `_user.ini`, under the
  section shown. For testing and fine-tuning; each preset's own numbers stay hidden until the
  preset editor after 1.0.
- **-1** means "the preset's own" wherever a description says so.


## [General]

| Setting | Default | Settings page | What it does |
|---|---|---|---|
| `bEnabled` | 1 | General | The whole mod on or off (off: the game's own conversation camera) |
| `bDebugLog` | 0 | General | Detailed log, for bug reports |

## [Director]

| Setting | Default | Settings page | What it does |
|---|---|---|---|
| `sPreset` | Nebula | Camera | The preset: Standard, Close, Room (CCC's); Frontier, Starborn, Constellation, Nebula (CCC SF's) |
| `fCuttingPace` | 0 | Camera | How lively the cutting is (also per preset: fCuttingPace<Preset>) |
| `bTrue180` | 0 | Camera | 180-degree rule |
| `bCameraMoves` | 1 | Camera | Camera moves (push-ins, drifts...) |
| `bEmotionCloseUpsNpc` | -1 | Close-ups and reactions | Emotion close-ups of the speaker |
| `bPersuasionBeatNpc` | 1 | Close-ups and reactions | Hold on their answer: a close-up of the NPC's answer to each persuasion round and skill check |
| `bReactionShotsPlayer` | -1 | Close-ups and reactions | Reaction shots of you |
| `bChoiceCloseUpsPlayer` | -1 | Close-ups and reactions | Close-up of you on skill choices |
| `bFacePlayer` | 1 | Close-ups and reactions | Your expressions |

## [Camera]

| Setting | Default | Settings page | What it does |
|---|---|---|---|
| `iCameraSway` | -1 | Camera | Camera sway (-1 = the preset's own) |
| `bGameCameraInScenes` | 0 | Camera | Vanilla camera in quest scenes (0: this mod films them too) |
| `sFocusStyle` | Wide | Camera | Focus style (Wide, Cinematic) |

## [Interface]

| Setting | Default | Settings page | What it does |
|---|---|---|---|
| `bLetterbox` | 1 | Screen | Letterbox bars |
| `fLetterboxHeight` | 0.12 | Screen | Bar height |
| `bSubtitlesInBars` | 0 | Screen | Subtitles in the bottom bar |
| `bHideOptionsWhileSpeaking` | 1 | Screen | Hide the options while they speak |

## [SceneFilming]

| Setting | Default | Settings page | What it does |
|---|---|---|---|
| `bFilmConversationsNpc` | 1 | Filming people talking | Film people talking near you |
| `bListenPrompt` | 1 | Filming people talking | Listen prompt |
| `bAutoListen` | 0 | Filming people talking | Start on its own |
| `bAutoListenSprinting` | 0 | Filming people talking | Start on its own while sprinting |
| `iListenKey` | 78 (N) |  | The listen key (keyboard) |
| `iListenButton` | 8192 (B) |  | The listen button (controller) |
| `fListenRange` | 12 | Filming people talking | Range |

## [Debug]

| Setting | Default | Settings page | What it does |
|---|---|---|---|
| `bShowMessages` | 0 | Screen | Testing messages |
| `fMessageSeconds` | 6 |  | How long a testing message stays |
| `iPresetKey` | 85 (U) |  | The preset key (U) |
| `bObstacleCheck` | 1 |  | Avoid walls and objects |
| `bAvoidBystandersNpc` | 1 |  | Avoid people in the way |
| `bFollowFace` | 1 |  | Close-ups follow the face |
| `bFocusOnSubject` | 1 |  | Focus on the person in shot |
| `bRevealPlayer` | 1 |  | Show your character |
| `bDirector` | 1 |  | The shot library (all of [Director]); 0 = only the plain over-the-shoulder shot |
| `bFixFirstPersonBodyPlayer` | 1 |  | Full body from first person (off shows the broken body) |

## [Advanced]

| Setting | Default | Settings page | What it does |
|---|---|---|---|
| `bEnforceLine` | 1 |  | Never across the eyeline |
| `iShotMode` | 2 |  | Who gets the camera (shot/reverse-shot, or one side only) |
| `bPlainShotsSeatedNpc` | 1 |  | Plain shot for seated people |
| `bPlainShotsMerchantsNpc` | 1 |  | Plain shot for merchants |
| `fMinSpeakerHeightNpc` | 1 |  | Shorter speakers (small robots) keep the game's camera |
| `fLargeSpeakerHeightNpc` | 1.70 |  | Large speaker from this height (Vasco) |
| `fLargeSpeakerAnglePlayer` | 38 |  | Large speaker: how wide the shots of you stay |
| `fLargeSpeakerPlainAnglePlayer` | 45 |  | Large speaker: the plain shot of you |
| `sCloseUpEmotionsMildNpc` | Amused,Confident,Thinking,Grateful,Qu... |  | Which expressions are mild |
| `sCloseUpEmotionsMediumNpc` | Concerned,Serious,Smirk,Happy,Confuse... |  | Which expressions are medium |
| `sCloseUpEmotionsStrongNpc` | VeryAngry,Rage,Angry,Irritated,Afraid... |  | Which expressions are strong |
| `sBeatTags` | Intimidate,Intimidation,Manipulate,Ma... |  | Skill tags that get the persuasion beat |
| `iExitStyle` | 2 |  | How a conversation ends (1 closing shot, 2 cut to your view) |
| `fLetterboxFade` | 0.4 |  | Bars' fade |
| `fOptionsFade` | 0 |  | Options' fade |
| `fFilmEndSecondsNpc` | 0.5 |  | Hand back this long after the last word |
| `fFilmEndSecondsSceneNpc` | 4 |  | ...or up to this long mid-scene |
| `fFilmStopDistance` | 0.25 |  | Moving this far ends it |
| `fFilmWalkOnDistance` | 3 |  | Listen-key start: walking on this far ends it |

## Hidden: each preset's own numbers (20, until the preset editor)

Add under `[Advanced]`. A number sets it for every preset; -1 (or 0 where it says so) is each
preset's own. Each preset's values: [PRESETS.md](PRESETS.md); the full explanations as the INI had
them: [INI_HISTORY.md](INI_HISTORY.md), the appendix.

| Setting | Default | What it does |
|---|---|---|
| `fRoomShotMaxIndoors` | -1 | Furthest back room shots stand, indoors |
| `fRoomShotMaxOutdoors` | -1 | Furthest back room shots stand, outdoors |
| `fConversationLens` | -1 | One lens for every angle (-1 = each angle's own) |
| `iQuestionChancePlayer` | 40 | Asking a question (%) |
| `iSignatureChancePlayer` | 30 | Your background's expression (%) |
| `iCloseUpChanceMildPlayer` | 25 | Close-up of you: mild expressions (%) |
| `iCloseUpChanceMediumPlayer` | 40 | Close-up of you: medium expressions (%) |
| `iCloseUpChanceStrongPlayer` | 100 | Close-up of you: strong expressions (%) |
| `iExtremeCloseUpChancePlayer` | 40 | Extreme close-ups of you (%) |
| `fMaxCameraDistance` | 0 | Furthest the camera stands, indoors (0 = the preset's own) |
| `fMaxCameraDistanceExterior` | 0 | Furthest the camera stands, outdoors (0 = the preset's own) |
| `fCloseUpLimitSecondsNpc` | -1 | Longest one close-up of them holds |
| `bListeningCloseUpsPlayer` | -1 | Listening reactions may be close-ups |
| `iReactionChancePlayer` | 100 | Chance a due reaction is taken |
| `fReactionAfterSecondsPlayer` | -1 | Their talking before a reaction of you is due |
| `fReactionLongLineSecondsPlayer` | 4 | Longer lines: how much of it is shown on you |
| `iExtremeCloseUpChanceNpc` | -1 | Share of their close-ups that are extreme |
| `iCloseUpChanceMildNpc` | 25 | Close-up chance, mild lines (%) |
| `iCloseUpChanceMediumNpc` | 40 | Close-up chance, medium lines (%) |
| `iCloseUpChanceStrongNpc` | 100 | Close-up chance, strong lines (%) |

## Hidden: technical (32)

Fine-tuning found by testing, rarely worth changing. Add under the section shown.

| Setting | Section | Default | What it does |
|---|---|---|---|
| `fListenHold` | [Advanced] | 0.5 | How long the controller's listen button is held |
| `fFilmStopSeconds` | [Advanced] | 0.3 | Standing still before the stop distance applies |
| `iRoomShotMinRoom` | [Advanced] | 60 | Room shots left out in rooms smaller than this (%) |
| `iMinLines` | [Advanced] | 0 | Fewest lines before a new angle (0 = the preset's own) |
| `iMaxLines` | [Advanced] | 0 | Most lines before a new angle (0 = the preset's own) |
| `fShortLineSeconds` | [Advanced] | 1.2 | Lines shorter than this don't count for cutting |
| `fShortestHold` | [Advanced] | 0 | Shortest hold of a shot (0 = the preset's own) |
| `fObstacleRadius` | [Advanced] | 0.05 | Width of the obstacle check (the game's own value) |
| `fChoiceCloseUpSecondsPlayer` | [Advanced] | 2 | How long the close-up of you on a tagged choice lasts (at least the preset's shortest hold) |
| `fReactionLineMinSecondsPlayer` | [Advanced] | 2.5 | Shortest line a reaction can take |
| `fReactionLineMaxSecondsPlayer` | [Advanced] | 7 | Longest line a reaction takes whole |
| `fBystanderStillNpc` | [Advanced] | 1.5 | How long someone stands still to count as in the way |
| `fFaceDeadband` | [Advanced] | 0.04 | Head movement the camera ignores |
| `fSeatedSwayCap` | [Advanced] | 0.08 | Someone sitting: how far the camera follows their head from where it rests, in metres (0 = no cap) |
| `fFollowSmoothing` | [Advanced] | 0.35 | Smoothing when following someone who moves |
| `fBusyFollowSmoothing` | [Advanced] | 1 | Smoothing for a speaker on the move |
| `fBusyHeadSpeed` | [Advanced] | 0.25 | Head speed that counts as on the move |
| `fWideFocusBeyond` | [Advanced] | 3.5 | Wide style: from how far the sharp zone deepens |
| `fDistanceBehind` | [Advanced] | 0.65 | Plain shot offset (when the shot library is off or nothing fits) |
| `fSideOffset` | [Advanced] | 0.32 | Plain shot offset (when the shot library is off or nothing fits) |
| `fHeightOffset` | [Advanced] | 0.06 | Plain shot offset (when the shot library is off or nothing fits) |
| `fLookHeightOffset` | [Advanced] | -0.03 | Plain shot offset (when the shot library is off or nothing fits) |
| `fSeatedForward` | [Advanced] | 0.25 | Plain shot offset (when the shot library is off or nothing fits) |
| `fSeatedSide` | [Advanced] | 0.9 | Plain shot offset (when the shot library is off or nothing fits) |
| `fSeatedHeight` | [Advanced] | 0.3 | Plain shot offset (when the shot library is off or nothing fits) |
| `fOverSeatedBehind` | [Advanced] | 0.35 | Plain shot offset (when the shot library is off or nothing fits) |
| `fOverSeatedSide` | [Advanced] | 0.6 | Plain shot offset (when the shot library is off or nothing fits) |
| `fOverSeatedHeight` | [Advanced] | -0.25 | Plain shot offset (when the shot library is off or nothing fits) |
| `fVendorSideNpc` | [Advanced] | 0.9 | Plain shot offset (when the shot library is off or nothing fits) |
| `fVendorHeightNpc` | [Advanced] | 0.05 | Plain shot offset (when the shot library is off or nothing fits) |
| `bSpeakerChangeRoot` | [Advanced] | 1 | At a speaker change in a scene, the shot placed against where the game's conversation camera is about to be (a frame of a wrong shot otherwise) |
| `fExitCutSeconds` | [Advanced] | 0.4 | First-person exit: seconds after the last line |

## Hidden sections and keys

| Where | What it does |
|---|---|
| `fCuttingPace<Preset>` in `[Director]` (e.g. `fCuttingPaceNebula`) | The cutting pace for one preset (-2 to +2, steps of 0.5); the settings page writes these for the preset in use. `fCuttingPace` is the fallback for presets without one |
| `[Shots]` | Changes to single shots, for every preset, one per line: `CloseUp=0` (never used), `LongNpc=100` (how often, 0-100), `LongNpcMove=Crane up`, `LongNpcMoveAmount=30` (0-100), `LongNpcMoveTime=5` (seconds). Shot and move names: INI_HISTORY.md, the appendix. Until the preset editor |
| `[GameSettings]` | The game's own settings changed while it runs, `name:Section=value` per line (e.g. `fDialogueCameraResumeTime:Dialogue=0`); take a line out to put the game's value back. Never tested in play |

## Counts

| Where | How many |
|---|---|
| Settings page | 24 |
| In the INI | 54 ([General] 2, [Director] 9, [Camera] 3, [Interface] 4, [SceneFilming] 7, [Debug] 10, [Advanced] 19) |
| Hidden settings | 52 (20 preset numbers, 32 technical), plus `[Shots]`, `[GameSettings]` and the per-preset cutting paces |
