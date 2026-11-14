#pragma once

namespace CCC::Camera
{
	struct Pose
	{
		RE::NiPoint3 position{};
		RE::NiPoint3 lookAt{};
	};

	// Where a character's head is in the world, or nullopt if their 3D isn't loaded.
	std::optional<RE::NiPoint3> HeadPosition(RE::TESObjectREFR* a_ref);

	// Whether the character has a head bone (HeadPosition otherwise guesses 1.6 m above
	// the model's root, which misses small robots entirely).
	bool HasHeadBone(RE::TESObjectREFR* a_ref);

	// Camera a_behind metres behind the near head (negative = in front of it), a_side to
	// the side and a_height above it, looking at the far character's face.
	std::optional<Pose> Shot(const RE::NiPoint3& a_nearHead, const RE::NiPoint3& a_farHead, float a_behind, float a_side, float a_height);

	// Over-the-shoulder shot: camera behind `a_near`'s head, looking at `a_far`'s face.
	// a_sideSign -1 puts it over the other shoulder (the side of the line being staged on).
	std::optional<Pose> OverShoulder(const RE::NiPoint3& a_nearHead, const RE::NiPoint3& a_farHead, float a_sideSign = 1.0f);

	// Which way a character's face points, from their head bone (close-ups follow the face,
	// as CCC's FaceFrame). The head bone's forward axis isn't named by the game, so it's
	// chosen once per character model: of the bone's axes, not the one along the neck,
	// the one best aligned with the body's forward and the direction to a_toward (the
	// other person). The facing is then eased (about half a second) and kept here.
	struct FaceTrack
	{
		const void*  root{ nullptr };  // the model the axis was chosen for
		bool         columns{ true };
		int          index{ -1 };      // -1: no axis clear enough to trust
		float        sign{ 1.0f };
		RE::NiPoint3 facing{};
		bool         primed{ false };
	};
	// True with a_track.facing set when the face direction is known this frame.
	bool TrackFacing(RE::TESObjectREFR* a_ref, const RE::NiPoint3& a_toward, FaceTrack& a_track, float a_delta, const char* a_who);

	// Slides the camera straight towards the subject until it is at most a_limit from them,
	// keeping the direction it looks in (the subject stays where it is in the frame, larger).
	void PullIn(Pose& a_pose, const RE::NiPoint3& a_subject, float a_limit);

	// Learns Starfield's camera axis convention from the game's own dialogue camera,
	// which is aimed at the speaker. Call before Apply overwrites the camera on a frame.
	// Returns true once a convention is known (now or from an earlier conversation).
	bool Calibrate(RE::NiAVObject* a_root, const RE::NiPoint3& a_speakerHead);
	bool IsCalibrated();

	// Writes the pose into the camera root node. Returns false if the pose was degenerate
	// or the axis convention isn't known yet.
	bool Apply(RE::NiAVObject* a_root, const Pose& a_pose, float a_delta);

	// Moves only the rendered view (the world camera hanging off the camera root),
	// leaving the game's logical camera where the game put it.
	// a_holdDialogueFrustum: the game is not in its dialogue camera right now; keep the
	// dialogue camera's field of view on this frame as well as its position.
	// a_predictedRoot: compute against this camera-root pose instead of the root's current one.
	bool ApplyRender(RE::NiAVObject* a_root, const Pose& a_pose, float a_delta, bool a_holdDialogueFrustum = false, const Pose* a_predictedRoot = nullptr);

	// Where the game's camera (the camera root) is and what it is looking at right now.
	std::optional<Pose> PoseOf(RE::NiAVObject* a_root);

	// Diagnostics: the camera root's stored and derived positions, its parent, and where the
	// render camera was last drawn from.
	std::string DescribeRoot(RE::NiAVObject* a_root);
	void*       RenderCameraObject();

	// Puts the rendered view back on the game's camera. Safe to call any time.
	void RestoreRender();

	// How far the picture the game last drew is from the shot we placed for it: nonzero
	// when something moved the game's camera after our update (scripted scenes, NPCs who
	// start the conversation while walking). Empty when we haven't placed a shot.

	struct Drift
	{
		float metres{ 0.0f };
		float degrees{ 0.0f };
	};
	std::optional<Drift> RenderDrift();
}
