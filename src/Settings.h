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
		bool     autoListenSprinting{ false };  // ... while you sprint past too (0.9.39; off: only walking and jogging)
		float    filmEndSeconds{ 0.5f };       // filming people talking ends this long after the last word (0.8.4)
		float    filmEndSecondsScene{ 4.0f };  // ... while they're still in one of the game's scenes
		// Moving ends filming (0.9.18, from the INI): once you've stood still this long, moving
		// this far from that spot ends it; before you've stopped (walking up to it), walking on
		// this far from where it started does.
		float    filmStopSeconds{ 0.3f };
		float    filmStopDistance{ 0.25f };
		float    filmWalkOnDistance{ 3.0f };
		int      roomShotMinRoom{ 60 };    // room shots only where the room gives this % of CCC's distance
		float    roomShotMaxIndoors{ -1.0f };   // metres; 0 = as far as CCC's framing wants, -1 = the preset's own (0.8.1)
		float    roomShotMaxOutdoors{ -1.0f };
		bool     playerFace{ true };      // your character's expressions during conversations
		int      questionFaceChance{ 40 };  // % of untagged questions you ask with a questioning look
		int      signatureChance{ 30 };     // % of neutral moments your background's expression shows
		int      closeUpChanceMildPlayer{ 25 };    // 0.8.6: by your expression's tier (the Npc tier lists)
		int      closeUpChanceMediumPlayer{ 40 };
		int      closeUpChanceStrongPlayer{ 100 };
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

		// Un-hide the player's body and head during conversations so over-the-shoulder
		// shots can show them. Their original visibility is restored afterwards.
		bool revealPlayer{ true };

		// When a conversation starts in first person, rebuild the player's full body before
		// showing them (briefly cycles the view), and return to first person afterwards.
		bool fixFirstPersonBody{ true };

		// iExitStyle (0.9.14), how a conversation ends, for the hold after the last line:
		// 1 = a closing shot of the NPC alone (you're out of the picture when the game swaps
		// your body back); 2 = a cut back to your own view fExitCutSeconds after the last line
		// (your body hidden from first person). The finished alternatives (start and exit
		// modes, the body method, test switches) are in docs/MEMORY.md (0.9.30).
		std::int32_t exitStyle{ 2 };
		float        exitCutSeconds{ 0.4f };

		// Game settings to change, from the [GameSettings] section: "name:Section" = value.
		std::vector<std::pair<std::string, std::string>> gameSettings;

		// [Director] CCC's shot library and cutting rules.
		bool        director{ true };
		std::string preset{ "Nebula" };  // the default since 0.9.26 (was Standard)
		// How lively the cutting is, relative to the preset's own rhythm (0.9.8; replaced the
		// named cutting styles): -2 calmest .. 0 the preset's own .. +2 liveliest.
		float       cuttingPace{ 0.0f };  // -2 to +2 in steps of 0.5 (0.9.26; whole steps before, iCuttingPace)
		// Each preset's own pace (0.9.27): fCuttingPace<Preset> (fCuttingPaceNebula...); NaN = not
		// set, fCuttingPace applies. In the order of kPresetNames.
		static constexpr std::array<const char*, 7> kPresetNames{ "Standard", "Close", "Room", "Frontier", "Starborn", "Constellation", "Nebula" };
		std::array<float, 7> cuttingPaces{ std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN(),
			std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN() };
		// The pace in use for a preset (any case): its own, or fCuttingPace.
		[[nodiscard]] float CuttingPaceFor(std::string_view a_preset) const
		{
			for (std::size_t i = 0; i < kPresetNames.size(); ++i) {
				const std::string_view name{ kPresetNames[i] };
				if (name.size() == a_preset.size() &&
					std::equal(name.begin(), name.end(), a_preset.begin(), [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b)); })) {
					return std::isnan(cuttingPaces[i]) ? cuttingPace : cuttingPaces[i];
				}
			}
			return cuttingPace;
		}
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
		float       choiceCloseUpSeconds{ 2.0f };  // ... shown as you pick, over the start of their answer, this long (0.9.23)
		float       closeUpLimit{ -1.0f };    // one close-up of them at most this long, then a wider shot of them: -1 the preset's own, 0 none (0.9.23)
		int         listeningCloseUps{ -1 };  // listening reactions on a close-up of you: -1 the preset's own (not in Frontier), 0 no, 1 yes (0.9.23)
		int         reactionShots{ -1 };      // one of the NPC's lines shown on you: -1 the preset's own (0.8.13: CCC's off, CCC SF's on), 0 off, 1 on
		bool        avoidBystanders{ true };  // a still bystander covering a face or most of the frame blocks a shot
		float       bystanderStill{ 1.5f };   // seconds without moving before a bystander counts
		int         reactionChance{ 100 };    // percent, once due (0.8.5: was 50, rolled on a sentence count)
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
		int         extremeCloseUpChance{ -1 };  // % of emotion close-ups that are extreme close-ups; -1 = the preset's own (0.9.25: Frontier 25, the rest 40)
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
		// Large speakers (0.9.19; settings 0.9.22): a speaker whose head is this high above
		// their feet (Vasco 1.75; people 1.58-1.63) hides you in the narrower shots of you, so
		// the shots of you keep to largeSpeakerAngle degrees off the line or wider. 0 = off.
		float largeSpeakerHeight{ 1.70f };
		float largeSpeakerAngle{ 38.0f };
		float largeSpeakerPlainAngle{ 45.0f };  // ... and the plain over-the-shoulder shot of you (0.9.25)
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
		// Load() only when either INI file has been written since the last read (0.9.41): a
		// full read takes up to 20 ms. True when it re-read. Main thread.
		bool             LoadIfChanged();
		// The preset switched to with the preset key: kept over sPreset until the game is
		// restarted ("" = sPreset). Main thread.
		static void SetPresetOverride(const std::string& a_preset);
	};
}
