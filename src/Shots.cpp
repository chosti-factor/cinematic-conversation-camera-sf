#include "Shots.h"

// Framing table and solver ported from Cinematic Conversation Camera by hashhbbrown
// (src/Camera/Shot.cpp, GPL-3.0-or-later). Skyrim units are converted to metres
// (70 units = 1 m).

namespace CCC::Shots
{
	namespace
	{
		constexpr float kPi = 3.14159265358979323846f;
		constexpr float kDeg = kPi / 180.0f;
		constexpr float kUnit = 1.0f / 70.0f;  // one Skyrim unit in metres

		// A person's head-and-shoulders extent (CCC: 42 units), and the closest the camera
		// may stand (68 units).
		constexpr float kExtent = 42.0f * kUnit;
		constexpr float kMinDistance = 68.0f * kUnit;
		constexpr float kHeadShare = 0.55f;  // extreme close-ups are sized on the head alone

		struct Spec
		{
			bool   onNpc;
			float  fill;      // share of the frame height the subject fills
			float  angleDeg;  // degrees off the eyeline
			float  rise;      // Skyrim units above eye level
			bool   overShoulder;
			Anchor anchor;
			bool   aimMidpoint;
			Move   move;
			float  moveAmount;  // CCC units: share of standoff, degrees, or Skyrim units
			float  headroom;
			float  lookRoom;
		};

		using A = Anchor;
		using M = Move;

		constexpr Spec SpecFor(ShotType a_type)
		{
			switch (a_type) {
			//                                      onNpc  fill   angle   rise   OTS    anchor      midAim move          amt     head   look
			case ShotType::kOverPlayerShoulder:     return { true,  0.42f, 20.0f,   6.0f, true,  A::kSubject, false, M::kLocked,    0.00f,  0.12f, 0.22f };
			case ShotType::kCloseUp:                return { true,  0.68f, 26.0f,   2.0f, false, A::kSubject, false, M::kLocked,    0.00f,  0.16f, 0.14f };
			case ShotType::kMediumNpc:              return { true,  0.40f, 34.0f,   4.0f, false, A::kSubject, false, M::kDrift,     8.0f,   0.14f, 0.16f };
			case ShotType::kLongNpc:                return { true,  0.16f, 42.0f,  16.0f, false, A::kSubject, false, M::kCraneUp,   34.0f, -0.10f, 0.12f };
			case ShotType::kLowAngle:               return { true,  0.55f, 30.0f, -26.0f, false, A::kSubject, false, M::kTiltUp,    18.0f,  0.10f, 0.14f };
			case ShotType::kCloseProfile:           return { true,  0.62f, 72.0f,   1.0f, false, A::kSubject, false, M::kLocked,    0.00f,  0.14f, 0.20f };
			case ShotType::kCloseLow:               return { true,  0.66f, 44.0f, -18.0f, false, A::kSubject, false, M::kPushIn,    0.14f,  0.12f, 0.14f };
			case ShotType::kCloseHigh:              return { true,  0.60f, 50.0f,  26.0f, false, A::kSubject, false, M::kTiltDown,  14.0f,  0.10f, 0.14f };
			case ShotType::kCloseWide:              return { true,  0.30f, 58.0f,   8.0f, false, A::kSubject, false, M::kPushIn,    0.12f,  0.08f, 0.18f };
			case ShotType::kExtremeClose:           return { true,  0.85f, 22.0f,   1.0f, false, A::kSubject, false, M::kLocked,    0.00f,  0.10f, 0.08f };
			case ShotType::kDirtyNpc:               return { true,  0.58f, 15.0f,   3.0f, true,  A::kSubject, false, M::kLocked,    0.00f,  0.14f, 0.24f };
			case ShotType::kThreeQuarterNpc:        return { true,  0.46f, 40.0f,   5.0f, false, A::kSubject, false, M::kDrift,     10.0f,  0.14f, 0.18f };
			case ShotType::kMediumProfile:          return { true,  0.34f, 66.0f,   4.0f, false, A::kSubject, false, M::kLocked,    0.00f,  0.12f, 0.22f };
			case ShotType::kLowProfile:             return { true,  0.50f, 78.0f, -22.0f, false, A::kSubject, false, M::kTiltUp,    16.0f,  0.08f, 0.20f };
			case ShotType::kOverhead:               return { true,  0.28f, 40.0f,  62.0f, false, A::kSubject, false, M::kCraneDown, 26.0f,  0.00f, 0.10f };
			case ShotType::kOverPlayerShoulderLow:  return { true,  0.46f, 22.0f, -26.0f, true,  A::kSubject, false, M::kPushIn,    0.12f,  0.10f, 0.22f };
			case ShotType::kOverPlayerShoulderHigh: return { true,  0.40f, 24.0f,  34.0f, true,  A::kSubject, false, M::kTiltDown,  16.0f,  0.10f, 0.22f };
			case ShotType::kOverPlayerShoulderWide: return { true,  0.24f, 28.0f,  12.0f, true,  A::kSubject, false, M::kPullOut,   0.14f,  0.02f, 0.18f };

			case ShotType::kOverNpcShoulder:        return { false, 0.40f, 20.0f,   6.0f, true,  A::kSubject, false, M::kLocked,    0.00f,  0.12f, 0.22f };
			case ShotType::kMediumPlayer:           return { false, 0.38f, 34.0f,   4.0f, false, A::kSubject, false, M::kDrift,     8.0f,   0.14f, 0.16f };
			case ShotType::kHighAngle:              return { false, 0.34f, 30.0f,  38.0f, false, A::kSubject, false, M::kCraneDown, 24.0f,  0.06f, 0.14f };
			case ShotType::kClosePlayer:            return { false, 0.60f, 26.0f,   2.0f, false, A::kSubject, false, M::kLocked,    0.00f,  0.16f, 0.14f };
			case ShotType::kExtremeClosePlayer:     return { false, 0.85f, 22.0f,   1.0f, false, A::kSubject, false, M::kLocked,    0.00f,  0.10f, 0.08f };
			case ShotType::kDirtyPlayer:            return { false, 0.55f, 15.0f,   3.0f, true,  A::kSubject, false, M::kLocked,    0.00f,  0.14f, 0.24f };
			case ShotType::kPlayerProfile:          return { false, 0.40f, 70.0f,   2.0f, false, A::kSubject, false, M::kLocked,    0.00f,  0.12f, 0.22f };
			case ShotType::kPlayerLow:              return { false, 0.52f, 32.0f, -20.0f, false, A::kSubject, false, M::kTiltUp,    16.0f,  0.10f, 0.14f };
			case ShotType::kThreeQuarterPlayer:     return { false, 0.44f, 40.0f,   5.0f, false, A::kSubject, false, M::kDrift,     10.0f,  0.14f, 0.18f };
			case ShotType::kOverNpcShoulderLow:     return { false, 0.44f, 22.0f, -26.0f, true,  A::kSubject, false, M::kPushIn,    0.12f,  0.10f, 0.22f };
			case ShotType::kOverNpcShoulderHigh:    return { false, 0.38f, 24.0f,  34.0f, true,  A::kSubject, false, M::kTiltDown,  16.0f,  0.10f, 0.22f };
			case ShotType::kOverNpcShoulderWide:    return { false, 0.24f, 28.0f,  12.0f, true,  A::kSubject, false, M::kPullOut,   0.14f,  0.02f, 0.18f };
			case ShotType::kLongPlayer:             return { false, 0.16f, 42.0f,  16.0f, false, A::kSubject, false, M::kCraneUp,   34.0f, -0.10f, 0.12f };
			case ShotType::kPlayerOverhead:         return { false, 0.28f, 40.0f,  62.0f, false, A::kSubject, false, M::kCraneDown, 26.0f,  0.00f, 0.10f };

			case ShotType::kTwoShot:                return { true,  0.20f, 90.0f,   8.0f, false, A::kMidpoint, true, M::kDrift,     7.0f,   0.06f, 0.00f };
			case ShotType::kProfile:                return { true,  0.30f, 90.0f,   2.0f, false, A::kMidpoint, true, M::kLocked,    0.00f,  0.08f, 0.00f };
			case ShotType::kWide:                   return { true,  0.11f,  0.0f,  24.0f, false, A::kScene,    false, M::kPullOut,  0.14f, -0.24f, 0.10f };
			case ShotType::kMaster:                 return { true,  0.075f, 24.0f, 76.0f, false, A::kScene,    false, M::kCraneUp,  46.0f, -0.30f, 0.14f };
			case ShotType::kGroundLevel:            return { true,  0.24f, -20.0f, -46.0f, false, A::kScene,   true,  M::kTiltUp,   24.0f, -0.12f, 0.00f };
			case ShotType::kDistant:                return { true,  0.09f,  0.0f,  28.0f, false, A::kScene,    true,  M::kLocked,   0.00f, -0.08f, 0.00f };
			case ShotType::kDistantLow:             return { true,  0.10f, 32.0f, -24.0f, false, A::kScene,    true,  M::kDrift,    6.0f,  -0.06f, 0.00f };
			default:                                return { true,  0.42f, 20.0f,   6.0f, true,  A::kSubject, false, M::kLocked,    0.00f,  0.12f, 0.18f };
			}
		}

		constexpr std::array<const char*, static_cast<std::size_t>(ShotType::kCount)> kKeys{
			"OverPlayerShoulder", "CloseUp", "MediumNpc", "LongNpc", "LowAngle", "CloseProfile", "CloseLow", "CloseHigh",
			"CloseWide", "ExtremeClose", "DirtyNpc", "ThreeQuarterNpc", "MediumProfile", "LowProfile", "Overhead",
			"OverPlayerShoulderLow", "OverPlayerShoulderHigh", "OverPlayerShoulderWide",
			"OverNpcShoulder", "MediumPlayer", "HighAngle", "ClosePlayer", "ExtremeClosePlayer", "DirtyPlayer",
			"PlayerProfile", "PlayerLow", "ThreeQuarterPlayer", "OverNpcShoulderLow", "OverNpcShoulderHigh",
			"OverNpcShoulderWide", "LongPlayer", "PlayerOverhead",
			"TwoShot", "Profile", "Wide", "Distant", "Master", "GroundLevel", "DistantLow"
		};

		constexpr std::array<const char*, static_cast<std::size_t>(Move::kCount)> kMoveNames{
			"None", "Push in", "Pull out", "Crane up", "Crane down", "Tilt up", "Tilt down", "Drift",
			"Zoom in", "Zoom out", "Orbit left", "Orbit right", "Slide left", "Slide right"
		};

		// Full travel of each move at amount 100, in CCC's units.
		constexpr float FullScale(Move a_move)
		{
			switch (a_move) {
			case M::kPushIn:
			case M::kPullOut:
			case M::kZoomIn:
			case M::kZoomOut:
				return 0.40f;  // share of the standoff
			case M::kCraneUp:
			case M::kCraneDown:
			case M::kTiltUp:
			case M::kTiltDown:
			case M::kTruckLeft:
			case M::kTruckRight:
				return 120.0f;  // Skyrim units
			case M::kDrift:
			case M::kOrbitLeft:
			case M::kOrbitRight:
				return 45.0f;  // degrees
			default:
				return 0.0f;
			}
		}

		float Length(const RE::NiPoint3& a_v) { return std::sqrt(a_v.x * a_v.x + a_v.y * a_v.y + a_v.z * a_v.z); }

		RE::NiPoint3 Cross(const RE::NiPoint3& a, const RE::NiPoint3& b)
		{
			return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
		}

		RE::NiPoint3 RotateAboutZ(const RE::NiPoint3& a_v, float a_radians)
		{
			const float c = std::cos(a_radians), s = std::sin(a_radians);
			return { a_v.x * c - a_v.y * s, a_v.x * s + a_v.y * c, a_v.z };
		}

		float Ease(float a_t)
		{
			const float t = std::clamp(a_t, 0.0f, 1.0f);
			return 1.0f - (1.0f - t) * (1.0f - t);
		}

		// Distance at which an extent fills a_fill of the frame height (CCC's DistanceForFill).
		float DistanceForFill(float a_fill, float a_lensDeg, float a_aspect, float a_extent)
		{
			const float horizontal = std::clamp(a_lensDeg, 10.0f, 150.0f) * kDeg;
			const float vertical = 2.0f * std::atan(std::tan(horizontal * 0.5f) / std::clamp(a_aspect, 1.0f, 3.0f));
			const float t = std::tan(vertical * std::clamp(a_fill, 0.04f, 0.95f) * 0.5f);
			return t > 1.0e-4f ? (std::max)((a_extent * 0.5f) / t, kMinDistance) : 3.0f;
		}

		// Aim point offset so the subject sits off centre: headroom above, look room on the
		// side they face (CCC's Compose).
		RE::NiPoint3 Compose(const RE::NiPoint3& a_position, const RE::NiPoint3& a_target, const RE::NiPoint3& a_facing,
			float a_lensDeg, float a_aspect, float a_headroom, float a_lookRoom, float& a_lookSign)
		{
			RE::NiPoint3 forward{ a_target.x - a_position.x, a_target.y - a_position.y, a_target.z - a_position.z };
			const float  distance = Length(forward);
			if (distance < 1.0e-3f) {
				return a_target;
			}
			forward = { forward.x / distance, forward.y / distance, forward.z / distance };
			RE::NiPoint3 right = Cross(forward, { 0.0f, 0.0f, 1.0f });
			const float  rl = Length(right);
			if (rl < 1.0e-3f) {
				return a_target;
			}
			right = { right.x / rl, right.y / rl, right.z / rl };
			const auto  up = Cross(right, forward);
			const float horizontal = std::clamp(a_lensDeg, 10.0f, 150.0f) * kDeg;
			const float vertical = 2.0f * std::atan(std::tan(horizontal * 0.5f) / std::clamp(a_aspect, 1.0f, 3.0f));
			const float halfWidth = distance * std::tan(horizontal * 0.5f);
			const float halfHeight = distance * std::tan(vertical * 0.5f);
			if (a_lookSign == 0.0f) {
				const float facing = right.x * a_facing.x + right.y * a_facing.y + right.z * a_facing.z;
				a_lookSign = facing > 0.0f ? -1.0f : 1.0f;
			}
			const float offsetX = a_lookRoom * a_lookSign * halfWidth;
			const float offsetY = a_headroom * halfHeight;
			return { a_target.x - right.x * offsetX - up.x * offsetY,
				a_target.y - right.y * offsetX - up.y * offsetY,
				a_target.z - right.z * offsetX - up.z * offsetY };
		}
	}

	const char* Key(ShotType a_type) noexcept
	{
		const auto i = static_cast<std::size_t>(a_type);
		return i < kKeys.size() ? kKeys[i] : "Unknown";
	}

	std::optional<ShotType> FromKey(std::string_view a_key) noexcept
	{
		for (std::size_t i = 0; i < kKeys.size(); ++i) {
			const std::string_view k{ kKeys[i] };
			if (k.size() == a_key.size() && std::equal(k.begin(), k.end(), a_key.begin(), [](char a, char b) {
					return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
				})) {
				return static_cast<ShotType>(i);
			}
		}
		return std::nullopt;
	}

	bool   OnNpc(ShotType a_type) noexcept { return SpecFor(a_type).onNpc; }

	bool FollowsFace(ShotType a_type) noexcept
	{
		const auto spec = SpecFor(a_type);
		return spec.anchor == Anchor::kSubject && !spec.aimMidpoint && !spec.overShoulder && spec.fill >= 0.5f;
	}
	bool   OverShoulder(ShotType a_type) noexcept { return SpecFor(a_type).overShoulder; }
	Anchor AnchorOf(ShotType a_type) noexcept { return SpecFor(a_type).anchor; }
	bool   Neutral(ShotType a_type) noexcept { return SpecFor(a_type).anchor != Anchor::kSubject; }
	Move   DefaultMove(ShotType a_type) noexcept { return SpecFor(a_type).move; }

	int DefaultMoveAmount(ShotType a_type) noexcept
	{
		const auto  spec = SpecFor(a_type);
		const float full = FullScale(spec.move);
		return full > 0.0f ? static_cast<int>(std::clamp(spec.moveAmount / full * 100.0f, 0.0f, 100.0f)) : 35;
	}

	const char* MoveName(Move a_move) noexcept
	{
		const auto i = static_cast<std::size_t>(a_move);
		return i < kMoveNames.size() ? kMoveNames[i] : "None";
	}

	std::optional<Move> MoveFromName(std::string_view a_name) noexcept
	{
		auto squash = [](std::string_view a_s) {
			std::string out;
			for (const char c : a_s) {
				if (c != ' ' && c != '_' && c != '-') {
					out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
				}
			}
			return out;
		};
		const auto wanted = squash(a_name);
		for (std::size_t i = 0; i < kMoveNames.size(); ++i) {
			if (squash(kMoveNames[i]) == wanted) {
				return static_cast<Move>(i);
			}
		}
		if (wanted == "locked" || wanted == "off") {
			return M::kLocked;
		}
		return std::nullopt;
	}

	Result Solve(ShotType a_type, const Input& a_in)
	{
		Result     result;
		const auto spec = SpecFor(a_type);
		if (spec.anchor == Anchor::kScene) {
			return result;  // needs a measurement of the room
		}

		const auto& subject = spec.onNpc ? a_in.npcHead : a_in.playerHead;
		const auto& other = spec.onNpc ? a_in.playerHead : a_in.npcHead;
		RE::NiPoint3 toOther{ other.x - subject.x, other.y - subject.y, other.z - subject.z };
		const float  separation = Length(toOther);
		if (separation < 0.05f) {
			return result;
		}
		// The eyeline's bearing from where the two stand, not from the swaying heads (its
		// length and height difference still come from the heads).
		if (a_in.feetValid) {
			const auto& sf = spec.onNpc ? a_in.npcFeet : a_in.playerFeet;
			const auto& of = spec.onNpc ? a_in.playerFeet : a_in.npcFeet;
			const float fx = of.x - sf.x, fy = of.y - sf.y;
			const float fl = std::sqrt(fx * fx + fy * fy);
			const float hl = std::sqrt(toOther.x * toOther.x + toOther.y * toOther.y);
			const float w = std::clamp(a_in.sway, 0.0f, 1.0f);
			if (fl > 0.3f && hl > 0.05f && w < 1.0f) {
				// Blend the two bearings: w of the heads' (swaying), the rest from the feet.
				const float bx = fx / fl * (1.0f - w) + toOther.x / hl * w;
				const float by = fy / fl * (1.0f - w) + toOther.y / hl * w;
				const float bl = std::sqrt(bx * bx + by * by);
				if (bl > 1.0e-3f) {
					toOther.x = bx / bl * hl;
					toOther.y = by / bl * hl;
				}
			}
		}
		toOther = { toOther.x / separation, toOther.y / separation, toOther.z / separation };
		const RE::NiPoint3 midpoint{ (a_in.npcHead.x + a_in.playerHead.x) * 0.5f, (a_in.npcHead.y + a_in.playerHead.y) * 0.5f,
			(a_in.npcHead.z + a_in.playerHead.z) * 0.5f };

		// Distance comes from how much of the frame the subject should fill, at this lens.
		float extent = kExtent;
		if (a_type == ShotType::kExtremeClose || a_type == ShotType::kExtremeClosePlayer) {
			extent *= kHeadShare;
		}
		float distance = DistanceForFill(spec.fill, a_in.lensDegrees, a_in.aspect, extent);
		result.wantedDistance = distance;

		// Two people side by side need the frame wide enough for both; past the cap that
		// framing can't be had, so the setup is skipped rather than cropped.
		if (spec.anchor == Anchor::kMidpoint && distance > a_in.maxDistance * 1.25f) {
			return result;
		}

		const float lineSide = (a_in.true180 && !spec.onNpc) ? -(a_in.side < 0.0f ? -1.0f : 1.0f) : (a_in.side < 0.0f ? -1.0f : 1.0f);
		float       rise = spec.rise * kUnit;
		float       angle = spec.angleDeg;

		// Close-ups that follow the face (CCC's FaceFrame): turn toward where the face points,
		// half of its bearing off the eyeline and at most 35 degrees, and drop to look up into a
		// lowered face (60% of its pitch, -30 to +10 degrees). Decided at the cut, then held.
		const bool         faced = a_in.followFace && FollowsFace(a_type) && (spec.onNpc ? a_in.npcFaceValid : a_in.playerFaceValid);
		float              faceYaw = 0.0f;
		float              facePitch = 0.0f;
		if (faced) {
			const auto& f = spec.onNpc ? a_in.npcFacing : a_in.playerFacing;
			RE::NiPoint3 eyeline{ toOther.x, toOther.y, 0.0f };
			RE::NiPoint3 flat{ f.x, f.y, 0.0f };
			const float  el = Length(eyeline), fl = Length(flat);
			if (el > 1.0e-3f && fl > 0.25f) {
				eyeline = { eyeline.x / el, eyeline.y / el, 0.0f };
				flat = { flat.x / fl, flat.y / fl, 0.0f };
				const float cross = eyeline.x * flat.y - eyeline.y * flat.x;
				const float dot = eyeline.x * flat.x + eyeline.y * flat.y;
				faceYaw = std::clamp(std::atan2(cross, dot) / kDeg * 0.5f, -35.0f, 35.0f);
			}
			facePitch = std::clamp(std::asin(std::clamp(f.z, -1.0f, 1.0f)) / kDeg * 0.6f, -30.0f, 10.0f);
			if (!std::isnan(a_in.heldFacePitch)) {
				facePitch = a_in.heldFacePitch;
			}
		}
		result.faced = faced;
		result.faceYaw = faceYaw;
		result.facePitch = facePitch;
		// Continuity limits (CCC): singles stay at least 8 degrees off the eyeline, shots of
		// the pair at least 34, and nothing swings round past 165.
		const float lineFloor = spec.anchor == Anchor::kMidpoint ? 34.0f : 8.0f;

		// Over-the-shoulder setups: CCC's angles assume a 40-60 degree lens. Through
		// Starfield's 30 the near person would fall outside the frame, so swing the camera
		// in toward the eyeline until their shoulder sits in the edge of the picture.
		if (!std::isnan(a_in.heldAngle)) {
			angle = a_in.heldAngle;
		} else if (faced) {
			// The turn is applied on the same side of the line as the setup's own angle.
			angle += faceYaw * lineSide;
			if (a_in.enforceLine) {
				angle = std::clamp(angle, lineFloor, 165.0f);
			}
		} else if (spec.overShoulder) {
			float standoff = std::clamp(distance, kMinDistance, (std::max)(a_in.maxDistance, kMinDistance));
			standoff = (std::max)(standoff, separation + 60.0f * kUnit);
			const float half = std::clamp(a_in.lensDegrees, 10.0f, 150.0f) * 0.5f;
			for (; angle > 2.0f; angle -= 0.25f) {
				// Camera on the far side of the other person, `angle` off the line; both in 2D.
				const float rad = angle * kDeg;
				const float cx = std::cos(rad) * standoff, cy = std::sin(rad) * standoff;  // subject at origin, other at (separation, 0)
				const float toSubject = std::atan2(-cy, -cx);
				const float toOther = std::atan2(-cy, separation - cx);
				float       off = std::abs(toOther - toSubject) / kDeg;
				if (off > 180.0f) {
					off = 360.0f - off;
				}
				if (off <= half * 0.8f) {
					break;
				}
			}
			// ...but never so close to the eyeline that the near person's head and shoulders
			// cover the subject (0.4.9 logs: 4-5 degrees at a shop counter, the merchant
			// hidden behind your shoulder). Keep the line to the subject this far beside the
			// near person's centre; their shoulder may then leave the frame, a clean single.
			const float clearance = 0.38f;
			const float minAngle = std::asin((std::min)(1.0f, clearance / separation)) / kDeg;
			angle = (std::max)(angle, (std::min)(minAngle, 40.0f));
		}
		float       aimLift = 0.0f;
		float       truck = 0.0f;

		result.angle = angle;
		const float p = Ease(a_in.progress);
		const float travel = FullScale(a_in.move) * std::clamp(a_in.moveAmount, 0, 100) / 100.0f;
		const float units = travel * kUnit;
		switch (a_in.move) {
		case M::kPushIn:
		case M::kZoomIn:
			distance *= 1.0f - travel * p;
			break;
		case M::kPullOut:
		case M::kZoomOut:
			distance *= 1.0f + travel * p;
			break;
		case M::kCraneUp:
			rise += units * p;
			break;
		case M::kCraneDown:
			rise -= units * p;
			break;
		case M::kTiltUp:
			aimLift = units * p;
			break;
		case M::kTiltDown:
			aimLift = -units * p;
			break;
		case M::kDrift:
		case M::kOrbitRight:
			angle += travel * p;
			break;
		case M::kOrbitLeft:
			angle -= travel * p;
			break;
		case M::kTruckLeft:
			truck = -units * p;
			break;
		case M::kTruckRight:
			truck = units * p;
			break;
		default:
			break;
		}

		// A move (drift, orbit) never carries the camera across the eyeline either.
		if (a_in.enforceLine && !spec.overShoulder && a_in.move != M::kLocked) {
			angle = std::clamp(angle, lineFloor, 165.0f);
		}

		// The furthest the camera may stand (per preset; obstacles are handled outside).
		distance = std::clamp(distance, kMinDistance, (std::max)(a_in.maxDistance, kMinDistance));
		// Drop for a lowered face, by the height that puts the lens on its line (after the
		// distance is known).
		if (faced) {
			rise += distance * std::sin(facePitch * kDeg);
		}
		if (spec.overShoulder) {
			// Past the other person, so their shoulder is in the corner of the frame.
			distance = (std::max)(distance, separation + 60.0f * kUnit);
		}
		result.distance = distance;

		const auto& anchorPoint = spec.anchor == Anchor::kSubject ? subject : midpoint;
		const auto& aimPoint = spec.aimMidpoint ? midpoint : subject;
		const RE::NiPoint3 target{ aimPoint.x, aimPoint.y, aimPoint.z + 3.0f * kUnit + aimLift };

		const auto direction = RotateAboutZ(toOther, angle * kDeg * lineSide);
		Camera::Pose pose;
		pose.position = { anchorPoint.x + direction.x * distance, anchorPoint.y + direction.y * distance,
			anchorPoint.z + direction.z * distance + rise };
		// Which side of the frame the subject sits on is decided from the setup's own angle
		// (CCC's are 15 degrees or more), not from a camera swung in close to the eyeline,
		// where a small sway flipped it and the picture jumped sideways.
		float lookSign = a_in.heldLookSign;
		if (lookSign == 0.0f) {
			const auto   refDir = RotateAboutZ(toOther, spec.angleDeg * kDeg * lineSide);
			const RE::NiPoint3 refPos{ anchorPoint.x + refDir.x * distance, anchorPoint.y + refDir.y * distance, anchorPoint.z + refDir.z * distance + rise };
			(void)Compose(refPos, target, toOther, a_in.lensDegrees, a_in.aspect, spec.headroom, spec.lookRoom, lookSign);
		}
		result.lookSign = lookSign;
		pose.lookAt = Compose(pose.position, target, toOther, a_in.lensDegrees, a_in.aspect, spec.headroom, spec.lookRoom, lookSign);
		if (truck != 0.0f) {
			RE::NiPoint3 fwd{ pose.lookAt.x - pose.position.x, pose.lookAt.y - pose.position.y, 0.0f };
			const float  fl = Length(fwd);
			if (fl > 1.0e-3f) {
				const RE::NiPoint3 right{ fwd.y / fl, -fwd.x / fl, 0.0f };
				pose.position = { pose.position.x + right.x * truck, pose.position.y + right.y * truck, pose.position.z };
				pose.lookAt = { pose.lookAt.x + right.x * truck, pose.lookAt.y + right.y * truck, pose.lookAt.z };
			}
		}
		// Never below the floor the subject stands on (a lowered face on a seated person).
		const float ground = spec.onNpc ? a_in.npcGround : a_in.playerGround;
		if (faced && std::isfinite(ground) && pose.position.z < ground + 0.45f) {
			const float lift = ground + 0.45f - pose.position.z;
			pose.position.z += lift;
		}
		result.pose = pose;
		result.ok = std::isfinite(pose.position.x) && std::isfinite(pose.lookAt.x);
		return result;
	}
}
