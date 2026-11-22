#pragma once

// The conversation's on-screen dressing (0.6.3, CCC's letterbox, subtitles in the bars and
// dialogue options fading while someone speaks). Starfield's menus are Scaleform (AS3)
// movies; this works inside the dialogue menu's movie: black bars are added under its own
// content, so its subtitles and options stay on top of them.
namespace CCC::Hud
{
	// Main thread, every frame. a_filming: this mod's camera is filming the conversation
	// (not the game's own camera in a quest scene). a_delta: seconds since the last call.
	// a_npcTalking: the NPC is speaking a line (the dialogue options fade out meanwhile).
	void Update(bool a_filming, bool a_npcTalking, float a_delta);

	// The NPC is speaking the conversation's first line (the greeting, when the game shows
	// the options at once and A chooses one): the options stay visible then (0.8.6).
	void SetFirstLine(bool a_first);

	// A persuasion just ended (the persuasion panel closed, or its success or failure
	// animation started), in steady_clock ticks; 0 = none yet. Safe from any thread.
	[[nodiscard]] std::int64_t PersuasionOutcomeAt() noexcept;

	// Called (main thread) when the player has just chosen a dialogue option, with its text;
	// a_persuasion: one of the persuasion panel's options (0.8.17).
	using ChoiceListener = void (*)(const std::string& a_text, bool a_persuasion);

	// Whether the Address Library has an entry for this ID (a missing one would end the game
	// with an error box when used).
	[[nodiscard]] bool AddressKnown(std::uint64_t a_id);
	void SetChoiceListener(ChoiceListener a_listener);

	// A testing message (bShowMessages) for the top bar while a conversation is filmed.
	// False when it can't go there (no conversation filmed): use the game's notifications.
	// Main thread.
	bool ShowMessage(const std::string& a_text);

	// The on-screen prompt below the crosshair ("" = none), e.g. "Listen in on nearby
	// conversation. Press N". Main thread; drawn by Update.
	void SetPrompt(const std::string& a_text);

	// Hide the game's interaction prompt and name tags (RolloverWidget_mc) while this mod
	// films people talking; put back when false. Main thread.
	void SetHideRollover(bool a_hide);

	// Filming people talking near you (0.8.10): the letterbox bars on the HUD, as in your own
	// conversations (bLetterbox). Main thread.
	void SetFilmingOthers(bool a_filming);

	// The preset on screen with the testing messages (bShowMessages), in the bottom bar while
	// a conversation is filmed, e.g. "PRESET: FRONTIER". Main thread.
	void SetPresetLabel(const std::string& a_text);

	// What each part of Update cost since the last call (0.9.40, the optimization pass):
	// average and worst per frame, for the log's cost lines. "" before any. Main thread.
	[[nodiscard]] std::string CostReport();
}
