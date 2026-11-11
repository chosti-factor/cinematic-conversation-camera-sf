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

		// After the last line, hold our final shot this long, then hand back to the game's
		// own camera while it finishes closing the conversation (about 1.5 s in total).
		float exitHoldSeconds{ 0.5f };

		// 0 = over the near character's shoulder (camera behind them; can end up inside a
		//     wall or chair they are backed against)
		// 1 = always from in front of and beside the near character (the seated shot), which
		//     keeps the camera in the open space between the two people
		std::int32_t shotStyle{ 0 };

		// Shot used when the near character is sitting: camera this far in front of their
		// head (toward the other person), to the side, and above it.
		float seatedForward{ 0.25f };
		float seatedSide{ 0.45f };
		float seatedHeight{ 0.12f };

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

		// Do that rebuild on the conversation's very first frames, holding the first-person
		// view, instead of a quarter second in (which visibly zooms the picture out and back).
		bool earlyBodyRebuild{ true };

		// How the camera transform is written. Kept as a test switch for the first builds.
		// 0 = world + local + node update, 1 = world + local only, 2 = world only.
		std::int32_t poseMode{ 0 };

		// Test switch: hook the first- and third-person camera updates (used to hide the
		// first-person body rebuild). Read once at game start.
		bool stateHooks{ true };

		// Test switch, re-read at the start of every conversation (no game restart needed):
		// while a conversation runs, set some of the game's own field-of-view settings to
		// fFovExperimentValue to find out which one the dialogue camera listens to.
		// 0 = off, 1 = fTPWorldFOV + fFPWorldFOV, 2 = fDialogueCameraFailsafeFPFOV,
		// 3 = fDefault1stPersonFOV + fFPGeometryFOV, 4 = a value inside the dialogue camera itself
		std::int32_t fovExperiment{ 0 };
		float        fovExperimentValue{ 50.0f };

		// Field of view of the conversation shots, in degrees. 0 = leave the game's own
		// (about 30, a tight close-up).
		float dialogueFov{ 0.0f };

		// How the player's full body is built for first-person conversations.
		// 1 = ask the game to refresh the player's 3D model directly (no camera switching;
		//     falls back to 0 if the body hasn't appeared after a moment)
		// 0 = briefly cycle the game through first and third person
		std::int32_t bodyRebuildMethod{ 1 };

		// Which parts the direct refresh asks for (1 model, 2 skin, 4 head, 8 face,
		// 16 scale, 32 skeleton; add them up). 63 = all of them, best in testing; 33 left
		// the head missing.
		std::int32_t bodyUpdateFlags{ 63 };

		// Test switch: when fDialogueFOV is set, how it is written.
		// 1 = just before the game's dialogue camera update (the game then works from it)
		// 2 = just after it, field of view and view cone (0.3.30; made the picture shake)
		// 3 = before the update, plus the view cone after it
		std::int32_t fovMethod{ 1 };

		// The conversation's first frames, while the game is still switching to its dialogue
		// camera. iStartMode (from first person) / iStartModeThirdPerson:
		// 0 = show the gameplay view, 1 = show the first shot placed against where the dialogue
		// camera is predicted to be, 2 = show the gameplay view placed that way,
		// 3 (first person only) = the 0.3.33 opening: a shot from the player's eyes until the
		// conversation's first cut.
		std::int32_t startMode{ 1 };
		std::int32_t startModeThird{ 2 };
		// iExitMode (first person): 0 = cover the game's first first-person frame, 1 = no
		// cover (best in testing), 2 = cover the first three first-person frames.
		std::int32_t exitMode{ 1 };

		// Game settings to change, from the [GameSettings] section: "name:Section" = value.
		std::vector<std::pair<std::string, std::string>> gameSettings;

		static Settings& Get();
		void             Load();
	};
}
