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

		struct State
		{
			std::array<Setup, kCount> setups{};
			std::optional<ShotType>   current;
			bool                      currentNpcSide{ true };
			clock::time_point         cutAt{};
			int                       lines{ 0 };
			int                       linesTarget{ 3 };
			bool                      changeDue{ false };
			Line                      npcLine;
			Line                      playerLine;
			std::mt19937              rng{ std::random_device{}() };
			bool                      loggedEmpty{ false };
		} g;

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
			in.maxDistance = s.maxCameraDistance;
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
			return solved.ok && solved.wantedDistance <= Settings::Get().maxCameraDistance * 2.5f;
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

		// Counts a line when it ends, unless it was short. Returns true on the frame a
		// line starts.
		bool TrackLine(Line& a_line, bool a_talking, bool a_countsForCurrentSide, clock::time_point a_now)
		{
			bool started = false;
			if (a_talking && !a_line.talking) {
				a_line.start = a_now;
				started = true;
			} else if (!a_talking && a_line.talking && a_countsForCurrentSide) {
				const float seconds = std::chrono::duration<float>(a_now - a_line.start).count();
				if (seconds >= Settings::Get().shortLineSeconds) {
					if (++g.lines >= g.linesTarget) {
						g.changeDue = true;
					}
				}
			}
			a_line.talking = a_talking;
			return started;
		}

		void Cut(ShotType a_type, bool a_npcSide, clock::time_point a_now, const char* a_why)
		{
			g.current = a_type;
			g.currentNpcSide = a_npcSide;
			g.cutAt = a_now;
			g.lines = 0;
			g.linesTarget = NewLineTarget();
			g.changeDue = false;
			const auto& setup = g.setups[static_cast<std::size_t>(a_type)];
			REX::INFO("Director: {} -> {} ({}; move {} {}, new angle after {} lines)", a_why, Shots::Key(a_type),
				a_npcSide ? "their turn" : "your turn", Shots::MoveName(Settings::Get().cameraMoves ? setup.move : Move::kLocked),
				setup.amount, g.linesTarget);
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
		g.lines = 0;
		g.linesTarget = NewLineTarget();
		g.changeDue = false;
		g.npcLine = {};
		g.playerLine = {};
		g.loggedEmpty = false;
		REX::INFO("Director: preset {}, camera at most {:.1f} m from the subject, new angle every {}-{} lines",
			preset, s.maxCameraDistance, s.minLines, s.maxLines);
	}

	std::optional<Camera::Pose> Update(const Frame& a_frame, clock::time_point a_now)
	{
		const auto& s = Settings::Get();
		const bool  npcStarted = TrackLine(g.npcLine, a_frame.npcTalking, g.currentNpcSide, a_now);
		const bool  playerStarted = TrackLine(g.playerLine, a_frame.playerTalking, !g.currentNpcSide, a_now);
		const bool  lineStarted = a_frame.npcTurn ? npcStarted : playerStarted;
		const float held = std::chrono::duration<float>(a_now - g.cutAt).count();

		if (!g.current || a_frame.npcTurn != g.currentNpcSide) {
			// The reverse: the other person's turn gets one of their setups.
			g.current.reset();
			if (const auto pick = Pick(a_frame)) {
				Cut(*pick, a_frame.npcTurn, a_now, "turn");
			} else {
				return std::nullopt;
			}
		} else if (g.changeDue && lineStarted && held >= s.shortestHold) {
			if (const auto pick = Pick(a_frame); pick && pick != g.current) {
				Cut(*pick, a_frame.npcTurn, a_now, "new angle");
			} else {
				g.changeDue = false;
			}
		} else if (!Usable(*g.current, a_frame)) {
			if (const auto pick = Pick(a_frame)) {
				Cut(*pick, a_frame.npcTurn, a_now, "shot no longer fits");
			} else {
				return std::nullopt;
			}
		}

		const auto& setup = g.setups[static_cast<std::size_t>(*g.current)];
		const float progress = setup.seconds > 0.0f ? std::chrono::duration<float>(a_now - g.cutAt).count() / setup.seconds : 1.0f;
		const auto  solved = Shots::Solve(*g.current, InputFor(a_frame, setup, progress));
		if (!solved.ok) {
			return std::nullopt;
		}
		return solved.pose;
	}

	const char* Current()
	{
		return g.current ? Shots::Key(*g.current) : "none";
	}
}
