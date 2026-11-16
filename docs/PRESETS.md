# Presets

The seven looks you can choose with `sPreset` in the `[Director]` section of the INI (or
switch on the fly with the preset key, U, while testing).

- **Standard, Close and Room** are CCC's own, as faithful to CCC 1.5.0 as Starfield allows:
  its angles, how often each comes up, how each moves, each angle's own lens and each
  preset's cutting rhythm, from CCC's own exported presets. No reaction shots (off as CCC
  ships); emotion close-ups only in Close, on strong lines.
- **Frontier, Starborn, Constellation and Nebula** are CCC SF's own, each with a film
  flavour drawn from Starfield's "NASA-punk" influences, its own lenses, movement, cutting
  and camera sway, room shots kept closer, and everything this mod adds: emotion close-ups
  on every tier, reaction shots of you, close-ups of your expressions.

Last updated: 0.8.14 (2026-11-15).

## At a glance

| | Standard | Close | Room | Frontier | Starborn | Constellation | Nebula |
| --- | --- | --- | --- | --- | --- | --- | --- |
| From | CCC | CCC | CCC | CCC SF, built on Standard | CCC SF, built on Close | CCC SF, built on Room | CCC SF |
| Feels like | A TV drama: classic shot / reverse shot | A film-noir conversation | A classic western or a stage play | A space western (*Firefly*, *The Expanse*) | Kubrick and *Interstellar*: the Unity, the artifacts, the uncanny | A hard sci-fi documentary (*The Martian*, *Apollo 13*) | A blockbuster trailer |
| The look | Shoulders, close-ups, three-quarters and mediums on both sides | Shoulders, tight close-ups and extreme close-ups | Wide and full-figure shots | Standard on slightly longer lenses, with the room now and then | Close-ups and shoulders, faces centred in the frame, one slow reveal of the room | Watched from across the room; faces now and then | Every angle in the library |
| Camera movement | Mostly still; close-ups push in; the two-shot drifts | Pushes and zooms on every close-up | Cranes and pull-outs on the wide shots, the rest still | Slow drifts on the two-shot, three-quarters and mediums, as on a dolly; close-ups push in | Very slow push-ins (12 s), held long; one slow crane up; nothing drifts | Small, slow drifts everywhere, as an operator following along; cranes on the master and long shots | Every move, bigger and quicker |
| Lenses (degrees, per angle) | 50-72 | 35-50 | 45-95 | 40-80 | 35-60 | 40-70 | 40-95 |
| Camera sway (`iCameraSway`) | 40 | 40 | 40 | 50, a touch of handheld | 0, locked off | 70, handheld | 40 |
| Framing | | | | | Faces centred, no room in the direction they look | | |
| Cutting (new angle) | Every 2-3 lines, held 2.4 s at least | Every 3-6 lines, 2.4 s | Every 3-5 lines, 3 s | Every 2-4 lines and a quarter of the turns, 2.4 s | Every 4-7 lines, 3.5 s | Every 3-5 lines and a quarter of the turns, 2.8 s | Every 2-3 lines and most turns, 1.6 s |
| Faces (close-ups and closer) | 26% | 67% | none | 22% | 59% | 7% | 24% |
| Wide and full-figure shots (long, overhead, room shots) | none | 7% (Distant) | 69% | 8% | 11% | 54% | 19% |
| Emotion close-ups | none | Strong lines only | none | Every tier | Every tier | Strong lines only | Every tier |
| Close-up of you on a tagged option | no | no | no | yes | yes | yes | yes |
| Reaction shots of you | off | off | off | after 10 s of their talking | after 9 s | after 12 s | after 7 s |
| Room shots go back | | As far as CCC's framing wants | As far as CCC's framing wants | Up to 9 m indoors, 12 m outdoors | The same | The same | The same |
| Other shots, camera at most | 7 m indoors, 10.5 m outdoors | 5 m | 7 m indoors, 13 m outdoors | 7 m indoors, 13 m outdoors | 5 m | 7 m indoors, 13 m outdoors | 7 m indoors, 13 m outdoors |

## Shot by shot

How often each shot comes up when the camera picks a new angle (its share of the preset's
weights), how the camera moves on it, and its lens. An empty cell: the preset doesn't use
that shot. (Generated from the presets in the code.)

| Shot | Standard | Close | Room | Frontier | Starborn | Constellation | Nebula |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Over your shoulder | 16% still, 55° | 25% still, 40° |  | 15% still, 50° | 18% still, 40° | 5% drift, 50° | 7% push in, 50° |
| Over the NPC's shoulder | 16% still, 55° |  |  | 15% still, 50° | 12% still, 40° | 5% drift, 50° | 7% push in, 50° |
| Over your shoulder, low |  |  |  |  |  |  | 2% push in, 55° |
| Over your shoulder, high |  |  |  |  |  |  | 2% tilt down, 55° |
| Over your shoulder, wide |  |  |  |  |  |  | 2% pull out, 65° |
| Over the NPC's shoulder, low |  |  |  |  |  |  | 2% push in, 55° |
| Over the NPC's shoulder, high |  |  |  |  |  |  | 2% tilt down, 55° |
| Over the NPC's shoulder, wide |  |  |  |  |  |  | 2% pull out, 65° |
| Dirty single (NPC, your shoulder edging in) |  |  |  |  |  |  | 2% drift, 45° |
| Dirty single (you, the NPC's shoulder edging in) |  |  |  |  |  |  | 2% drift, 45° |
| Close-up (NPC) | 13% push in, 50° | 27% push in, 50° |  | 10% push in, 45° | 27% push in, 45° | 3% push in, 45° | 6% push in, 50° |
| Close-up (you) | 13% push in, 50° | 22% zoom out, 40° |  | 10% push in, 45° | 21% push in, 40° | 3% push in, 45° | 6% push in, 50° |
| Close-up from below (NPC) |  |  |  |  |  |  | 2% push in, 50° |
| Close-up from above (NPC) |  |  |  |  |  |  | 2% tilt down, 50° |
| Wide close-up (NPC) |  |  |  |  |  |  | 2% push in, 60° |
| Close profile (NPC) |  |  |  |  |  |  | 2% push in, 50° |
| Extreme close-up (NPC) |  | 9% zoom in, 40° |  | 1% push in, 40° | 5% push in, 40° | 1% push in, 40° | 1% push in, 40° |
| Extreme close-up (you) |  | 9% push in, 35° |  | 1% push in, 40° | 5% push in, 35° | 1% push in, 40° | 1% push in, 40° |
| Three-quarter (NPC) | 10% still, 58° |  | 6% still, 72° | 9% drift, 52° |  | 6% drift, 50° | 5% drift, 58° |
| Three-quarter (you) | 10% still, 58° |  | 6% still, 72° | 9% drift, 52° |  | 6% drift, 50° | 5% drift, 58° |
| Medium (NPC) | 7% still, 62° |  | 6% still, 70° | 7% drift, 56° |  | 5% drift, 50° | 4% drift, 62° |
| Medium (you) | 7% still, 62° |  | 6% still, 70° | 7% drift, 56° |  | 5% drift, 50° | 4% drift, 62° |
| Medium profile (NPC) |  |  |  |  |  |  | 2% drift, 60° |
| Profile (you) |  |  |  |  |  |  | 2% drift, 60° |
| Low profile (NPC) |  |  |  |  |  |  | 1% tilt up, 60° |
| Low angle (NPC) |  |  |  |  |  |  | 2% tilt up, 60° |
| Low angle (you) |  |  |  |  |  |  | 2% tilt up, 60° |
| High angle (you) |  |  |  |  |  |  | 2% crane down, 65° |
| Profile (both, side on) | 2% still, 60° |  |  | 2% still, 55° |  |  | 2% drift, 60° |
| Two-shot | 4% drift, 72° |  | 7% drift, 80° | 6% drift, 65° |  | 7% drift, 60° | 4% drift, 72° |
| Long, full figure (NPC) |  |  | 11% crane up, 80° | 2% crane up, 70° |  | 8% crane up, 55° | 3% crane up, 80° |
| Long, full figure (you) |  |  | 11% crane up, 80° | 2% crane up, 70° |  | 8% crane up, 55° | 3% crane up, 80° |
| Overhead (NPC) |  |  | 6% still, 85° |  |  | 4% crane down, 70° | 2% crane down, 85° |
| Overhead (you) |  |  | 6% still, 85° |  |  | 4% crane down, 70° | 2% crane down, 85° |
| Wide (room shot) |  |  | 13% pull out, 88° | 2% pull out, 80° |  | 8% drift, 60° | 3% pull out, 88° |
| Distant (room shot) |  | 7% pull out, 40° | 9% still, 45° | 1% still, 45° | 6% still, 40° | 11% drift, 45° | 2% pull out, 45° |
| Distant, low (room shot) |  |  |  |  |  |  | 2% drift, 45° |
| Master (room shot) |  |  | 13% crane up, 95° |  | 5% crane up, 60° | 11% crane up, 70° | 3% crane up, 95° |

## Notes

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
  distances and one lens for every angle; `iCameraSway`, `sCuttingStyle`,
  `bEmotionCloseUpsNpc` and `bReactionShotsPlayer` override the preset's own.
