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
		bool     showMessages{ true };  // on-screen messages: each line's expression tag and each shot (testing aid)
		float    messageSeconds{ 6.0f };  // how long a message stays in the top bar
		int      presetKey{ 0x55 };       // switches presets on the fly (U; 0 = none), 0.8.3
		float    conversationLens{ -1.0f };  // the conversation lens, degrees (-1 = the preset's own)
		bool     filmConversations{ true }; // film people talking near you (the listen key)
		int      listenKey{ 0x4E };          // virtual-key code, 0 = no prompt or key (N)
		float    listenRange{ 12.0f };       // how far from you a conversation may be
		int      listenButton{ 0x2000 };     // XInput button mask, held to listen (B); 0 = none
		float    listenHold{ 0.5f };         // seconds the button is held
		bool     listenPrompt{ true };       // "Listen in on nearby conversation" on screen (0.8.2)
		bool     autoListen{ true };         // start on its own when you sit, or stand still near them (0.8.2)
		float    filmEndSeconds{ 0.5f };       // filming people talking ends this long after the last word (0.8.4)
		float    filmEndSecondsScene{ 4.0f };  // ... while they're still in one of the game's scenes
		// Moving ends filming (0.9.18, from the INI): once you've stood still this long, moving
		// this far from that spot ends it; before you've stopped (walking up to it), walking on
		// this far from where it started does.
		float    filmStopSeconds{ 0.3f };
		float    filmStopDistance{ 0.25f };
		float    filmWalkOnDistance{ 3.0f };
		float    autoListenAfter{ 1.0f };    // seconds standing still first (sitting: 1 s at most), 0.8.4: was 2
		int      roomShotMinRoom{ 60 };    // room shots only where the room gives this % of CCC's distance
		float    roomShotMaxIndoors{ -1.0f };   // metres; 0 = as far as CCC's framing wants, -1 = the preset's own (0.8.1)
		float    roomShotMaxOutdoors{ -1.0f };
		bool     playerFace{ true };      // your character's expressions during conversations
		int      questionFaceChance{ 40 };  // % of untagged questions you ask with a questioning look
		int      signatureChance{ 30 };     // % of neutral moments your background's expression shows
		int      closeUpChanceMildPlayer{ 25 };    // 0.8.6: by your expression's tier (the Npc tier lists)
		int      closeUpChanceMediumPlayer{ 40 };
		int      closeUpChanceStrongPlayer{ 100 };
		bool     dumpMenus{ true };     // log the dialogue menu's and HUD's layout once per session (testing aid)
		bool     letterbox{ true };     // black bars while the camera films a conversation
		float    letterboxHeight{ 0.12f };  // each bar, share of the screen height
		float    letterboxFade{ 0.4f };     // seconds for the bars to slide in or out
		bool     subtitlesInBars{ false };   // centre the conversation's subtitle in the bottom bar (Starfield's own spot is already at its top edge)
		bool     hideOptionsWhileSpeaking{ true };  // the options fade out while the NPC speaks (not on their first line, 0.8.6)
		float    optionsFade{ 0.0f };       // seconds (0 = no fade)
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
		float overSeatedBehind{ 0.35f };   // over your shoulder onto someone seated (0.8.17)
		float overSeatedSide{ 0.6f };
		float overSeatedHeight{ -0.25f };
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
		std::int32_t startModeThird{ 1 };
		// iExitMode (first person): 0 = cover the game's first first-person frame, 1 = no
		// cover, 2 = cover the first three first-person frames, 3 = skip the first-person
		// camera's entry step (0.9.4).
		std::int32_t exitMode{ 5 };
		std::int32_t exitSkipFrames{ 1 };  // iExitMode 5: first-person updates held back
		// iExitModeThirdPerson: 1 = your third-person view drawn on the switch frame (0.9.4),
		// 0 = the game's (a frame of its own conversation camera).
		std::int32_t exitModeThird{ 1 };
		// bExitLens (0.9.13): your gameplay lens put back on the first frame of your view at an
		// exit (otherwise that frame is drawn through the conversation's lens, zoomed in).
		bool         exitLens{ true };
		// iExitStyle (0.9.14), how a conversation ends, for the hold after the last line:
		// 0 = the last shot stays to the end; 1 = a closing shot of the NPC alone (you're out
		// of the picture when the game swaps your body back); 2 = a cut back to your own view
		// fExitCutSeconds after the last line (your body hidden from first person).
		std::int32_t exitStyle{ 2 };
		float        exitCutSeconds{ 0.4f };
		// bExitCutStaged (0.9.16): style 2's cut built to look right whatever order its parts
		// reach the screen in: the lens set with the camera, from first person the camera 12 cm
		// ahead of your eyes easing back and your body hidden from the third frame, the blur off
		// from the second. 0 = everything at once.
		bool         exitCutStaged{ true };

		// Game settings to change, from the [GameSettings] section: "name:Section" = value.
		std::vector<std::pair<std::string, std::string>> gameSettings;

		// [Director] CCC's shot library and cutting rules.
		bool        director{ true };
		std::string preset{ "Standard" };
		// How lively the cutting is, relative to the preset's own rhythm (0.9.8; replaced the
		// named cutting styles): -2 calmest .. 0 the preset's own .. +2 liveliest.
		int         cuttingPace{ 0 };
		int         minLines{ 0 };             // new angle after this many lines... (0 = the preset's)
		int         maxLines{ 0 };             // ...up to this many, random in between (0 = the preset's)
		float       shortLineSeconds{ 1.2f };  // lines shorter than this don't count
		float       shortestHold{ 0.0f };      // never change angle sooner than this (0 = the preset's)
		float       maxCameraDistance{ 0.0f }; // metres from the person framed, indoors (0 = the preset's own)
		float       maxCameraDistanceExterior{ 0.0f };  // outdoors (0 = the preset's own)
		bool        obstacleCheck{ true };       // skip or pull in shots the game's collision says are blocked
		float       obstacleRadius{ 0.05f };     // width of the check (the game's own dialogue camera uses 0.05)
		bool        true180{ false };
		bool        enforceLine{ true };      // no angle (with face turns and moves) crosses the eyeline
		bool        followFace{ true };       // close-ups turn toward where the face points
		int         choiceCloseUps{ -1 };     // a close-up of you on a tagged option: -1 the preset's own (CCC SF's), 0 off, 1 on (0.8.16)
		int         reactionShots{ -1 };      // one of the NPC's lines shown on you: -1 the preset's own (0.8.13: CCC's off, CCC SF's on), 0 off, 1 on
		int         reactionEvery{ 2 };       // NPC lines heard before reactions may come
		bool        avoidBystanders{ true };  // a still bystander covering a face or most of the frame blocks a shot
		float       bystanderStill{ 1.5f };   // seconds without moving before a bystander counts
		int         reactionChance{ 100 };    // percent, once due (0.8.5: was 50, rolled on a sentence count)
		float       reactionSeconds{ 2.5f };  // on you, then back to the speaker; 0 = the whole line (0.8.1)
		float       reactionMinSeconds{ 2.0f };  // (0.8.3; unused since 0.8.5: reactions last their line)
		float       reactionAfterSeconds{ -1.0f };  // their talking without a cut to you before a reaction is due (0.8.5); -1 = the preset's own (0.8.15)
		float       reactionLineMin{ 2.5f };     // the line a reaction takes: at least ... seconds
		float       reactionLineMax{ 7.0f };     // ... and at most
		float       reactionLongLineHold{ 4.0f };  // a longer line: on you for its first this many seconds (0 = wait for one that suits)
		bool        persuasionBeat{ true };   // the answer to a persuasion's outcome or a skill check: held close-up
		std::string beatTags{ "Intimidate,Intimidation,Manipulate,Manipulation,Bribe,Deceive,Deception,Lie,Threaten,Diplomacy,Negotiate,Negotiation,Commerce" };
		float       faceDeadband{ 0.04f };    // head movement ignored before the camera follows (m)
		int         cameraSway{ -1 };         // % of the heads' sway the camera's angle follows (0 = none, 100 = full); -1 = the preset's own (0.8.14)
		bool        cameraMoves{ true };
		// Emotion close-ups (0.4.14): when the speaker starts a line whose expression tag is
		// in this list, the camera pushes in to a close-up (a third of the time an extreme
		// close-up), as CCC does for the lines Skyrim's writers marked at full strength.
		int         emotionCloseUps{ -1 };    // -1 the preset's own (0.8.13: Standard and Room none, Close strong lines only, CCC SF's every tier), 0 off, 1 every tier
		int         extremeCloseUpChance{ 40 };  // % of emotion close-ups that are extreme close-ups
		int         extremeCloseUpChancePlayer{ 40 };  // % of close-ups of you that are extreme (0.8.3)
		// Emotion close-ups by tier (0.8.6): each list with its own chance (%).
		std::string closeUpEmotionsMild{ "Amused,Confident,Thinking,Grateful,Question,Impressed,Tired,Concentrating,Relieved" };
		std::string closeUpEmotionsMedium{ "Concerned,Serious,Smirk,Happy,Confused,Puzzled,Apologetic,Awed" };
		std::string closeUpEmotionsStrong{ "VeryAngry,Rage,Angry,Irritated,Afraid,Shocked,Surprised,Disgust,Somber,Depressed,Worried,Nervous,Pleading,Disbelief,Defiant,Stern,Suspicious,Smug,Sarcastic,Flirting,SinisterSmile,InPain,Dying,EyesClosed" };
		int         closeUpChanceMild{ 25 };
		int         closeUpChanceMedium{ 40 };
		int         closeUpChanceStrong{ 100 };
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
		// The preset switched to with the preset key: kept over sPreset until the game is
		// restarted ("" = sPreset). Main thread.
		static void SetPresetOverride(const std::string& a_preset);
	};
}
