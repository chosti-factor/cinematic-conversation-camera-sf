#pragma once

#include "Shots.h"

// Chooses which of the shot library's setups to show and when to change it, following
// Cinematic Conversation Camera's rules: shot/reverse-shot on the speaker, a new angle
// every few lines, short lines don't count, a shortest hold, per-preset weights.
namespace CCC::Director
{
	struct Frame
	{
		bool         npcTurn{ true };      // whose shot it is (from the plugin's speaker logic)
		bool         npcTalking{ false };
		bool         playerTalking{ false };
		bool         nearSeated{ false };  // the person nearest the camera sits lower: no over-the-shoulders
		bool         intenseLine{ false }; // the speaker's current line carries an expression from sCloseUpEmotions
		RE::NiPoint3 npcHead{};
		RE::NiPoint3 playerHead{};
	};

	// Call when a conversation starts (reads the INI's preset and per-shot settings).
	void Reset();

	// The camera for this frame, or nullopt when no setup fits (the caller falls back to
	// its plain over-the-shoulder shot).
	[[nodiscard]] std::optional<Camera::Pose> Update(const Frame& a_frame, std::chrono::steady_clock::time_point a_now);

	// With the plain shots (seated people, merchants): the emotion close-up for an intense
	// line, held until the line ends; nullopt = keep the plain shot.
	[[nodiscard]] std::optional<Camera::Pose> UpdatePlain(const Frame& a_frame, std::chrono::steady_clock::time_point a_now);

	// The last emotion close-up the camera cut to: 1 = close-up, 2 = extreme close-up (0 =
	// none yet), and when (for the on-screen messages).
	[[nodiscard]] int LastEmotionCut(std::chrono::steady_clock::time_point& a_when);

	// The setup on screen, for the log.
	[[nodiscard]] const char* Current();
}
