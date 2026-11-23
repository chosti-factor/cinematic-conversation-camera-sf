# INI and settings page review (proposal)

Last updated: 0.9.59 (2026-11-22). A proposal for the user to decide on; nothing here is in
the code yet. Once decided, this file records what was chosen and is then folded into the
CHANGELOG and README.

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
| `iPresetKey` | 85 (U) | - | keep | a feature (presets on the fly), but INI only |

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
