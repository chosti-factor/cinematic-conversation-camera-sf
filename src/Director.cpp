#include "Director.h"

#include "Settings.h"

#include <random>
#include <span>

// Shot choice ported from Cinematic Conversation Camera by hashhbbrown (src/Camera/
// Director.cpp and Presets.cpp, GPL-3.0-or-later), reduced to what Starfield exposes.
namespace CCC::Director
{
	namespace
	{
		using Shots::Move;
		using Shots::ShotType;
		using clock = std::chrono::steady_clock;

		constexpr std::size_t kCount = static_cast<std::size_t>(ShotType::kCount);

		// Starfield's dialogue camera lens (horizontal degrees), measured in 0.3.28.
		constexpr float kLens = 29.58f;  // the game's 30 degree setting, as drawn

		// The lens the shots are framed for: the game's, or fConversationLens (0.8). The 0.8.0
		// log showed the game hands its set-FOV the 29.6 degrees drawn (not the setting's 30),
		// so the lens set there is the lens drawn.
		float PresetLens();

		bool PresetIs(const char* a_name)
		{
			std::string preset{ Settings::Get().preset };
			std::ranges::transform(preset, preset.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
			return preset == a_name;
		}

		// One of CCC's own presets (Standard, Close, Room).
		bool CccPresetNow()
		{
			return PresetIs("standard") || PresetIs("close") || PresetIs("room");
		}

		// The lens a shot without its own is framed for: the preset's, or the game's.
		float Lens()
		{
			const float wanted = PresetLens();
			return wanted < 10.0f ? kLens : wanted;
		}

		// 0.8.12, CCC's lens per angle, as CCC wrote them (0.8.14: measured against Skyrim
		// clips, Skyrim's lens is measured as Starfield's is: across the frame as it is).
		std::atomic<float> g_shotLens{ 0.0f };  // the shot on screen's own lens (0 = the preset's)

		// An angle's own lens, unless your fConversationLens sets one for everything (0 = none).
		float OwnLens(float a_lens)
		{
			return a_lens < 10.0f || Settings::Get().conversationLens >= 0.0f ? 0.0f : a_lens;
		}

		struct Setup
		{
			int   weight{ 0 };  // 0 = not used
			Move  move{ Move::kLocked };
			int   amount{ 0 };   // 0-100
			float seconds{ 4.2f };
			float lens{ 0.0f };  // this angle's lens, CCC's number (0.8.12, as CCC 1.3+); 0 = the preset's
		};

		struct Line
		{
			bool             talking{ false };
			clock::time_point start{};
		};

		// Each person keeps their setup across the back-and-forth (shot, reverse, shot...)
		// and gets a new one after a few of their own lines, as in CCC.
		struct Side
		{
			std::optional<ShotType> shot;
			int                     lines{ 0 };
			int                     linesTarget{ 3 };
			bool                    changeDue{ false };
			int                     roomTurns{ 0 };  // your turns in a row on a room or shared shot (0.9.23)
		};

		struct State
		{
			std::array<Setup, kCount> setups{};
			std::optional<ShotType>   current;
			bool                      currentNpcSide{ true };
			clock::time_point         cutAt{};
			Side                      npc;
			Side                      player;
			float                     heldAngle{ std::numeric_limits<float>::quiet_NaN() };
			float                     heldLookSign{ 0.0f };
			float                     heldFacePitch{ std::numeric_limits<float>::quiet_NaN() };
			float                     maxDistance{ 3.0f };
			float                     roomIndoor{ 0.0f };   // the preset's room shot limits (0 = none)
			float                     roomOutdoor{ 0.0f };
			Line                      npcLine;
			Line                      playerLine;
			std::mt19937              rng{ std::random_device{}() };
			bool                      loggedEmpty{ false };
			bool                      intensePending{ false };  // an intense line started; close-up owed
			std::optional<ShotType>   intenseShot;              // this line's emotion close-up, to come back to after a reaction (0.8.3)
			std::optional<Frame>      lastFrame;                // the last frame seen, for the closing shot (0.9.14)
			bool                      extremeLineOver{ false }; // an extreme close-up's line is over: it holds one line only (0.8.9)
			// The cutting rhythm in use: the preset's own, nudged by the cutting pace (0.9.8; fCuttingPace and fCuttingPace<Preset> since 0.9.26/0.9.27).
			float pace{ 0.0f };  // the cutting pace in use (the preset's own or fCuttingPace, 0.9.27)
			int   freshAngleChance{ 50 };
			int   minLines{ 2 };
			int   maxLines{ 5 };
			float shortestHold{ 2.0f };
			int                       npcSerial{ 0 };           // the last line counter seen (0.8.3)
			bool                      inReaction{ false };      // a reaction shot is on (0.8.6: an insert)
			std::optional<ShotType>   reactionReturn;           // the speaker's shot to go back to after it
			std::optional<Setup>      reactionReturnSetup;      // and its move, if an emotion close-up's
			bool                      playerCloseDone{ false }; // this reaction's close-up of you is done
			std::optional<Setup>      intenseSetup;             // the move used by an emotion close-up
			float                     limit{ std::numeric_limits<float>::infinity() };  // obstacle: camera at most this far from the subject

			// Obstacle checks: one cast per candidate per decision, kept for this Update call.
			std::array<std::optional<Clearance>, kCount> clearance{};
			clock::time_point                            clearanceAt{};
			std::string                                  skipped;  // shots left out for obstacles, for the next Director line
			bool                                         beatActive{ false };  // holding the persuasion beat's close-up
			clock::time_point                            beatAt{};             // when it started (0.8.5)
			bool                                         beatWidened{ false }; // the one wider shot of them in a long answer, taken
			std::uint32_t                                cuts{ 0 };
			const char*                                  why{ "" };
			std::optional<bool>                          plainTurn;  // nothing fitted this turn (true = theirs): plain shot until the turn changes
			// 0.9.23: a cut owed at a line's start waits until the line's reaction is decided (a
			// cut, then a reaction a frame later, flashed one frame of the first: 22 in the
			// 2026-11-18 run).
			bool              lineCutOwed{ false };
			clock::time_point lineCutOwedAt{};
			clock::time_point moveFrom{};          // the shot's move counts from here (the cut, or an emotion push-in on a held close-up)
			clock::time_point closeUpCooldown{};   // after the close-up limit's cut, no emotion close-up before this
			bool              noRoomPick{ false }; // Pick leaves out the room and shared shots (your angle held on one for 2 turns)
			std::optional<ShotType> avoidShot;     // ...and this one: the shot on screen as the turn passes (0.9.28)
		} g;

		ObstacleTest g_obstacleTest = nullptr;

		// Emotion close-ups with the plain shots (seated people, merchants): held for the line.
		struct PlainIntense
		{
			std::optional<ShotType> shot;
			clock::time_point       cutAt{};
			float                   heldAngle{ std::numeric_limits<float>::quiet_NaN() };
			float                   heldLookSign{ 0.0f };
			float                   heldFacePitch{ std::numeric_limits<float>::quiet_NaN() };
			bool                    wasTalking{ false };
			float                   limit{ std::numeric_limits<float>::infinity() };
			bool                    playerClose{ false };  // holding your emotion close-up
			bool                    intenseShown{ false };  // this line's emotion close-up was shown (to come back to after a reaction)
			int                     serial{ 0 };            // the last line counter seen (0.8.3)
			clock::time_point       lineAt{};               // when their current line started
			bool                    opening{ false };       // the first shot from first person, you out of the picture (0.9.2)
		} gp;

		std::atomic<std::int64_t> g_emotionCutAt{ 0 };  // steady_clock ticks
		std::atomic<int>          g_emotionCutKind{ 0 };

		void NoteEmotionCut(ShotType a_type)
		{
			g_emotionCutKind.store(a_type == ShotType::kExtremeClose ? 2 : 1, std::memory_order_relaxed);
			g_emotionCutAt.store(clock::now().time_since_epoch().count(), std::memory_order_release);
		}

		// CCC's Close preset: a slower, stronger push-in, so the emotion close-up reads as a
		// deliberate move rather than the regular close-up of the rotation.
		Setup IntenseSetup(ShotType a_type)
		{
			auto setup = a_type == ShotType::kExtremeClose || a_type == ShotType::kExtremeClosePlayer ? Setup{ 1, Move::kZoomIn, 40, 9.0f } : Setup{ 1, Move::kPushIn, 40, 8.0f };
			setup.lens = g.setups[static_cast<std::size_t>(a_type)].lens;  // (the angle's own lens)
			return setup;
		}

		struct PresetEntry
		{
			ShotType type;
			int      weight;
			Move     move;
			int      amount;
			float    seconds;
			float    lens{ 0.0f };  // CCC's lens for the angle, in Skyrim degrees (0 = the preset's own); converted when loaded
		};

		// The three built-in looks (CCC's Presets.cpp). Zoom moves stay listed as zooms;
		// the solver performs them as push-ins and pull-outs. 0.8.12: each angle's lens, as
		// CCC 1.5.0 exports them (Skyrim degrees), and Room's medium, three-quarter and overhead
		// shots still, as in CCC 1.5.0 (they drifted and craned in the older CCC this came from).
		constexpr std::array kStandard{
			PresetEntry{ ShotType::kOverPlayerShoulder, 100, Move::kLocked, 0, 4.2f, 55.0f },
			PresetEntry{ ShotType::kOverNpcShoulder, 100, Move::kLocked, 0, 4.2f, 55.0f },
			PresetEntry{ ShotType::kCloseUp, 80, Move::kPushIn, 14, 5.0f, 50.0f },
			PresetEntry{ ShotType::kClosePlayer, 80, Move::kPushIn, 14, 5.0f, 50.0f },
			PresetEntry{ ShotType::kThreeQuarterNpc, 60, Move::kLocked, 0, 4.2f, 58.0f },
			PresetEntry{ ShotType::kThreeQuarterPlayer, 60, Move::kLocked, 0, 4.2f, 58.0f },
			PresetEntry{ ShotType::kMediumNpc, 45, Move::kLocked, 0, 4.2f, 62.0f },
			PresetEntry{ ShotType::kMediumPlayer, 45, Move::kLocked, 0, 4.2f, 62.0f },
			PresetEntry{ ShotType::kTwoShot, 25, Move::kDrift, 10, 7.0f, 72.0f },
			PresetEntry{ ShotType::kProfile, 15, Move::kLocked, 0, 4.2f, 60.0f },
		};
		// CCC offers the extreme close-ups only on lines the writers marked as intense,
		// which this plugin can't read yet, so their weight is cut from 75 to 12.
		constexpr std::array kClose{
			PresetEntry{ ShotType::kOverPlayerShoulder, 34, Move::kLocked, 0, 4.0f, 40.0f },
			PresetEntry{ ShotType::kCloseUp, 36, Move::kPushIn, 40, 8.0f, 50.0f },
			PresetEntry{ ShotType::kExtremeClose, 12, Move::kZoomIn, 40, 9.0f, 40.0f },
			PresetEntry{ ShotType::kClosePlayer, 30, Move::kZoomOut, 20, 5.0f, 40.0f },
			PresetEntry{ ShotType::kExtremeClosePlayer, 12, Move::kPushIn, 75, 7.0f, 35.0f },
			PresetEntry{ ShotType::kDistant, 10, Move::kPullOut, 55, 8.0f, 40.0f },
		};
		constexpr std::array kRoom{
			PresetEntry{ ShotType::kMaster, 85, Move::kCraneUp, 40, 9.0f, 95.0f },
			PresetEntry{ ShotType::kWide, 80, Move::kPullOut, 16, 8.0f, 88.0f },
			PresetEntry{ ShotType::kLongNpc, 70, Move::kCraneUp, 25, 7.0f, 80.0f },
			PresetEntry{ ShotType::kLongPlayer, 70, Move::kCraneUp, 25, 7.0f, 80.0f },
			PresetEntry{ ShotType::kDistant, 55, Move::kLocked, 0, 4.2f, 45.0f },
			PresetEntry{ ShotType::kTwoShot, 45, Move::kDrift, 12, 8.0f, 80.0f },
			PresetEntry{ ShotType::kOverhead, 40, Move::kLocked, 0, 4.2f, 85.0f },
			PresetEntry{ ShotType::kPlayerOverhead, 40, Move::kLocked, 0, 4.2f, 85.0f },
			PresetEntry{ ShotType::kThreeQuarterNpc, 40, Move::kLocked, 0, 4.2f, 72.0f },
			PresetEntry{ ShotType::kThreeQuarterPlayer, 40, Move::kLocked, 0, 4.2f, 72.0f },
			PresetEntry{ ShotType::kMediumNpc, 35, Move::kLocked, 0, 4.2f, 70.0f },
			PresetEntry{ ShotType::kMediumPlayer, 35, Move::kLocked, 0, 4.2f, 70.0f },
		};

		// CCC SF's own presets, rebuilt in 0.8.14 on CCC 1.5.0's lenses, each with a film
		// flavour (Starfield's "NASA-punk" influences) and its own movement, cutting and sway;
		// room shots no further back than 9 m indoors and 12 m outdoors. Each angle's lens is
		// in the same measure as CCC's (horizontal degrees, as drawn). The extreme close-ups
		// are mostly the emotion close-ups' (every tier, in these presets), so their own
		// weights stay low.
		//
		// Frontier, a space western (Firefly, The Expanse): Akila, Neon, the outposts. Built on
		// Standard: shoulders and singles a little longer in the lens (5 degrees narrower), the
		// two-shot, three-quarters and mediums drifting slowly as on a dolly in a saloon, close-ups
		// pushing in; now and then a long shot craning up or the room pulling out.
		constexpr std::array kFrontier{
			PresetEntry{ ShotType::kOverPlayerShoulder, 100, Move::kLocked, 0, 4.2f, 50.0f },
			PresetEntry{ ShotType::kOverNpcShoulder, 100, Move::kLocked, 0, 4.2f, 50.0f },
			PresetEntry{ ShotType::kCloseUp, 70, Move::kPushIn, 14, 6.0f, 45.0f },
			PresetEntry{ ShotType::kClosePlayer, 70, Move::kPushIn, 14, 6.0f, 45.0f },
			PresetEntry{ ShotType::kExtremeClose, 5, Move::kPushIn, 20, 7.0f, 40.0f },
			PresetEntry{ ShotType::kExtremeClosePlayer, 5, Move::kPushIn, 20, 7.0f, 40.0f },
			PresetEntry{ ShotType::kThreeQuarterNpc, 60, Move::kDrift, 8, 6.0f, 52.0f },
			PresetEntry{ ShotType::kThreeQuarterPlayer, 60, Move::kDrift, 8, 6.0f, 52.0f },
			PresetEntry{ ShotType::kMediumNpc, 45, Move::kDrift, 8, 6.0f, 56.0f },
			PresetEntry{ ShotType::kMediumPlayer, 45, Move::kDrift, 8, 6.0f, 56.0f },
			PresetEntry{ ShotType::kTwoShot, 40, Move::kDrift, 12, 8.0f, 65.0f },
			PresetEntry{ ShotType::kProfile, 15, Move::kLocked, 0, 4.2f, 55.0f },
			PresetEntry{ ShotType::kLongNpc, 15, Move::kCraneUp, 20, 8.0f, 70.0f },
			PresetEntry{ ShotType::kLongPlayer, 15, Move::kCraneUp, 20, 8.0f, 70.0f },
			PresetEntry{ ShotType::kWide, 15, Move::kPullOut, 14, 8.0f, 80.0f },
			PresetEntry{ ShotType::kDistant, 10, Move::kLocked, 0, 4.2f, 45.0f },
			// 0.9.24: singles with a shoulder edging in (Firefly, The Expanse), the western's
			// heroic low angle (pushing in with a little tilt, as Nebula's), and the wide
			// over-the-shoulder; all wider than a close-up.
			PresetEntry{ ShotType::kDirtyNpc, 30, Move::kDrift, 8, 6.0f, 50.0f },
			PresetEntry{ ShotType::kDirtyPlayer, 30, Move::kDrift, 8, 6.0f, 50.0f },
			PresetEntry{ ShotType::kLowAngle, 20, Move::kPushTiltUp, 22, 5.0f, 56.0f },
			PresetEntry{ ShotType::kPlayerLow, 20, Move::kPushTiltUp, 22, 5.0f, 56.0f },
			PresetEntry{ ShotType::kOverPlayerShoulderWide, 20, Move::kLocked, 0, 4.2f, 62.0f },
			PresetEntry{ ShotType::kOverNpcShoulderWide, 20, Move::kLocked, 0, 4.2f, 62.0f },
		};
		// Starborn, Kubrick and Interstellar: the Unity, the artifacts, the uncanny. Built on
		// Close: long, compressed lenses (35-45), faces centred in the frame (no look room,
		// the subject straight down the middle), very slow push-ins held long, a perfectly
		// still camera (no sway), and one slow crane up on the wide shots as a reveal. No
		// drifting: the stillness is the point.
		// 0.9.23, less Kubrick and more Interstellar (the 2026-11-18 review): the regular close-ups
		// held still (the push-ins are kept for the emotion close-ups), close-up lenses a step
		// wider (45 -> 50, 40 -> 45, 40 -> 45, 35 -> 40; 0.9.25, after the 0.9.24 test: half that
		// step, 47.5, 42.5, 42.5, 37.5, as it lost some of its character), the distant shot and the master more
		// often (10 -> 25, 8 -> 20: a wide shot for scale every few minutes; the crane-up reveal
		// stays on the master). Faces half off-centre and a touch of handheld: InputFor, CameraSway.
		constexpr std::array kStarborn{
			PresetEntry{ ShotType::kOverPlayerShoulder, 30, Move::kLocked, 0, 4.2f, 40.0f },
			PresetEntry{ ShotType::kOverNpcShoulder, 20, Move::kLocked, 0, 4.2f, 40.0f },
			PresetEntry{ ShotType::kCloseUp, 45, Move::kLocked, 0, 12.0f, 47.5f },
			PresetEntry{ ShotType::kClosePlayer, 35, Move::kLocked, 0, 12.0f, 42.5f },
			PresetEntry{ ShotType::kExtremeClose, 8, Move::kLocked, 0, 12.0f, 42.5f },
			PresetEntry{ ShotType::kExtremeClosePlayer, 8, Move::kLocked, 0, 12.0f, 37.5f },
			PresetEntry{ ShotType::kDistant, 25, Move::kLocked, 0, 4.2f, 40.0f },
			PresetEntry{ ShotType::kMaster, 20, Move::kCraneUp, 30, 12.0f, 60.0f },
			// 0.9.24: Interstellar's cockpit singles with a shoulder in the foreground, the close
			// profile (Nolan's and Kubrick's both), and now and then a medium, room to breathe.
			PresetEntry{ ShotType::kDirtyNpc, 10, Move::kLocked, 0, 4.2f, 40.0f },
			PresetEntry{ ShotType::kDirtyPlayer, 10, Move::kLocked, 0, 4.2f, 40.0f },
			PresetEntry{ ShotType::kCloseProfile, 8, Move::kLocked, 0, 4.2f, 45.0f },
			PresetEntry{ ShotType::kMediumNpc, 6, Move::kLocked, 0, 4.2f, 45.0f },
			PresetEntry{ ShotType::kMediumPlayer, 6, Move::kLocked, 0, 4.2f, 45.0f },
		};
		// Constellation, a hard sci-fi documentary (The Martian, Apollo 13): the Lodge, labs,
		// ships. Built on Room, watched from across the room on longer lenses (45-70, Room's
		// are 45-95): the distant shot and the master most, small slow drifts everywhere as an
		// operator following along, the most handheld sway, cranes on the master and the long
		// shots to show the space and the work; faces now and then (about a fifth).
		// 0.9.27: more of the room (the tests measured 18% wide indoors, 32% outdoors, against
		// about half designed): Distant 75 -> 95, Master 70 -> 85, the long shots 55 -> 65; the
		// three-quarters 40 -> 25 and the mediums 35 -> 20. Faces unchanged. 0.9.28 (the 0.9.27
		// test: too far, people too small): the same room share, framed larger: Distant 95 -> 65,
		// Master 85 -> 95, the long shots 65 -> 80.
		constexpr std::array kConstellation{
			PresetEntry{ ShotType::kDistant, 65, Move::kDrift, 8, 9.0f, 45.0f },
			PresetEntry{ ShotType::kMaster, 95, Move::kDrift, 10, 10.0f, 70.0f },  // 0.9.25: drifts (was a crane up; an operator on the floor)
			PresetEntry{ ShotType::kWide, 55, Move::kDrift, 10, 9.0f, 60.0f },
			PresetEntry{ ShotType::kLongNpc, 80, Move::kCraneUp, 15, 9.0f, 55.0f },
			PresetEntry{ ShotType::kLongPlayer, 80, Move::kCraneUp, 15, 9.0f, 55.0f },
			PresetEntry{ ShotType::kTwoShot, 45, Move::kDrift, 10, 9.0f, 60.0f },
			PresetEntry{ ShotType::kThreeQuarterNpc, 25, Move::kDrift, 8, 7.0f, 50.0f },
			PresetEntry{ ShotType::kThreeQuarterPlayer, 25, Move::kDrift, 8, 7.0f, 50.0f },
			PresetEntry{ ShotType::kMediumNpc, 20, Move::kDrift, 8, 7.0f, 50.0f },
			PresetEntry{ ShotType::kMediumPlayer, 20, Move::kDrift, 8, 7.0f, 50.0f },
			PresetEntry{ ShotType::kOverhead, 25, Move::kCraneDown, 15, 8.0f, 70.0f },
			PresetEntry{ ShotType::kPlayerOverhead, 25, Move::kCraneDown, 15, 8.0f, 70.0f },
			PresetEntry{ ShotType::kOverPlayerShoulder, 30, Move::kDrift, 6, 7.0f, 50.0f },
			PresetEntry{ ShotType::kOverNpcShoulder, 30, Move::kDrift, 6, 7.0f, 50.0f },
			PresetEntry{ ShotType::kCloseUp, 20, Move::kPushIn, 10, 6.0f, 45.0f },
			PresetEntry{ ShotType::kClosePlayer, 20, Move::kPushIn, 10, 6.0f, 45.0f },
			PresetEntry{ ShotType::kExtremeClose, 4, Move::kPushIn, 15, 7.0f, 40.0f },
			PresetEntry{ ShotType::kExtremeClosePlayer, 4, Move::kPushIn, 15, 7.0f, 40.0f },
			// 0.9.24: the observer shooting past people (singles with a shoulder edging in) and
			// catching them side on (the profile of you both, their medium profile).
			PresetEntry{ ShotType::kDirtyNpc, 25, Move::kDrift, 8, 7.0f, 50.0f },
			PresetEntry{ ShotType::kDirtyPlayer, 25, Move::kDrift, 8, 7.0f, 50.0f },
			PresetEntry{ ShotType::kProfile, 25, Move::kDrift, 8, 9.0f, 55.0f },
			PresetEntry{ ShotType::kMediumProfile, 20, Move::kDrift, 8, 7.0f, 50.0f },
		};
		// Nebula, a trailer: every angle in the library, every move CCC has (cranes, tilts,
		// drifts, pushes, pulls) at bigger amounts over shorter times, wide and long lenses
		// both, quick cutting. The ground-level shot waits for room measurement to place it.
		constexpr std::array kNebula{
			PresetEntry{ ShotType::kOverPlayerShoulder, 60, Move::kPushIn, 10, 5.0f, 50.0f },
			PresetEntry{ ShotType::kOverNpcShoulder, 60, Move::kPushIn, 10, 5.0f, 50.0f },
			PresetEntry{ ShotType::kCloseUp, 50, Move::kPushIn, 30, 5.0f, 50.0f },
			PresetEntry{ ShotType::kClosePlayer, 50, Move::kPushIn, 30, 5.0f, 50.0f },
			PresetEntry{ ShotType::kExtremeClose, 8, Move::kPushIn, 40, 5.0f, 40.0f },
			PresetEntry{ ShotType::kExtremeClosePlayer, 8, Move::kPushIn, 40, 5.0f, 40.0f },
			PresetEntry{ ShotType::kThreeQuarterNpc, 40, Move::kDrift, 18, 5.0f, 58.0f },
			PresetEntry{ ShotType::kThreeQuarterPlayer, 40, Move::kDrift, 18, 5.0f, 58.0f },
			PresetEntry{ ShotType::kMediumNpc, 30, Move::kDrift, 20, 5.0f, 62.0f },
			PresetEntry{ ShotType::kMediumPlayer, 30, Move::kDrift, 20, 5.0f, 62.0f },
			PresetEntry{ ShotType::kOverPlayerShoulderLow, 15, Move::kPushIn, 25, 5.0f, 55.0f },
			PresetEntry{ ShotType::kOverPlayerShoulderHigh, 15, Move::kTiltDown, 25, 5.0f, 55.0f },
			PresetEntry{ ShotType::kOverPlayerShoulderWide, 15, Move::kPullOut, 25, 5.0f, 65.0f },
			PresetEntry{ ShotType::kOverNpcShoulderLow, 15, Move::kPushIn, 25, 5.0f, 55.0f },
			PresetEntry{ ShotType::kOverNpcShoulderHigh, 15, Move::kTiltDown, 25, 5.0f, 55.0f },
			PresetEntry{ ShotType::kOverNpcShoulderWide, 15, Move::kPullOut, 25, 5.0f, 65.0f },
			PresetEntry{ ShotType::kDirtyNpc, 15, Move::kDrift, 12, 5.0f, 45.0f },
			PresetEntry{ ShotType::kDirtyPlayer, 15, Move::kDrift, 12, 5.0f, 45.0f },
			PresetEntry{ ShotType::kCloseProfile, 15, Move::kPushIn, 20, 5.0f, 50.0f },
			PresetEntry{ ShotType::kCloseLow, 15, Move::kPushIn, 20, 5.0f, 50.0f },
			PresetEntry{ ShotType::kCloseHigh, 15, Move::kTiltDown, 22, 5.0f, 50.0f },
			PresetEntry{ ShotType::kCloseWide, 15, Move::kPushIn, 20, 5.0f, 60.0f },
			PresetEntry{ ShotType::kMediumProfile, 15, Move::kDrift, 15, 5.0f, 60.0f },
			PresetEntry{ ShotType::kPlayerProfile, 15, Move::kDrift, 15, 5.0f, 60.0f },
			// 0.9.23: the low angles push in with a third of the tilt (the tilt up alone sank the
			// face under the subtitles).
			PresetEntry{ ShotType::kLowProfile, 10, Move::kPushTiltUp, 22, 5.0f, 60.0f },
			PresetEntry{ ShotType::kLowAngle, 15, Move::kPushTiltUp, 22, 5.0f, 60.0f },
			PresetEntry{ ShotType::kPlayerLow, 15, Move::kPushTiltUp, 22, 5.0f, 60.0f },
			PresetEntry{ ShotType::kHighAngle, 15, Move::kCraneDown, 13, 5.0f, 65.0f },  // 0.9.38: half the crane (the 0.9.37 test: Sarah's head went half under the top bar at 26)
			PresetEntry{ ShotType::kTwoShot, 30, Move::kDrift, 18, 6.0f, 72.0f },
			PresetEntry{ ShotType::kProfile, 15, Move::kDrift, 12, 5.0f, 60.0f },
			PresetEntry{ ShotType::kLongNpc, 25, Move::kCraneUp, 30, 6.0f, 80.0f },
			PresetEntry{ ShotType::kLongPlayer, 25, Move::kCraneUp, 30, 6.0f, 80.0f },
			// 0.9.23: a flourish, a little rarer (15 -> 12).
			PresetEntry{ ShotType::kOverhead, 12, Move::kCraneDown, 26, 5.0f, 85.0f },
			PresetEntry{ ShotType::kPlayerOverhead, 12, Move::kCraneDown, 26, 5.0f, 85.0f },
			PresetEntry{ ShotType::kWide, 25, Move::kPullOut, 22, 6.0f, 88.0f },
			PresetEntry{ ShotType::kMaster, 25, Move::kCraneUp, 45, 7.0f, 95.0f },
			PresetEntry{ ShotType::kDistant, 20, Move::kPullOut, 35, 6.0f, 45.0f },
			PresetEntry{ ShotType::kDistantLow, 15, Move::kDrift, 15, 6.0f, 45.0f },
		};

		template <std::size_t N>
		void LoadPreset(const std::array<PresetEntry, N>& a_entries)
		{
			for (auto& setup : g.setups) {
				setup = Setup{};
			}
			for (const auto& e : a_entries) {
				g.setups[static_cast<std::size_t>(e.type)] = Setup{ e.weight, e.move, e.amount, e.seconds, e.lens };
			}
		}

		// Every angle gets a lens from the preset, also those the preset doesn't use (an
		// emotion close-up in Room, the opening shot from your eyes, the plain shots for seated
		// people and merchants): the lens of the preset's nearest angle of the same person, so
		// each shot keeps the preset's feel (0.9.1; until then, one general lens per preset).
		void FillMissingLenses()
		{
			using enum ShotType;
			const auto lensOf = [](ShotType a_type) { return g.setups[static_cast<std::size_t>(a_type)].lens; };
			for (std::size_t i = 0; i < g.setups.size(); ++i) {  // (kCount here is ShotType::kCount, from the using enum)
				auto& setup = g.setups[i];
				if (setup.lens >= 10.0f) {
					continue;
				}
				const auto type = static_cast<ShotType>(i);
				static constexpr std::array kBoth{ kTwoShot, kProfile, kWide, kMaster, kDistant, kOverPlayerShoulder, kThreeQuarterNpc };
				static constexpr std::array kNpcClose{ kCloseUp, kThreeQuarterNpc, kMediumNpc, kOverPlayerShoulder, kLongNpc, kDistant, kMaster };
				static constexpr std::array kNpcOther{ kThreeQuarterNpc, kMediumNpc, kOverPlayerShoulder, kCloseUp, kLongNpc, kDistant, kMaster };
				static constexpr std::array kYouClose{ kClosePlayer, kThreeQuarterPlayer, kMediumPlayer, kOverNpcShoulder, kLongPlayer, kDistant, kMaster };
				static constexpr std::array kYouOther{ kThreeQuarterPlayer, kMediumPlayer, kOverNpcShoulder, kClosePlayer, kLongPlayer, kDistant, kMaster };
				const std::span<const ShotType> order = Shots::Neutral(type) ? std::span<const ShotType>{ kBoth } :
				                                        Shots::OnNpc(type)   ? (Shots::FollowsFace(type) ? std::span<const ShotType>{ kNpcClose } : std::span<const ShotType>{ kNpcOther }) :
				                                                               (Shots::FollowsFace(type) ? std::span<const ShotType>{ kYouClose } : std::span<const ShotType>{ kYouOther });
				for (const auto other : order) {
					if (lensOf(other) >= 10.0f) {
						setup.lens = lensOf(other);
						break;
					}
				}
			}
		}

		// [Shots] lines: "CloseUp=60" sets a weight (0 turns it off), "CloseUpMove=Push in",
		// "CloseUpMoveAmount=40", "CloseUpMoveTime=6".
		void ApplyOverrides()
		{
			for (const auto& [key, value] : Settings::Get().shotOverrides) {
				std::string_view k{ key };
				std::string_view field;
				for (const std::string_view suffix : { std::string_view{ "MoveAmount" }, std::string_view{ "MoveTime" }, std::string_view{ "Move" } }) {
					if (k.size() > suffix.size() && k.ends_with(suffix)) {
						field = suffix;
						k.remove_suffix(suffix.size());
						break;
					}
				}
				const auto type = Shots::FromKey(k);
				if (!type) {
					REX::WARN("[Shots] {}: no shot by that name", key);
					continue;
				}
				auto& setup = g.setups[static_cast<std::size_t>(*type)];
				try {
					if (field.empty()) {
						const int weight = std::clamp(std::stoi(value), 0, 100);
						if (setup.weight == 0 && weight > 0) {
							setup.move = Shots::DefaultMove(*type);
							setup.amount = Shots::DefaultMoveAmount(*type);
						}
						setup.weight = weight;
					} else if (field == "Move") {
						if (const auto move = Shots::MoveFromName(value)) {
							setup.move = *move;
						} else {
							REX::WARN("[Shots] {}: unknown move '{}'", key, value);
						}
					} else if (field == "MoveAmount") {
						setup.amount = std::clamp(std::stoi(value), 0, 100);
					} else {
						setup.seconds = std::clamp(std::stof(value), 0.3f, 20.0f);
					}
				} catch (...) {
					REX::WARN("[Shots] {}: '{}' is not a number", key, value);
				}
			}
		}

		int NewLineTarget()
		{
			const auto& s = Settings::Get();
			(void)s;
			const int lo = std::clamp(g.minLines, 1, 20);
			const int hi = std::clamp((std::max)(g.maxLines, lo), 1, 20);
			return std::uniform_int_distribution<int>{ lo, hi }(g.rng);
		}

		Shots::Input InputFor(const Frame& a_frame, const Setup& a_setup, float a_progress)
		{
			const auto&  s = Settings::Get();
			Shots::Input in;
			in.npcHead = a_frame.npcHead;
			in.playerHead = a_frame.playerHead;
			const float own = OwnLens(a_setup.lens);
			in.lensDegrees = own > 0.0f ? own : Lens();
			in.maxDistance = g.maxDistance;
			in.true180 = s.true180;
			in.side = a_frame.side;
			in.enforceLine = s.enforceLine || s.true180;
			in.followFace = s.followFace;
			in.npcFaceValid = a_frame.npcFaceValid;
			in.playerFaceValid = a_frame.playerFaceValid;
			in.npcFacing = a_frame.npcFacing;
			in.playerFacing = a_frame.playerFacing;
			in.npcGround = a_frame.npcGround;
			in.playerGround = a_frame.playerGround;
			in.feetValid = a_frame.feetValid;
			in.sway = static_cast<float>(CameraSway()) / 100.0f;
			in.lookRoomScale = PresetIs("starborn") ? 0.25f : 1.0f;  // Starborn: faces centred (0.8.14); half off-centre (0.9.23); a quarter (0.9.25)
			in.roomAngle = a_frame.roomAngle;
			in.roomDistance = a_frame.roomDistance;
			in.roomRise = a_frame.roomRise;
			in.npcFeet = a_frame.npcFeet;
			in.playerFeet = a_frame.playerFeet;
			in.npcSwayHead = a_frame.npcSwayHead;
			in.playerSwayHead = a_frame.playerSwayHead;
			in.progress = a_progress;
			in.move = s.cameraMoves ? a_setup.move : Move::kLocked;
			in.moveAmount = a_setup.amount;
			return in;
		}

		// The person a setup frames (for the neutral ones, whoever's turn it is), and the other.
		const RE::NiPoint3& SubjectHead(ShotType a_type, const Frame& a_frame)
		{
			const bool onNpc = Shots::Neutral(a_type) ? a_frame.npcTurn : Shots::OnNpc(a_type);
			return onNpc ? a_frame.npcHead : a_frame.playerHead;
		}

		const RE::NiPoint3& NearHead(ShotType a_type, const Frame& a_frame)
		{
			const bool onNpc = Shots::Neutral(a_type) ? a_frame.npcTurn : Shots::OnNpc(a_type);
			return onNpc ? a_frame.playerHead : a_frame.npcHead;
		}

		// Obstacle check for a setup: where the camera starts and where its move ends.
		Clearance ClearanceFor(ShotType a_type, const Frame& a_frame, const Setup& a_setup)
		{
			if (!g_obstacleTest) {
				return {};
			}
			const auto now = clock::now();
			if (now - g.clearanceAt > std::chrono::milliseconds(50)) {
				g.clearance = {};
				g.clearanceAt = now;
			}
			auto& cached = g.clearance[static_cast<std::size_t>(a_type)];
			if (cached) {
				return *cached;
			}
			const auto& subject = SubjectHead(a_type, a_frame);
			const auto& nearHead = NearHead(a_type, a_frame);
			const bool  overShoulder = Shots::OverShoulder(a_type);
			Clearance   result;
			bool        first = true;
			for (const float progress : { 0.0f, 1.0f }) {
				const auto solved = Shots::Solve(a_type, InputFor(a_frame, a_setup, progress));
				if (!solved.ok) {
					continue;
				}
				const auto c = g_obstacleTest(subject, solved.pose.position, nearHead, overShoulder);
				if (first) {
					result = c;
					first = false;
				} else {
					result.usable = result.usable && c.usable;
					result.limit = (std::min)(result.limit, c.limit);
					if (c.blockedAt >= 0.0f && (result.blockedAt < 0.0f || c.blockedAt < result.blockedAt)) {
						result.blockedAt = c.blockedAt;
						result.length = c.length;
					}
					result.pastNear = result.pastNear || c.pastNear;
					result.bystander = result.bystander || c.bystander;
					result.fallbackOk = result.fallbackOk && c.fallbackOk;
				}
				if (InputFor(a_frame, a_setup, 0.0f).move == Move::kLocked) {
					break;  // no move: start and end are the same
				}
			}
			if (!result.usable) {
				g.skipped += std::format("{}{} (blocked {:.1f} m from them{})", g.skipped.empty() ? "" : ", ", Shots::Key(a_type), result.blockedAt,
					result.bystander ? " by someone standing there" : result.pastNear ? ", beyond the near person" : "");
			}
			cached = result;
			return result;
		}

		// 0.9.18: a large speaker (Vasco: head 1.75 m above his feet; people 1.61-1.62) between
		// the camera and you hides you in the narrower shots of you (0.9.18 test: at 22-27
		// degrees off the line his back filled the close-ups; at 34 you were half behind him; at
		// 40 clear). The obstacle check leaves the near person out on purpose (their shoulder
		// is part of an over-the-shoulder shot), so with a large speaker the shots of you keep
		// to wider angles.
		// (0.9.22: fLargeSpeakerHeightNpc, 1.70 m, head above feet; fLargeSpeakerAnglePlayer,
		// 38 degrees off the line, for shots of you.)
		bool HiddenBehindLargeSpeaker(ShotType a_type, const Frame& a_frame, const Shots::Result& a_solved)
		{
			const auto& s = Settings::Get();
			const bool  onYou = Shots::Neutral(a_type) ? !a_frame.npcTurn : !Shots::OnNpc(a_type);
			if (!onYou || !a_frame.feetValid || s.largeSpeakerHeight <= 0.0f || a_frame.npcHead.z - a_frame.npcFeet.z < s.largeSpeakerHeight) {
				return false;
			}
			return std::abs(a_solved.angle) < s.largeSpeakerAngle;
		}

		// 0.9.23 (the 2026-11-18 preset review):
		// The room and shared shots (the two of you, the room, the overhead of you): as your angle,
		// 2 of your turns in a row at most (Constellation's distant shot held for 5).
		bool RoomOrShared(ShotType a_type)
		{
			return Shots::Neutral(a_type) || a_type == ShotType::kPlayerOverhead;
		}

		// How long one close-up of them may hold before a wider shot of them (runs of emotional
		// lines held one for up to 28 s): fCloseUpLimitSecondsNpc, or the preset's own: Frontier
		// 12 s, Starborn 18, Constellation 14, Nebula 12; CCC's presets none, as CCC. 0 = none.
		float CloseUpLimit()
		{
			const float mine = Settings::Get().closeUpLimit;
			if (mine >= 0.0f) {
				return mine;
			}
			return PresetIs("frontier") ? 12.0f : PresetIs("starborn") ? 18.0f : PresetIs("constellation") ? 14.0f : PresetIs("nebula") ? 12.0f : 0.0f;
		}

		// Listening reactions on a close-up of you: bListeningCloseUpsPlayer, or the preset's own
		// (Frontier's in mediums and three-quarters; close-ups of you for your expressions).
		bool ListeningCloseUps()
		{
			const int mode = Settings::Get().listeningCloseUps;
			return mode >= 0 ? mode == 1 : !PresetIs("frontier");
		}

		// Extreme close-ups of you: none in Constellation (a documentary doesn't go that close).
		bool ExtremeCloseUpsOfYou()
		{
			return !PresetIs("constellation");
		}

		// A shot whose framing wants the camera more than 2.5 times the preset's cap away; room
		// shots excepted (0.9.26, see Usable).
		bool TooFarForCap(ShotType a_type, const Shots::Result& a_solved)
		{
			return Shots::AnchorOf(a_type) != Shots::Anchor::kScene && a_solved.wantedDistance > g.maxDistance * 2.5f;
		}

		bool Usable(ShotType a_type, const Frame& a_frame, bool a_checkObstacles = true)
		{
			const auto& setup = g.setups[static_cast<std::size_t>(a_type)];
			if (setup.weight <= 0) {
				return false;
			}
			if (!Shots::Neutral(a_type) && Shots::OnNpc(a_type) != a_frame.npcTurn) {
				return false;
			}
			if (a_frame.nearSeated && Shots::OverShoulder(a_type)) {
				return false;  // the camera would stand in the seat's back
			}
			// A reaction shows you (0.8.3, as CCC): your regular shots, never the room, the
			// two-shot or the profile of you both.
			if (a_frame.reaction && Shots::Neutral(a_type)) {
				return false;
			}
			// 0.9.23: never the overhead of you (it reads as someone watching you); and in
			// presets without listening close-ups, your close-ups only for your expressions.
			if (a_frame.reaction && a_type == ShotType::kPlayerOverhead) {
				return false;
			}
			if (a_frame.reaction && !a_frame.playerCloseUp && !ListeningCloseUps() &&
				(a_type == ShotType::kClosePlayer || a_type == ShotType::kExtremeClosePlayer)) {
				return false;
			}
			const auto solved = Shots::Solve(a_type, InputFor(a_frame, setup, 0.0f));
			// A framing that wants the camera far beyond the cap would become a different
			// shot entirely (a full figure turned into a close-up); leave it out. Not the room
			// shots (0.9.26): they have their own limit and the room measurement's checks, and
			// this one left out Starborn's and Close's distant shot (16.5 m wanted against a
			// 12.5 m line) and Starborn's master (12.7 m) every time.
			if (!solved.ok || TooFarForCap(a_type, solved) || HiddenBehindLargeSpeaker(a_type, a_frame, solved)) {
				return false;
			}
			return !a_checkObstacles || ClearanceFor(a_type, a_frame, setup).usable;
		}

		// Like Usable, but regardless of the preset's weight: the emotion close-ups use the
		// close-up and extreme close-up even in presets that leave them out (CCC does the same).
		bool Placeable(ShotType a_type, const Frame& a_frame, bool a_checkObstacles = true)
		{
			if (!Shots::Neutral(a_type) && Shots::OnNpc(a_type) != a_frame.npcTurn) {
				return false;
			}
			const auto solved = Shots::Solve(a_type, InputFor(a_frame, g.setups[static_cast<std::size_t>(a_type)], 0.0f));
			if (!solved.ok || TooFarForCap(a_type, solved) || HiddenBehindLargeSpeaker(a_type, a_frame, solved)) {
				return false;
			}
			return !a_checkObstacles || ClearanceFor(a_type, a_frame, IntenseSetup(a_type)).usable;
		}

		// Logs the shots just left out for obstacles (one line per decision).
		void LogSkipped()
		{
			if (!g.skipped.empty()) {
				REX::INFO("Obstacles: skipped {}", g.skipped);
				g.skipped.clear();
			}
		}

		// The close-up for an intense line: the extreme close-up when the line was rolled one
		// (iExtremeCloseUpChanceNpc, decided with the line since 0.8.3).
		std::optional<ShotType> IntenseShot(const Frame& a_frame)
		{
			const bool extreme = a_frame.intenseExtreme;
			if (extreme && Placeable(ShotType::kExtremeClose, a_frame)) {
				return ShotType::kExtremeClose;
			}
			if (Placeable(ShotType::kCloseUp, a_frame)) {
				return ShotType::kCloseUp;
			}
			if (Placeable(ShotType::kExtremeClose, a_frame)) {
				return ShotType::kExtremeClose;
			}
			return std::nullopt;
		}

		// Your emotion close-up (0.7.11): a reaction shot on a line your face answers with an
		// expression, as a close-up of you; extreme as often as the NPC's.
		std::optional<ShotType> PlayerCloseShot(const Frame& a_frame)
		{
			const bool extreme = ExtremeCloseUpsOfYou() &&
			                     (a_frame.playerExtreme || std::uniform_int_distribution<int>{ 1, 100 }(g.rng) <= Settings::Get().extremeCloseUpChancePlayer);
			if (extreme && Placeable(ShotType::kExtremeClosePlayer, a_frame)) {
				return ShotType::kExtremeClosePlayer;
			}
			if (Placeable(ShotType::kClosePlayer, a_frame)) {
				return ShotType::kClosePlayer;
			}
			return std::nullopt;
		}

		// A single of the speaker with no one else in the picture (no shoulder in the foreground,
		// not the two of you), and not a close-up (they're for emphasis; opening on one every
		// time would wear them out): the conversation's first shot from first person, while
		// your body is still being built (0.9.2).
		bool CleanSingle(ShotType a_type)
		{
			switch (a_type) {
			case ShotType::kThreeQuarterNpc:
			case ShotType::kMediumNpc:
			case ShotType::kMediumProfile:
				return true;
			default:
				return false;
			}
		}

		// The opening single in Close and Starborn, which have no three-quarter or medium of
		// their own: their close-up (their main shot rather than emphasis) or a three-quarter
		// through the lens of their nearest angle, half the time each (0.9.2, for variety).
		std::optional<ShotType> CloseOpening(const Frame& a_frame)
		{
			if (!PresetIs("close") && !PresetIs("starborn")) {
				return std::nullopt;
			}
			const bool closeFirst = std::uniform_int_distribution<int>{ 0, 1 }(g.rng) == 0;
			const auto first = closeFirst ? ShotType::kCloseUp : ShotType::kThreeQuarterNpc;
			const auto second = closeFirst ? ShotType::kThreeQuarterNpc : ShotType::kCloseUp;
			for (const auto type : { first, second, ShotType::kMediumNpc }) {
				if (Placeable(type, a_frame)) {
					return type;
				}
			}
			return std::nullopt;
		}

		std::optional<ShotType> Pick(const Frame& a_frame)
		{
			if (a_frame.openingSingle) {
				if (const auto shot = CloseOpening(a_frame)) {
					return shot;
				}
			}
			std::vector<std::pair<ShotType, int>> pool;
			int                                   total = 0;
			// The conversation's first shot is never a close-up (0.9.2: they're for emphasis;
			// opening on one wears them out), except in Close and Starborn, where they're the
			// main shot. (A strong first line's emotion close-up still can, as any line's.)
			const bool noCloseUp = g.cuts == 0 && !PresetIs("close") && !PresetIs("starborn");
			for (std::size_t i = 0; i < kCount; ++i) {
				const auto type = static_cast<ShotType>(i);
				if (g.current && type == *g.current) {
					continue;  // a change of angle should change the angle
				}
				if (a_frame.openingSingle && !CleanSingle(type)) {
					continue;
				}
				if (noCloseUp && Shots::FollowsFace(type)) {
					continue;
				}
				if (type == ShotType::kExtremeClose || type == ShotType::kExtremeClosePlayer) {
					continue;  // only for the lines that call for one (0.8.15, as CCC)
				}
				if (g.avoidShot && type == *g.avoidShot) {
					continue;  // never a cut to the shot already on screen (0.9.28)
				}
				if (g.noRoomPick && RoomOrShared(type)) {
					continue;  // your angle has been on one for 2 turns (0.9.23)
				}
				if (Usable(type, a_frame)) {
					const int weight = g.setups[i].weight;
					pool.emplace_back(type, weight);
					total += weight;
				}
			}
			if (pool.empty() && a_frame.openingSingle) {
				// The preset has no three-quarter or medium of them that fits (Close, Starborn): one
				// anyway, through the lens of the preset's nearest angle; a close-up only if
				// nothing else can be placed.
				for (const auto type : { ShotType::kThreeQuarterNpc, ShotType::kMediumNpc, ShotType::kCloseUp }) {
					if (Placeable(type, a_frame)) {
						return type;
					}
				}
			}
			if (pool.empty() && g.current && Usable(*g.current, a_frame)) {
				return g.current;
			}
			if (pool.empty()) {
				// Nothing fully clear: the blocked shot with the most room, pulled in.
				std::optional<ShotType> best;
				float                   bestLimit = 0.0f;
				for (std::size_t i = 0; i < kCount; ++i) {
					const auto type = static_cast<ShotType>(i);
					if (type == ShotType::kExtremeClose || type == ShotType::kExtremeClosePlayer || !Usable(type, a_frame, false)) {
						continue;
					}
					const auto c = ClearanceFor(type, a_frame, g.setups[i]);
					if (c.fallbackOk && c.limit > bestLimit) {
						best = type;
						bestLimit = c.limit;
					}
				}
				LogSkipped();
				if (best) {
					REX::INFO("Obstacles: nothing fully clear; {} pulled in to {:.1f} m fits best", Shots::Key(*best), bestLimit);
					return best;
				}
				if (!g.loggedEmpty) {
					g.loggedEmpty = true;
					REX::WARN("Director: no shot in the preset fits this conversation ({} turn); using the plain over-the-shoulder shot",
						a_frame.npcTurn ? "their" : "your");
				}
				return std::nullopt;
			}
			int roll = std::uniform_int_distribution<int>{ 1, total }(g.rng);
			for (const auto& [type, weight] : pool) {
				roll -= weight;
				if (roll <= 0) {
					return type;
				}
			}
			return pool.back().first;
		}

		// Counts a line for its speaker when it ends, unless it was short. Returns true on
		// the frame a line starts.
		bool TrackLine(Line& a_line, Side& a_side, bool a_talking, clock::time_point a_now, bool a_count = true)
		{
			bool started = false;
			if (a_talking && !a_line.talking) {
				a_line.start = a_now;
				started = true;
			} else if (!a_talking && a_line.talking && a_count) {
				const float seconds = std::chrono::duration<float>(a_now - a_line.start).count();
				if (seconds >= Settings::Get().shortLineSeconds && ++a_side.lines >= a_side.linesTarget) {
					a_side.changeDue = true;
				}
			}
			a_line.talking = a_talking;
			return started;
		}

		void Cut(ShotType a_type, const Frame& a_frame, clock::time_point a_now, const char* a_why, bool a_newAngle)
		{
			auto& side = a_frame.npcTurn ? g.npc : g.player;
			g.intenseSetup.reset();
			g.extremeLineOver = false;
			g.lineCutOwed = false;  // (0.9.23) whatever cut was owed at the line's start is made
			g.moveFrom = a_now;
			g_shotLens.store(g.setups[static_cast<std::size_t>(a_type)].lens, std::memory_order_relaxed);
			g.current = a_type;
			++g.cuts;
			g.why = a_why;
			g.currentNpcSide = a_frame.npcTurn;
			g.cutAt = a_now;
			// An extreme close-up is never a person's usual angle (0.8.15, as CCC: it's reserved for
			// the lines that call for it; the 0.8.14 test kept you on one for ten turns in Close).
			if (a_newAngle && a_type != ShotType::kExtremeClose && a_type != ShotType::kExtremeClosePlayer) {
				side.shot = a_type;
				side.lines = 0;
				side.linesTarget = NewLineTarget();
				side.changeDue = false;
			}
			// Settle the angle and framing side now and hold them for the shot.
			const auto& setup = g.setups[static_cast<std::size_t>(a_type)];
			Shots::Input in = InputFor(a_frame, setup, 0.0f);
			const auto   solved = Shots::Solve(a_type, in);
			g.heldAngle = solved.angle;
			g.heldLookSign = solved.lookSign;
			g.heldFacePitch = solved.faced ? solved.facePitch : std::numeric_limits<float>::quiet_NaN();
			if (solved.faced && (std::abs(solved.faceYaw) >= 3.0f || std::abs(solved.facePitch) >= 3.0f)) {
				REX::INFO("Face: {} turned {:+.0f} degrees toward where the face points, camera {} {:.0f} degrees", Shots::Key(a_type), solved.faceYaw,
					solved.facePitch < 0.0f ? "lowered" : "raised", std::abs(solved.facePitch));
			}
			const bool close = a_type == ShotType::kCloseUp || a_type == ShotType::kExtremeClose;
			const auto clear = ClearanceFor(a_type, a_frame, close && a_why && std::string_view{ a_why }.find("emotion close-up") != std::string_view::npos ? IntenseSetup(a_type) : setup);
			g.limit = clear.usable || clear.fallbackOk ? clear.limit : std::numeric_limits<float>::infinity();
			LogSkipped();
			REX::INFO("Director: {} -> {} ({}; {:.1f} m away at {:.1f} degrees; move {} {}; new angle after {} more lines){}", a_why, Shots::Key(a_type),
				a_frame.npcTurn ? "their turn" : "your turn", solved.distance, solved.angle,
				Shots::MoveName(Settings::Get().cameraMoves ? setup.move : Move::kLocked), setup.amount, side.linesTarget - side.lines,
				std::isfinite(g.limit) ? std::format("; obstacle {:.1f} m from them, camera pulled in to {:.1f} m", clear.blockedAt, g.limit) : std::string{});
		}
	}

	void SetObstacleTest(ObstacleTest a_test)
	{
		g_obstacleTest = a_test;
	}

	namespace
	{
		// The preset's shots, distance caps and room shot limits (from Reset, and from the
		// preset key mid-conversation, 0.8.3). Returns its name, lower case.
		std::string ApplyPreset(float& presetIndoor, float& presetOutdoor)
		{
			const auto& s = Settings::Get();
			std::string preset{ s.preset };
			std::ranges::transform(preset, preset.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
			// Each preset's own camera distance caps (indoors, outdoors): as far as its widest
			// shot wants at Starfield's lens, now that obstacle detection keeps the camera out of
			// walls (Close: the over-the-shoulder, 4.8 m; Standard: the profile, 6.8 m, and
			// outdoors the two-shot, 10.2 m; Room: the long shots, 12.7 m).
			presetIndoor = 7.0f;
			presetOutdoor = 10.5f;
			// The cutting rhythm: CCC 1.5.0's per preset (Standard a new angle every 2-3 lines,
			// Close 3-6, Room 3-5; the turn never asks for one; held at least 2.4 s, Room 3 s);
			// CCC SF's their own (0.8.14). 0.9.8: the cutting pace nudges it (the named styles that
			// replaced it are gone); iMinLines, iMaxLines and fShortestHold override.
			{
				static constexpr float kLineScale[] = { 1.6f, 1.3f, 1.0f, 0.75f, 0.55f };
				// 0.9.26: half steps too, between the whole steps' scales.
				const float pace = std::clamp(s.CuttingPaceFor(preset), -2.0f, 2.0f) + 0.0f;  // (the preset's own pace, 0.9.27; + 0 so -0 shows as 0)
				g.pace = pace;
				const float at = pace + 2.0f;
				const int   lo = std::clamp(static_cast<int>(std::floor(at)), 0, 4);
				const int   hi = (std::min)(lo + 1, 4);
				const float k = kLineScale[lo] + (kLineScale[hi] - kLineScale[lo]) * (at - static_cast<float>(lo));
				const auto  cut = [&](int a_fresh, int a_min, int a_max, float a_hold) {
                    g.freshAngleChance = std::clamp(a_fresh + static_cast<int>(std::lround(20.0f * pace)), 0, 100);
                    g.minLines = (std::max)(1, static_cast<int>(std::lround(a_min * k)));
                    g.maxLines = (std::max)(g.minLines, static_cast<int>(std::lround(a_max * k)));
                    g.shortestHold = (std::max)(1.0f, a_hold * k);
                    if (s.minLines > 0) {
                        g.minLines = s.minLines;
                    }
                    if (s.maxLines > 0) {
                        g.maxLines = s.maxLines;
                    }
                    g.maxLines = (std::max)(g.maxLines, g.minLines);
                    if (s.shortestHold > 0.0f) {
                        g.shortestHold = s.shortestHold;
                    }
				};
				if (preset == "close") {
					cut(0, 3, 6, 2.4f);
				} else if (preset == "room") {
					cut(0, 3, 5, 3.0f);
				} else if (preset == "frontier") {
					cut(25, 2, 4, 2.4f);   // relaxed: a fresh angle on a quarter of the turns
				} else if (preset == "starborn") {
					cut(0, 4, 7, 3.5f);    // long holds, few cuts
				} else if (preset == "constellation") {
					cut(25, 3, 5, 2.8f);   // an observer's rhythm
				} else if (preset == "nebula") {
					cut(40, 3, 5, 2.5f);   // space opera: the variety, not the speed (0.9.31: what +0.5 on 0.9.28's gave, a touch more varied, the user's call; 0.9.28 30%, 3-6 lines, 2.9 s: what -1.5 gave, the user's pick; 0.9.26 60%, 2-4 lines, 2 s; before 75%, 2-3, 1.6 s)
				} else {
					cut(0, 2, 3, 2.4f);    // Standard
				}
			}
			// Room shots' own limits: CCC's presets none (as far as CCC's framing wants), CCC
			// SF's 9 m indoors and 12 m outdoors.
			g.roomIndoor = g.roomOutdoor = 0.0f;
			if (preset == "close") {
				LoadPreset(kClose);
				presetIndoor = presetOutdoor = 5.0f;
			} else if (preset == "room") {
				LoadPreset(kRoom);
				presetIndoor = 7.0f;
				presetOutdoor = 13.0f;
			} else if (preset == "frontier") {
				LoadPreset(kFrontier);
				presetIndoor = 7.0f;
				presetOutdoor = 13.0f;
				g.roomIndoor = 9.0f;
				g.roomOutdoor = 12.0f;
			} else if (preset == "starborn") {
				LoadPreset(kStarborn);
				presetIndoor = presetOutdoor = 5.0f;
				g.roomIndoor = 9.0f;
				g.roomOutdoor = 12.0f;
			} else if (preset == "nebula") {
				LoadPreset(kNebula);
				presetIndoor = 7.0f;
				presetOutdoor = 13.0f;
				g.roomIndoor = 9.0f;
				g.roomOutdoor = 12.0f;
			} else if (preset == "constellation") {
				LoadPreset(kConstellation);
				presetIndoor = 7.0f;
				presetOutdoor = 13.0f;
				g.roomIndoor = 9.0f;
				g.roomOutdoor = 12.0f;
			} else {
				if (preset != "standard") {
					REX::WARN("Director: unknown preset '{}', using Standard", s.preset);
				}
				LoadPreset(kStandard);
				preset = "standard";
			}
			ApplyOverrides();
			FillMissingLenses();
			return preset;
		}

		std::atomic<bool> g_presetSwitch{ false };

		// On the Director's own thread: the preset key was pressed during a conversation.
		void SwitchPresetNow(const Frame& a_frame)
		{
			if (!g_presetSwitch.exchange(false)) {
				return;
			}
			const auto& s = Settings::Get();
			float       indoor = 0.0f, outdoor = 0.0f;
			const auto  preset = ApplyPreset(indoor, outdoor);
			auto*       player = RE::PlayerCharacter::GetSingleton();
			const bool  interior = player && player->parentCell && player->parentCell->IsInterior();
			const float playerCap = interior ? s.maxCameraDistance : s.maxCameraDistanceExterior;
			g.maxDistance = playerCap > 0.0f ? playerCap : interior ? indoor : outdoor;
			// New angles from the new preset on the next line of each.
			g.npc.shot.reset();
			g.player.shot.reset();
			g.npc.changeDue = true;
			g.player.changeDue = true;
			REX::INFO("Director: preset switched to {} (camera at most {:.1f} m from the subject)", preset, g.maxDistance);
			(void)a_frame;
		}
	}

	void SwitchPreset()
	{
		g_presetSwitch.store(true);
	}

	void Reset()
	{
		const auto& s = Settings::Get();
		float       presetIndoor = 7.0f, presetOutdoor = 10.5f;
		const auto  preset = ApplyPreset(presetIndoor, presetOutdoor);
		g_presetSwitch.store(false);
		g.current.reset();
		g.lastFrame.reset();
		g.npc = Side{};
		g.player = Side{};
		g.npc.linesTarget = NewLineTarget();
		g.player.linesTarget = NewLineTarget();
		g.heldAngle = std::numeric_limits<float>::quiet_NaN();
		g.heldLookSign = 0.0f;
		g.heldFacePitch = std::numeric_limits<float>::quiet_NaN();
		auto*      player = RE::PlayerCharacter::GetSingleton();
		const bool interior = player && player->parentCell && player->parentCell->IsInterior();
		const float playerCap = interior ? s.maxCameraDistance : s.maxCameraDistanceExterior;
		g.maxDistance = playerCap > 0.0f ? playerCap : interior ? presetIndoor : presetOutdoor;
		const char* capFrom = playerCap > 0.0f ? (interior ? "your fMaxCameraDistance" : "your fMaxCameraDistanceExterior") : "the preset's own";
		g.npcLine = {};
		g.playerLine = {};
		g.loggedEmpty = false;
		g.intensePending = false;
		g.intenseSetup.reset();
		g.extremeLineOver = false;
		g_shotLens.store(0.0f, std::memory_order_relaxed);
		g.inReaction = false;
		g.reactionReturn.reset();
		g.reactionReturnSetup.reset();
		g.limit = std::numeric_limits<float>::infinity();
		g.plainTurn.reset();
		g.lineCutOwed = false;
		g.closeUpCooldown = {};
		g.noRoomPick = false;
		g.avoidShot.reset();
		g.beatActive = false;
		g.cuts = 0;
		g.why = "";
		g.clearance = {};
		g.skipped.clear();
		gp = PlainIntense{};
		REX::INFO("Director: preset {}, {}: camera at most {:.1f} m from the subject ({}); cutting pace {:+.1f} ({}% fresh angle per turn, new angle every {}-{} lines, hold at least {:.1f} s)",
			preset, interior ? "interior" : "exterior", g.maxDistance, capFrom, g.pace, g.freshAngleChance, g.minLines, g.maxLines, g.shortestHold);
		REX::INFO("Settings in effect: focus on the person in shot {}, focus style {} (wide beyond {:.1f} m), emotion close-ups {} ({}% extreme): {}",
			s.focusOnSubject ? "on" : "off", s.deepFocusLongShots ? "Wide" : "Cinematic", s.deepFocusBeyond, EmotionCloseUps() == 0 ? "off" : EmotionCloseUps() == 3 ? "strong lines only" : "on", ExtremeCloseUpChanceNpc(),
			std::format("mild {}% ({}), medium {}% ({}), strong {}% ({})", s.closeUpChanceMild, s.closeUpEmotionsMild, s.closeUpChanceMedium, s.closeUpEmotionsMedium,
				s.closeUpChanceStrong, s.closeUpEmotionsStrong));
		REX::INFO("Settings in effect: obstacle check {} ({:.2f} m wide){}", s.obstacleCheck ? "on" : "off", s.obstacleRadius,
			s.obstacleCheck && !g_obstacleTest ? ", but the game's collision check isn't available" : "");
		REX::INFO("Settings in effect: close-ups follow the face {}, 180-degree rule {}, never across the eyeline {}, camera sway {}%, head dead band {:.2f} m",
			s.followFace ? "on" : "off", s.true180 ? "on (one side for the whole conversation)" : "off (each person over their own side)",
			s.enforceLine || s.true180 ? "on" : "off", CameraSway(), s.faceDeadband);
	}

	std::optional<Camera::Pose> Update(const Frame& a_frame, clock::time_point a_now)
	{
		g.lastFrame = a_frame;
		SwitchPresetNow(a_frame);
		const auto& s = Settings::Get();
		bool        npcStarted = TrackLine(g.npcLine, g.npc, a_frame.npcTalking, a_now);
		// A new line of theirs that follows the last without a pause (0.8.3): it ends the
		// last one and starts. Not within half a second of a start (the counter catching up).
		if (!npcStarted && a_frame.npcTalking && a_frame.npcLineSerial != g.npcSerial &&
			std::chrono::duration<float>(a_now - g.npcLine.start).count() > 0.5f) {
			const float seconds = std::chrono::duration<float>(a_now - g.npcLine.start).count();
			if (seconds >= s.shortLineSeconds && ++g.npc.lines >= g.npc.linesTarget) {
				g.npc.changeDue = true;
			}
			g.npcLine.start = a_now;
			npcStarted = true;
		}
		g.npcSerial = a_frame.npcLineSerial;
		// Your lines are counted by turn, not by voice (0.8.17): the player is silent, so the
		// 0.8.16 test held you on one angle for nine turns (Constellation) and never changed it in
		// the presets with no fresh-angle roll.
		const bool  playerStarted = TrackLine(g.playerLine, g.player, a_frame.playerTalking, a_now, false);
		const bool  lineStarted = a_frame.npcTurn ? npcStarted : playerStarted;
		// How long the shot on screen has held, as of now: a cut earlier in this frame starts it
		// again (0.9.23: an emotion close-up's cut, then "new angle" on the same frame from the
		// time held before it, flashed one frame of the close-up).
		const auto  heldNow = [&]() { return std::chrono::duration<float>(a_now - g.cutAt).count(); };
		auto&       side = a_frame.npcTurn ? g.npc : g.player;
		// A cut owed at a line's start waits for the line's reaction to be decided (0.9.23),
		// at most a second.
		if (lineStarted) {
			g.lineCutOwed = true;
			g.lineCutOwedAt = a_now;
		} else if (g.lineCutOwed && a_now - g.lineCutOwedAt > std::chrono::seconds(1)) {
			g.lineCutOwed = false;
		}
		const bool lineCutNow = g.lineCutOwed && a_frame.lineDecided;

		// Emotion close-ups: a line tagged with one of the chosen expressions starts.
		if (npcStarted) {
			g.intensePending = a_frame.intenseLine && EmotionCloseUps() != 0;
			g.intenseShot.reset();
			// 0.8.7 test: extreme close-ups ran 13 s on average, through the emotion lines after
			// them. One holds its own line only (0.8.9).
			if (g.current && *g.current == ShotType::kExtremeClose && g.currentNpcSide) {
				g.extremeLineOver = true;
			}
		}
		if (!a_frame.npcTalking) {
			g.intensePending = false;
		}
		// Nothing fitted when this turn began: stay on the plain shot until the turn changes
		// (0.5.3 tried again every frame, filling the log).
		if (g.plainTurn && *g.plainTurn != a_frame.npcTurn) {
			g.plainTurn.reset();
		}
		if (g.plainTurn) {
			g.intensePending = false;
			return std::nullopt;
		}
		const bool alreadyClose = g.current && (*g.current == ShotType::kCloseUp || *g.current == ShotType::kExtremeClose);

		// Persuasion beat (CCC's "Hold On Their Answer"): the NPC's answer to a persuasion's
		// outcome is a close-up of them, pushing in, held for their whole reply (0.8.5, as CCC:
		// was the first line only). In a long answer (over 10 s), once the push-in is done, one
		// cut to a medium or three-quarter shot of them; never away from them.
		if (a_frame.persuasionBeat && a_frame.npcTurn && !g.beatActive) {
			std::optional<ShotType> shot;
			if (Placeable(ShotType::kCloseUp, a_frame)) {
				shot = ShotType::kCloseUp;
			} else if (Placeable(ShotType::kExtremeClose, a_frame)) {
				shot = ShotType::kExtremeClose;
			}
			if (shot) {
				Cut(*shot, a_frame, a_now, "persuasion beat", true);
				g.intenseSetup = IntenseSetup(ShotType::kCloseUp);
				g.beatActive = true;
				g.beatAt = a_now;
				g.beatWidened = false;
			} else {
				REX::INFO("Director: persuasion beat skipped (no clear close-up)");
				g.beatActive = true;  // don't retry every frame
			}
			g.intensePending = false;
		}
		if (!a_frame.persuasionBeat) {
			g.beatActive = false;
		}
		if (g.beatActive && a_frame.npcTurn && !g.beatWidened && g.current && (*g.current == ShotType::kCloseUp || *g.current == ShotType::kExtremeClose) &&
			std::chrono::duration<float>(a_now - g.beatAt).count() > 10.0f) {
			g.beatWidened = true;
			for (const auto type : { ShotType::kMediumNpc, ShotType::kThreeQuarterNpc }) {
				if (Placeable(type, a_frame)) {
					Cut(type, a_frame, a_now, "persuasion beat, a long answer: wider", false);
					break;
				}
			}
		}
		const bool holding = g.beatActive && a_frame.npcTurn && g.current &&
		                     (*g.current == ShotType::kCloseUp || *g.current == ShotType::kExtremeClose || g.beatWidened);
		if (!holding) {
			if (g.intensePending && a_now < g.closeUpCooldown) {
				g.intensePending = false;  // just widened at the close-up limit (0.9.23)
			}
			if (g.intensePending && a_frame.npcTurn && (!g.current || a_frame.npcTurn != g.currentNpcSide)) {
				// The turn passes to them on an intense line: the turn's cut is the close-up.
				if (const auto shot = IntenseShot(a_frame)) {
					Cut(*shot, a_frame, a_now, "turn, emotion close-up", true);
					g.intenseSetup = IntenseSetup(*shot);
					NoteEmotionCut(*shot);
					g.intensePending = false;
					g.intenseShot = shot;
				}
			}
			// 0.8.6: a reaction is an insert. It starts: remember the speaker's shot. It ends,
			// back to them: exactly that shot again, no new angle (the turn's 50% fresh-angle roll
			// isn't made, and their line count goes on as if it hadn't happened).
			if (a_frame.reaction && !a_frame.npcTurn && !g.inReaction) {
				g.inReaction = true;
				g.reactionReturn = g.current && g.currentNpcSide ? g.current : g.npc.shot;
				g.reactionReturnSetup = g.intenseSetup;
			}
			if (g.inReaction && a_frame.npcTurn) {
				g.inReaction = false;
				auto shot = g.reactionReturn;
				if (shot && *shot == ShotType::kExtremeClose) {
					shot = ShotType::kCloseUp;  // an extreme close-up holds one line only (0.8.9)
				}
				const bool close = shot && (*shot == ShotType::kCloseUp || *shot == ShotType::kExtremeClose);
				if (shot && g.current && !g.currentNpcSide && (Usable(*shot, a_frame) || (close && Placeable(*shot, a_frame)))) {
					Cut(*shot, a_frame, a_now, "back from the reaction", false);
					g.intenseSetup = g.reactionReturnSetup;
				}
				g.reactionReturn.reset();
				g.reactionReturnSetup.reset();
			} else if (g.inReaction && !a_frame.reaction) {
				g.inReaction = false;  // their line ended on you: it's your turn now
				g.reactionReturn.reset();
				g.reactionReturnSetup.reset();
			}

			// Your emotion close-up: the reaction's cut is a close-up of you, pushing in, once per
			// reaction, also when the camera is already on you (0.7.11 test: it stayed on the
			// three-quarter shot of you). It doesn't become your usual angle.
			if (!a_frame.playerCloseUp) {
				g.playerCloseDone = false;
			} else if (!a_frame.npcTurn && !g.playerCloseDone) {
				g.playerCloseDone = true;
				const bool closeNow = g.current && (*g.current == ShotType::kExtremeClosePlayer ||
				                                       (*g.current == ShotType::kClosePlayer && !a_frame.playerExtreme));
				if (const auto shot = closeNow ? std::optional<ShotType>{} : PlayerCloseShot(a_frame)) {
					Cut(*shot, a_frame, a_now, a_frame.playerChoice ? "your tagged choice" : "reaction, your expression", false);
					g.intenseSetup = IntenseSetup(*shot);
				}
			}
			if ((!g.current || a_frame.npcTurn != g.currentNpcSide) && a_frame.reaction && !a_frame.npcTurn) {
				// A reaction (an insert, 0.8.6): your usual angle if it suits a reaction, else one
				// picked for this reaction only; your own angle for your turns stays as it was.
				std::optional<ShotType> shot = side.shot && Usable(*side.shot, a_frame) ? side.shot : std::nullopt;
				if (!shot) {
					const auto was = g.current;
					g.current = side.shot;  // so Pick avoids repeating it
					shot = Pick(a_frame);
					g.current = was;
				}
				if (shot) {
					Cut(*shot, a_frame, a_now, "reaction", false);
				} else {
					g.current.reset();
					g.plainTurn = a_frame.npcTurn;
					return std::nullopt;
				}
			} else if (!g.current || a_frame.npcTurn != g.currentNpcSide) {
				// The reverse: back to this person's own setup, unless they're due a new one.
				// Each of your turns counts as one of your lines (0.8.17).
				if (!a_frame.npcTurn && side.shot && ++side.lines >= side.linesTarget) {
					side.changeDue = true;
				}
				// 0.9.23: your angle on a room or shared shot for 2 turns in a row at most; this
				// turn gets another (it can come back later).
				const bool roomHeld = !a_frame.npcTurn && side.shot && RoomOrShared(*side.shot) && side.roomTurns >= 2;
				// 0.9.28: never a cut to the shot already on screen. A room or shared shot frames
				// you both, so both sides can hold the same one as their angle; the turn passing
				// then cut from it to itself (0.9.27 test: four distant-to-distant cuts in
				// Constellation). This side takes another angle, and not a room shot, so a wide
				// shot is followed by a closer one.
				const auto onScreen = g.current;
				const bool sameAgain = side.shot && onScreen && *side.shot == *onScreen;
				const bool fresh = std::uniform_int_distribution<int>{ 1, 100 }(g.rng) <= g.freshAngleChance;
				if (side.shot && !side.changeDue && !fresh && !roomHeld && !sameAgain && Usable(*side.shot, a_frame)) {
					Cut(*side.shot, a_frame, a_now, "turn", false);
					side.roomTurns = RoomOrShared(*side.shot) ? side.roomTurns + 1 : 0;
				} else {
					g.current = side.shot;  // so Pick avoids repeating it
					g.noRoomPick = roomHeld || sameAgain;
					g.avoidShot = onScreen;
					const auto pick = Pick(a_frame);
					g.noRoomPick = false;
					g.avoidShot.reset();
					if (pick) {
						Cut(*pick, a_frame, a_now,
							sameAgain ? "turn, not the same shot again" : roomHeld ? "turn, after 2 turns on a room shot" : side.shot ? "turn, new angle" : "turn", true);
						side.roomTurns = RoomOrShared(*pick) ? 1 : 0;
					} else {
						g.current.reset();
						g.plainTurn = a_frame.npcTurn;
						return std::nullopt;
					}
				}
			} else if (const float limit = CloseUpLimit();
					   limit > 0.0f && a_frame.npcTurn && g.currentNpcSide && alreadyClose && !g.beatActive && !g.extremeLineOver &&
					   ((lineCutNow && heldNow() >= limit) || heldNow() >= limit + 6.0f)) {
				// 0.9.23: one close-up of them held this long (a run of emotional lines held one
				// for 28 s): a wider shot of them, at a line's start (mid-line 6 s later at most).
				// No emotion close-up for half the limit after it.
				std::optional<ShotType> wider;
				const bool              threeQuarterFirst = std::uniform_int_distribution<int>{ 0, 1 }(g.rng) == 0;
				const std::array        order{ threeQuarterFirst ? ShotType::kThreeQuarterNpc : ShotType::kMediumNpc,
                    threeQuarterFirst ? ShotType::kMediumNpc : ShotType::kThreeQuarterNpc };
				for (const auto type : order) {
					if (Usable(type, a_frame)) {
						wider = type;
						break;
					}
				}
				for (const auto type : order) {
					if (!wider && Placeable(type, a_frame)) {
						wider = type;
					}
				}
				if (wider) {
					Cut(*wider, a_frame, a_now, "close-up limit, wider", false);
					g.closeUpCooldown = a_now + std::chrono::duration_cast<clock::duration>(std::chrono::duration<float>(limit * 0.5f));
					g.intensePending = false;
				} else {
					g.cutAt = a_now;  // nothing wider fits: try again after another stretch
				}
			} else if (g.intensePending && a_frame.npcTurn && !alreadyClose && heldNow() >= 0.7f && a_frame.lineDecided) {
				// An intense line in the middle of their turn: push in (not within 0.7 s of the
				// last cut, so a turn cut and this one never come back to back). 0.9.25: once the
				// line's reaction is decided (the 0.9.24 test: three one-frame close-ups, each
				// taken over by a reaction on the next frame).
				if (const auto shot = IntenseShot(a_frame)) {
					Cut(*shot, a_frame, a_now, "emotion close-up", true);
					g.intenseSetup = IntenseSetup(*shot);
					NoteEmotionCut(*shot);
					g.intenseShot = shot;
				}
				g.intensePending = false;
			} else if (g.extremeLineOver && a_frame.npcTurn && g.current && *g.current == ShotType::kExtremeClose && a_frame.lineDecided) {
				// Their next line after an extreme close-up (0.8.9): a regular close-up if it's an
				// emotion line too, otherwise back to their usual angle (or a new one). Not before
				// fShortestHold on screen. (0.9.23: once the line's reaction is decided.)
				if (heldNow() >= g.shortestHold) {
					std::optional<ShotType> shot;
					const char*             why = "after the extreme close-up";
					if (g.intensePending && Placeable(ShotType::kCloseUp, a_frame)) {
						shot = ShotType::kCloseUp;
						why = "emotion close-up, after the extreme close-up";
					} else if (side.shot && *side.shot != ShotType::kCloseUp && *side.shot != ShotType::kExtremeClose && Usable(*side.shot, a_frame)) {
						shot = side.shot;
					} else {
						shot = Pick(a_frame);
					}
					if (shot && shot != g.current) {
						const bool intense = *shot == ShotType::kCloseUp && g.intensePending;
						Cut(*shot, a_frame, a_now, why, !intense && shot != side.shot);
						if (intense) {
							g.intenseSetup = IntenseSetup(*shot);
							NoteEmotionCut(*shot);
							g.intenseShot = shot;
						}
					}
					g.extremeLineOver = false;
					g.intensePending = false;
				}
			} else if (g.intensePending && alreadyClose) {
				// Already on a close-up of them. In CCC SF's presets, a still one starts the
				// emotion push-in from here (0.9.23: Starborn's regular close-ups are held still,
				// the push-ins kept for the emotion lines).
				if (!CccPresetNow() && *g.current == ShotType::kCloseUp && !g.intenseSetup &&
					g.setups[static_cast<std::size_t>(ShotType::kCloseUp)].move == Move::kLocked) {
					g.intenseSetup = IntenseSetup(ShotType::kCloseUp);
					g.moveFrom = a_now;
					NoteEmotionCut(ShotType::kCloseUp);
				}
				g.intensePending = false;
			} else if (side.changeDue && lineCutNow && heldNow() >= g.shortestHold) {
				if (const auto pick = Pick(a_frame); pick && pick != g.current) {
					Cut(*pick, a_frame, a_now, "new angle", true);
				} else {
					side.changeDue = false;
					side.lines = 0;
				}
			} else if (!Usable(*g.current, a_frame, false) &&
					   !((*g.current == ShotType::kCloseUp || *g.current == ShotType::kExtremeClose || *g.current == ShotType::kClosePlayer ||
						   *g.current == ShotType::kExtremeClosePlayer) &&
						 Placeable(*g.current, a_frame, false)) &&
					   // A shot the preset doesn't list (the opening three-quarter in Close and
					   // Starborn) still fits while it can be placed: 0.9.3's were replaced one
					   // frame in, a second cut at the start (0.9.4).
					   !(g.setups[static_cast<std::size_t>(*g.current)].weight <= 0 && Placeable(*g.current, a_frame, false))) {
				if (const auto pick = Pick(a_frame)) {
					Cut(*pick, a_frame, a_now, "shot no longer fits", true);
				} else {
					g.current.reset();
					g.plainTurn = a_frame.npcTurn;
					return std::nullopt;
				}
			}
		}

		const auto& setup = g.intenseSetup ? *g.intenseSetup : g.setups[static_cast<std::size_t>(*g.current)];
		const float progress = setup.seconds > 0.0f ? std::chrono::duration<float>(a_now - g.moveFrom).count() / setup.seconds : 1.0f;
		auto        in = InputFor(a_frame, setup, progress);
		in.heldAngle = g.heldAngle;
		in.heldLookSign = g.heldLookSign;
		in.heldFacePitch = g.heldFacePitch;
		const auto solved = Shots::Solve(*g.current, in);
		if (!solved.ok) {
			return std::nullopt;
		}
		auto pose = solved.pose;
		Camera::PullIn(pose, SubjectHead(*g.current, a_frame), g.limit);
		return pose;
	}

	std::optional<Camera::Pose> UpdatePlain(const Frame& a_frame, clock::time_point a_now)
	{
		g.lastFrame = a_frame;
		SwitchPresetNow(a_frame);
		bool        started = a_frame.npcTalking && !gp.wasTalking;
		// A new line that follows the last without a pause (0.8.3).
		if (!started && a_frame.npcTalking && a_frame.npcLineSerial != gp.serial && a_now - gp.lineAt > std::chrono::milliseconds(500)) {
			started = true;
		}
		if (started) {
			gp.lineAt = a_now;
		}
		gp.serial = a_frame.npcLineSerial;
		gp.wasTalking = a_frame.npcTalking;
		// Start a close-up over the plain shot: solve it once, hold its angle.
		// The opening single moves as the preset moves that angle (locked if the preset doesn't
		// use it); the close-ups push in as emotion close-ups do.
		const auto plainSetup = [&](ShotType a_shot) {
			if (!gp.opening && !(a_frame.openingSingle && a_frame.npcTurn && !gp.shot)) {
				return IntenseSetup(a_shot);
			}
			auto setup = g.setups[static_cast<std::size_t>(a_shot)];
			if (setup.weight <= 0) {
				setup.move = Move::kLocked;
				setup.amount = 0;
				setup.seconds = 4.2f;
			}
			return setup;
		};
		auto begin = [&](ShotType a_shot, const char* a_why, bool a_npcEmotion) {
			const auto setup = plainSetup(a_shot);
			const auto solved = Shots::Solve(a_shot, InputFor(a_frame, setup, 0.0f));
			if (!solved.ok) {
				return;
			}
			const auto clear = ClearanceFor(a_shot, a_frame, setup);
			g_shotLens.store(setup.lens, std::memory_order_relaxed);  // drawn through its own lens too (0.9.1)
			gp.shot = a_shot;
			gp.cutAt = a_now;
			++g.cuts;
			g.why = a_why;
			gp.heldAngle = solved.angle;
			gp.heldLookSign = solved.lookSign;
			gp.heldFacePitch = solved.faced ? solved.facePitch : std::numeric_limits<float>::quiet_NaN();
			if (solved.faced && (std::abs(solved.faceYaw) >= 3.0f || std::abs(solved.facePitch) >= 3.0f)) {
				REX::INFO("Face: {} turned {:+.0f} degrees toward where the face points, camera {} {:.0f} degrees", Shots::Key(a_shot), solved.faceYaw,
					solved.facePitch < 0.0f ? "lowered" : "raised", std::abs(solved.facePitch));
			}
			gp.limit = clear.usable ? clear.limit : std::numeric_limits<float>::infinity();
			if (a_npcEmotion) {
				NoteEmotionCut(a_shot);
			}
			REX::INFO("Director: {} -> {} over the plain shot ({:.1f} m away at {:.1f} degrees){}", a_why, Shots::Key(a_shot), solved.distance, solved.angle,
				std::isfinite(gp.limit) ? std::format("; obstacle {:.1f} m from them, camera pulled in to {:.1f} m", clear.blockedAt, gp.limit) : std::string{});
		};
		// Your emotion close-up, on a reaction (0.7.11: reactions with seated people and
		// merchants too, shown on the plain shot of you otherwise).
		// Your tagged choice's close-up too (0.8.17: the 0.8.16 test's persuasion was with a seated
		// recruit, where it couldn't show: you don't speak, so it waited for an NPC line).
		// From first person, while your body is built: their first shot is a single of them
		// with you out of the picture (the plain shots have you in them), held for their turn
		// (0.9.2).
		if (gp.opening && !a_frame.npcTurn) {
			gp.opening = false;
			gp.shot.reset();
		}
		if (a_frame.openingSingle && a_frame.npcTurn && !gp.opening && !gp.shot) {
			if (const auto shot = CloseOpening(a_frame)) {
				begin(*shot, "opening, you out of the picture", false);
				gp.opening = gp.shot.has_value();
			} else {
				for (const auto type : { ShotType::kThreeQuarterNpc, ShotType::kMediumNpc, ShotType::kCloseUp }) {
					if (Placeable(type, a_frame)) {
						begin(type, "opening, you out of the picture", false);
						gp.opening = gp.shot.has_value();
						break;
					}
				}
			}
			LogSkipped();
		}
		if (!gp.opening) {
			const bool playerClose = a_frame.playerCloseUp && !a_frame.npcTurn && (a_frame.npcTalking || a_frame.playerChoice);
			if (playerClose) {
				if (!gp.playerClose) {
					gp.playerClose = true;
					gp.shot.reset();
					if (const auto shot = PlayerCloseShot(a_frame)) {
						begin(*shot, a_frame.playerChoice ? "your tagged choice" : "reaction: your expression", false);
					}
					LogSkipped();
				}
			} else {
				if (gp.playerClose) {
					gp.playerClose = false;
					gp.shot.reset();
				}
				// A persuasion's answer gets its close-up whatever the preset's emotion close-ups (0.8.17:
				// in Standard and Room, which have none, seated people never got one).
				if (!a_frame.npcTalking || !a_frame.npcTurn || (EmotionCloseUps() == 0 && !a_frame.persuasionBeat)) {
					if (gp.shot) {
						REX::INFO("Director: emotion close-up over; back to the plain shot");
					}
					gp.shot.reset();
					return std::nullopt;
				}
				if (started) {
					gp.shot.reset();
					gp.intenseShown = false;
					if (a_frame.intenseLine || a_frame.persuasionBeat) {
						if (const auto shot = a_frame.persuasionBeat && Placeable(ShotType::kCloseUp, a_frame) ? std::optional<ShotType>{ ShotType::kCloseUp } : IntenseShot(a_frame)) {
							begin(*shot, a_frame.persuasionBeat ? "persuasion beat" : "emotion close-up", true);
							gp.intenseShown = a_frame.intenseLine;
						}
						LogSkipped();
					}
				} else if (!gp.shot && a_frame.persuasionBeat) {
					// The beat noticed a frame after the line started (0.8.17).
					if (Placeable(ShotType::kCloseUp, a_frame)) {
						begin(ShotType::kCloseUp, "persuasion beat", true);
					}
					LogSkipped();
				} else if (!gp.shot && gp.intenseShown && a_frame.intenseLine) {
					// Back from a reaction that interrupted the emotion close-up (0.8.3).
					if (const auto shot = IntenseShot(a_frame)) {
						begin(*shot, "emotion close-up, back from the reaction", false);
					}
					gp.intenseShown = false;
					LogSkipped();
				}
			}
		}
		if (!gp.shot) {
			return std::nullopt;
		}
		const auto  setup = plainSetup(*gp.shot);
		const float progress = std::chrono::duration<float>(a_now - gp.cutAt).count() / setup.seconds;
		auto        in = InputFor(a_frame, setup, progress);
		in.heldAngle = gp.heldAngle;
		in.heldLookSign = gp.heldLookSign;
		in.heldFacePitch = gp.heldFacePitch;
		const auto solved = Shots::Solve(*gp.shot, in);
		if (!solved.ok) {
			return std::nullopt;
		}
		auto pose = solved.pose;
		Camera::PullIn(pose, SubjectHead(*gp.shot, a_frame), gp.limit);
		return pose;
	}

	namespace
	{
		// fConversationLens, or the preset's one lens (CCC's presets 45 degrees, CCC SF's 37.5,
		// 0.8.3); below 10 = the game's own.
		float PresetLens()
		{
			const auto& s = Settings::Get();
			if (s.conversationLens >= 10.0f) {
				return s.conversationLens;  // your own
			}
			if (s.conversationLens >= 0.0f) {
				return 0.0f;  // the game's
			}
			std::string preset{ s.preset };
			std::ranges::transform(preset, preset.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
			return preset == "frontier" || preset == "starborn" || preset == "constellation" || preset == "nebula" ? 37.5f : 45.0f;
		}
	}

	std::optional<Camera::Pose> ClosingShot(bool a_plainOnScreen, const char*& a_name, float& a_distance)
	{
		// The angles on the NPC that leave you out of the picture.
		const auto alone = [](ShotType a_type) {
			switch (a_type) {
			case ShotType::kCloseUp:
			case ShotType::kMediumNpc:
			case ShotType::kLowAngle:
			case ShotType::kCloseProfile:
			case ShotType::kCloseLow:
			case ShotType::kCloseHigh:
			case ShotType::kExtremeClose:
			case ShotType::kThreeQuarterNpc:
			case ShotType::kMediumProfile:
			case ShotType::kLowProfile:
				return true;
			default:
				return false;
			}
		};
		if (!g.lastFrame || (!a_plainOnScreen && g.current && alone(*g.current))) {
			return std::nullopt;
		}
		Frame frame = *g.lastFrame;
		frame.npcTurn = true;
		frame.reaction = false;
		for (const auto type : { ShotType::kMediumNpc, ShotType::kCloseUp, ShotType::kThreeQuarterNpc }) {
			if (!Placeable(type, frame)) {
				continue;
			}
			const auto& setup = g.setups[static_cast<std::size_t>(type)];
			auto        in = InputFor(frame, setup, 0.0f);
			in.move = Move::kLocked;
			const auto solved = Shots::Solve(type, in);
			if (!solved.ok) {
				continue;
			}
			auto pose = solved.pose;
			Camera::PullIn(pose, frame.npcHead, ClearanceFor(type, frame, setup).limit);
			g.current = type;
			g_shotLens.store(setup.lens, std::memory_order_relaxed);
			a_name = Shots::Key(type);
			const float dx = pose.position.x - frame.npcHead.x, dy = pose.position.y - frame.npcHead.y, dz = pose.position.z - frame.npcHead.z;
			a_distance = std::sqrt(dx * dx + dy * dy + dz * dz);
			return pose;
		}
		return std::nullopt;
	}

	float ShortestHold()
	{
		return g.shortestHold;  // set once per conversation, at its start
	}

	float WantedLens()
	{
		// The shot's own (0.8.12, CCC's lens per angle), unless your own lens is set or the
		// lens key asked for one lens.
		// (Worked out each time from CCC's number, so the lens key takes effect at once: 0.8.12
		// kept the lens the last cut had worked out until the next cut.)
		if (const float shot = OwnLens(g_shotLens.load(std::memory_order_relaxed)); shot >= 10.0f) {
			return shot;
		}
		return PresetLens();
	}

	namespace
	{
		bool CccPreset()
		{
			std::string preset{ Settings::Get().preset };
			std::ranges::transform(preset, preset.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
			return preset.empty() || preset == "standard" || preset == "close" || preset == "room";
		}

		bool ClosePreset()
		{
			std::string preset{ Settings::Get().preset };
			std::ranges::transform(preset, preset.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
			return preset == "close";
		}
	}

	int EmotionCloseUps()
	{
		const int mode = Settings::Get().emotionCloseUps;
		if (mode >= 0) {
			return mode;  // yours: 0 none, 1 every tier
		}
		// The preset's own (0.8.13): CCC pushes in for emotion only on lines marked full
		// intensity, and only Close has the angle for it; CCC SF's presets use every tier.
		if (CccPreset()) {
			return ClosePreset() ? 3 : 0;
		}
		// Constellation, the documentary, observes from a distance and cuts in tight only for
		// the strong lines (0.8.15: emotion close-ups and reactions took a quarter of it).
		return PresetIs("constellation") ? 3 : 1;
	}

	int ExtremeCloseUpChanceNpc()
	{
		const int mine = Settings::Get().extremeCloseUpChance;
		// The preset's own: Frontier 25 (the 0.9.24 test: four of nine in the Andreja and
		// Barrett conversations looked too many for the western), the others 40.
		return mine >= 0 ? mine : PresetIs("frontier") ? 25 : 40;
	}

	float ReactionAfterSeconds()
	{
		const float mine = Settings::Get().reactionAfterSeconds;
		if (mine > 0.0f) {
			return mine;
		}
		// The preset's own (0.8.15; the 0.8.14 test had reactions at 16-36% of the time in CCC
		// SF's presets): Frontier's relaxed pace 10 s, Starborn's long holds 9 s,
		// Constellation's observer 12 s, Nebula 7 s.
		return PresetIs("frontier") ? 10.0f : PresetIs("starborn") ? 9.0f : PresetIs("constellation") ? 12.0f : 7.0f;
	}

	int CameraSway()
	{
		const int mine = Settings::Get().cameraSway;
		// The preset's own (0.8.14; 0.8.17 after the sway test): CCC's presets none, as CCC
		// (a tripod); Frontier a touch of handheld, Starborn locked off (until 0.9.23), Constellation the
		// documentary operator's, Nebula 40. 0.9.23: Starborn 30, a touch of handheld (less
		// Kubrick, more Interstellar).
		// 0.9.24: Constellation 70 -> 60 (the user). 0.9.25: Starborn 30 -> 15 (the 0.9.24
		// test: overshot). 0.9.27: Constellation 60 -> 50 (with the wander a touch strong at
		// times), Frontier 50 -> 45 (the user).
		return mine >= 0 ? mine : PresetIs("frontier") ? 45 : PresetIs("constellation") ? 50 : PresetIs("nebula") ? 40 : PresetIs("starborn") ? 15 : 0;
	}

	float Handheld()
	{
		// The handheld wander (0.8.15 testing; 0.8.17 after the sway test): Constellation's
		// operator only, at half the strength tested. Off with iCameraSway=0 (a tripod).
		if (Settings::Get().cameraSway == 0) {
			return 0.0f;
		}
		return PresetIs("constellation") ? 0.5f : 0.0f;
	}

	bool ChoiceCloseUps()
	{
		const int mode = Settings::Get().choiceCloseUps;
		return mode >= 0 ? mode == 1 : !CccPreset();
	}

	bool ReactionShots()
	{
		const int mode = Settings::Get().reactionShots;
		return mode >= 0 ? mode == 1 : !CccPreset();  // the preset's own: CCC's off (CCC's default), CCC SF's on
	}

	void UsePlainShotLens(Shots::ShotType a_like)
	{
		g_shotLens.store(g.setups[static_cast<std::size_t>(a_like)].lens, std::memory_order_relaxed);
	}

	float LensFor(Shots::ShotType a_type)
	{
		const float own = OwnLens(g.setups[static_cast<std::size_t>(a_type)].lens);
		return own > 0.0f ? own : Lens();
	}

	float RoomShotLimit(bool a_interior)
	{
		const auto& s = Settings::Get();
		const float own = a_interior ? s.roomShotMaxIndoors : s.roomShotMaxOutdoors;
		return own >= 0.0f ? own : a_interior ? g.roomIndoor : g.roomOutdoor;
	}

	float LensDegrees()
	{
		return Lens();
	}

	int LastEmotionCut(clock::time_point& a_when)
	{
		a_when = clock::time_point{ clock::duration{ g_emotionCutAt.load(std::memory_order_acquire) } };
		return g_emotionCutKind.load(std::memory_order_relaxed);
	}

	const char* Current()
	{
		if (gp.shot) {
			return Shots::Key(*gp.shot);  // an emotion close-up over the plain shot
		}
		return g.current ? Shots::Key(*g.current) : "none";
	}

	std::uint32_t Cuts()
	{
		return g.cuts;
	}

	const char* Why()
	{
		return g.why;
	}

	bool PulledIn()
	{
		return gp.shot ? std::isfinite(gp.limit) : std::isfinite(g.limit);
	}
}
