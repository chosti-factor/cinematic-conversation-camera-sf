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

	bool HasHeadBone(RE::TESObjectREFR* a_ref)
	{
		auto* root = Root3D(a_ref);
		if (!root) {
			return false;
		}
		for (const auto* name : kHeadNodes) {
			if (root->GetObjectByName(RE::BSFixedString{ name })) {
				return true;
			}
		}
		return false;
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

	std::optional<Pose> Shot(const RE::NiPoint3& a_nearHead, const RE::NiPoint3& a_farHead, float a_behind, float a_side, float a_height)
	{
		// Work on the ground plane so the shot stays level even if heights differ.
		const auto flat = Normalized({ a_farHead.x - a_nearHead.x, a_farHead.y - a_nearHead.y, 0.0f });
		if (!flat) {
			return std::nullopt;
		}
		const RE::NiPoint3 up{ 0.0f, 0.0f, 1.0f };
		const auto         right = Cross(*flat, up);

		Pose pose;
		pose.position = Add(a_nearHead, Add(Scale(*flat, -a_behind), Add(Scale(right, a_side), Scale(up, a_height))));
		pose.lookAt = Add(a_farHead, Scale(up, Settings::Get().lookHeightOffset));
		return pose;
	}

	std::optional<Pose> OverShoulder(const RE::NiPoint3& a_nearHead, const RE::NiPoint3& a_farHead)
	{
		const auto& s = Settings::Get();
		return Shot(a_nearHead, a_farHead, s.distanceBehind, s.sideOffset, s.heightOffset);
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
		std::optional<RE::NiTransform> g_lastWritten;  // the render camera's world transform we last wrote
		RE::NiTransform g_savedRenderLocal{};

		float Distance(const RE::NiPoint3& a, const RE::NiPoint3& b) { return Length(Sub(a, b)); }

		// Reads an object's C++ class name from the MSVC RTTI that every polymorphic object
		// carries (vtable[-1] -> complete object locator -> type descriptor). This needs no
		// game-version-specific addresses. Returns "" if anything looks wrong.
		const char* ClassNameUnsafe(const void* a_obj)
		{
			const auto* vtbl = *reinterpret_cast<const std::uintptr_t* const*>(a_obj);
			const auto* col = reinterpret_cast<const std::uint32_t*>(vtbl[-1]);
			// x64 locator: signature(1), offset, cdOffset, typeDescriptor RVA, classDescriptor RVA, self RVA
			if (col[0] != 1) {
				return "";
			}
			const auto imageBase = reinterpret_cast<std::uintptr_t>(col) - col[5];
			const auto typeDescriptor = imageBase + col[3];
			return reinterpret_cast<const char*>(typeDescriptor + 16);  // skip pVFTable, spare
		}

		const char* ClassName(const void* a_obj)
		{
			if (!a_obj) {
				return "";
			}
			__try {
				return ClassNameUnsafe(a_obj);
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				return "";
			}
		}

		bool IsClass(const void* a_obj, const char* a_mangled)
		{
			const char* name = ClassName(a_obj);
			__try {
				return std::strcmp(name, a_mangled) == 0;
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				return false;
			}
		}

		// The render camera hangs off the camera root, possibly one node down.
		RE::NiAVObject* FindRenderCamera(RE::NiAVObject* a_root, bool a_log)
		{
			if (!a_root || (!IsClass(a_root, ".?AVNiNode@@") && !IsClass(a_root, ".?AVBSFadeNode@@"))) {
				if (a_log) {
					REX::WARN("Camera root is a {}, not a node; can't search it", ClassName(a_root));
				}
				return nullptr;
			}
			auto* node = static_cast<RE::NiNode*>(a_root);
			for (auto& childPtr : node->children) {
				auto* child = childPtr.get();
				if (!child) {
					continue;
				}
				if (a_log) {
					REX::INFO("  camera root child {:p}: {}", static_cast<void*>(child), ClassName(child));
				}
				if (IsClass(child, ".?AVNiCamera@@")) {
					return child;
				}
				if (IsClass(child, ".?AVNiNode@@")) {
					for (auto& grandPtr : static_cast<RE::NiNode*>(child)->children) {
						auto* grand = grandPtr.get();
						if (a_log && grand) {
							REX::INFO("    grandchild {:p}: {}", static_cast<void*>(grand), ClassName(grand));
						}
						if (grand && IsClass(grand, ".?AVNiCamera@@")) {
							return grand;
						}
					}
				}
			}
			return nullptr;
		}

		bool g_loggedCameraSearch = false;
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

	namespace
	{
		// How the engine combines a child's local rotation with its parent's world rotation.
		// Measured from the live camera each conversation rather than assumed.
		enum class Compose
		{
			kLocalTimesParent,  // world = local * parent (row layout)
			kParentTimesLocal   // world = parent * local (column layout)
		};
		Compose g_compose = Compose::kLocalTimesParent;

		float MatrixError(const RE::NiMatrix3& a, const RE::NiMatrix3& b)
		{
			float e = 0.0f;
			for (int r = 0; r < 3; ++r) {
				for (int c = 0; c < 3; ++c) {
					e += std::abs(a.entry[r][c] - b.entry[r][c]);
				}
			}
			return e;
		}

		std::string MatrixText(const RE::NiMatrix3& m)
		{
			return std::format("[{:.2f} {:.2f} {:.2f} | {:.2f} {:.2f} {:.2f} | {:.2f} {:.2f} {:.2f}]",
				m.entry[0][0], m.entry[0][1], m.entry[0][2],
				m.entry[1][0], m.entry[1][1], m.entry[1][2],
				m.entry[2][0], m.entry[2][1], m.entry[2][2]);
		}
	}

	namespace
	{
		// A node's world transform computed from its local transform right now. At the time
		// our camera hooks run, the game has set the camera root's local transform for this
		// frame but may not have refreshed its world transform yet; in dialogue that goes
		// unnoticed (the root barely moves), but when the game jumps between first person,
		// third person and dialogue, the stale value gave one wrongly aimed frame per jump.
		RE::NiTransform FreshWorld(const RE::NiAVObject* a_node)
		{
			const auto* parent = a_node->parent;
			if (!parent) {
				return a_node->local;
			}
			const auto&     pw = parent->world;
			RE::NiTransform out = a_node->world;
			out.rotate = a_node->local.rotate * pw.rotate;
			out.translate = Add(pw.translate, Scale(pw.rotate.Transpose() * a_node->local.translate, pw.scale));
			out.scale = pw.scale * a_node->local.scale;
			return out;
		}
	}

	namespace
	{
		// FreshWorld is only right if the game keeps the camera root's local transform up to
		// date. Don't assume: trust it only after it has matched the stored world transform
		// for a run of frames (which it will whenever the camera is steady), and use the
		// stored world transform until then.
		std::atomic_bool g_trustLocal{ false };
		std::uint32_t    g_localAgreeFrames = 0;

		RE::NiTransform RootWorld(const RE::NiAVObject* a_root)
		{
			const auto fresh = FreshWorld(a_root);
			if (!g_trustLocal.load(std::memory_order_acquire)) {
				const auto  d = Sub(fresh.translate, a_root->world.translate);
				const bool  agree = Length(d) < 0.02f && MatrixError(fresh.rotate, a_root->world.rotate) < 0.02f;
				g_localAgreeFrames = agree ? g_localAgreeFrames + 1 : 0;
				if (g_localAgreeFrames >= 20) {
					g_trustLocal.store(true, std::memory_order_release);
					REX::INFO("Camera root local transform is kept current by the game; using it for exact per-frame placement");
				}
				return a_root->world;
			}
			return fresh;
		}
	}

	std::string DescribeRoot(RE::NiAVObject* a_root)
	{
		if (!a_root) {
			return "no root";
		}
		const auto  fresh = FreshWorld(a_root);
		const auto& w = a_root->world.translate;
		const auto& l = a_root->local.translate;
		auto        text = std::format("root world ({:.2f} {:.2f} {:.2f}) local ({:.2f} {:.2f} {:.2f}) derived ({:.2f} {:.2f} {:.2f})",
				   w.x, w.y, w.z, l.x, l.y, l.z, fresh.translate.x, fresh.translate.y, fresh.translate.z);
		if (const auto* parent = a_root->parent) {
			const auto& pw = parent->world.translate;
			const auto& pl = parent->local.translate;
			text += std::format("; parent {:p} world ({:.2f} {:.2f} {:.2f}) local ({:.2f} {:.2f} {:.2f})", static_cast<const void*>(parent), pw.x, pw.y, pw.z, pl.x, pl.y, pl.z);
		} else {
			text += "; no parent";
		}
		if (g_renderCam) {
			const auto& cw = g_renderCam->world.translate;
			const auto& cl = g_renderCam->local.translate;
			text += std::format("; render camera world ({:.2f} {:.2f} {:.2f}) local ({:.2f} {:.2f} {:.2f})", cw.x, cw.y, cw.z, cl.x, cl.y, cl.z);
		}
		return text;
	}

	void* RenderCameraObject()
	{
		return g_renderCam;
	}

	std::optional<Pose> PoseOf(RE::NiAVObject* a_root)
	{
		if (!a_root || !g_axes) {
			return std::nullopt;
		}
		const auto world = RootWorld(a_root);
		const auto forward = Scale(Vector(world.rotate, g_axes->columns, g_axes->forward), g_axes->forwardSign);
		if (!Finite(world.translate) || !Finite(forward)) {
			return std::nullopt;
		}
		Pose pose;
		pose.position = world.translate;
		pose.lookAt = Add(world.translate, Scale(forward, 5.0f));
		return pose;
	}

	namespace
	{
		// The dialogue camera uses a much narrower field of view than the gameplay cameras.
		// When we hold a dialogue shot while the game is briefly in first or third person
		// (the body rebuild), the picture would visibly zoom out and back in (seen in the
		// user's 0.3.14 recording). Remember the dialogue camera's frustum and put it back on
		// those frames.
		std::optional<RE::NiFrustum> g_dialogueFrustum;
		bool                         g_loggedDialogueFrustum = false;
		bool                         g_loggedGameplayFrustum = false;
	}

	bool ApplyRender(RE::NiAVObject* a_root, const Pose& a_pose, float a_delta, bool a_holdDialogueFrustum, const Pose* a_predictedRoot)
	{
		(void)a_delta;
		if (!a_root) {
			return false;
		}
		const auto rot = BuildRotation(a_pose);  // the shot, in the camera ROOT's axis layout
		if (!rot) {
			return false;
		}
		// No address-library lookup here: Starfield 1.16.244 has no ID for the world root.
		auto* cam = g_renderCam ? g_renderCam : FindRenderCamera(a_root, !g_loggedCameraSearch);
		if (!g_renderCam && !g_loggedCameraSearch) {
			g_loggedCameraSearch = true;
			REX::INFO("Render camera search: {}", cam ? "found NiCamera under the camera root" : "not found");
		}
		if (!cam || cam->parent != a_root) {
			return false;
		}
		// Normally relative to where the camera root is now. Right after a camera-mode change
		// the game moves the root before our hook on the new mode gets a turn; the caller can
		// say where the root is about to be, so that frame still comes out right.
		auto pw = RootWorld(a_root);
		if (a_predictedRoot) {
			if (const auto predictedRot = BuildRotation(*a_predictedRoot)) {
				pw.rotate = *predictedRot;
				pw.translate = a_predictedRoot->position;
			}
		}

		if (g_renderCam != cam) {
			RestoreRender();
			g_renderCam = cam;
			g_savedRenderLocal = cam->local;

			// The render camera has its own axis layout: its local rotation relative to the
			// camera root is not identity (0.2.2 assumed it was and the view came out rotated
			// 90 degrees). Work out from the live values how the engine composes the two.
			const float errLP = MatrixError(cam->local.rotate * pw.rotate, cam->world.rotate);
			const float errPL = MatrixError(pw.rotate * cam->local.rotate, cam->world.rotate);
			// Always local*parent (Starfield's row layout). This used to be decided from these live
			// values, but on a conversation's first frame the camera root still holds the gameplay
			// camera, and when you face a particular way the two errors come out nearly equal:
			// about one conversation in five picked the wrong one, and every shot came out
			// rotated and swinging (the "flipped" merchant and Ularu conversations).
			g_compose = Compose::kLocalTimesParent;

			REX::INFO("Render camera {:p}: local offset ({:.3f}, {:.3f}, {:.3f}), compose error local*parent {:.3f}, parent*local {:.3f} -> using {}",
				static_cast<void*>(cam),
				cam->local.translate.x, cam->local.translate.y, cam->local.translate.z,
				errLP, errPL, "local*parent (fixed)");
			REX::INFO("  camera local rotation {}", MatrixText(cam->local.rotate));
			REX::INFO("  camera world rotation {}", MatrixText(cam->world.rotate));
			REX::INFO("  root   world rotation {}", MatrixText(pw.rotate));
		}

		// Pretend the camera root sat at the shot (rotation *rot, position a_pose.position)
		// and derive the render camera's local transform that produces the same picture
		// while the real root stays where the game put it. Transforms only: no scene-graph
		// update call from the hook's worker thread.
		const auto&        l0 = g_savedRenderLocal;
		const float        invScale = pw.scale > 1e-6f ? 1.0f / pw.scale : 1.0f;

		RE::NiTransform world = cam->world;
		if (g_compose == Compose::kLocalTimesParent) {
			world.rotate = l0.rotate * *rot;
			world.translate = Add(a_pose.position, rot->Transpose() * l0.translate);
			cam->local.rotate = world.rotate * pw.rotate.Transpose();
			cam->local.translate = Scale(pw.rotate * Sub(world.translate, pw.translate), invScale);
		} else {
			world.rotate = *rot * l0.rotate;
			world.translate = Add(a_pose.position, *rot * l0.translate);
			cam->local.rotate = pw.rotate.Transpose() * world.rotate;
			cam->local.translate = Scale(pw.rotate.Transpose() * Sub(world.translate, pw.translate), invScale);
		}
		cam->world = world;
		g_lastWritten = world;

		auto& frustum = static_cast<RE::NiCamera*>(cam)->viewFrustum;
		if (!a_holdDialogueFrustum) {
			if (frustum.right > 0.0f && frustum.top > 0.0f && !frustum.ortho) {
				g_dialogueFrustum = frustum;
				if (!g_loggedDialogueFrustum) {
					g_loggedDialogueFrustum = true;
					REX::INFO("Dialogue camera frustum: left {:.4f} right {:.4f} top {:.4f} bottom {:.4f}",
						frustum.left, frustum.right, frustum.top, frustum.bottom);
				}
			}
		} else if (g_dialogueFrustum) {
			if (!g_loggedGameplayFrustum) {
				g_loggedGameplayFrustum = true;
				REX::INFO("Gameplay camera frustum (being replaced while holding a dialogue shot): left {:.4f} right {:.4f} top {:.4f} bottom {:.4f}",
					frustum.left, frustum.right, frustum.top, frustum.bottom);
			}
			frustum.left = g_dialogueFrustum->left;
			frustum.right = g_dialogueFrustum->right;
			frustum.top = g_dialogueFrustum->top;
			frustum.bottom = g_dialogueFrustum->bottom;
		}
		return true;
	}

	void RestoreRender()
	{
		if (g_renderCam) {
			g_renderCam->local = g_savedRenderLocal;
			if (const auto* parent = g_renderCam->parent) {
				const auto pw = RootWorld(parent);
				if (g_compose == Compose::kLocalTimesParent) {
					g_renderCam->world.rotate = g_savedRenderLocal.rotate * pw.rotate;
					g_renderCam->world.translate = Add(pw.translate, Scale(pw.rotate.Transpose() * g_savedRenderLocal.translate, pw.scale));
				} else {
					g_renderCam->world.rotate = pw.rotate * g_savedRenderLocal.rotate;
					g_renderCam->world.translate = Add(pw.translate, Scale(pw.rotate * g_savedRenderLocal.translate, pw.scale));
				}
			}
			REX::INFO("Render camera restored");
		}
		g_renderCam = nullptr;
		g_lastWritten.reset();
	}

	std::optional<Drift> RenderDrift()
	{
		if (!g_renderCam || !g_lastWritten) {
			return std::nullopt;
		}
		const auto& w = g_renderCam->world;
		const auto& b = *g_lastWritten;
		Drift       d;
		d.metres = Length(Sub(w.translate, b.translate));
		float trace = 0.0f;  // trace(A * B^T): 1 + 2 cos(angle between the two rotations)
		for (int r = 0; r < 3; ++r) {
			for (int c = 0; c < 3; ++c) {
				trace += w.rotate.entry[r][c] * b.rotate.entry[r][c];
			}
		}
		d.degrees = std::acos(std::clamp((trace - 1.0f) * 0.5f, -1.0f, 1.0f)) * 57.29578f;
		return d;
	}
}
