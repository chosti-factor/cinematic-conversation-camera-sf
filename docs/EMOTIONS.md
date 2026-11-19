# Line expressions and emotion close-ups

Every spoken NPC line carries one of the game's facial-expression tags
(`AnimFaceArchetype*` keywords), read by this mod for the emotion close-ups and for your
character's face. Kept up to date with every build that changes them.

Last updated: 0.9.23 (2026-11-18: the close-up limit; the tagged option's close-up as you pick;
none extreme of you in Constellation).

## The rules now (0.9.0)

- **Which presets:** Frontier, Starborn and Nebula use every tier; Constellation, the
  documentary, strong lines only.
  CCC's keep CCC's vision: none in Standard and Room, strong lines only in Close (the nearest
  thing to CCC's "full intensity"). `bEmotionCloseUpsNpc`: -1 the preset's own (default),
  0 none, 1 every tier in every preset.
- **Three tiers, each with its own chance** that a line pushes in to a close-up of the
  speaker (`sCloseUpEmotions...Npc`, `iCloseUpChance...Npc`):

  | Tier | Expressions | Chance |
  | --- | --- | --- |
  | Mild | Amused, Confident, Thinking, Grateful, Question, Impressed, Tired, Concentrating, Relieved | 25% |
  | Medium | Concerned, Serious, Smirk, Happy, Confused, Puzzled, Apologetic, Awed | 40% |
  | Strong | VeryAngry, Rage, Angry, Irritated, Afraid, Shocked, Surprised, Disgust, Somber, Depressed, Worried, Nervous, Pleading, Disbelief, Defiant, Stern, Suspicious, Smug, Sarcastic, Flirting, SinisterSmile, InPain, Dying, EyesClosed | always |
  | None | Neutral, Friendly, no tag | never |

- About one NPC line in five overall (from the tally below); emotional characters get more
  (Sarah: about a third).
- **Extreme close-ups:** `iExtremeCloseUpChanceNpc` (40%) of them, each for its own line
  only, never two in a row; reactions never cut into one.
- **One close-up at most this long** (0.9.23, CCC SF's presets): Frontier and Nebula 12 s,
  Constellation 14, Starborn 18 (`fCloseUpLimitSecondsNpc`; CCC's presets none). Then a
  three-quarter or medium of them at the next line's start (mid-line 6 s later at most), and
  no emotion close-up for half that time. The 2026-11-18 run had runs of emotional lines
  holding one close-up for 20 to 28 s in every CCC SF preset (Andreja's four Nervous lines:
  28 s).
- **Close-ups of you** in reaction shots, by your expression's tier, the same lists and
  chances, and when you choose a tagged option (0.8.16): every tag but [Pay ...], an extreme
  close-up for Intimidate, Threaten, Manipulate, Deceive, Lie and Attack, the others extreme
  40% of the time (CCC SF's presets; none extreme in Constellation, 0.9.23), and every option
  in a persuasion (0.8.17). Since 0.9.23 it comes as you pick, over the start of their
  answer (2 s, `fChoiceCloseUpSecondsPlayer`); it used to come a turn late. After a
  persuasion's verdict, a reaction shot of you, close by your expression's tier (0.8.18,
  CCC SF's presets). See "Your character's face" below.
- **Persuasion answers** get a close-up of the NPC whatever their expression
  (`bPersuasionBeatNpc`), in every preset, seated people and merchants too.

## The tally

A running tally from the test logs, used to choose the tiers. The last column is the rule
now (0.8.9 on): ✱ mild, 25% of the time; ✱✱ medium, 40%; ✱✱✱ strong, always; none, never.
(In the presets that use emotion close-ups: see "The rules now" above.)

Last updated: 2026-11-13, logs from 0.4.11 to 0.5.4, 707 NPC lines (every spoken line
counted, so lines heard twice count twice).

| Tag | Lines | Share | Close-up tier now |
|---|---|---|---|
| (no tag: greetings, shop barks) | 173 | 24% | none |
| Friendly | 171 | 24% | none |
| Amused | 63 | 9% | ✱ mild (25%) |
| Confident | 61 | 9% | ✱ mild (25%) |
| Thinking | 37 | 5% | ✱ mild (25%) |
| Grateful | 37 | 5% | ✱ mild (25%) |
| Concerned | 31 | 4% | ✱✱ medium (40%) |
| Serious | 23 | 3% | ✱✱ medium (40%) |
| Smirk | 18 | 3% | ✱✱ medium (40%) |
| Neutral | 12 | 2% | none |
| Question | 10 | 1% | ✱ mild (25%) |
| Nervous | 10 | 1% | ✱✱✱ strong (always) |
| Apologetic | 10 | 1% | ✱✱ medium (40%) |
| Worried | 9 | 1% | ✱✱✱ strong (always) |
| Happy | 7 | <1% | ✱✱ medium (40%) |
| Confused | 7 | <1% | ✱✱ medium (40%) |
| Irritated | 5 | <1% | ✱✱✱ strong (always) |
| Impressed | 5 | <1% | ✱ mild (25%) |
| Surprised | 3 | <1% | ✱✱✱ strong (always) |
| Somber | 3 | <1% | ✱✱✱ strong (always) |
| Awed | 3 | <1% | ✱✱ medium (40%) |
| Tired | 2 | <1% | ✱ mild (25%) |
| Puzzled | 2 | <1% | ✱✱ medium (40%) |
| Suspicious | 1 | <1% | ✱✱✱ strong (always) |
| SinisterSmile | 1 | <1% | ✱✱✱ strong (always) |
| Relieved | 1 | <1% | ✱ mild (25%) |
| Disgust | 1 | <1% | ✱✱✱ strong (always) |
| Concentrating | 1 | <1% | ✱ mild (25%) |

Notes:

- 0.6.2 added Happy, Smirk, Puzzled, Suspicious, Relieved and Concentrating to the list.
  The list now covers about 19% of all lines (was 15%); Concerned,
  Serious and Smirk account for over half of that.
- New since the last tally: SinisterSmile (one line, Ularu Chen). On the strong tier since
  0.8.6.
- Starfield has no emotion strength per line (Skyrim's writers set one, which CCC uses), so
  the tag is all there is to go on.

## All the game's expressions

The game's facial expressions (`AnimFaceArchetype*` keywords) that an actor's face can be set to,
from the list in Player Camera During Dialogue by rrrafau, which applies them to the player.
The same set tags NPC lines; several have never come up in testing (they may be rare in the
game's dialogue, or used mainly by scenes). These are also the expressions your character's
face can show (below).

| Expression | Seen in testing | Close-up tier now |
|---|---|---|
| Neutral | yes | none |
| Afraid | - | ✱✱✱ strong (always) |
| Amused | yes | ✱ mild (25%) |
| Angry | - | ✱✱✱ strong (always) |
| Apologetic | yes | ✱✱ medium (40%) |
| Awed | yes | ✱✱ medium (40%) |
| Concentrating | yes | ✱ mild (25%) |
| Concerned | yes | ✱✱ medium (40%) |
| Confident | yes | ✱ mild (25%) |
| Confused | yes | ✱✱ medium (40%) |
| Defiant | - | ✱✱✱ strong (always) |
| Depressed | - | ✱✱✱ strong (always) |
| Disbelief | - | ✱✱✱ strong (always) |
| Disgust | yes | ✱✱✱ strong (always) |
| Dying | - | ✱✱✱ strong (always) |
| EyesClosed | - | ✱✱✱ strong (always) |
| Flirting | - | ✱✱✱ strong (always) |
| Friendly | yes | none |
| Grateful | yes | ✱ mild (25%) |
| Happy | yes | ✱✱ medium (40%) |
| Impressed | yes | ✱ mild (25%) |
| InPain | - | ✱✱✱ strong (always) |
| Irritated | yes | ✱✱✱ strong (always) |
| Nervous | yes | ✱✱✱ strong (always) |
| Pleading | - | ✱✱✱ strong (always) |
| Puzzled | yes | ✱✱ medium (40%) |
| Question | yes | ✱ mild (25%) |
| Rage | - | ✱✱✱ strong (always) |
| Relieved | yes | ✱ mild (25%) |
| Sarcastic | - | ✱✱✱ strong (always) |
| Serious | yes | ✱✱ medium (40%) |
| Shocked | - | ✱✱✱ strong (always) |
| SinisterSmile | yes | ✱✱✱ strong (always) |
| Smirk | yes | ✱✱ medium (40%) |
| Smug | - | ✱✱✱ strong (always) |
| Somber | yes | ✱✱✱ strong (always) |
| Stern | - | ✱✱✱ strong (always) |
| Surprised | yes | ✱✱✱ strong (always) |
| Suspicious | yes | ✱✱✱ strong (always) |
| Thinking | yes | ✱ mild (25%) |
| Tired | yes | ✱ mild (25%) |
| VeryAngry | - | ✱✱✱ strong (always) |
| Worried | yes | ✱✱✱ strong (always) |

Notes:

- 0.7.4 added Stern, Defiant, Smug, Sarcastic, Flirting, Pleading and Disbelief to the
  close-up list.
- 0.7.4 also replaced six guessed names that aren't real expressions (Angry, Annoyed, Fearful,
  Scared, Disgusted, Sad) with the game's own: VeryAngry, Rage, Afraid, Depressed and Shocked.
- 0.8.3: every expression but Neutral and Friendly is on the list, and a listed line gets its close-up 50% of the time (`iCloseUpChanceNpc`).
  From the tally at the top, that's about half of all NPC lines tagged for it (no tag, 24%,
  and Friendly, 24%, are the rest), so about a quarter of all lines get an emotion
  close-up. A reaction shot of you can interrupt one after its minimum time on screen.
- 0.8.6: three tiers with their own chances. Mild, 30% (Amused, Confident, Thinking,
  Grateful, Question, Impressed, Tired, Concentrating, Relieved: about 31% of lines in the
  tally); medium, 50% (Concerned, Serious, Smirk, Happy, Confused, Puzzled, Apologetic,
  Awed: about 14%); strong, 100% (the rest but Neutral and Friendly: about 5%). About 21%
  of all lines get an emotion close-up, most of them on lines with real feeling.
  First test (0.8.6, two conversations with Sarah): 9 close-up cuts; of the 17 tagged lines
  that didn't get one, 10 were mild (Confident 7 times) and 7 medium (Happy, Apologetic,
  Smirk). (That count missed lines that carried on a close-up already on screen.)
- 0.8.7 test (Sarah again, 34 lines): 26 tagged, 16 on an emotion close-up (47% of her
  lines: strong 6 of 6, medium 6 of 8, mild 4 of 11), in 9 close-up shots, 4 of them
  extreme. Close-ups of her were 38% of the time on screen (extreme ones 18%, 13 s each on
  average). Sarah's lines are more often tagged than the tally's average (76% against 52%,
  strong 18% against 5%).
- 0.8.9: an extreme close-up holds its own line only; never two in a row. Tier chances
  lowered to mild 25%, medium 40% (strong stays 100%), for your close-ups too: from the
  tally's shares, about 18% of all lines (was 21%); on the 0.8.7 Sarah test's mix, about
  12 of 34 lines (was 13).
- 0.8.13: the tiers apply in CCC SF's presets; CCC's keep CCC's vision (no emotion close-ups
  in Standard and Room, strong lines only in Close). `bEmotionCloseUpsNpc=1` puts every
  tier in every preset.
- 0.8.14: no change to the tiers (CCC SF's presets rebuilt; they keep every tier).

## Your character's face

The game keeps your face neutral and your lines carry no expression tags, so the mod sets
your expression (`[Face]` in the INI). Kept up to date with every build.

Last updated: 0.8.14 (2026-11-15).

### Listening: your face's answer to the NPC's line

Mirrored where it's natural, otherwise a listener's response. Lines with no expression (or
Neutral) leave your face to the game, or show your background now and then (below).

| NPC's expression | Your face |
|---|---|
| Friendly, Grateful | Friendly |
| Confident | Confident |
| Amused | Amused |
| Happy, Impressed | Happy |
| Relieved | Relieved |
| Smirk, Sarcastic, Flirting, Smug | Smirk |
| SinisterSmile | Suspicious |
| Thinking, Concentrating | Concentrating |
| Question, Puzzled | Thinking |
| Confused | Confused |
| Concerned, Worried, Nervous, Apologetic, Pleading, InPain, Tired | Concerned |
| Afraid | Worried |
| Somber, Depressed, Dying | Somber |
| Serious, Stern, Defiant | Serious |
| Suspicious | Suspicious |
| Surprised, Shocked, Disbelief | Surprised |
| Awed | Awed |
| Disgust | Disgust |
| Irritated | Serious or Irritated (at random) |
| Angry, VeryAngry | Stern, Irritated or VeryAngry (at random) |
| Rage | Afraid, VeryAngry or Rage (at random) |

### Choosing an option with a skill tag

| Tag contains | Your face |
|---|---|
| Persua, Commerce | Confident |
| Intimidat, Threat | Stern |
| Manipulat, Deceiv, Lie, Bribe | Smirk |
| Flirt | Flirting |
| Charm, Diplomacy | Friendly |
| Lecture, Theology | Serious |
| Medicine | Concerned |
| Attack | VeryAngry |
| Pay | Neutral |
| Leadership | Confident |
| Stealth | Suspicious |
| Astrophysics, Geology, Botany, Zoology, Xenosociology, Research, Surveying, Scanning | Thinking |
| Chemistry, Security, Engineering, Robotics, Piloting, Starship, Outpost, Weapon, Spacesuit | Concentrating |

An option without a tag that asks a question ("...?"): Question, `iQuestionChancePlayer` (40%)
of the time; otherwise your background may show (below).

### Choosing a background or trait option

Options tagged with a background ("[Space Scoundrel] ...") take that background's
expression (table below). Trait options:

| Trait tag | Your face |
|---|---|
| Introvert | Concentrating |
| Extrovert | Happy |
| Empath | Concerned |
| Wanted | Suspicious |
| Kid Stuff | Friendly |
| Neon Street Rat | Smirk |
| Hero Worshipped | Puzzled |
| UC Native, Freestar Settler, Raised Enlightened, Alien DNA | Confident |
| Raised Universal, Spaced | Thinking |
| Serpent's Embrace | Serious |
| Dream Home | Concerned |
| Taskmaster | Stern |
| Terra Firma | Relieved |

### Your background (`iSignatureChancePlayer`, 30%)

At moments your face would otherwise be neutral (an NPC line with no expression, an option
without a skill tag), this chance of your background's expression instead.

| Background | Expression |
|---|---|
| Space Scoundrel, Cyber Runner | Smirk |
| Gangster | Smug |
| Bounty Hunter | Suspicious |
| Diplomat, Homesteader | Friendly |
| Professor, Sculptor | Thinking |
| Soldier, Beast Hunter | Serious |
| Bouncer, Ronin | Stern |
| Chef | Happy |
| Combat Medic | Concerned |
| Explorer | Awed |
| Industrialist | Confident |
| Long Hauler | Tired |
| Pilgrim | Relieved |
| Cyberneticist, Xenobiologist | Concentrating |

### Reaction shots

Reaction shots are in CCC SF's presets (CCC's have them off, as CCC ships). A reaction shot
on a line your face has no answer for (no expression on it) gives your face
a listening look for it: your background's expression, or Concentrating. The shot message
names your expression.

### Your emotion close-ups (by tier, 0.8.6)

A kind of reaction shot: when a line is shown on you as a reaction and your face answers it
with an expression, a close-up of you, pushing in, rather than the preset's angle of you
listening, by the expression's tier (the same lists as the NPCs' emotion close-ups): mild
25%, medium 40%, strong always (`iCloseUpChanceMildPlayer`, `iCloseUpChanceMediumPlayer`,
`iCloseUpChanceStrongPlayer`); `iExtremeCloseUpChancePlayer` (40%) of those are extreme
close-ups. How often reactions come: docs/REACTIONS.md.
