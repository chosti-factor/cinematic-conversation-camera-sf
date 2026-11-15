# Line expressions seen in testing

Every spoken NPC line carries one of the game's facial-expression tags
(`AnimFaceArchetype*` keywords), read by this mod for the emotion close-ups
(`sCloseUpEmotions...Npc` tiers). This is a running tally from the test logs, to choose which
tags push in. ✱ = on the close-up list at the time of writing (0.8.3: every expression but
Neutral and Friendly, see the notes below the second table).

Last updated: 2026-11-13, logs from 0.4.11 to 0.5.4, 707 NPC lines (every spoken line
counted, so lines heard twice count twice).

| Tag | Lines | Share |
|---|---|---|
| (no tag: greetings, shop barks) | 173 | 24% |
| Friendly | 171 | 24% |
| Amused | 63 | 9% |
| Confident | 61 | 9% |
| Thinking | 37 | 5% |
| Grateful | 37 | 5% |
| Concerned ✱ | 31 | 4% |
| Serious ✱ | 23 | 3% |
| Smirk ✱ | 18 | 3% |
| Neutral | 12 | 2% |
| Question | 10 | 1% |
| Nervous ✱ | 10 | 1% |
| Apologetic ✱ | 10 | 1% |
| Worried ✱ | 9 | 1% |
| Happy ✱ | 7 | <1% |
| Confused ✱ | 7 | <1% |
| Irritated ✱ | 5 | <1% |
| Impressed | 5 | <1% |
| Surprised ✱ | 3 | <1% |
| Somber ✱ | 3 | <1% |
| Awed ✱ | 3 | <1% |
| Tired | 2 | <1% |
| Puzzled ✱ | 2 | <1% |
| Suspicious ✱ | 1 | <1% |
| SinisterSmile | 1 | <1% |
| Relieved ✱ | 1 | <1% |
| Disgust ✱ | 1 | <1% |
| Concentrating ✱ | 1 | <1% |

Notes:

- 0.6.2 added Happy, Smirk, Puzzled, Suspicious, Relieved and Concentrating to the list.
  The list now covers about 19% of all lines (was 15%); Concerned,
  Serious and Smirk account for over half of that.
- New since the last tally: SinisterSmile (one line, Ularu Chen). Not on the list yet; a
  candidate for close-ups.
- Starfield has no emotion strength per line (Skyrim's writers set one, which CCC uses), so
  the tag is all there is to go on.

## All the game's expressions

The game's facial expressions (`AnimFaceArchetype*` keywords) that an actor's face can be set to,
from the list in Player Camera During Dialogue by rrrafau, which applies them to the player.
The same set tags NPC lines; several have never come up in testing (they may be rare in the
game's dialogue, or used mainly by scenes). These are also the expressions available for your
character's face (planned).

| Expression | Seen in testing | On the close-up list |
|---|---|---|
| Neutral | yes |  |
| Afraid | - | ✱ |
| Amused | yes |  |
| Angry | - | ✱ |
| Apologetic | yes | ✱ |
| Awed | yes | ✱ |
| Concentrating | yes | ✱ |
| Concerned | yes | ✱ |
| Confident | yes |  |
| Confused | yes | ✱ |
| Defiant | - | ✱ |
| Depressed | - | ✱ |
| Disbelief | - | ✱ |
| Disgust | yes | ✱ |
| Dying | - |  |
| EyesClosed | - |  |
| Flirting | - | ✱ |
| Friendly | yes |  |
| Grateful | yes |  |
| Happy | yes | ✱ |
| Impressed | yes |  |
| InPain | - |  |
| Irritated | yes | ✱ |
| Nervous | yes | ✱ |
| Pleading | - | ✱ |
| Puzzled | yes | ✱ |
| Question | yes |  |
| Rage | - | ✱ |
| Relieved | yes | ✱ |
| Sarcastic | - | ✱ |
| Serious | yes | ✱ |
| Shocked | - | ✱ |
| SinisterSmile | yes |  |
| Smirk | yes | ✱ |
| Smug | - | ✱ |
| Somber | yes | ✱ |
| Stern | - | ✱ |
| Surprised | yes | ✱ |
| Suspicious | yes | ✱ |
| Thinking | yes |  |
| Tired | yes |  |
| VeryAngry | - | ✱ |
| Worried | yes | ✱ |

Notes:

- 0.7.4 added Stern, Defiant, Smug, Sarcastic, Flirting, Pleading and Disbelief to the
  close-up list.
- 0.7.4 also replaced six guessed names that aren't real expressions (Angry, Annoyed, Fearful,
  Scared, Disgusted, Sad) with the game's own: VeryAngry, Rage, Afraid, Depressed and Shocked.
- 0.8.3: every expression but Neutral and Friendly is on the list (the ✱ column above is
  from before), and a listed line gets its close-up 50% of the time (`iCloseUpChanceNpc`).
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

## Your character's face

The game keeps your face neutral and your lines carry no expression tags, so the mod sets
your expression (`[Face]` in the INI). Kept up to date with every build.

Last updated: 0.8.9 (2026-11-15).

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

A reaction shot on a line your face has no answer for (no expression on it) gives your face
a listening look for it: your background's expression, or Concentrating. The shot message
names your expression.

### Your emotion close-ups (by tier, 0.8.6)

A kind of reaction shot: when a line is shown on you as a reaction and your face answers it
with an expression, a close-up of you, pushing in, rather than the preset's angle of you
listening, by the expression's tier (the same lists as the NPCs' emotion close-ups): mild
25%, medium 40%, strong always (`iCloseUpChanceMildPlayer`, `iCloseUpChanceMediumPlayer`,
`iCloseUpChanceStrongPlayer`); `iExtremeCloseUpChancePlayer` (40%) of those are extreme
close-ups. How often reactions come: docs/REACTIONS.md.
