#pragma once

namespace CCC::Visibility
{
	// Starfield hides the player's body (and head) during dialogue because its dialogue
	// camera is first-person. Call every frame of a conversation to keep them visible.
	void RevealPlayer();

	// Puts back whatever hidden/visible state we changed. Safe to call any time.
	void Restore();
}
