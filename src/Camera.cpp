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
		// No skeleton match: estimate from the 3D root (about eye height for a human, in meters).
		auto pos = root->world.translate;
		pos.z += 1.6f;
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

	namespace
	{
		// How the camera's facing, up and side directions are stored in its rotation
		// matrix: as columns or rows, which index holds each, and with which sign.
		struct Axes
		{
			bool  columns{ true };
			int   forward{ 1 };
			int   up{ 2 };
			int   side{ 0 };
			float forwardSign{ 1.0f };
			float upSign{ 1.0f };
			float sideSign{ 1.0f };  // sign relative to cross(forward, up)
		};

		// Starfield 1.16.244: the camera's rotation stores side / forward / up as rows 0 / 1 / 2.
		// (Skyrim stores the same axes as columns.) Measured from the vanilla dialogue camera
		// across eight conversations at different headings; Calibrate() keeps checking it.
		std::optional<Axes> g_axes = Axes{ .columns = false, .forward = 1, .up = 2, .side = 0 };

		RE::NiPoint3 Vector(const RE::NiMatrix3& a_m, bool a_columns, int a_i)
		{
			return a_columns ?
				RE::NiPoint3{ a_m.entry[0][a_i], a_m.entry[1][a_i], a_m.entry[2][a_i] } :
				RE::NiPoint3{ a_m.entry[a_i][0], a_m.entry[a_i][1], a_m.entry[a_i][2] };
		}

		void SetVector(RE::NiMatrix3& a_m, bool a_columns, int a_i, const RE::NiPoint3& a_v)
		{
			if (a_columns) {
				a_m.entry[0][a_i] = a_v.x;
				a_m.entry[1][a_i] = a_v.y;
				a_m.entry[2][a_i] = a_v.z;
			} else {
				a_m.entry[a_i][0] = a_v.x;
				a_m.entry[a_i][1] = a_v.y;
				a_m.entry[a_i][2] = a_v.z;
			}
		}

		float Dot(const RE::NiPoint3& a, const RE::NiPoint3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
	}

	bool IsCalibrated() { return g_axes.has_value(); }

	bool Calibrate(RE::NiAVObject* a_root, const RE::NiPoint3& a_speakerHead)
	{
		if (!a_root) {
			return IsCalibrated();
		}
		const auto& m = a_root->world.rotate;
		const auto  toSpeaker = Normalized(Sub(a_speakerHead, a_root->world.translate));
		if (!toSpeaker) {
			return IsCalibrated();
		}
		const RE::NiPoint3 worldUp{ 0.0f, 0.0f, 1.0f };

		// Try both storage layouts; keep the one whose best axis points most directly at the speaker.
		Axes  best{};
		float bestForward = -1.0f;
		float bestUp = -1.0f;
		for (const bool columns : { true, false }) {
			int   f = 0;
			float fDot = 0.0f;
			for (int i = 0; i < 3; ++i) {
				const float d = Dot(Vector(m, columns, i), *toSpeaker);
				if (std::abs(d) > std::abs(fDot)) {
					fDot = d;
					f = i;
				}
			}
			int   u = -1;
			float uDot = 0.0f;
			for (int i = 0; i < 3; ++i) {
				if (i == f) {
					continue;
				}
				const float d = Dot(Vector(m, columns, i), worldUp);
				if (u < 0 || std::abs(d) > std::abs(uDot)) {
					uDot = d;
					u = i;
				}
			}
			if (std::abs(fDot) > bestForward) {
				bestForward = std::abs(fDot);
				bestUp = std::abs(uDot);
				best.columns = columns;
				best.forward = f;
				best.up = u;
				best.side = 3 - f - u;
				best.forwardSign = fDot >= 0.0f ? 1.0f : -1.0f;
				best.upSign = uDot >= 0.0f ? 1.0f : -1.0f;
				const auto fv = Scale(Vector(m, columns, f), best.forwardSign);
				const auto uv = Scale(Vector(m, columns, u), best.upSign);
				best.sideSign = Dot(Vector(m, columns, best.side), Cross(fv, uv)) >= 0.0f ? 1.0f : -1.0f;
			}
		}

		// The vanilla camera points at the speaker's face, so the facing axis should line up
		// closely. Below this, the game probably wasn't aimed at them yet; try again next time.
		constexpr float kMinForward = 0.85f;
		constexpr float kMinUp = 0.6f;
		const bool      confident = bestForward >= kMinForward && bestUp >= kMinUp;

		REX::INFO("Camera axes: {} forward={}{} up={}{} side={}{} (forward match {:.2f}, up match {:.2f}) {}",
			best.columns ? "columns" : "rows",
			best.forwardSign > 0 ? "+" : "-", best.forward,
			best.upSign > 0 ? "+" : "-", best.up,
			best.sideSign > 0 ? "+" : "-", best.side,
			bestForward, bestUp,
			!confident ? "(not confident)" : "");
		REX::INFO("Vanilla camera at ({:.2f}, {:.2f}, {:.2f}), speaker head at ({:.2f}, {:.2f}, {:.2f})",
			a_root->world.translate.x, a_root->world.translate.y, a_root->world.translate.z,
			a_speakerHead.x, a_speakerHead.y, a_speakerHead.z);

		// Measurement only. At some headings (e.g. exactly diagonal) the row and column
		// layouts look the same, so a single reading can't be trusted to override the default.
		if (confident && g_axes) {
			const auto& a = *g_axes;
			const bool  same = a.columns == best.columns && a.forward == best.forward && a.up == best.up &&
			                  a.forwardSign == best.forwardSign && a.upSign == best.upSign && a.sideSign == best.sideSign;
			if (same) {
				REX::INFO("Camera axes match the built-in Starfield layout");
			} else {
				REX::WARN("Camera axes differ from the built-in layout this time (can happen at diagonal headings); keeping the built-in layout");
			}
		}
		return IsCalibrated();
	}

	namespace
	{
		// Rotation for a camera at a_pose looking at its target, in Starfield's layout.
		std::optional<RE::NiMatrix3> BuildRotation(const Pose& a_pose)
		{
			if (!g_axes) {
				return std::nullopt;
			}
			const auto forward = Normalized(Sub(a_pose.lookAt, a_pose.position));
			if (!forward) {
				return std::nullopt;
			}
			const RE::NiPoint3 worldUp = std::abs(forward->z) > 0.9999f ? RE::NiPoint3{ 0.0f, 1.0f, 0.0f } : RE::NiPoint3{ 0.0f, 0.0f, 1.0f };
			const auto         right = Normalized(Cross(*forward, worldUp));
			if (!right) {
				return std::nullopt;
			}
			const auto up = Cross(*right, *forward);

			const auto&   ax = *g_axes;
			RE::NiMatrix3 rot{};
			SetVector(rot, ax.columns, ax.forward, Scale(*forward, ax.forwardSign));
			SetVector(rot, ax.columns, ax.up, Scale(up, ax.upSign));
			SetVector(rot, ax.columns, ax.side, Scale(Cross(*forward, up), ax.sideSign));
			return rot;
		}

		// Render-camera override state. We only ever change the render camera's local
		// transform, so restoring that one value puts everything back.
		RE::NiAVObject* g_renderCam = nullptr;
		RE::NiTransform g_savedRenderLocal{};
		RE::NiTransform g_rootToRender{};  // render camera relative to the camera root

		float Distance(const RE::NiPoint3& a, const RE::NiPoint3& b) { return Length(Sub(a, b)); }
	}

	bool Apply(RE::NiAVObject* a_root, const Pose& a_pose, float a_delta)
	{
		if (!a_root) {
			return false;
		}
		const auto rot = BuildRotation(a_pose);
		if (!rot) {
			return false;
		}

		const auto mode = Settings::Get().poseMode;

		a_root->world.translate = a_pose.position;
		a_root->world.rotate = *rot;
		if (mode == 2) {
			return true;
		}
		a_root->local.translate = a_pose.position;
		a_root->local.rotate = *rot;
		if (mode == 1) {
			return true;
		}
		RE::NiUpdateData update{};
		update.delta = a_delta;
		a_root->Update(&update);
		return true;
	}

	bool ApplyRender(RE::NiAVObject* a_root, const Pose& a_pose, float a_delta)
	{
		if (!a_root) {
			return false;
		}
		const auto rot = BuildRotation(a_pose);
		if (!rot) {
			return false;
		}
		auto  camPtr = RE::Main::GetWorldRootCamera();
		auto* cam = static_cast<RE::NiAVObject*>(camPtr.get());
		if (!cam) {
			return false;
		}

		if (g_renderCam != cam) {
			RestoreRender();
			g_renderCam = cam;
			g_savedRenderLocal = cam->local;
			// If the render camera hangs off the camera root (expected), its offset from the
			// root is exactly its local transform. Otherwise measure it from world transforms.
			g_rootToRender = cam->parent == a_root ? cam->local : a_root->world.Inverse() * cam->world;

			const auto composed = cam->parent ? cam->parent->world * cam->local : cam->local;
			REX::INFO("Render camera {:p}: parent {:p} ({}), local offset ({:.3f}, {:.3f}, {:.3f}), parent*local vs world error {:.4f}",
				static_cast<void*>(cam), static_cast<void*>(cam->parent),
				cam->parent == a_root ? "camera root" : "other",
				cam->local.translate.x, cam->local.translate.y, cam->local.translate.z,
				Distance(composed.translate, cam->world.translate));
		}

		RE::NiTransform desiredRoot = a_root->world;
		desiredRoot.rotate = *rot;
		desiredRoot.translate = a_pose.position;
		const auto desiredCam = desiredRoot * g_rootToRender;

		cam->local = cam->parent ? cam->parent->world.Inverse() * desiredCam : desiredCam;
		cam->world = desiredCam;

		RE::NiUpdateData update{};
		update.delta = a_delta;
		cam->Update(&update);
		return true;
	}

	void RestoreRender()
	{
		if (g_renderCam) {
			g_renderCam->local = g_savedRenderLocal;
			if (g_renderCam->parent) {
				g_renderCam->world = g_renderCam->parent->world * g_savedRenderLocal;
			}
			RE::NiUpdateData update{};
			g_renderCam->Update(&update);
			REX::INFO("Render camera restored");
		}
		g_renderCam = nullptr;
	}
}
