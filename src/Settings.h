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
		bool     showEmotions{ true };  // on-screen message with each line's expression tag (testing aid)
		bool     dumpMenus{ true };     // log the dialogue menu's and HUD's layout once per session (testing aid)
		bool     letterbox{ true };     // black bars while the camera films a conversation
		float    letterboxHeight{ 0.12f };  // each bar, share of the screen height
		float    letterboxFade{ 0.4f };     // seconds for the bars to slide in or out
		bool     fadeOptions{ true };       // the dialogue options fade in when the game shows them
		float    optionsFade{ 0.35f };      // seconds
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
		float seatedSide{ 0.9f };
		float seatedHeight{ 0.3f };
		// Merchants on your lines: the camera stands in front of and beside them, facing you,
		// like the seated shot (behind them it was close enough that the game faded them out,
		// and shops put shelves and counters there).
		float vendorSide{ 0.9f };
		float vendorHeight{ 0.05f };

		// Seconds of smoothing when the camera follows a moving head. Higher = steadier
		// but slower to follow.
		float followSmoothing{ 0.35f };
		// While the speaker's head moves fast (walking), follow it more steadily: the head bone
		// bobs with each step and is sampled mid-animation, which made the picture jitter.
		float busyFollowSmoothing{ 1.0f };
		float busyHeadSpeed{ 0.25f };  // m/s; back to normal below 60% of this

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

		// [Director] CCC's shot library and cutting rules.
		bool        director{ true };
		std::string preset{ "Standard" };
		// How lively the cutting is: Cinematic, Odyssey, SpaceWestern, Dogfight or SpaceOpera.
		// Sets the three values below plus the chance of a fresh angle when the turn passes;
		// any of the three given in the INI overrides the style's.
		std::string cuttingStyle{ "SpaceWestern" };
		int         freshAngleChance{ 50 };    // % chance of a new angle each time the turn passes
		int         minLines{ 2 };             // new angle after this many lines...
		int         maxLines{ 5 };             // ...up to this many (random in between)
		float       shortLineSeconds{ 1.2f };  // lines shorter than this don't count
		float       shortestHold{ 2.0f };      // never change angle sooner than this
		float       maxCameraDistance{ 0.0f }; // metres from the person framed, indoors (0 = the preset's own)
		float       maxCameraDistanceExterior{ 0.0f };  // outdoors (0 = the preset's own)
		bool        obstacleCheck{ true };       // skip or pull in shots the game's collision says are blocked
		float       obstacleRadius{ 0.05f };     // width of the check (the game's own dialogue camera uses 0.05)
		bool        true180{ false };
		bool        enforceLine{ true };      // no angle (with face turns and moves) crosses the eyeline
		bool        followFace{ true };       // close-ups turn toward where the face points
		float       faceDeadband{ 0.04f };    // head movement ignored before the camera follows (m)
		int         cameraSway{ 40 };         // % of the heads' sway the camera's angle follows (0 = none, 100 = full)
		bool        cameraMoves{ true };
		// Emotion close-ups (0.4.14): when the speaker starts a line whose expression tag is
		// in this list, the camera pushes in to a close-up (a third of the time an extreme
		// close-up), as CCC does for the lines Skyrim's writers marked at full strength.
		bool        emotionCloseUps{ true };
		int         extremeCloseUpChance{ 40 };  // % of emotion close-ups that are extreme close-ups
		std::string closeUpEmotions{ "Angry,Irritated,Annoyed,Surprised,Fearful,Scared,Disgusted,Somber,Sad,Nervous,Worried,Concerned,Awed,Apologetic,Disgust,Serious,Confused,Happy,Smirk,Puzzled,Suspicious,Relieved,Concentrating" };
		// Plain shots (over the shoulder) for seated speakers and merchants: the shot library's
		// angles often put the chair or the counter between the camera and them.
		bool        plainShotsSeated{ true };
		bool        plainShotsMerchants{ true };
		float       seatedHeadHeight{ 1.35f };  // head this close above the feet = sitting
		std::vector<std::pair<std::string, std::string>> shotOverrides;  // [Shots]

		// Leave the game's camera alone for speakers this short (head above feet, metres)
		// or without a head bone (small robots).
		float minSpeakerHeight{ 1.0f };
		bool  gameCameraInScenes{ true };
		// Focus on the person in shot (0.5.0): the game's conversation blur is focused at the
		// distance from the camera to whoever is in shot. Focus style (0.5.1): Cinematic = the
		// game's own sharp zone (more background blur); Wide = a much deeper sharp zone on shots
		// further than deepFocusBeyond (softer background blur).
		bool        focusOnSubject{ true };
		std::string focusStyle{ "Wide" };
		bool        deepFocusLongShots{ false };  // derived from focusStyle
		float       deepFocusBeyond{ 3.5f };

		static Settings& Get();
		void             Load();
	};
}
