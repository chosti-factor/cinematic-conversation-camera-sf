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

		// Over-the-shoulder geometry, in game units (roughly 1 unit = 1.4 cm).
		float distanceBehind{ 45.0f };   // how far behind the near character's head
		float sideOffset{ 22.0f };       // shift sideways so the near shoulder frames the shot
		float heightOffset{ 4.0f };      // raise or lower the camera relative to the head
		float lookHeightOffset{ -2.0f }; // aim slightly below the eyes of the far character

		// Seconds to blend between shots. 0 = hard cut (CCC's default style).
		float blendSeconds{ 0.0f };

		// How the camera transform is written. Kept as a test switch for the first builds.
		// 0 = world + local + node update, 1 = world + local only, 2 = world only.
		std::int32_t poseMode{ 0 };

		static Settings& Get();
		void             Load();
	};
}
