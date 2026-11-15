#pragma once

namespace CCC::Visibility
{
	// Starfield hides the player's body (and head) during dialogue because its dialogue
	// camera is first-person. Call every frame of a conversation to keep them visible.
	void RevealPlayer();

	// The opposite: keep the player hidden (used when their full body could not be built,
	// so the broken first-person form never shows in a shot).
	void HidePlayer();

	// Puts back whatever hidden/visible state we changed. Safe to call any time.
	// Puts back what we changed. a_visible: leave the player shown (the conversation started
	// in third person, where the game shows them) or hidden (first person).
	void Restore(bool a_visible);

	// While filming people talking from first person (0.8.6): also hide the game's
	// first-person body ("Player1stPersonBody": your armour, posed for the first-person
	// view, T-posed in a filmed shot), wherever it hangs. Every frame; the Restore below puts
	// it back. Logs where it was found the first time.
	void HideFirstPersonBody();
	void RestoreFirstPersonBody();
}
