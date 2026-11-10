#pragma once

namespace CCC
{
	enum class ShotMode : std::int32_t
	{
		kOverPlayerShoulder = 0,  // camera behind the player, looking at the NPC
		kOverNpcShoulder = 1,     // camera behind the NPC, looking at the player
		kFollowSpeaker = 2        // whoever is talking gets the camera (shot/reverse-shot)
	};

	struct Settings
	{
		bool     enabled{ true };
		bool     debugLog{ true };
		ShotMode shotMode{ ShotMode::kFollowSpeaker };

		// Over-the-shoulder geometry, in meters (Starfield's world unit; Skyrim's was ~1.43 cm).
		float distanceBehind{ 0.65f };    // how far behind the near character's head
		float sideOffset{ 0.32f };        // shift sideways so the near shoulder frames the shot
		float heightOffset{ 0.06f };      // raise or lower the camera relative to the head
		float lookHeightOffset{ -0.03f }; // aim slightly below the eyes of the far character

		// Seconds of smoothing when the camera follows a moving head. Higher = steadier
		// but slower to follow.
		float followSmoothing{ 0.35f };

		// Seconds to blend between shots. 0 = hard cut (CCC's default style).
		float blendSeconds{ 0.0f };

		// Which camera to move. 0 = the game's logical camera (NPCs react to it and turn
		// toward it), 1 = only the rendered view (the game keeps thinking the camera is at
		// the player's eyes, so NPCs keep facing the player).
		std::int32_t cameraTarget{ 1 };

		// Un-hide the player's body and head during conversations so over-the-shoulder
		// shots can show them. Their original visibility is restored afterwards.
		bool revealPlayer{ true };

		// When a conversation starts in first person, rebuild the player's full body before
		// showing them (briefly cycles the view), and return to first person afterwards.
		bool fixFirstPersonBody{ true };

		// How the camera transform is written. Kept as a test switch for the first builds.
		// 0 = world + local + node update, 1 = world + local only, 2 = world only.
		std::int32_t poseMode{ 0 };

		static Settings& Get();
		void             Load();
	};
}
