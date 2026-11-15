# Close-ups and reaction shots

How the camera decides, on each NPC line, between their emotion close-up, a reaction shot
of you listening, and a close-up of you. The same in every preset (the presets only decide
which angle of you a listening reaction uses). Settings are in the `[Director]` section of
the INI. Settings ending in `Npc` are about shots of the speaker, `Player` shots of you.

Last updated: 0.8.5 (2026-11-15). 0.8.5 reworked reactions to break the monotony of long
stretches, cutting only at the edges of their lines.

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
4. (Extreme close-ups are never cut away from: the camera stays on them.)
5. **Time on them.** Their talking adds up while the camera stays on them without showing
   you. Your own turns, and every reaction, start it again.
6. **A reaction due?** When that time reaches `fReactionAfterSecondsPlayer` (7 s), a
   reaction is due, `iReactionChancePlayer` (100%) of the time; a no waits another stretch.
7. **Taken by the next line that suits it:** one of theirs that starts after it's due and
   lasts `fReactionLineMinSecondsPlayer` to `fReactionLineMaxSecondsPlayer` (2.5 to 7 s,
   guessed from its subtitle at about 14 characters a second), not a persuasion answer.
   Their emotion close-ups are part of the stretch: when the time on them reaches 7 s
   during a regular one, the camera cuts to you there, to the end of the line (if 2.5 to
   7 s of it is left); never during an extreme one. A reaction already due when an emotion
   line starts waits for the next line, so the close-up isn't cut away from at once. The line is shown on you from its start to its end, so the cuts
   fall at the edges of their lines, never in the middle.
   - Your face shows an expression on that line: `iCloseUpChancePlayer` (50%) a close-up of
     you, pushing in (`iExtremeCloseUpChancePlayer`, 40%, an extreme one).
   - Otherwise, you listening, in the preset's angle of you: your regular shots (shoulder,
     three-quarter, medium, close, long...), never the room, the two-shot or the profile of
     you both.
8. **In a long speech,** again after every 7 s stretch.

Lines that follow each other without a pause count as separate lines (0.8.4), so a long
reply made of several lines gets its edit points between them.

## How often

Short exchanges (a line or two of theirs, then you) never get a reaction. A long reply or
speech gets one about every 7 to 10 seconds of their talking, at the first line that suits
it.

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
| When | Every 3rd line in a reply, a 50% roll | After 7 s of their talking without a cut to you (a long stretch), every time |
| Which line | The next eligible line | The next line of 2.5 to 7 s |
| Short exchanges | Can get one if the reply has 3 lines | Never |
| Angle of you | Your regular shots, never the room | The same |
| Length on you | The whole line | The whole line |
| Emotion close-ups | Lines the writers marked at full intensity (a few percent) | Lines with an expression other than Neutral or Friendly, half the time (about a quarter of lines) |
| Extreme close-ups | 1 in 3 | 40% |
| Reaction during an emotion close-up | Never: it waits for the next line | When the 7 s are reached during a regular one, to the end of the line; never an extreme one |
| Close-ups of you | No (your listening face is animated, but not cut to) | Half the reactions, when your face shows an expression |
