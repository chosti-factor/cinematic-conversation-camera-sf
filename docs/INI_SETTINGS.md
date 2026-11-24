# INI settings

Last updated: 0.9.66 (2026-11-23).

`Data\SFSE\Plugins\CinematicConversationCameraSF.ini` (what each of its sections holds: the
README's Settings table). Here: the settings page and the INI, the hidden settings, every
setting's place (section 6), and the review that set them out (sections 1-5).

## The settings page and the INI

Each setting is explained in the INI itself. A few more, found by testing and not worth
changing, aren't in the INI but are read from `[Advanced]` if you add them (defaults shown):
`fShortLineSeconds=1.2` (lines shorter don't count for cutting), `fObstacleRadius=0.05`,
`fBystanderStillNpc=1.5`, `fFaceDeadband=0.04` (head movement the camera ignores),
`fBusyFollowSmoothing=1.0` and `fBusyHeadSpeed=0.25` (following a speaker on the move),
`fExitCutSeconds=0.4` (the first-person exit's cut after the last line),
`fFilmStopSeconds=0.3`, and the plain over-the-shoulder shot's offsets in metres:
`fDistanceBehind=0.65`, `fSideOffset=0.32`, `fHeightOffset=0.06`, `fLookHeightOffset=-0.03`,
`fSeatedForward=0.25`, `fSeatedSide=0.9`, `fSeatedHeight=0.3`, `fOverSeatedBehind=0.35`,
`fOverSeatedSide=0.6`, `fOverSeatedHeight=-0.25`, `fVendorSideNpc=0.9`,
`fVendorHeightNpc=0.05`. And since 0.9.63: `fReactionLineMinSecondsPlayer=2.5` and
`fReactionLineMaxSecondsPlayer=7` (the lengths of their lines a reaction of you can start on),
`fChoiceCloseUpSecondsPlayer=2` (the tagged-choice close-up's length; never shorter than the preset's shortest hold, 0.9.67), `iRoomShotMinRoom=60`
(room shots left out in rooms giving less than this % of the distance they want),
`fFollowSmoothing=0.35` (seconds of smoothing when the camera follows someone who moves),
`fWideFocusBeyond=3.5` (focus style Wide: the sharp zone deepens beyond this many metres),
`fListenHold=0.5` (the controller's listen button, held this long), and `iMinLines`,
`iMaxLines`, `fShortestHold` (the cutting rhythm's lines between new angles and shortest hold,
overriding the preset's paced values; the cutting pace covers them).

With [SFSE Menu Framework](https://www.nexusmods.com/starfield/mods/18201) installed, the
main choices are also in the game: Mod Menu (F1) → **Cinematic Conversation Camera SF**, on
five pages (General, Camera, Close-ups and reactions, Screen, Filming people talking; General
since 0.9.70: the mod's switch and the detailed log). The rest
stays in the INI: a preset's own numbers (close-up chances, lens, sway, distances, reaction
timing), meant for a preset editor after 1.0, and technical tuning. The page shows
your `_user.ini`'s value where you've changed a setting and the main INI's otherwise, and
writes changes to the `_user.ini`; **Reset** beside a setting removes your change. Every
setting has a tooltip; on/off settings are switches (green when on). Each page's settings come
in groups, each under a greyed line with its name. A setting that only works while another is
on sits indented under it, joined to it by a greyed corner line, and is greyed out with "(off)"
while that one is off (its tooltip says which). Changes apply from the next conversation. If there's no `_user.ini` yet, the page creates one; with
Mod Organizer it lands in Mod Organizer's `overwrite` folder (`overwrite\SFSE\Plugins`),
where the game still reads it. Per-shot tweaks ([Shots]) and everything in [Advanced] are in
the INI only.

## The review (2026-11-21 to 22)

First pass decided (the user, 2026-11-22: "go with your recommendations"; start on its own
off, the prompt on) and built in 0.9.60: the page as in section 2, the defaults in section 3,
the text fixes in section 4. `bSubtitlesInBars` and `bAutoListenSprinting` stay off by
default. Second step (the user, 2026-11-22): section 6's levels as proposed, shown with
option 2 (one [Advanced] section) and the 20 Hidden settings out of the INI; no old names or
sections read (nothing released yet). Built in 0.9.60 too.

Section by section (the user, 2026-11-23, after playing 0.9.60: many INI settings still
belong in the editor, in [Advanced] or hidden): [General] and [Director] first. A new
[PresetTuning] section before [Shots] and [Advanced] holds each preset's own numbers for the
editor: the distances, the room shots' distances, their close-up chances, extreme share and
limit, the reaction timing and chance, listening close-ups, and the close-ups of you by tier;
the tagged-choice close-up's length to [Advanced]; the obstacle and bystander checks,
close-ups following the face and focus on the person in shot to [Debug] (the user: only for
troubleshooting; off the settings page too). [Director] keeps 10 settings, all choices.
[Camera]: the lens to [PresetTuning]; it keeps the sway (and it's back on the settings page:
the user wants the override at hand; the preset editor would set each preset's own), the
game's camera in quest scenes (its default to decide after the quest-scene log) and the
focus style. [Body] gone: showing your character to [Debug]. [Interface]: the letterbox's
slide and the options' fade to [Advanced]. [Overhear] unchanged (renamed [SceneFilming] in 0.9.65, the user). [Face] (0.9.62): the
background and question chances to [PresetTuning]. The settings page: 24 entries on four
pages ("Your expressions" on "Close-ups and reactions"; the "You" page gone).
[Advanced] (0.9.63, the user's call): 10 settings hidden, the shot library, the full body from
first person and the testing messages' time to [Debug], the long-line reaction length to
[PresetTuning]; 19 left, in four groups (shots, close-ups, exits and screen, filming people
talking). [PresetTuning] in five groups (lens and distances, their close-ups, reactions of you,
close-ups of you, your face). [Face] merged into [Director] (0.9.65): `bFacePlayer` sits after
the reaction and tagged-choice switches.
(Section 6's "Where now" column is as of the first pass.)

The aim (the user, 2026-11-21): a settings page that's clear for people, with fewer options
on it; some options hidden (INI only), some kept for a preset editor after 1.0 (Mulling).

## 1. The principle proposed

- **Settings page:** choices a player makes about what the mod does: on or off, which preset,
  how lively, bars, subtitles, filming. 30 entries instead of 55.
- **Preset editor (after 1.0):** the numbers that shape a preset's style (close-up chances,
  lens, sway, distances, reaction timing). Until it exists they stay in the INI, still
  readable and settable as now; they only leave the page.
- **INI only:** technical tuning and workarounds people rarely need (smoothing, dead bands,
  large-speaker angles, the stop-moving distances, keys).
- A line at the bottom of the page: "More settings in CinematicConversationCameraSF.ini
  (each explained there)."

## 2. The settings page, entry by entry

Now: 55 entries on 5 pages. Proposed: keep 30, the rest to the INI (E = preset editor later,
I = INI only).

**Camera**

| Group | Entry | Proposal |
|---|---|---|
| The mod | Cinematic conversations | keep |
| Look | Preset | keep |
| Look | Cutting pace | keep |
| Look | Camera sway | E |
| Look | Lens (degrees) | E |
| Look | Camera moves | keep |
| Look | Close-ups follow the face | keep (CCC's own option) |
| Distance | Furthest indoors / outdoors (m) | E (both) |
| Staging | 180-degree rule | keep |
| Staging | Never across the eyeline | I (on, rarely worth changing) |
| Staging | Avoid walls and objects | keep |
| Staging | Avoid people in the way | keep |
| Staging | Plain shot for seated people / merchants | I (both; workarounds, on) |
| Staging | Game's camera in quest scenes | keep |
| Staging | Large speaker from, shots of you from, plain shot of you | I (all three) |
| Focus | Focus on the person in shot | keep |
| Focus | Focus style | keep |

**Close-ups and reactions**

| Group | Entry | Proposal |
|---|---|---|
| Their close-ups | Emotion close-ups | keep |
| Their close-ups | Mild / Medium / Strong lines (%) | E (all three) |
| Their close-ups | Extreme close-ups | E |
| Their close-ups | Longest close-up (s) | E |
| Persuasion | Hold on their answer | keep |
| You | Reaction shots of you | keep |
| You | Reaction chance | E |
| You | Listening reactions in close-up | E |
| You | Close-up on skill choices | keep |
| You | Skill-choice close-up (s) | E |
| You | Extreme close-ups of you | E |

**You**

| Group | Entry | Proposal |
|---|---|---|
| Your character | Show your character | keep |
| Your character | Full body from first person | I (a fix; off only shows the broken body) |
| Your face | Your expressions | keep |
| Your face | Your background's expression (%) | I |
| Your face | Asking a question (%) | I |

**Screen**

| Group | Entry | Proposal |
|---|---|---|
| Letterbox | Letterbox bars, Bar height, Fade, Subtitles in the bottom bar | keep (all four) |
| Dialogue options | Hide while they speak, Fade | keep (both) |
| Testing | Testing messages, Detailed log | keep (both, for bug reports) |

**Filming people talking**

| Group | Entry | Proposal |
|---|---|---|
| People talking near you | Film them, Start on its own, Start on its own while sprinting, Listen prompt, Range | keep (all five) |
| Ending it by moving | Distance that ends it, Standing still first, Walking on before you stop | I (all three) |

Kept: 30. To the preset editor: 13. INI only: 12.

## 3. Defaults to decide

| Setting | Now | Your own | Proposal | Why |
|---|---|---|---|---|
| `bDebugLog` | 1 | 1 | **0** | the RC plan; on for bug reports |
| `bShowMessages` | 1 | 1 | **0** | the RC plan; a testing aid |
| `bSubtitlesInBars` | 0 | 1 | your call | 0 was chosen for mods that place subtitles themselves; you play with 1 |
| `bAutoListenSprinting` | 0 | 1 | your call | 0 was your rule (0.9.31): a sprint past isn't stopped |
| `bAutoListen` | 1 | 1 | your call | it takes the camera and holds you still unasked; new players may not expect it, and the overlapping-lines known issue only follows filming |
| `iPresetKey` | 85 (U) | - | keep | a feature (presets on the fly), but INI only; only while a conversation or people talking are filmed (0.9.66) |

## 4. Comment and text fixes (whatever is decided above)

INI:
- The header says the file "is read again before every conversation": since 0.9.41 it's read
  when it changes on disk (checked twice a second outside conversations). Also say the
  settings page writes to the `_user.ini`.
- Version history in comments, not needed by players: "(0.9.27)", "(Before 0.9.26 this was
  iCuttingPace ... since 0.9.30.)", "(The named cutting styles of earlier versions are gone
  ...)", "(0.9.23)", "(0.9.14)", "(0.8)", "(0.9.25)", and [Transitions]' paragraph about 0.9.30's
  removed test modes. Proposal: out (the CHANGELOG keeps them).
- [Shots]: "(GroundLevel isn't available yet)": out, or into README's after-1.0 list.
- [Transitions] `iExitStyle`: the long explanation of style 1 vs 2 could be shortened to what
  each does on screen.
- [Debug] `bShowMessages`: say it's a testing aid, off by default.

Settings page help texts:
- "Emotion close-ups": says "on lines with a strong expression"; every tier gets them (mild
  25%, medium 40%, strong 100%). Proposal: "A close-up of the speaker on lines with an
  expression (how often: by its tier, in the INI)."
- "Reaction shots of you": "Cutting to you listening during their longer lines." Fine.

## 5. Not changing

- Setting names and sections (players' `_user.ini` files and old guides keep working).
- The INI's order, apart from grouping notes.
- [GameSettings] (one commented example).

## 6. Every setting: where it is, and how advanced (proposal, 2026-11-22)

The user asked (after 0.9.60) for every setting in one table, with a suggestion of which are
advanced, as some would be better kept from players, labelled advanced, or put in a separate
advanced section.

Levels proposed:
- **Page:** on the settings page (and in the INI). The main choices.
- **Standard:** in the INI's normal sections: a preset's own numbers (for the preset editor
  after 1.0), your keys, your face's chances. Worth changing for players who want to.
- **Advanced:** technical tuning and workarounds that are rarely worth changing; kept in the
  INI but marked or moved (see the options below).
- **Hidden:** fine-tuning found by testing that a player shouldn't need: out of the shipped
  INI, still read if someone adds it (listed in a doc for whoever wants them).

Counts: Page 30, Standard 25, Advanced 31, Hidden 20 (106 in all; `fCuttingPace<Preset>` and the
[Shots] lines counted once).

| Section | Setting | Default | Where now | Level | What it does |
|---|---|---|---|---|---|
| General | `bEnabled` | 1 | INI + page | Page | The mod on or off |
| General | `bDebugLog` | 0 | INI + page | Page | Detailed log, for bug reports |
| Director | `bEnabled` | 1 | INI | Advanced | The shot library on (off: plain over-the-shoulder shots only) |
| Director | `sPreset` | Nebula | INI + page | Page | The preset |
| Director | `fCuttingPace` | 0 | INI + page | Page | How lively the cutting is (also per preset: fCuttingPace<Preset>) |
| Director | `iMinLines` | 2 | INI | Advanced | Fewest lines before a new angle (commented out: the preset's own) |
| Director | `iMaxLines` | 5 | INI | Advanced | Most lines before a new angle (commented out: the preset's own) |
| Director | `fShortestHold` | 2.0 | INI | Advanced | Shortest hold of a shot (commented out: the preset's own) |
| Director | `fShortLineSeconds` | 1.2 | INI | Hidden | Lines shorter than this don't count for cutting |
| Director | `fMaxCameraDistance` | 0 | INI | Standard | Furthest the camera stands, indoors (0 = the preset's own) |
| Director | `fMaxCameraDistanceExterior` | 0 | INI | Standard | Furthest the camera stands, outdoors (0 = the preset's own) |
| Director | `bObstacleCheck` | 1 | INI + page | Page | Avoid walls and objects |
| Director | `fObstacleRadius` | 0.05 | INI | Hidden | Width of the obstacle check (the game's own value) |
| Director | `bTrue180` | 0 | INI + page | Page | 180-degree rule |
| Director | `bEnforceLine` | 1 | INI | Advanced | Never across the eyeline |
| Director | `bFollowFace` | 1 | INI + page | Page | Close-ups follow the face |
| Director | `bReactionShotsPlayer` | -1 | INI + page | Page | Reaction shots of you |
| Director | `bChoiceCloseUpsPlayer` | -1 | INI + page | Page | Close-up of you on skill choices |
| Director | `fChoiceCloseUpSecondsPlayer` | 2 | INI | Standard | How long that close-up lasts |
| Director | `fReactionAfterSecondsPlayer` | -1 | INI | Standard | Their talking before a reaction of you is due |
| Director | `iReactionChancePlayer` | 100 | INI | Standard | Chance a due reaction is taken |
| Director | `fReactionLineMinSecondsPlayer` | 2.5 | INI | Advanced | Shortest line a reaction can take |
| Director | `fReactionLineMaxSecondsPlayer` | 7 | INI | Advanced | Longest line a reaction takes whole |
| Director | `fReactionLongLineSecondsPlayer` | 4 | INI | Advanced | Longer lines: how much of it is shown on you |
| Director | `iCloseUpChanceMildPlayer` | 25 | INI | Standard | Close-up of you: mild expressions (%) |
| Director | `iCloseUpChanceMediumPlayer` | 40 | INI | Standard | Close-up of you: medium expressions (%) |
| Director | `iCloseUpChanceStrongPlayer` | 100 | INI | Standard | Close-up of you: strong expressions (%) |
| Director | `iExtremeCloseUpChancePlayer` | 40 | INI | Standard | Extreme close-ups of you (%) |
| Director | `bListeningCloseUpsPlayer` | -1 | INI | Standard | Listening reactions may be close-ups |
| Director | `bAvoidBystandersNpc` | 1 | INI + page | Page | Avoid people in the way |
| Director | `fBystanderStillNpc` | 1.5 | INI | Hidden | How long someone stands still to count as in the way |
| Director | `bPersuasionBeatNpc` | 1 | INI + page | Page | Hold on their answer (persuasion) |
| Director | `sBeatTags` | Intimidate,Intimidati... | INI | Advanced | Skill tags that get the persuasion beat |
| Director | `bEmotionCloseUpsNpc` | -1 | INI + page | Page | Emotion close-ups of the speaker |
| Director | `sCloseUpEmotionsMildNpc` | Amused,Confident,Thin... | INI | Advanced | Which expressions are mild |
| Director | `iCloseUpChanceMildNpc` | 25 | INI | Standard | Close-up chance, mild lines (%) |
| Director | `sCloseUpEmotionsMediumNpc` | Concerned,Serious,Smi... | INI | Advanced | Which expressions are medium |
| Director | `iCloseUpChanceMediumNpc` | 40 | INI | Standard | Close-up chance, medium lines (%) |
| Director | `sCloseUpEmotionsStrongNpc` | VeryAngry,Rage,Angry,... | INI | Advanced | Which expressions are strong |
| Director | `iCloseUpChanceStrongNpc` | 100 | INI | Standard | Close-up chance, strong lines (%) |
| Director | `iExtremeCloseUpChanceNpc` | -1 | INI | Standard | Share of their close-ups that are extreme |
| Director | `fCloseUpLimitSecondsNpc` | -1 | INI | Standard | Longest one close-up of them holds |
| Director | `iRoomShotMinRoom` | 60 | INI | Advanced | Room shots left out in rooms smaller than this (%) |
| Director | `fRoomShotMaxIndoors` | -1 | INI | Standard | Furthest back room shots stand, indoors |
| Director | `fRoomShotMaxOutdoors` | -1 | INI | Standard | Furthest back room shots stand, outdoors |
| Director | `bPlainShotsSeatedNpc` | 1 | INI | Advanced | Plain shot for seated people |
| Director | `bPlainShotsMerchantsNpc` | 1 | INI | Advanced | Plain shot for merchants |
| Director | `bCameraMoves` | 1 | INI + page | Page | Camera moves (push-ins, drifts...) |
| Shots | `(per shot, e.g. CloseUp)` | 80 | INI | Standard | Per-shot tweaks: weight, move, amount, time (examples, commented out) |
| Camera | `fConversationLens` | -1 | INI | Standard | One lens for every angle (-1 = each angle's own) |
| Camera | `iShotMode` | 2 | INI | Advanced | Who gets the camera (shot/reverse-shot, or one side only) |
| Camera | `fFaceDeadband` | 0.04 | INI | Hidden | Head movement the camera ignores |
| Camera | `iCameraSway` | -1 | INI | Standard | Camera sway (-1 = the preset's own) |
| Camera | `fFollowSmoothing` | 0.35 | INI | Advanced | Smoothing when following someone who moves |
| Camera | `fBusyFollowSmoothing` | 1.0 | INI | Hidden | Smoothing for a speaker on the move |
| Camera | `fBusyHeadSpeed` | 0.25 | INI | Hidden | Head speed that counts as on the move |
| Camera | `fMinSpeakerHeightNpc` | 1.0 | INI | Advanced | Shorter speakers (small robots) keep the game's camera |
| Camera | `fLargeSpeakerHeightNpc` | 1.70 | INI | Advanced | Large speaker from this height (Vasco) |
| Camera | `fLargeSpeakerAnglePlayer` | 38 | INI | Advanced | Large speaker: how wide the shots of you stay |
| Camera | `fLargeSpeakerPlainAnglePlayer` | 45 | INI | Advanced | Large speaker: the plain shot of you |
| Camera | `bGameCameraInScenes` | 1 | INI + page | Page | Game's camera in quest scenes |
| Camera | `bFocusOnSubject` | 1 | INI + page | Page | Focus on the person in shot |
| Camera | `sFocusStyle` | Wide | INI + page | Page | Focus style (Wide, Cinematic) |
| Camera | `fWideFocusBeyond` | 3.5 | INI | Advanced | Wide style: from how far the sharp zone deepens |
| Body | `bRevealPlayer` | 1 | INI + page | Page | Show your character |
| Body | `bFixFirstPersonBodyPlayer` | 1 | INI | Advanced | Full body from first person (off shows the broken body) |
| Transitions | `iExitStyle` | 2 | INI | Advanced | How a conversation ends (1 closing shot, 2 cut to your view) |
| Transitions | `fExitCutSeconds` | 0.4 | INI | Hidden | First-person exit: seconds after the last line |
| PlainCamera | `fDistanceBehind` | 0.65 | INI | Hidden | Plain shot offset (when the shot library is off or nothing fits) |
| PlainCamera | `fSideOffset` | 0.32 | INI | Hidden | Plain shot offset (when the shot library is off or nothing fits) |
| PlainCamera | `fHeightOffset` | 0.06 | INI | Hidden | Plain shot offset (when the shot library is off or nothing fits) |
| PlainCamera | `fLookHeightOffset` | -0.03 | INI | Hidden | Plain shot offset (when the shot library is off or nothing fits) |
| PlainCamera | `fSeatedForward` | 0.25 | INI | Hidden | Plain shot offset (when the shot library is off or nothing fits) |
| PlainCamera | `fSeatedSide` | 0.9 | INI | Hidden | Plain shot offset (when the shot library is off or nothing fits) |
| PlainCamera | `fSeatedHeight` | 0.3 | INI | Hidden | Plain shot offset (when the shot library is off or nothing fits) |
| PlainCamera | `fOverSeatedBehind` | 0.35 | INI | Hidden | Plain shot offset (when the shot library is off or nothing fits) |
| PlainCamera | `fOverSeatedSide` | 0.6 | INI | Hidden | Plain shot offset (when the shot library is off or nothing fits) |
| PlainCamera | `fOverSeatedHeight` | -0.25 | INI | Hidden | Plain shot offset (when the shot library is off or nothing fits) |
| PlainCamera | `fVendorSideNpc` | 0.9 | INI | Hidden | Plain shot offset (when the shot library is off or nothing fits) |
| PlainCamera | `fVendorHeightNpc` | 0.05 | INI | Hidden | Plain shot offset (when the shot library is off or nothing fits) |
| Interface | `bLetterbox` | 1 | INI + page | Page | Letterbox bars |
| Interface | `fLetterboxHeight` | 0.12 | INI + page | Page | Bar height |
| Interface | `fLetterboxFade` | 0.4 | INI + page | Page | Bars' fade |
| Interface | `bSubtitlesInBars` | 0 | INI + page | Page | Subtitles in the bottom bar |
| Interface | `bHideOptionsWhileSpeaking` | 1 | INI + page | Page | Hide the options while they speak |
| Interface | `fOptionsFade` | 0 | INI + page | Page | Options' fade |
| SceneFilming | `bFilmConversationsNpc` | 1 | INI + page | Page | Film people talking near you |
| SceneFilming | `bListenPrompt` | 1 | INI + page | Page | Listen prompt |
| SceneFilming | `bAutoListen` | 0 | INI + page | Page | Start on its own |
| SceneFilming | `bAutoListenSprinting` | 0 | INI + page | Page | Start on its own while sprinting |
| SceneFilming | `fFilmEndSecondsNpc` | 0.5 | INI | Advanced | Hand back this long after the last word |
| SceneFilming | `fFilmEndSecondsSceneNpc` | 4 | INI | Advanced | ...or up to this long mid-scene |
| SceneFilming | `fFilmStopSeconds` | 0.3 | INI | Hidden | Standing still before the stop distance applies |
| SceneFilming | `fFilmStopDistance` | 0.25 | INI | Advanced | Moving this far ends it |
| SceneFilming | `fFilmWalkOnDistance` | 3 | INI | Advanced | Listen-key start: walking on this far ends it |
| SceneFilming | `iListenKey` | 78 | INI | Standard | The listen key (keyboard) |
| SceneFilming | `iListenButton` | 8192 | INI | Standard | The listen button (controller) |
| SceneFilming | `fListenHold` | 0.5 | INI | Advanced | How long the button is held |
| SceneFilming | `fListenRange` | 12 | INI + page | Page | Range |
| Face | `bFacePlayer` | 1 | INI + page | Page | Your expressions |
| Face | `iSignatureChancePlayer` | 30 | INI | Standard | Your background's expression (%) |
| Face | `iQuestionChancePlayer` | 40 | INI | Standard | Asking a question (%) |
| Debug | `bShowMessages` | 0 | INI + page | Page | Testing messages |
| Debug | `fMessageSeconds` | 6 | INI | Advanced | How long a testing message stays |
| Debug | `iPresetKey` | 85 | INI | Standard | The preset key (U) |
| GameSettings | (the game's own settings) | - | INI | Advanced | Changing game settings while it runs (one commented example) |

Ways to show Advanced (to decide):
1. **A marked block at the end of each section:** `; ---- Advanced: rarely worth changing ----`
   with those settings under it. Names and sections stay as they are (your `_user.ini` keeps
   working); the least change.
2. **One [Advanced] section at the end of the INI**, with everything advanced in it. Clearest
   for players; the code reads each setting from [Advanced] first and its old section second,
   so `_user.ini` files keep working.
3. **"(advanced)" in each comment**, settings left in place. Simplest, least clear.
