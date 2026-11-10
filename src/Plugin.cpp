// Cinematic Conversation Camera SF
//
// Starfield port of Cinematic Conversation Camera by hashhbbrown
// (https://github.com/fatalCMD/ccc, GPL-3.0-or-later).
// Dialogue camera hook locations for Starfield 1.16.244 come from
// Player Camera During Dialogue by rrrafau
// (https://github.com/rrrafau1337/player-camera-during-dialogue, GPL-3.0-or-later).
//
// Milestone 1: prove we can take the dialogue camera, frame over-the-shoulder
// shot/reverse-shot, and give the camera back cleanly.

#include "Camera.h"
#include "Settings.h"

namespace CCC
{
	namespace
	{
		// --- Starfield 1.16.244 only -------------------------------------------------
		constexpr REL::Version         kSupportedRuntime{ 1, 16, 244, 0 };
		constexpr std::uintptr_t       kDialogueStateVtableRva = 0x4C27500;
		constexpr std::size_t          kDialogueUpdateVtableIndex = 12;
		// -----------------------------------------------------------------------------

		enum class Subject
		{
			kNone,
			kNpc,    // camera over the player's shoulder, NPC in frame
			kPlayer  // camera over the NPC's shoulder, player in frame
		};

		constexpr const char* SubjectName(Subject a_s)
		{
			switch (a_s) {
			case Subject::kNpc:
				return "NPC";
			case Subject::kPlayer:
				return "player";
			default:
				return "none";
			}
		}

		struct State
		{
			bool                                  active{ false };
			Subject                               subject{ Subject::kNone };
			std::chrono::steady_clock::time_point lastFrame{};
			std::chrono::steady_clock::time_point npcQuietSince{};
			bool                                  npcWasTalking{ false };
			std::uint32_t                         framesThisConversation{ 0 };
			std::uint32_t                         failuresLogged{ 0 };
		} g_state;

		// Short pause after an NPC line ends before cutting to the player,
		// so we don't flick back and forth between consecutive NPC lines.
		constexpr auto kPlayerCutDelay = std::chrono::milliseconds(350);

		bool g_hooksSafe = false;

		RE::TESObjectREFR* Speaker()
		{
			auto* topics = RE::MenuTopicManager::GetSingleton();
			if (!topics) {
				return nullptr;
			}
			auto speaker = topics->speaker.get();
			return speaker ? speaker.get() : nullptr;
		}

		void LogFailure(std::string_view a_what)
		{
			if (Settings::Get().debugLog && g_state.failuresLogged < 20) {
				++g_state.failuresLogged;
				REX::WARN("frame {}: {}", g_state.framesThisConversation, a_what);
			}
		}

		void Begin()
		{
			g_state = State{};
			g_state.active = true;
			g_state.lastFrame = std::chrono::steady_clock::now();
			REX::INFO("Conversation started");
		}

		void End()
		{
			if (g_state.active) {
				REX::INFO("Conversation ended after {} camera frames", g_state.framesThisConversation);
			}
			g_state = State{};
		}

		Subject ChooseSubject(RE::TESObjectREFR* a_speaker, std::chrono::steady_clock::time_point a_now)
		{
			const auto& s = Settings::Get();
			switch (s.shotMode) {
			case ShotMode::kOverPlayerShoulder:
				return Subject::kNpc;
			case ShotMode::kOverNpcShoulder:
				return Subject::kPlayer;
			default:
				break;
			}

			// Shot/reverse-shot: the NPC holds the camera while talking; once they've been
			// quiet briefly it's the player's turn, so cut to the player.
			const bool talking = a_speaker->IsTalking();
			if (talking) {
				g_state.npcWasTalking = true;
				return Subject::kNpc;
			}
			if (g_state.npcWasTalking) {
				g_state.npcWasTalking = false;
				g_state.npcQuietSince = a_now;
			}
			if (g_state.subject == Subject::kNpc && a_now - g_state.npcQuietSince < kPlayerCutDelay) {
				return Subject::kNpc;
			}
			return g_state.subject == Subject::kNone ? Subject::kNpc : Subject::kPlayer;
		}

		// Which camera state the game is in, as an index into CameraState, or -1 if unknown.
		int CurrentCameraStateIndex(RE::PlayerCamera* a_camera)
		{
			for (std::uint32_t i = 0; i < RE::CameraState::kTotal; ++i) {
				if (a_camera->QCameraEquals(static_cast<RE::CameraState>(i))) {
					return static_cast<int>(i);
				}
			}
			return -1;
		}

		// Logs why a frame was skipped, but only when the reason changes, so the log
		// shows each transition once instead of every frame.
		std::string g_lastSkipReason;
		void Skip(std::string a_reason)
		{
			if (a_reason != g_lastSkipReason) {
				REX::INFO("Skipping camera frames: {}", a_reason);
				g_lastSkipReason = std::move(a_reason);
			}
			End();
		}

		std::uint64_t g_hookCalls = 0;

		void DirectFrame()
		{
			const auto& s = Settings::Get();
			if (++g_hookCalls == 1) {
				REX::INFO("Dialogue camera update hook is running");
			}

			auto* camera = RE::PlayerCamera::GetSingleton();
			if (!s.enabled) {
				return Skip("disabled in INI");
			}
			if (!camera) {
				return Skip("no PlayerCamera");
			}

			auto* speaker = Speaker();
			auto* player = RE::PlayerCharacter::GetSingleton();
			const int  stateIndex = CurrentCameraStateIndex(camera);
			const bool inDialogueState = stateIndex == static_cast<int>(RE::CameraState::kDialogue);

			if (!speaker || !player || speaker == player) {
				return Skip(std::format("no dialogue speaker (camera state {}, speaker {:p})", stateIndex, static_cast<void*>(speaker)));
			}
			if (!inDialogueState) {
				// Not fatal: the hook only runs while the dialogue camera updates, so a state
				// mismatch most likely means our enum is off for this game version. Log it and
				// keep going so the test build still tells us whether placement works.
				if (g_state.framesThisConversation == 0) {
					REX::WARN("Camera state is {} (expected {} for dialogue); continuing anyway", stateIndex, static_cast<int>(RE::CameraState::kDialogue));
				}
			}
			g_lastSkipReason.clear();

			if (!g_state.active) {
				Begin();
			}

			const auto now = std::chrono::steady_clock::now();
			const float delta = std::chrono::duration<float>(now - g_state.lastFrame).count();
			g_state.lastFrame = now;
			++g_state.framesThisConversation;

			const auto npcHead = Camera::HeadPosition(speaker);
			const auto playerHead = Camera::HeadPosition(player);
			if (!npcHead || !playerHead) {
				LogFailure("head position unavailable; leaving vanilla camera");
				return;
			}

			const auto subject = ChooseSubject(speaker, now);
			if (subject != g_state.subject) {
				REX::INFO("Cut to {} (frame {})", SubjectName(subject), g_state.framesThisConversation);
				g_state.subject = subject;
			}

			const auto pose = subject == Subject::kNpc ?
				Camera::OverShoulder(*playerHead, *npcHead) :
				Camera::OverShoulder(*npcHead, *playerHead);
			if (!pose) {
				LogFailure("characters overlap; leaving vanilla camera");
				return;
			}

			// Safety net while we confirm Starfield's distance units: never put the camera
			// further from the near character than a few times the gap between the two heads.
			{
				const auto& nearHead = subject == Subject::kNpc ? *playerHead : *npcHead;
				const float hx = npcHead->x - playerHead->x, hy = npcHead->y - playerHead->y, hz = npcHead->z - playerHead->z;
				const float headGap = std::sqrt(hx * hx + hy * hy + hz * hz);
				const float cx = pose->position.x - nearHead.x, cy = pose->position.y - nearHead.y, cz = pose->position.z - nearHead.z;
				const float camGap = std::sqrt(cx * cx + cy * cy + cz * cz);
				if (camGap > 3.0f * headGap) {
					LogFailure(std::format("camera would be {:.2f} units from the near head but the heads are only {:.2f} apart; not moving it", camGap, headGap));
					return;
				}
			}

			auto* root = camera->cameraRoot.get();
			if (!Camera::Apply(root, *pose, delta)) {
				LogFailure("could not apply pose");
				return;
			}

			if (g_state.framesThisConversation == 1) {
				const float dx = npcHead->x - playerHead->x, dy = npcHead->y - playerHead->y, dz = npcHead->z - playerHead->z;
				REX::INFO("Distance between heads: {:.3f} game units (camera state {})", std::sqrt(dx * dx + dy * dy + dz * dz), stateIndex);
			}
			if (s.debugLog && g_state.framesThisConversation == 1) {
				REX::INFO("First frame: camera root {:p}, npc head ({:.1f}, {:.1f}, {:.1f}), player head ({:.1f}, {:.1f}, {:.1f}), camera ({:.1f}, {:.1f}, {:.1f})",
					static_cast<void*>(root),
					npcHead->x, npcHead->y, npcHead->z,
					playerHead->x, playerHead->y, playerHead->z,
					pose->position.x, pose->position.y, pose->position.z);
			}
		}

		// Runs every frame the game's dialogue camera is active. We let the game do its
		// normal update first, then overwrite where the camera is and what it looks at.
		struct DialogueCameraUpdateHook
		{
			static void Thunk(void* a_dialogueState)
			{
				Hook(a_dialogueState);
				if (g_hooksSafe) {
					DirectFrame();
				}
			}

			// Step None: not installed automatically. We enable it ourselves only after
			// confirming the game version, because the address is version-specific.
			static inline REL::THookVFT<void(void*)> Hook{
				"CCC_DialogueCameraUpdate",
				REL::EHookStep::None,
				REL::Offset(kDialogueStateVtableRva),
				kDialogueUpdateVtableIndex,
				Thunk
			};
		};
	}
}

SFSE_PLUGIN_PRELOAD(const SFSE::PreLoadInterface* a_sfse)
{
	SFSE::Init(a_sfse, { .trampoline = true, .trampolineSize = 64 });
	return true;
}

SFSE_PLUGIN_LOAD(const SFSE::LoadInterface* a_sfse)
{
	SFSE::Init(a_sfse);

	auto& settings = CCC::Settings::Get();
	settings.Load();

	const auto runtime = a_sfse->RuntimeVersion();
	if (runtime != CCC::kSupportedRuntime) {
		REX::CRITICAL("Starfield {} is not supported (this build targets 1.16.244). Camera hooks disabled.", runtime.string());
		CCC::g_hooksSafe = false;
		return true;
	}

	if (!CCC::DialogueCameraUpdateHook::Hook.Enable()) {
		REX::CRITICAL("Could not install the dialogue camera hook. Camera features disabled.");
		return true;
	}
	CCC::g_hooksSafe = true;
	REX::INFO("Cinematic Conversation Camera SF 0.1.2 loaded (enabled={}, shotMode={}, poseMode={})",
		settings.enabled, static_cast<int>(settings.shotMode), settings.poseMode);
	return true;
}
