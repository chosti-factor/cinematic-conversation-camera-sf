# Line expressions seen in testing

Every spoken NPC line carries one of the game's facial-expression tags
(`AnimFaceArchetype*` keywords), read by this mod for the emotion close-ups
(`sCloseUpEmotions`). This is a running tally from the test logs, to choose which
tags push in. ✱ = on the close-up list at the time of writing.

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

## Your character's face

The game keeps your face neutral and your lines carry no expression tags, so the mod sets
your expression (`[Face]` in the INI). Kept up to date with every build.

Last updated: 0.7.11 (2026-11-14).

### Listening: your face's answer to the NPC's line

Mirrored where it's natural, otherwise a listener's response. Lines with no expression (or
Neutral) leave your face to the game, or show your background now and then (below).

| NPC's expression | Your face |
|---|---|
| Friendly, Grateful | Friendly |
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

### Your background (`iSignatureChance`, 30%)

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

### Your traits

| Trait | Effect |
|---|---|
| Introvert | Subtler: 40% of lines get no visible answer; background expressions and your emotion close-ups half as often |
| Extrovert | Fuller: background expressions and your emotion close-ups half as often again |
| Empath | Mirrors the speaker: Worried, Nervous, Afraid, Apologetic, Somber, Depressed, Pleading, InPain, Happy, Amused, Grateful, Relieved, Surprised, Awed, Confused, Thinking, Concerned and Tired are worn as they are; anger brings concern (Rage: Afraid) |
| Wanted | Wary: 15% of neutral moments show Suspicious, whatever your background |

Other traits don't change your face.

### Your emotion close-ups (`iPlayerCloseUpChance`, 20%)

When your face answers a line with an expression (40 characters or longer, not an NPC
emotion close-up), this chance the line is shown on you as a close-up, pushing in; an
extreme close-up as often as `iExtremeCloseUpChance`. Counts as a reaction shot.
