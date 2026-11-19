# Presets

The seven looks you can choose with `sPreset` in the `[Director]` section of the INI (or
switch on the fly with the preset key, U, while testing).

- **Standard, Close and Room** are CCC's own, as faithful to CCC 1.5.0 as Starfield allows:
  its angles, how often each comes up, how each moves, each angle's own lens and each
  preset's cutting rhythm, from CCC's own exported presets. No reaction shots (off as CCC
  ships); emotion close-ups only in Close, on strong lines; no camera sway (a tripod, as CCC);
  CCC's "Hold On Their Answer" (on, as CCC ships): a close-up of the NPC's answer to every
  persuasion round, seated people included.
- **Frontier, Starborn, Constellation and Nebula** are CCC SF's own, each with a film
  flavour drawn from Starfield's "NASA-punk" influences, its own lenses, movement, cutting
  and camera sway, room shots kept closer, and everything this mod adds: emotion close-ups
  on every tier, reaction shots of you, close-ups of your expressions.

Last updated: 0.9.25 (2026-11-18; the review after the 2026-11-18 run: Starborn towards
*Interstellar*, Nebula a space opera, close-up limits, and the rest below; 0.9.24: more shots
in Frontier, Starborn and Constellation).

## At a glance

| | Standard | Close | Room | Frontier | Starborn | Constellation | Nebula |
| --- | --- | --- | --- | --- | --- | --- | --- |
| From | CCC | CCC | CCC | CCC SF, built on Standard | CCC SF, built on Close | CCC SF, built on Room | CCC SF |
| Feels like | A TV drama: classic shot / reverse shot | A film-noir conversation | A classic western or a stage play | A space western (*Firefly*, *The Expanse*) | *Interstellar* with a touch of Kubrick: the Unity, the artifacts, the uncanny | A hard sci-fi documentary (*The Martian*, *Apollo 13*) | A space opera (*Guardians of the Galaxy*, *The Fifth Element*, *Valerian*, the original *Star Wars*) |
| The look | Shoulders, close-ups, three-quarters and mediums on both sides | Shoulders, tight close-ups and extreme close-ups | Wide and full-figure shots | Standard on slightly longer lenses, with the room now and then; singles with a shoulder edging in, heroic low angles, wide over-the-shoulders | Close-ups and shoulders, faces a little off-centre, singles with a shoulder edging in, a close profile, now and then a medium; a wide shot for scale every few minutes | Watched from across the room, past people's shoulders and side on; faces now and then | Every angle in the library |
| Camera movement | Mostly still; close-ups push in; the two-shot drifts | Pushes and zooms on every close-up | Cranes and pull-outs on the wide shots, the rest still | Slow drifts on the two-shot, three-quarters and mediums, as on a dolly; close-ups push in | Regular shots held still; the emotion close-ups push in; a slow crane up on the master; nothing drifts | Small, slow drifts everywhere (the master too, since 0.9.25), as an operator following along; cranes on the long shots | Every move, bold and varied; the low angles push in with a little tilt up |
| Lenses (degrees, per angle) | 50-72 | 35-50 | 45-95 | 40-80 | 37.5-60 | 40-70 | 40-95 |
| Camera sway (`iCameraSway`) | 0, a tripod | 0, a tripod | 0, a tripod | 50, a touch of handheld | 15, barely there | 60 plus a slow handheld wander | 40 |
| Framing | | | | | Faces a little off-centre: a quarter of the usual room in the direction they look | | |
| Cutting (new angle) | Every 2-3 lines, held 2.4 s at least | Every 3-6 lines, 2.4 s | Every 3-5 lines, 3 s | Every 2-4 lines and a quarter of the turns, 2.4 s | Every 4-7 lines, 3.5 s | Every 3-5 lines and a quarter of the turns, 2.8 s | Every 2-3 lines and most turns, 1.6 s |
| Faces (close-ups and closer) | 26% | 67% | none | 19% | 45% | 6% | 24% |
| Wide and full-figure shots (long, overhead, room shots) | none | 7% (Distant) | 69% | 7% | 19% | 47% | 19% |
| Emotion close-ups | none | Strong lines only | none | Every tier | Every tier | Strong lines only | Every tier |
| Extreme share of their emotion close-ups (`iExtremeCloseUpChanceNpc`) | | 40% | | 20% | 40% | 40% | 40% |
| Close-up of you on a tagged option | no | no | no | yes | yes | yes | yes |
| Reaction shots of you | off | off | off | after 10 s of their talking | after 9 s | after 12 s | after 7 s |
| Listening reactions (`bListeningCloseUpsPlayer`) | | | | Mediums and three-quarters; close-ups of you only for your expressions | Any of its shots of you | Any of its shots of you | Any of its shots of you |
| Extreme close-ups of you | | | | 40% of the close-ups of you | 40% | none | 40% |
| One close-up of them at most (`fCloseUpLimitSecondsNpc`) | no limit | no limit | no limit | 12 s, then a wider shot of them | 18 s | 14 s | 12 s |
| Room shots go back | | As far as CCC's framing wants | As far as CCC's framing wants | Up to 9 m indoors, 12 m outdoors | The same | The same | The same |
| Other shots, camera at most | 7 m indoors, 10.5 m outdoors | 5 m | 7 m indoors, 13 m outdoors | 7 m indoors, 13 m outdoors | 5 m | 7 m indoors, 13 m outdoors | 7 m indoors, 13 m outdoors |

In every preset (0.9.23): a reaction is never the overhead shot of you; a room or shared
shot (the two of you, the room, the overhead of you) stays your angle for 2 of your turns
at most; the close-up of you for a tagged option comes as you pick it, over the start of
their answer (`fChoiceCloseUpSecondsPlayer`, 2 s). While testing there's an eighth preset,
NebulaTest: Nebula with its low angles as half of the angles picked, to judge their new move;
it comes out before 1.0.

## Opening shots from first person

A conversation started in first person opens on a shot of the NPC alone, so your body is
built off screen (it takes about a tenth of a second) and never pops into the picture.
Close-ups are kept for emphasis, except in Close and Starborn, where they're the main
shot.

| Preset | Openings | Shots (share, lens) |
| --- | --- | --- |
| Standard | 2 | Three-quarter (57%, 58°), medium (43%, 62°) |
| Close | 2 | Close-up (50%, 50°), three-quarter (50%, 40°) |
| Room | 2 | Three-quarter (53%, 72°), medium (47%, 70°) |
| Frontier | 2 | Three-quarter (57%, 52°, drifting), medium (43%, 56°, drifting) |
| Starborn | 2 | Close-up (50%, 47.5°, still), three-quarter (50%, 40°, a little off-centre) |
| Constellation | 3 | Three-quarter (42%, 50°, drifting), medium (37%, 50°, drifting), medium profile (21%, 50°, drifting) |
| Nebula | 3 | Three-quarter (47%, 58°), medium (35%, 62°), medium profile (18%, 60°) |

- In Close and Starborn the three-quarter isn't one of the preset's own angles; it takes the
  lens of their over-the-shoulder shot (40°).
- If the NPC's first line carries a strong emotion, its emotion close-up can open instead
  (where the preset uses emotion close-ups).
- Merchants and seated people: the three-quarter (Close and Starborn: the close-up or the
  three-quarter), or the medium if it's blocked; held for their turn.
- Where walls block the preset's openings, the other one, or a close-up as a last resort.

## Opening shots from third person

Your body is already there, so a third-person conversation can open on any of the preset's
shots (over the shoulder, two-shot, room shots...), by the preset's weights, except a
close-up: close-ups are kept for emphasis. Close and Starborn can open on their close-ups,
their main shot. As from first person, a strong first line's emotion close-up can open
instead.

| Preset | Openings from third person |
| --- | --- |
| Standard | Any of its shots but the close-ups |
| Close | Any of its shots but the extreme close-ups (never an opening, in any preset) |
| Room | Any of its shots (it has no close-ups) |
| Frontier | Any of its shots but the close-ups |
| Starborn | Any of its shots but the extreme close-ups |
| Constellation | Any of its shots but the close-ups |
| Nebula | Any of its shots but the close-ups (its close profile, low and high close-ups too) |

## Shot by shot

How often each shot comes up when the camera picks a new angle (its share of the preset's
weights), how the camera moves on it, and its lens. An empty cell: the preset doesn't use
that shot. (Generated from the presets in the code.)

| Shot | Standard | Close | Room | Frontier | Starborn | Constellation | Nebula |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Over your shoulder | 16% still, 55° | 25% still, 40° |  | 12% still, 50° | 13% still, 40° | 4% drift, 50° | 7% push in, 50° |
| Over the NPC's shoulder | 16% still, 55° |  |  | 12% still, 50° | 9% still, 40° | 4% drift, 50° | 7% push in, 50° |
| Over your shoulder, low |  |  |  |  |  |  | 2% push in, 55° |
| Over your shoulder, high |  |  |  |  |  |  | 2% tilt down, 55° |
| Over your shoulder, wide |  |  |  | 2% still, 62° |  |  | 2% pull out, 65° |
| Over the NPC's shoulder, low |  |  |  |  |  |  | 2% push in, 55° |
| Over the NPC's shoulder, high |  |  |  |  |  |  | 2% tilt down, 55° |
| Over the NPC's shoulder, wide |  |  |  | 2% still, 62° |  |  | 2% pull out, 65° |
| Dirty single (NPC, your shoulder edging in) |  |  |  | 4% drift, 50° | 4% still, 40° | 3% drift, 50° | 2% drift, 45° |
| Dirty single (you, the NPC's shoulder edging in) |  |  |  | 4% drift, 50° | 4% still, 40° | 3% drift, 50° | 2% drift, 45° |
| Close-up (NPC) | 13% push in, 50° | 27% push in, 50° |  | 9% push in, 45° | 19% still, 47.5° | 3% push in, 45° | 6% push in, 50° |
| Close-up (you) | 13% push in, 50° | 22% zoom out, 40° |  | 9% push in, 45° | 15% still, 42.5° | 3% push in, 45° | 6% push in, 50° |
| Close-up from below (NPC) |  |  |  |  |  |  | 2% push in, 50° |
| Close-up from above (NPC) |  |  |  |  |  |  | 2% tilt down, 50° |
| Wide close-up (NPC) |  |  |  |  |  |  | 2% push in, 60° |
| Close profile (NPC) |  |  |  |  | 3% still, 45° |  | 2% push in, 50° |
| Extreme close-up (NPC) |  | 9% zoom in, 40° |  | 1% push in, 40° | 3% still, 42.5° | 1% push in, 40° | 1% push in, 40° |
| Extreme close-up (you) |  | 9% push in, 35° |  | 1% push in, 40° | 3% still, 37.5° | 1% push in, 40° | 1% push in, 40° |
| Three-quarter (NPC) | 10% still, 58° |  | 6% still, 72° | 7% drift, 52° |  | 5% drift, 50° | 5% drift, 58° |
| Three-quarter (you) | 10% still, 58° |  | 6% still, 72° | 7% drift, 52° |  | 5% drift, 50° | 5% drift, 58° |
| Medium (NPC) | 7% still, 62° |  | 6% still, 70° | 6% drift, 56° | 3% still, 45° | 5% drift, 50° | 4% drift, 62° |
| Medium (you) | 7% still, 62° |  | 6% still, 70° | 6% drift, 56° | 3% still, 45° | 5% drift, 50° | 4% drift, 62° |
| Medium profile (NPC) |  |  |  |  |  | 3% drift, 50° | 2% drift, 60° |
| Profile (you) |  |  |  |  |  |  | 2% drift, 60° |
| Low profile (NPC) |  |  |  |  |  |  | 1% push in, a little tilt up, 60° |
| Low angle (NPC) |  |  |  | 2% push in, a little tilt up, 56° |  |  | 2% push in, a little tilt up, 60° |
| Low angle (you) |  |  |  | 2% push in, a little tilt up, 56° |  |  | 2% push in, a little tilt up, 60° |
| High angle (you) |  |  |  |  |  |  | 2% crane down, 65° |
| Profile (both, side on) | 2% still, 60° |  |  | 2% still, 55° |  | 3% drift, 55° | 2% drift, 60° |
| Two-shot | 4% drift, 72° |  | 7% drift, 80° | 5% drift, 65° |  | 6% drift, 60° | 4% drift, 72° |
| Long, full figure (NPC) |  |  | 11% crane up, 80° | 2% crane up, 70° |  | 7% crane up, 55° | 3% crane up, 80° |
| Long, full figure (you) |  |  | 11% crane up, 80° | 2% crane up, 70° |  | 7% crane up, 55° | 3% crane up, 80° |
| Overhead (NPC) |  |  | 6% still, 85° |  |  | 3% crane down, 70° | 1% crane down, 85° |
| Overhead (you) |  |  | 6% still, 85° |  |  | 3% crane down, 70° | 1% crane down, 85° |
| Wide (room shot) |  |  | 13% pull out, 88° | 2% pull out, 80° |  | 7% drift, 60° | 3% pull out, 88° |
| Distant (room shot) |  | 7% pull out, 40° | 9% still, 45° | 1% still, 45° | 11% still, 40° | 10% drift, 45° | 2% pull out, 45° |
| Distant, low (room shot) |  |  |  |  |  |  | 2% drift, 45° |
| Master (room shot) |  |  | 13% crane up, 95° |  | 9% crane up, 60° | 9% drift, 70° | 3% crane up, 95° |

## Notes

- **Emotion close-ups move on their own.** Whatever the table says for a close-up, an
  emotion close-up pushes in (8 s; 9 s for an extreme one), in every preset. In Starborn
  (0.9.23) that's the only push-in: its regular close-ups are held still, and an emotional line
  on a close-up already on screen starts the push-in from there.

- **Shares are before the room has its say.** Room shots (Wide, Distant, Distant low,
  Master) stand only where the room measurement finds space for them on the conversation's
  side with both faces in view (`iRoomShotMinRoom`). Where it doesn't, the preset picks from
  its other shots, so indoors the wider presets lean more on their long shots, two-shots
  and singles. Any shot the obstacle check finds blocked is skipped the same way.
- **Nebula is the least tested.** Its rarer angles (low, high, dirty, profiles, the shoulder
  variants) come from CCC's library but no other preset uses them yet. The ground-level
  shot is left out until room measurement can place it.
- **CCC SF's presets only (0.8.13):** emotion close-ups on lines with an expression (every
  one but Neutral and Friendly, by tier: mild 25%, medium 40%, strong always), reaction shots
  of you listening in long stretches (`fReactionAfterSecondsPlayer`), and close-ups of your
  expressions in them (`iCloseUpChance...Player`, by tier). CCC's presets keep CCC's vision:
  no reactions, and emotion close-ups only in Close, on strong lines
  (`bEmotionCloseUpsNpc`, `bReactionShotsPlayer`: -1 the preset's own, 0 off, 1 on).
- **The same in every preset:** holding on the answer after a persuasion
  (`bPersuasionBeatNpc`), and the plain over-the-shoulder shots for seated people and
  merchants.
- **Zooms are pushes.** Starfield's lens doesn't change during a shot, so a zoom in is done
  as a push in and a zoom out as a pull out.
- **Extreme close-ups only for the lines that call for one.** As in CCC, which keeps them for
  lines the writers marked as intense, they're never picked as a regular angle (0.8.15):
  they come with the emotion close-ups (strong lines in Close, every tier in CCC SF's
  presets), and in reactions when your face shows something. Their weights in the table
  above only matter there.
- **Your own mix.** The `[Shots]` section of the INI changes any preset: `CloseUp=60` sets a
  shot's weight (0 turns it off), `CloseUpMove=Push in`, `CloseUpMoveAmount=40`,
  `CloseUpMoveTime=6`. `fMaxCameraDistance`, `fMaxCameraDistanceExterior`,
  `fRoomShotMaxIndoors`, `fRoomShotMaxOutdoors` and `fConversationLens` set your own
  distances and one lens for every angle; `iCameraSway`, `iCuttingPace` (nudges the preset's rhythm; -2 to +2),
  `bEmotionCloseUpsNpc` and `bReactionShotsPlayer` override the preset's own.
