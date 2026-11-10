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
	bool ApplyRender(RE::NiAVObject* a_root, const Pose& a_pose, float a_delta);

	// Puts the rendered view back on the game's camera. Safe to call any time.
	void RestoreRender();
}
