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
	void Restore();
}
