# Close-ups and reaction shots

How the camera decides, on each NPC line, between their emotion close-up, a reaction shot
of you listening, and a close-up of you. Since 0.8.13 these are CCC SF's presets' own
(Frontier, Starborn, Constellation, Nebula): CCC's keep CCC's vision, with no reactions
(off as CCC ships) and emotion close-ups only in Close, on strong lines
(`bEmotionCloseUpsNpc` and `bReactionShotsPlayer`: -1 the preset's own, 0 off, 1 on in
every preset). Settings are in the `[Director]` section of
the INI. Settings ending in `Npc` are about shots of the speaker, `Player` shots of you.

Last updated: 0.9.11 (2026-11-17; no change to reactions since 0.9.0). 0.8.5 reworked reactions to break the monotony of long
stretches, cutting only at the edges of their lines; 0.8.6 made them inserts.

## On each NPC line

1. **Emotion line?** When the line starts, its expression tag is read. If it's on one of
   the three tiers, a roll of the tier's chance makes it an emotion line: mild
   (`sCloseUpEmotionsMildNpc`: Amused, Confident, Thinking, Grateful, Question, Impressed,
   Tired, Concentrating, Relieved) 25%, medium (`...MediumNpc`: Concerned, Serious, Smirk,
   Happy, Confused, Puzzled, Apologetic, Awed) 40%, strong (`...StrongNpc`: all the rest
   but Neutral and Friendly) 100%; and a roll of
   `iExtremeCloseUpChanceNpc` (40%) makes it an extreme one. The camera cuts to a close-up
   (or extreme close-up) of them, pushing in slowly. If it's already on their close-up, it
   stays.
2. **Persuasion answer?** After each round of a persuasion (the verdict included) or a
   chosen skill check, their answer is held on them (`bPersuasionBeatNpc`): a close-up,
   pushing in; in a long answer (over 10 s), once the push-in is done, one cut to a medium
   or three-quarter shot of them. In CCC's presets their whole reply stays on them, as CCC
   does. In CCC SF's, the hold covers their answer and the next line (the verdict); the
   line after that is a reaction shot of you, whatever its length or expression (0.8.18:
   Sieghart's won persuasion stayed on him for 23 s). No other reaction cuts into the hold.
3. **Wait** up to 0.8 s for the line's subtitle and your face's expression to be set.
4. (Extreme close-ups are never cut away from during their line; each holds its own line
   only, 0.8.9: the next line goes to a regular close-up if it's an emotion line too,
   otherwise back to their usual angle. Never two in a row.)
5. **Time on them.** Their talking adds up while the camera stays on them without showing
   you. Your own turns, and every reaction, start it again; a reaction still due when your
   turn comes is dropped (0.8.7).
6. **A reaction due?** When that time reaches `fReactionAfterSecondsPlayer` (the preset's
   own: Frontier 10 s, Starborn 9 s, Constellation 12 s, Nebula 7 s), a
   reaction is due, `iReactionChancePlayer` (100%) of the time; a no waits another stretch.
7. **Taken by the next line that suits it:** one of theirs that starts after it's due and
   lasts `fReactionLineMinSecondsPlayer` to `fReactionLineMaxSecondsPlayer` (2.5 to 7 s,
   guessed from its subtitle at about 14 characters a second), not a persuasion answer.
   Their emotion close-ups are part of the stretch: when the time on them reaches 7 s
   during a regular one, the camera cuts to you there, to the end of the line (if 2.5 to
   7 s of it is left); never during an extreme one. When the 7 s come within the close-up's
   first 2 s (`fShortestHold`), or were already reached when it started, the reaction waits
   for the next line, so the close-up isn't cut away from at once. The line is shown on you
   from its start to its end, so the cuts fall at the edges of their lines.
   A line up to a second longer (7 to 8 s) is shown on you whole; a longer one takes it too
   (0.8.7): on you from its start for `fReactionLongLineSecondsPlayer` (4 s), then back to
   them for the rest of the line.
   - An emotion line that carries on the close-up already on screen from their last line
     (a regular one, on at least 2 s, no cut since) is taken like any line (0.8.8): the
     close-up isn't new, so it isn't cut away from at once.
   - Your face shows an expression on that line: a close-up of you, pushing in, by the
     expression's tier (the same lists as theirs): mild 25%, medium 40%, strong always
     (`iCloseUpChanceMildPlayer`, `...MediumPlayer`, `...StrongPlayer`);
     `iExtremeCloseUpChancePlayer` (40%) of those an extreme one.
   - Otherwise, you listening, in the preset's angle of you: your regular shots (shoulder,
     three-quarter, medium, close, long...), never the room, the two-shot or the profile of
     you both.
8. **In a long speech,** again after every 7 s stretch.

Lines that follow each other without a pause count as separate lines (0.8.4), so a long
reply made of several lines gets its edit points between them.

## An insert, not a turn

A reaction is a brief aside: the angle of you is picked for it alone (your usual angle if it
suits a reaction, never a room shot or a two-shot), and when it ends the camera goes back
to exactly the speaker's shot from before. The preset's rhythm (a new angle every 2 to 5
lines, a 50% chance of a fresh angle when the turn passes) and its mix of shots are the same
with or without reactions.

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
| Reactions by default | Off | On in CCC SF's presets, off in CCC's (as CCC) |
| When | Every 3rd line in a reply, a 50% roll | After 7 s of their talking without a cut to you (a long stretch), every time |
| Which line | The next eligible line | The next line of 2.5 to 8 s; a longer one for its first 4 s |
| Short exchanges | Can get one if the reply has 3 lines | Never |
| Angle of you | Your regular shots, never the room | The same |
| Length on you | The whole line | The whole line |
| Emotion close-ups | Lines the writers marked at full intensity (a few percent) | Lines with an expression other than Neutral or Friendly, half the time (about a quarter of lines) |
| Extreme close-ups | 1 in 3 | 40% |
| Reaction during an emotion close-up | Never: it waits for the next line | When the 7 s are reached during a regular one, to the end of the line; never an extreme one |
| Close-ups of you | No (your listening face is animated, but not cut to) | Half the reactions, when your face shows an expression |
