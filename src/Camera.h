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
	std::optional<Pose> OverShoulder(const RE::NiPoint3& a_nearHead, const RE::NiPoint3& a_farHead);

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
	// Moves the game's camera (not the picture) along the line from a_target through where it
	// is now, to a_distance from a_target. Its direction from a_target doesn't change, so the
	// people facing it keep facing the same way. Returns where it was, or nothing if unchanged.
	std::optional<RE::NiPoint3> SetRootDistance(RE::NiAVObject* a_root, const RE::NiPoint3& a_target, float a_distance);
	void                        SetRootPosition(RE::NiAVObject* a_root, const RE::NiPoint3& a_position);

	struct Drift
	{
		float metres{ 0.0f };
		float degrees{ 0.0f };
	};
	std::optional<Drift> RenderDrift();
}
