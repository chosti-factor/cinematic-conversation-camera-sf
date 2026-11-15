# Close-ups and reaction shots

How the camera decides, on each NPC line, between their emotion close-up, a reaction shot
of you listening, and a close-up of you. The same in every preset (the presets only decide
which angle of you a listening reaction uses). Settings are in the `[Director]` section of
the INI unless noted.

Last updated: 0.8.3 (2026-11-15).

## On each NPC line

1. **Emotion line?** When the line starts, its expression tag is read. If it's on
   `sCloseUpEmotions` (every expression but Neutral and Friendly), a roll of
   `iCloseUpChance` (50%) makes it an emotion line: the camera cuts to a close-up of them,
   pushing in slowly; `iExtremeCloseUpChance` (40%) of those are extreme close-ups. If the
   camera is already on their close-up, it stays.
2. **Persuasion answer?** After a persuasion's outcome or a chosen skill check, their answer
   is held on a close-up for the whole line (`bPersuasionBeat`). Never a reaction.
3. **Wait** up to 0.8 s for the line's subtitle and your face's expression to be set.
4. **Short lines** (under 40 characters) are never shown on you.
5. **The rolls:**

   | Step | When | Chance | Result |
   | --- | --- | --- | --- |
   | a. Reaction | `iReactionEvery` (2) sentences heard since the last reaction | `iReactionChance` (50%) | You listening, the preset's angle of you |
   | b. Close-up of you (if a. didn't happen) | Your face shows an expression on this line | `iPlayerCloseUpChance` (50%, `[Face]`) | A close-up of you, pushing in; 40% extreme |
   | c. Neither | | | The line's sentences add to the count |

   Either a. or b. starts the count again. Sentences end in `.`, `!`, `?` or `...`; a line
   skipped within half a second doesn't count. The count runs across the whole
   conversation.
6. **On an emotion line**, a. and b. can still happen, but only on a line long enough
   (guessed from the subtitle, about 14 characters a second) for their close-up's minimum
   time on screen (`fShortestHold`, 2 s), then you, then at least 1.5 s of their close-up
   again. The camera stays on their close-up first, then cuts to you, then back to the same
   close-up.

## How long it stays on you

- `fReactionSeconds` (2.5 s), counted from the cut to you.
- Never less than `fReactionMinSeconds` (2 s): a reaction on a short line followed at once
  by another carries into the next line until it has had its 2 s.
- When less than about 1.5 s of their line would be left after it, it stays on you to the
  end of the line rather than cut back for a moment.
- When their line ends while the camera is on you, it stays on you: it's your turn.

## What the presets change

Nothing about how often. They change what you see:

- A listening reaction uses the preset's angle of you (Standard: a shoulder, three-quarter
  or medium of you; Room: possibly a long or wide shot).
- Emotion close-ups and close-ups of you are the same close-up shots in every preset, even
  in Room, which has none of its own; their push-in is the same too (8 s, 9 s for the
  extreme ones).
- In Close and Starborn the camera is often on their close-up already, so an emotion line
  may not show a new cut.
- Seated people and merchants: the same rules, over the plain shot.

## Compared with CCC

| | CCC | CCC SF |
| --- | --- | --- |
| Reactions by default | Off | On |
| Counting | Lines in the current reply (every 3rd); starts again each reply | Sentences across the conversation (every 2) |
| The roll | Once when the count is reached; a hit makes the next eligible line the reaction | On every line once the count is reached |
| Short lines | Under 4 words: don't count, can't be reactions | Under 40 characters: can't be reactions, still count |
| Length on you | The whole line | 2.5 s from the cut (2 s at least; whole line if short) |
| Emotion close-ups | Lines the writers marked at full intensity (a few percent) | Lines with an expression other than Neutral or Friendly, half the time (about a quarter of lines) |
| Extreme close-ups | 1 in 3 | 40% |
| Reaction during an emotion close-up | Never: it waits for the next line | After the close-up's 2 s, on long enough lines, then back to it |
| Close-ups of you | No (your listening face is animated, but not cut to) | 50% when your face shows an expression |
