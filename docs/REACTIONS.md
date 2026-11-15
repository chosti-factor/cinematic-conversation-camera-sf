# Close-ups and reaction shots

How the camera decides, on each NPC line, between their emotion close-up, a reaction shot
of you listening, and a close-up of you. The same in every preset (the presets only decide
which angle of you a listening reaction uses). Settings are in the `[Director]` section of
the INI. Settings ending in `Npc` are about shots of the speaker, `Player` shots of you.

Last updated: 0.8.3 (2026-11-15).

## On each NPC line

1. **Emotion line?** When the line starts, its expression tag is read. If it's on
   `sCloseUpEmotionsNpc` (every expression but Neutral and Friendly), a roll of
   `iCloseUpChanceNpc` (50%) makes it an emotion line, and a roll of
   `iExtremeCloseUpChanceNpc` (40%) makes it an extreme one. The camera cuts to a close-up
   (or extreme close-up) of them, pushing in slowly. If it's already on their close-up, it
   stays.
2. **Persuasion answer?** After a persuasion's outcome or a chosen skill check, their answer
   is held on a close-up for the whole line (`bPersuasionBeatNpc`). It never takes a
   reaction, nor counts towards one.
3. **Wait** up to 0.8 s for the line's subtitle and your face's expression to be set.
4. **Short lines** (under 40 characters) neither count nor take a reaction.
5. **A reaction owed?** If the last roll said yes, this line takes it, if it can (step 6):
   - your face shows an expression on this line: `iCloseUpChancePlayer` (50%) a close-up of
     you, pushing in (`iExtremeCloseUpChancePlayer`, 40%, an extreme one);
   - otherwise, you listening, in the preset's angle of you: your regular shots
     (shoulder, three-quarter, medium, close, long...), never the room, the two-shot or
     the profile of you both.
   If it can't take it, the reaction stays owed for the next line.
6. **Can an emotion line take it?** Only a regular (not extreme) close-up, and only on a
   line long enough (guessed from the subtitle, about 14 characters a second) for their
   close-up's minimum time on screen (`fShortestHold`, 2 s), then you, then at least 1.5 s
   of their close-up again. The camera stays on their close-up first, then cuts to you,
   then back to the same close-up.
7. **Otherwise, the count.** The line's sentences (ending in `.`, `!`, `?` or `...`) are
   added. When the count reaches `iReactionSentencesPlayer` (2), one roll of
   `iReactionChancePlayer` (50%): yes owes a reaction to the next line that can take it,
   no starts the count again. A line skipped within half a second is taken back, roll
   included. The count runs across the whole conversation.

## How long it stays on you

- `fReactionSecondsPlayer` (2.5 s), counted from the cut to you.
- Never less than `fReactionMinSecondsPlayer` (2 s): a reaction on a short line followed at
  once by another carries into the next line until it has had its 2 s.
- When less than about 1.5 s of their line would be left after it, it stays on you to the
  end of the line rather than cut back for a moment.
- When their line ends while the camera is on you, it stays on you: it's your turn.

The log notes each line's real length against the 14-characters-a-second guess
("Line length: ..."), to tune it.

## How often

At the defaults, after two sentences a roll; half of them owe a reaction, taken by the next
line long enough. Roughly one reaction every four or five sentences of theirs, about half of
them close-ups of you when your face shows something. About a quarter of their lines get an
emotion close-up.

## What the presets change

Nothing about how often. They change what you see:

- A listening reaction uses the preset's regular angles of you (Standard: a shoulder,
  three-quarter or medium of you; Room: possibly a long or overhead shot of you).
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
| The roll | Once when the count is reached; a yes owes the next eligible line | The same |
| Short lines | Under 4 words: don't count, can't take it | Under 40 characters: the same |
| Angle of you | Your regular shots, never the room | The same |
| Length on you | The whole line | 2.5 s from the cut (2 s at least; whole line if short) |
| Emotion close-ups | Lines the writers marked at full intensity (a few percent) | Lines with an expression other than Neutral or Friendly, half the time (about a quarter of lines) |
| Extreme close-ups | 1 in 3 | 40% |
| Reaction during an emotion close-up | Never: it waits for the next line | After a regular close-up's 2 s, on long enough lines, then back to it; never on an extreme one |
| Close-ups of you | No (your listening face is animated, but not cut to) | Half the reactions, when your face shows an expression |
