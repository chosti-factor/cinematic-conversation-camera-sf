#pragma once

#include "Camera.h"

// The shot library, ported from Cinematic Conversation Camera by hashhbbrown
// (https://github.com/fatalCMD/ccc, GPL-3.0-or-later): the same 39 setups, framing
// table and camera moves, converted to Starfield's metres. What differs is the lens:
// CCC sets a field of view per shot, while Starfield's dialogue camera has a fixed one
// (about 30 degrees), so the distance for each framing is solved at that lens and then
// capped, to keep the camera near the conversation.
namespace CCC::Shots
{
	enum class ShotType : std::uint8_t
	{
		kOverPlayerShoulder = 0,
		kCloseUp,
		kMediumNpc,
		kLongNpc,
		kLowAngle,
		kCloseProfile,
		kCloseLow,
		kCloseHigh,
		kCloseWide,
		kExtremeClose,
		kDirtyNpc,
		kThreeQuarterNpc,
		kMediumProfile,
		kLowProfile,
		kOverhead,
		kOverPlayerShoulderLow,
		kOverPlayerShoulderHigh,
		kOverPlayerShoulderWide,

		kOverNpcShoulder,
		kMediumPlayer,
		kHighAngle,
		kClosePlayer,
		kExtremeClosePlayer,
		kDirtyPlayer,
		kPlayerProfile,
		kPlayerLow,
		kThreeQuarterPlayer,
		kOverNpcShoulderLow,
		kOverNpcShoulderHigh,
		kOverNpcShoulderWide,
		kLongPlayer,
		kPlayerOverhead,

		kTwoShot,
		kProfile,
		kWide,
		kDistant,
		kMaster,
		kGroundLevel,
		kDistantLow,

		kCount
	};

	enum class Anchor : std::uint8_t
	{
		kSubject,
		kMidpoint,
		kScene
	};

	enum class Move : std::uint8_t
	{
		kLocked,
		kPushIn,
		kPullOut,
		kCraneUp,
		kCraneDown,
		kTiltUp,
		kTiltDown,
		kDrift,
		kZoomIn,   // the lens can't change here: done as a push in
		kZoomOut,  // done as a pull out
		kOrbitLeft,
		kOrbitRight,
		kTruckLeft,
		kTruckRight,
		kCount
	};

	[[nodiscard]] const char* Key(ShotType a_type) noexcept;  // INI / log name, as in CCC
	[[nodiscard]] bool        OnNpc(ShotType a_type) noexcept;
	[[nodiscard]] bool        OverShoulder(ShotType a_type) noexcept;
	[[nodiscard]] bool        Neutral(ShotType a_type) noexcept;
	[[nodiscard]] Anchor      AnchorOf(ShotType a_type) noexcept;
	[[nodiscard]] Move        DefaultMove(ShotType a_type) noexcept;
	[[nodiscard]] int         DefaultMoveAmount(ShotType a_type) noexcept;  // 0-100
	[[nodiscard]] const char* MoveName(Move a_move) noexcept;
	[[nodiscard]] std::optional<Move> MoveFromName(std::string_view a_name) noexcept;
	[[nodiscard]] std::optional<ShotType> FromKey(std::string_view a_key) noexcept;

	struct Input
	{
		RE::NiPoint3 npcHead{};
		RE::NiPoint3 playerHead{};
		float        lensDegrees{ 29.6f };  // horizontal
		float        aspect{ 1.7778f };
		float        maxDistance{ 3.0f };   // from the framed subject (or midpoint)
		float        side{ 1.0f };          // which side of the eyeline (180-degree rule)
		bool         true180{ false };
		float        progress{ 0.0f };      // 0..1 through the shot's move
		Move         move{ Move::kLocked };
		int          moveAmount{ 0 };       // 0-100 of the move's full travel

		// Decided when the shot cuts and then held, so the camera doesn't step when
		// someone sways: the angle off the eyeline (before any move) and which side of the
		// frame the subject sits on. NaN / 0 = decide now.
		float heldAngle{ std::numeric_limits<float>::quiet_NaN() };
		float heldLookSign{ 0.0f };

		// Close-ups that follow the face (CCC's bFollowFace): where each face points (unit
		// vector, from the head bone), so a close-up turns toward a face turned to its work
		// and drops to look up into a lowered one. Decided at the cut and held (heldFacePitch;
		// the turn is part of heldAngle).
		bool         followFace{ false };
		bool         npcFaceValid{ false };
		bool         playerFaceValid{ false };
		RE::NiPoint3 npcFacing{};
		RE::NiPoint3 playerFacing{};
		float        heldFacePitch{ std::numeric_limits<float>::quiet_NaN() };
		// Floor heights under each person (NaN = unknown): a lowered face never puts the
		// camera in the floor.
		float npcGround{ std::numeric_limits<float>::quiet_NaN() };
		float playerGround{ std::numeric_limits<float>::quiet_NaN() };

		// Where each person stands (their root, which idle animation doesn't move). The
		// eyeline's direction is taken from these rather than from the heads: a camera 7 m
		// out orbits with the eyeline, so a head swaying 5 cm swung it a quarter of a metre
		// (0.6.0). CCC measures its eyeline between stable points for the same reason.
		bool         feetValid{ false };
		RE::NiPoint3 npcFeet{};
		RE::NiPoint3 playerFeet{};
		// How much of the heads' sway the camera follows, 0-1 (iCameraSway / 100): 0 = the
		// bearing from where they stand only, 1 = from the heads (the full sway of 0.6.0).
		float        sway{ 0.4f };
		RE::NiPoint3 npcSwayHead{};   // the heads before the dead band, whose bearing is the swaying one
		RE::NiPoint3 playerSwayHead{};

		// 180-degree rule: never let an angle (after face turns and moves) cross the eyeline
		// or come within a few degrees of it (CCC's bEnforceLine).
		bool enforceLine{ true };
	};

	struct Result
	{
		Camera::Pose pose{};
		float        angle{ 0.0f };           // as decided (before the move), to hold
		float        lookSign{ 0.0f };        // as decided, to hold
		float        wantedDistance{ 0.0f };  // before the cap
		float        distance{ 0.0f };        // used
		bool         ok{ false };
		bool         faced{ false };          // framed on the face (close singles with followFace)
		float        faceYaw{ 0.0f };         // degrees turned toward where the face points
		float        facePitch{ 0.0f };       // degrees, negative = camera dropped for a lowered face
	};

	// Close singles frame the face (CCC: anchored on the subject, aimed at them, not over a
	// shoulder, filling at least half the frame).
	[[nodiscard]] bool FollowsFace(ShotType a_type) noexcept;

	// Where the camera goes for this setup. Not ok when the setup can't be framed here:
	// scene setups (they need to measure the room, not possible yet) and two-person
	// setups that would need the camera much further away than the cap.
	[[nodiscard]] Result Solve(ShotType a_type, const Input& a_input);
}
