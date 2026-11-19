#pragma once

#include "Shots.h"

// Chooses which of the shot library's setups to show and when to change it, following
// Cinematic Conversation Camera's rules: shot/reverse-shot on the speaker, a new angle
// every few lines, short lines don't count, a shortest hold, per-preset weights.
namespace CCC::Director
{
	struct Frame
	{
		bool         npcTurn{ true };      // whose shot it is (from the plugin's speaker logic)
		bool         npcTalking{ false };
		bool         playerTalking{ false };
		bool         nearSeated{ false };  // the person nearest the camera sits lower: no over-the-shoulders
		bool         intenseLine{ false }; // the speaker's current line gets an emotion close-up
		bool         intenseExtreme{ false };  // ... an extreme one (decided with the line, 0.8.3)
		bool         reaction{ false };    // a reaction shot of you: your regular shots only, never the room (0.8.3, as CCC)
		int          npcLineSerial{ 0 };   // counts their lines, including ones that follow without a pause (0.8.3)
		bool         persuasionBeat{ false };  // the NPC's answer to a persuasion's outcome: held close-up
		bool         playerCloseUp{ false };   // this reaction is your emotion close-up, or you chose a tagged option (0.8.16)
		bool         playerChoice{ false };    // ... the tagged option (0.8.16)
		bool         playerExtreme{ false };   // ... as an extreme close-up, whatever the roll (0.8.16)
		bool         openingSingle{ false };   // from first person, your body not built yet: their first shot leaves you out (0.9.2)
		bool         lineDecided{ true };      // the line that just started has had its reaction decided (0.9.23: cuts at a line's start wait for it)
		std::array<float, 3> roomAngle{ std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN() };
		std::array<float, 3> roomDistance{ 0.0f, 0.0f, 0.0f };  // room shots' space (Shots::Input)
		float                roomRise{ 10.0f };
		RE::NiPoint3 npcHead{};
		RE::NiPoint3 playerHead{};
		float        side{ 1.0f };          // which side of the eyeline the conversation is staged on
		bool         npcFaceValid{ false }; // where each face points (close-ups follow the face)
		bool         playerFaceValid{ false };
		RE::NiPoint3 npcFacing{};
		RE::NiPoint3 playerFacing{};
		float        npcGround{ std::numeric_limits<float>::quiet_NaN() };
		float        playerGround{ std::numeric_limits<float>::quiet_NaN() };
		bool         feetValid{ false };    // where each stands (steadies the eyeline)
		RE::NiPoint3 npcFeet{};
		RE::NiPoint3 playerFeet{};
		RE::NiPoint3 npcSwayHead{};         // the heads before the dead band (iCameraSway)
		RE::NiPoint3 playerSwayHead{};
	};

	// Obstacle detection (0.5.3): what the game's collision says about a camera spot.
	struct Clearance
	{
		bool  usable{ true };                                    // false = skip this shot
		float limit{ std::numeric_limits<float>::infinity() };  // pull the camera in to at most this far from the subject
		float length{ 0.0f };                                    // subject to camera, as asked
		float blockedAt{ -1.0f };                                // where the line was blocked, m from the subject (-1 = clear)
		bool  pastNear{ false };                                 // the line touched the near person, and was checked beyond them
		bool  bystander{ false };                                // blocked by someone standing still, not by the scenery
		bool  fallbackOk{ false };                               // blocked, but pulled in to limit it still makes a shot when nothing else fits
	};
	using ObstacleTest = Clearance (*)(const RE::NiPoint3& a_subjectHead, const RE::NiPoint3& a_camera, const RE::NiPoint3& a_nearHead, bool a_overShoulder);

	// The plugin's sphere cast; nullptr = no obstacle checks.
	void SetObstacleTest(ObstacleTest a_test);

	// Call when a conversation starts (reads the INI's preset and per-shot settings).
	void Reset();

	// The camera for this frame, or nullopt when no setup fits (the caller falls back to
	// its plain over-the-shoulder shot).
	[[nodiscard]] std::optional<Camera::Pose> Update(const Frame& a_frame, std::chrono::steady_clock::time_point a_now);

	// With the plain shots (seated people, merchants): the emotion close-up for an intense
	// line, held until the line ends; nullopt = keep the plain shot.
	[[nodiscard]] std::optional<Camera::Pose> UpdatePlain(const Frame& a_frame, std::chrono::steady_clock::time_point a_now);

	// The last emotion close-up the camera cut to: 1 = close-up, 2 = extreme close-up (0 =
	// none yet), and when (for the on-screen messages).
	[[nodiscard]] int LastEmotionCut(std::chrono::steady_clock::time_point& a_when);

	// The lens the shots are framed for, degrees (horizontal, as drawn).
	[[nodiscard]] float LensDegrees();

	// The preset key was pressed (any thread): the Director takes the new preset
	// (Settings::preset) on its next frame, mid-conversation (0.8.3).
	void SwitchPreset();

	// The conversation lens wanted, degrees: fConversationLens, or the preset's own (CCC's
	// presets 45, CCC SF's 37.5); below 10 = the game's own.
	[[nodiscard]] float WantedLens();
	// The shortest time a shot stays on screen in this conversation (the preset's, paced).
	[[nodiscard]] float ShortestHold();

	// iExitStyle 1 (0.9.14): the closing shot for the hold after the last line, the NPC
	// alone (medium, close-up or three-quarter, the first one that fits), so you aren't in
	// the picture when the game swaps your body back. nullopt when the shot on screen already
	// shows them alone (and isn't the plain shot) or nothing fits. a_name: the angle's key;
	// a_distance: camera to their head, m (for the focus).
	[[nodiscard]] std::optional<Camera::Pose> ClosingShot(bool a_plainOnScreen, const char*& a_name, float& a_distance);

	// An angle's lens in the preset in use (0.8.12): its own, CCC's, or the preset's one lens.
	[[nodiscard]] float LensFor(Shots::ShotType a_type);

	// A shot outside the director (the opening shot from your eyes, the plain shots): drawn
	// through the lens of the preset's angle it's most like (0.9.1).
	void UsePlainShotLens(Shots::ShotType a_like);

	// Emotion close-ups and reaction shots in the preset in use (0.8.13): your setting, or the
	// preset's own. EmotionCloseUps: 0 none, 1 every tier, 3 strong lines only.
	[[nodiscard]] int  EmotionCloseUps();
	// The share of their emotion close-ups that are extreme (0.9.25): iExtremeCloseUpChanceNpc,
	// or the preset's own (Frontier 25%, the others 40%).
	[[nodiscard]] int  ExtremeCloseUpChanceNpc();
	[[nodiscard]] bool ReactionShots();

	// A close-up of you when you choose a tagged option (0.8.16): your setting, or the
	// preset's own (CCC SF's presets; CCC doesn't).
	[[nodiscard]] bool ChoiceCloseUps();

	// Seconds of their talking without a cut to you before a reaction is due (0.8.15): your
	// fReactionAfterSecondsPlayer, or the preset's own.
	[[nodiscard]] float ReactionAfterSeconds();

	// The camera sway in use (0.8.14): your iCameraSway, or the preset's own.
	[[nodiscard]] int CameraSway();

	// The handheld wander's strength, 0-1 (0.8.15; Constellation's own since 0.8.17).
	[[nodiscard]] float Handheld();

	// How far back room shots may stand, metres (0 = as far as CCC's framing wants): your
	// fRoomShotMaxIndoors/Outdoors, or the preset's own.
	[[nodiscard]] float RoomShotLimit(bool a_interior);

	// The setup on screen, for the log.
	[[nodiscard]] const char* Current();

	// For the on-screen shot messages: how many cuts so far this conversation, why the last
	// one was made, and whether its camera was pulled in for an obstacle.
	[[nodiscard]] std::uint32_t Cuts();
	[[nodiscard]] const char*   Why();
	[[nodiscard]] bool          PulledIn();
}
