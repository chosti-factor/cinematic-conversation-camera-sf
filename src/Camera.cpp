#include "Camera.h"
#include "Settings.h"

namespace CCC::Camera
{
	namespace
	{
		// Skeleton names tried in order when looking for a head. Starfield uses the C_ prefix;
		// the rest are fallbacks seen on creature and modded skeletons.
		constexpr std::array kHeadNodes{ "C_Head", "Head", "NPC Head [Head]", "C_Neck1", "C_Neck", "Neck" };

		RE::NiPoint3 Sub(const RE::NiPoint3& a, const RE::NiPoint3& b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
		RE::NiPoint3 Add(const RE::NiPoint3& a, const RE::NiPoint3& b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
		RE::NiPoint3 Scale(const RE::NiPoint3& a, float s) { return { a.x * s, a.y * s, a.z * s }; }
		RE::NiPoint3 Cross(const RE::NiPoint3& a, const RE::NiPoint3& b)
		{
			return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
		}
		float Length(const RE::NiPoint3& a) { return std::sqrt(a.x * a.x + a.y * a.y + a.z * a.z); }
		bool  Finite(const RE::NiPoint3& a) { return std::isfinite(a.x) && std::isfinite(a.y) && std::isfinite(a.z); }

		std::optional<RE::NiPoint3> Normalized(const RE::NiPoint3& a)
		{
			const float len = Length(a);
			if (!(len > 1e-4f) || !std::isfinite(len)) {
				return std::nullopt;
			}
			return Scale(a, 1.0f / len);
		}

		RE::NiAVObject* Root3D(RE::TESObjectREFR* a_ref)
		{
			if (!a_ref) {
				return nullptr;
			}
			auto  lock = a_ref->loadedData.LockRead();
			auto* data = *lock;
			return data ? data->data3D.get() : nullptr;
		}
	}

	std::optional<RE::NiPoint3> HeadPosition(RE::TESObjectREFR* a_ref)
	{
		auto* root = Root3D(a_ref);
		if (!root) {
			return std::nullopt;
		}
		for (const auto* name : kHeadNodes) {
			if (auto* node = root->GetObjectByName(RE::BSFixedString{ name })) {
				if (Finite(node->world.translate)) {
					return node->world.translate;
				}
			}
		}
		// No skeleton match: estimate from the reference position (about eye height for a human).
		auto pos = a_ref->GetPosition();
		pos.z += 120.0f;
		return Finite(pos) ? std::optional{ pos } : std::nullopt;
	}

	std::optional<Pose> OverShoulder(const RE::NiPoint3& a_nearHead, const RE::NiPoint3& a_farHead)
	{
		const auto& s = Settings::Get();

		// Work on the ground plane so the shot stays level even if heights differ.
		const auto flat = Normalized({ a_farHead.x - a_nearHead.x, a_farHead.y - a_nearHead.y, 0.0f });
		if (!flat) {
			return std::nullopt;
		}
		const RE::NiPoint3 up{ 0.0f, 0.0f, 1.0f };
		const auto         right = Cross(*flat, up);

		Pose pose;
		pose.position = Add(a_nearHead, Add(Scale(*flat, -s.distanceBehind), Add(Scale(right, s.sideOffset), Scale(up, s.heightOffset))));
		pose.lookAt = Add(a_farHead, Scale(up, s.lookHeightOffset));
		return pose;
	}

	bool Apply(RE::NiAVObject* a_root, const Pose& a_pose, float a_delta)
	{
		if (!a_root) {
			return false;
		}
		const auto forward = Normalized(Sub(a_pose.lookAt, a_pose.position));
		if (!forward) {
			return false;
		}
		const RE::NiPoint3 worldUp = std::abs(forward->z) > 0.9999f ? RE::NiPoint3{ 0.0f, 1.0f, 0.0f } : RE::NiPoint3{ 0.0f, 0.0f, 1.0f };
		const auto         right = Normalized(Cross(*forward, worldUp));
		if (!right) {
			return false;
		}
		const auto up = Cross(*right, *forward);

		// Columns are right / forward / up, the Creation Engine camera convention
		// (the same layout Cinematic Conversation Camera uses in Skyrim).
		RE::NiMatrix3 rot{};
		rot.entry[0][0] = right->x;  rot.entry[0][1] = forward->x;  rot.entry[0][2] = up.x;
		rot.entry[1][0] = right->y;  rot.entry[1][1] = forward->y;  rot.entry[1][2] = up.y;
		rot.entry[2][0] = right->z;  rot.entry[2][1] = forward->z;  rot.entry[2][2] = up.z;

		const auto mode = Settings::Get().poseMode;

		a_root->world.translate = a_pose.position;
		a_root->world.rotate = rot;
		if (mode == 2) {
			return true;
		}
		a_root->local.translate = a_pose.position;
		a_root->local.rotate = rot;
		if (mode == 1) {
			return true;
		}
		RE::NiUpdateData update{};
		update.delta = a_delta;
		a_root->Update(&update);
		return true;
	}
}
