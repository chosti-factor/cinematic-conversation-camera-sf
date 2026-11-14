#pragma once

// The conversation's on-screen dressing (0.6.3, CCC's letterbox, subtitles in the bars and
// dialogue options fading while someone speaks). Starfield's menus are Scaleform (AS3)
// movies; this works inside the dialogue menu's movie: black bars are added under its own
// content, so its subtitles and options stay on top of them.
namespace CCC::Hud
{
	// Main thread, every frame. a_filming: this mod's camera is filming the conversation
	// (not the game's own camera in a quest scene). a_delta: seconds since the last call.
	void Update(bool a_filming, float a_delta);
}
