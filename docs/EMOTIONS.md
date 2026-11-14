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
