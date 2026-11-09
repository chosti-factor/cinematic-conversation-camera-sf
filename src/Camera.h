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

	// Writes the pose into the camera root node. Returns false if the pose was degenerate.
	bool Apply(RE::NiAVObject* a_root, const Pose& a_pose, float a_delta);
}
