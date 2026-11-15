# Presets

The six looks you can choose with `sPreset` in the `[Director]` section of the INI.
Standard, Close and Room are CCC's own, kept as faithful to CCC as Starfield allows.
Stardust, Starborn and Constellation are CCC SF's own: each mirrors one of CCC's with a
Starfield flavour (room shots kept closer, and a few shots from the other presets mixed in,
moving).

Last updated: 0.8.2 (2026-11-15).

## At a glance

| | Standard | Close | Room | Stardust | Starborn | Constellation |
| --- | --- | --- | --- | --- | --- | --- |
| From | CCC | CCC | CCC | CCC SF, mirrors Standard | CCC SF, mirrors Close | CCC SF, mirrors Room |
| The look | Shoulders, close-ups, three-quarters and mediums on both sides | Shoulders, tight close-ups and extreme close-ups | Wide and full-figure shots | Standard with Room's wider shots mixed in, about 60/40 | Close, plus the reverse shoulder and three-quarter singles | Room, plus close-ups and a shoulder now and then |
| Camera movement | Mostly locked off; close-ups push in; the two-shot drifts | Pushes and zooms on every close-up | Cranes, pull-outs and drifts on almost everything | Standard's locked off; the wider shots move | Close's pushes and zooms; the three-quarters drift | Room's moves; close-ups push in |
| Faces (close-ups and closer) | 26% | 67% | none | 17% | 49% | 12% |
| Wide and full-figure shots (long, overhead, room shots) | none | 7% (Distant) | 69% | 33% | 5% (Distant) | 58% |
| Room shots (Wide, Distant, Master) | none | Distant | All three | All three | Distant | All three |
| Room shots go back | | As far as CCC's framing wants | As far as CCC's framing wants (12-18 m) | Master up to 9 m indoors, 12 m outdoors; the others closer | Up to 9 m indoors, 12 m outdoors | Master up to 9 m indoors, 12 m outdoors; the others closer |
| Other shots, camera at most | 7 m indoors, 10.5 m outdoors | 5 m | 7 m indoors, 13 m outdoors | 7 m indoors, 13 m outdoors | 5 m | 7 m indoors, 13 m outdoors |
| Lens | 45 degrees | 45 degrees | 45 degrees | 45 degrees | 45 degrees | 45 degrees |

## Shot by shot

How often each shot comes up when the camera picks a new angle (its share of the preset's
weights), and how the camera moves on it. An empty cell: the preset doesn't use that shot.

| Shot | Standard | Close | Room | Stardust | Starborn | Constellation |
| --- | --- | --- | --- | --- | --- | --- |
| Over your shoulder | 16% locked | 25% locked |  | 11% locked | 18% locked | 5% locked |
| Over the NPC's shoulder | 16% locked |  |  | 11% locked | 12% locked |  |
| Close-up (NPC) | 13% push in | 27% push in |  | 8% push in | 20% push in | 6% push in |
| Close-up (you) | 13% push in | 22% zoom out |  | 8% push in | 16% zoom out | 6% push in |
| Extreme close-up (NPC) |  | 9% zoom in |  |  | 7% zoom in |  |
| Extreme close-up (you) |  | 9% push in |  |  | 7% push in |  |
| Three-quarter (NPC) | 10% locked |  | 6% drift | 6% locked | 8% drift | 5% drift |
| Three-quarter (you) | 10% locked |  | 6% drift | 6% locked | 8% drift | 5% drift |
| Medium (NPC) | 7% locked |  | 6% drift | 5% locked |  | 5% drift |
| Medium (you) | 7% locked |  | 6% drift | 5% locked |  | 5% drift |
| Profile (both, side on) | 2% locked |  |  | 3% drift |  |  |
| Two-shot | 4% drift |  | 7% drift | 5% drift |  | 6% drift |
| Long, full figure (NPC) |  |  | 11% crane up | 5% crane up |  | 9% crane up |
| Long, full figure (you) |  |  | 11% crane up | 5% crane up |  | 9% crane up |
| Overhead (NPC) |  |  | 6% crane down | 3% crane down |  | 5% crane down |
| Overhead (you) |  |  | 6% crane down | 3% crane down |  | 5% crane down |
| Wide (room shot) |  |  | 13% pull out | 6% pull out |  | 10% pull out |
| Distant (room shot) |  | 7% pull out | 9% locked | 4% pull out | 5% pull out | 7% pull out |
| Master (room shot) |  |  | 13% crane up | 6% crane up |  | 11% crane up |

## Notes

- **Shares are before the room has its say.** Room shots (Wide, Distant, Master) stand only
  where the room measurement finds space for them on the conversation's side with both
  faces in view (`iRoomShotMinRoom`). Where it doesn't, the preset picks from its other
  shots, so indoors the wider presets lean more on their long shots, two-shots and singles.
- **The same in every preset:** emotion close-ups on intense lines (`bEmotionCloseUps`),
  reaction shots of you listening (`iReactionEvery`, `iReactionChance`, `fReactionSeconds`),
  close-ups of your expressions (`iPlayerCloseUpChance`), holding on the answer after a
  persuasion (`bPersuasionBeat`), and the plain over-the-shoulder shots for seated people
  and merchants.
- **Zooms are pushes.** Starfield's lens doesn't change during a shot, so a zoom in is done
  as a push in and a zoom out as a pull out.
- **Close leaves out extreme close-ups mostly.** CCC offers them on lines the writers marked
  as intense; Starfield's lines carry no such strength, so their weight is cut from 75 to 12
  (Close and Starborn). Intense lines still get them through the emotion close-ups.
- **Your own mix.** The `[Shots]` section of the INI changes any preset: `CloseUp=60` sets a
  shot's weight (0 turns it off), `CloseUpMove=Push in`, `CloseUpMoveAmount=40`,
  `CloseUpMoveTime=6`. `fMaxCameraDistance`, `fMaxCameraDistanceExterior`,
  `fRoomShotMaxIndoors`, `fRoomShotMaxOutdoors` and `fConversationLens` set your own
  distances and lens.
