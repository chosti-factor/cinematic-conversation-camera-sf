#include "Director.h"

#include "Settings.h"

#include <random>

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
		constexpr float kLens = 29.58f;

		struct Setup
		{
			int   weight{ 0 };  // 0 = not used
			Move  move{ Move::kLocked };
			int   amount{ 0 };   // 0-100
			float seconds{ 4.2f };
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
			float                     maxDistance{ 3.0f };
			Line                      npcLine;
			Line                      playerLine;
			std::mt19937              rng{ std::random_device{}() };
			bool                      loggedEmpty{ false };
			bool                      intensePending{ false };  // an intense line started; close-up owed
			std::optional<Setup>      intenseSetup;             // the move used by an emotion close-up
		} g;

		// Emotion close-ups with the plain shots (seated people, merchants): held for the line.
		struct PlainIntense
		{
			std::optional<ShotType> shot;
			clock::time_point       cutAt{};
			float                   heldAngle{ std::numeric_limits<float>::quiet_NaN() };
			float                   heldLookSign{ 0.0f };
			bool                    wasTalking{ false };
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
			return a_type == ShotType::kExtremeClose ? Setup{ 1, Move::kZoomIn, 40, 9.0f } : Setup{ 1, Move::kPushIn, 40, 8.0f };
		}

		struct PresetEntry
		{
			ShotType type;
			int      weight;
			Move     move;
			int      amount;
			float    seconds;
		};

		// The three built-in looks (CCC's Presets.cpp). Zoom moves stay listed as zooms;
		// the solver performs them as push-ins and pull-outs.
		constexpr std::array kStandard{
			PresetEntry{ ShotType::kOverPlayerShoulder, 100, Move::kLocked, 0, 4.2f },
			PresetEntry{ ShotType::kOverNpcShoulder, 100, Move::kLocked, 0, 4.2f },
			PresetEntry{ ShotType::kCloseUp, 80, Move::kPushIn, 14, 5.0f },
			PresetEntry{ ShotType::kClosePlayer, 80, Move::kPushIn, 14, 5.0f },
			PresetEntry{ ShotType::kThreeQuarterNpc, 60, Move::kLocked, 0, 4.2f },
			PresetEntry{ ShotType::kThreeQuarterPlayer, 60, Move::kLocked, 0, 4.2f },
			PresetEntry{ ShotType::kMediumNpc, 45, Move::kLocked, 0, 4.2f },
			PresetEntry{ ShotType::kMediumPlayer, 45, Move::kLocked, 0, 4.2f },
			PresetEntry{ ShotType::kTwoShot, 25, Move::kDrift, 10, 7.0f },
			PresetEntry{ ShotType::kProfile, 15, Move::kLocked, 0, 4.2f },
		};
		// CCC offers the extreme close-ups only on lines the writers marked as intense,
		// which this plugin can't read yet, so their weight is cut from 75 to 12.
		constexpr std::array kClose{
			PresetEntry{ ShotType::kOverPlayerShoulder, 34, Move::kLocked, 0, 4.0f },
			PresetEntry{ ShotType::kCloseUp, 36, Move::kPushIn, 40, 8.0f },
			PresetEntry{ ShotType::kExtremeClose, 12, Move::kZoomIn, 40, 9.0f },
			PresetEntry{ ShotType::kClosePlayer, 30, Move::kZoomOut, 20, 5.0f },
			PresetEntry{ ShotType::kExtremeClosePlayer, 12, Move::kPushIn, 75, 7.0f },
			PresetEntry{ ShotType::kDistant, 10, Move::kPullOut, 55, 8.0f },
		};
		constexpr std::array kRoom{
			PresetEntry{ ShotType::kMaster, 85, Move::kCraneUp, 40, 9.0f },
			PresetEntry{ ShotType::kWide, 80, Move::kPullOut, 16, 8.0f },
			PresetEntry{ ShotType::kLongNpc, 70, Move::kCraneUp, 25, 7.0f },
			PresetEntry{ ShotType::kLongPlayer, 70, Move::kCraneUp, 25, 7.0f },
			PresetEntry{ ShotType::kDistant, 55, Move::kLocked, 0, 4.2f },
			PresetEntry{ ShotType::kTwoShot, 45, Move::kDrift, 12, 8.0f },
			PresetEntry{ ShotType::kOverhead, 40, Move::kCraneDown, 22, 4.2f },
			PresetEntry{ ShotType::kPlayerOverhead, 40, Move::kCraneDown, 22, 4.2f },
			PresetEntry{ ShotType::kThreeQuarterNpc, 40, Move::kDrift, 22, 4.2f },
			PresetEntry{ ShotType::kThreeQuarterPlayer, 40, Move::kDrift, 22, 4.2f },
			PresetEntry{ ShotType::kMediumNpc, 35, Move::kDrift, 18, 4.2f },
			PresetEntry{ ShotType::kMediumPlayer, 35, Move::kDrift, 18, 4.2f },
		};

		template <std::size_t N>
		void LoadPreset(const std::array<PresetEntry, N>& a_entries)
		{
			for (auto& setup : g.setups) {
				setup = Setup{};
			}
			for (const auto& e : a_entries) {
				g.setups[static_cast<std::size_t>(e.type)] = Setup{ e.weight, e.move, e.amount, e.seconds };
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
			const int   lo = std::clamp(s.minLines, 1, 20);
			const int   hi = std::clamp((std::max)(s.maxLines, lo), 1, 20);
			return std::uniform_int_distribution<int>{ lo, hi }(g.rng);
		}

		Shots::Input InputFor(const Frame& a_frame, const Setup& a_setup, float a_progress)
		{
			const auto&  s = Settings::Get();
			Shots::Input in;
			in.npcHead = a_frame.npcHead;
			in.playerHead = a_frame.playerHead;
			in.lensDegrees = kLens;
			in.maxDistance = g.maxDistance;
			in.true180 = s.true180;
			in.progress = a_progress;
			in.move = s.cameraMoves ? a_setup.move : Move::kLocked;
			in.moveAmount = a_setup.amount;
			return in;
		}

		bool Usable(ShotType a_type, const Frame& a_frame)
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
			const auto solved = Shots::Solve(a_type, InputFor(a_frame, setup, 0.0f));
			// A framing that wants the camera far beyond the cap would become a different
			// shot entirely (a full figure turned into a close-up); leave it out.
			return solved.ok && solved.wantedDistance <= g.maxDistance * 2.5f;
		}

		// Like Usable, but regardless of the preset's weight: the emotion close-ups use the
		// close-up and extreme close-up even in presets that leave them out (CCC does the same).
		bool Placeable(ShotType a_type, const Frame& a_frame)
		{
			if (!Shots::Neutral(a_type) && Shots::OnNpc(a_type) != a_frame.npcTurn) {
				return false;
			}
			const auto solved = Shots::Solve(a_type, InputFor(a_frame, g.setups[static_cast<std::size_t>(a_type)], 0.0f));
			return solved.ok && solved.wantedDistance <= g.maxDistance * 2.5f;
		}

		// The close-up for an intense line: a third of the time the extreme close-up.
		std::optional<ShotType> IntenseShot(const Frame& a_frame)
		{
			const bool extreme = std::uniform_int_distribution<int>{ 1, 100 }(g.rng) <= Settings::Get().extremeCloseUpChance;
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

		std::optional<ShotType> Pick(const Frame& a_frame)
		{
			std::vector<std::pair<ShotType, int>> pool;
			int                                   total = 0;
			for (std::size_t i = 0; i < kCount; ++i) {
				const auto type = static_cast<ShotType>(i);
				if (g.current && type == *g.current) {
					continue;  // a change of angle should change the angle
				}
				if (Usable(type, a_frame)) {
					const int weight = g.setups[i].weight;
					pool.emplace_back(type, weight);
					total += weight;
				}
			}
			if (pool.empty() && g.current && Usable(*g.current, a_frame)) {
				return g.current;
			}
			if (pool.empty()) {
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
		bool TrackLine(Line& a_line, Side& a_side, bool a_talking, clock::time_point a_now)
		{
			bool started = false;
			if (a_talking && !a_line.talking) {
				a_line.start = a_now;
				started = true;
			} else if (!a_talking && a_line.talking) {
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
			g.current = a_type;
			g.currentNpcSide = a_frame.npcTurn;
			g.cutAt = a_now;
			if (a_newAngle) {
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
			REX::INFO("Director: {} -> {} ({}; {:.1f} m away at {:.1f} degrees; move {} {}; new angle after {} more lines)", a_why, Shots::Key(a_type),
				a_frame.npcTurn ? "their turn" : "your turn", solved.distance, solved.angle,
				Shots::MoveName(Settings::Get().cameraMoves ? setup.move : Move::kLocked), setup.amount, side.linesTarget - side.lines);
		}
	}

	void Reset()
	{
		const auto& s = Settings::Get();
		std::string preset{ s.preset };
		std::ranges::transform(preset, preset.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		if (preset == "close") {
			LoadPreset(kClose);
		} else if (preset == "room") {
			LoadPreset(kRoom);
		} else {
			if (preset != "standard") {
				REX::WARN("Director: unknown preset '{}', using Standard", s.preset);
			}
			LoadPreset(kStandard);
			preset = "standard";
		}
		ApplyOverrides();
		g.current.reset();
		g.npc = Side{};
		g.player = Side{};
		g.npc.linesTarget = NewLineTarget();
		g.player.linesTarget = NewLineTarget();
		g.heldAngle = std::numeric_limits<float>::quiet_NaN();
		g.heldLookSign = 0.0f;
		auto*      player = RE::PlayerCharacter::GetSingleton();
		const bool interior = player && player->parentCell && player->parentCell->IsInterior();
		g.maxDistance = interior ? s.maxCameraDistance : s.maxCameraDistanceExterior;
		g.npcLine = {};
		g.playerLine = {};
		g.loggedEmpty = false;
		g.intensePending = false;
		g.intenseSetup.reset();
		gp = PlainIntense{};
		REX::INFO("Director: preset {}, {}: camera at most {:.1f} m from the subject; cutting style {} ({}% fresh angle per turn, new angle every {}-{} lines, hold at least {:.1f} s)",
			preset, interior ? "interior" : "exterior", g.maxDistance, s.cuttingStyle, s.freshAngleChance, s.minLines, s.maxLines, s.shortestHold);
		REX::INFO("Test settings in effect: iBlurExperiment={}, sTestModifier={:08X} (mode {}), fNoBlurBeyond={:.1f}, bEmotionCloseUps={}, iExtremeCloseUpChance={}, sCloseUpEmotions={}",
			s.blurExperiment, s.testModifier, s.testModifierMode, s.noBlurBeyond, s.emotionCloseUps ? 1 : 0, s.extremeCloseUpChance, s.closeUpEmotions);
	}

	std::optional<Camera::Pose> Update(const Frame& a_frame, clock::time_point a_now)
	{
		const auto& s = Settings::Get();
		const bool  npcStarted = TrackLine(g.npcLine, g.npc, a_frame.npcTalking, a_now);
		const bool  playerStarted = TrackLine(g.playerLine, g.player, a_frame.playerTalking, a_now);
		const bool  lineStarted = a_frame.npcTurn ? npcStarted : playerStarted;
		const float held = std::chrono::duration<float>(a_now - g.cutAt).count();
		auto&       side = a_frame.npcTurn ? g.npc : g.player;

		// Emotion close-ups: a line tagged with one of the chosen expressions starts.
		if (npcStarted) {
			g.intensePending = a_frame.intenseLine && s.emotionCloseUps;
		}
		if (!a_frame.npcTalking) {
			g.intensePending = false;
		}
		const bool alreadyClose = g.current && (*g.current == ShotType::kCloseUp || *g.current == ShotType::kExtremeClose);

		if (g.intensePending && a_frame.npcTurn && (!g.current || a_frame.npcTurn != g.currentNpcSide)) {
			// The turn passes to them on an intense line: the turn's cut is the close-up.
			if (const auto shot = IntenseShot(a_frame)) {
				Cut(*shot, a_frame, a_now, "turn, intense line", true);
				g.intenseSetup = IntenseSetup(*shot);
				NoteEmotionCut(*shot);
				g.intensePending = false;
			}
		}

		if (!g.current || a_frame.npcTurn != g.currentNpcSide) {
			// The reverse: back to this person's own setup, unless they're due a new one.
			const bool fresh = std::uniform_int_distribution<int>{ 1, 100 }(g.rng) <= s.freshAngleChance;
			if (side.shot && !side.changeDue && !fresh && Usable(*side.shot, a_frame)) {
				Cut(*side.shot, a_frame, a_now, "turn", false);
			} else {
				g.current = side.shot;  // so Pick avoids repeating it
				if (const auto pick = Pick(a_frame)) {
					Cut(*pick, a_frame, a_now, side.shot ? "turn, new angle" : "turn", true);
				} else {
					g.current.reset();
					return std::nullopt;
				}
			}
		} else if (g.intensePending && a_frame.npcTurn && !alreadyClose && held >= 0.7f) {
			// An intense line in the middle of their turn: push in (not within 0.7 s of the
			// last cut, so a turn cut and this one never come back to back).
			if (const auto shot = IntenseShot(a_frame)) {
				Cut(*shot, a_frame, a_now, "intense line", true);
				g.intenseSetup = IntenseSetup(*shot);
				NoteEmotionCut(*shot);
			}
			g.intensePending = false;
		} else if (g.intensePending && alreadyClose) {
			g.intensePending = false;
		} else if (side.changeDue && lineStarted && held >= s.shortestHold) {
			if (const auto pick = Pick(a_frame); pick && pick != g.current) {
				Cut(*pick, a_frame, a_now, "new angle", true);
			} else {
				side.changeDue = false;
				side.lines = 0;
			}
		} else if (!Usable(*g.current, a_frame) &&
				   !((*g.current == ShotType::kCloseUp || *g.current == ShotType::kExtremeClose) && Placeable(*g.current, a_frame))) {
			if (const auto pick = Pick(a_frame)) {
				Cut(*pick, a_frame, a_now, "shot no longer fits", true);
			} else {
				return std::nullopt;
			}
		}

		const auto& setup = g.intenseSetup ? *g.intenseSetup : g.setups[static_cast<std::size_t>(*g.current)];
		const float progress = setup.seconds > 0.0f ? std::chrono::duration<float>(a_now - g.cutAt).count() / setup.seconds : 1.0f;
		auto        in = InputFor(a_frame, setup, progress);
		in.heldAngle = g.heldAngle;
		in.heldLookSign = g.heldLookSign;
		const auto solved = Shots::Solve(*g.current, in);
		if (!solved.ok) {
			return std::nullopt;
		}
		return solved.pose;
	}

	std::optional<Camera::Pose> UpdatePlain(const Frame& a_frame, clock::time_point a_now)
	{
		const auto& s = Settings::Get();
		const bool  started = a_frame.npcTalking && !gp.wasTalking;
		gp.wasTalking = a_frame.npcTalking;
		if (!a_frame.npcTalking || !a_frame.npcTurn || !s.emotionCloseUps) {
			if (gp.shot) {
				REX::INFO("Director: emotion close-up over; back to the plain shot");
			}
			gp.shot.reset();
			return std::nullopt;
		}
		if (started) {
			gp.shot.reset();
			if (a_frame.intenseLine) {
				if (const auto shot = IntenseShot(a_frame)) {
					const auto setup = IntenseSetup(*shot);
					const auto solved = Shots::Solve(*shot, InputFor(a_frame, setup, 0.0f));
					if (solved.ok) {
						gp.shot = shot;
						gp.cutAt = a_now;
						gp.heldAngle = solved.angle;
						gp.heldLookSign = solved.lookSign;
						NoteEmotionCut(*shot);
						REX::INFO("Director: intense line -> {} over the plain shot ({:.1f} m away at {:.1f} degrees)", Shots::Key(*shot), solved.distance, solved.angle);
					}
				}
			}
		}
		if (!gp.shot) {
			return std::nullopt;
		}
		const auto  setup = IntenseSetup(*gp.shot);
		const float progress = std::chrono::duration<float>(a_now - gp.cutAt).count() / setup.seconds;
		auto        in = InputFor(a_frame, setup, progress);
		in.heldAngle = gp.heldAngle;
		in.heldLookSign = gp.heldLookSign;
		const auto solved = Shots::Solve(*gp.shot, in);
		return solved.ok ? std::optional<Camera::Pose>{ solved.pose } : std::nullopt;
	}

	int LastEmotionCut(clock::time_point& a_when)
	{
		a_when = clock::time_point{ clock::duration{ g_emotionCutAt.load(std::memory_order_acquire) } };
		return g_emotionCutKind.load(std::memory_order_relaxed);
	}

	const char* Current()
	{
		return g.current ? Shots::Key(*g.current) : "none";
	}
}
