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
#include "Visibility.h"

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
			std::chrono::steady_clock::time_point started{};
			bool                                  needsBodyRepair{ false };  // started from first person
			bool                                  bodyReady{ true };         // safe to show the player
			bool                                  restoreFirstPerson{ false };
			std::optional<Camera::Pose>           lastPose{};       // last shot we showed
			bool                                  npcEverTalked{ false };
			std::chrono::steady_clock::time_point lastCut{};
			std::chrono::steady_clock::time_point bodyReadyAt{};   // earliest time the player may be shown
			std::uint32_t                         lowHeadChecks{ 0 };
			std::uint32_t                         reRepairs{ 0 };
			std::optional<RE::NiPoint3>           smoothNpcHead{};
			std::optional<RE::NiPoint3>           smoothPlayerHead{};
			bool                                  startedFirstPerson{ false };
			bool                                  loggedReturnPose{ false };
			bool                                  holdingExit{ false };
			std::chrono::steady_clock::time_point holdStarted{};
		} g_state;

		// --- First-person body repair -------------------------------------------------
		// In first person the player's body is a cut-down rig; seen from outside it looks
		// broken (head at waist height, limbs detached). Before the first shot of the player
		// we cycle the game through real first person -> real third person -> back to the
		// dialogue camera, which makes it build the full third-person body, and we put first
		// person back after the conversation. Sequence and timings follow Player Camera
		// During Dialogue by rrrafau (GPL-3.0). Runs on the main thread (Watchdog).
		enum class RepairStage
		{
			kIdle,
			kAcquireFirst,
			kHoldFirst,
			kAcquireThird,
			kHoldThird,
			kRestoreDialogue,
			kSettle
		};
		struct Repair
		{
			RepairStage                           stage{ RepairStage::kIdle };
			std::chrono::steady_clock::time_point stageStart{};
			std::chrono::steady_clock::time_point nextRetry{};
			std::chrono::steady_clock::time_point deadline{};
			std::uint32_t                         settleFrames{ 0 };
			std::uint32_t                         cycle{ 0 };
			std::optional<RE::NiPoint3>           lastHead{};
			std::chrono::steady_clock::time_point lastHeadSample{};
			std::uint32_t                         stableSamples{ 0 };
		} g_repair;

		using namespace std::chrono_literals;
		constexpr auto kRepairStartDelay = 400ms;  // let the game's own dialogue opening finish
		constexpr auto kRepairHoldFirst = 48ms;
		constexpr auto kRepairHoldThird = 150ms;     // minimum; then until the pose is stable
		constexpr auto kRepairHoldThirdMax = 700ms;
		constexpr auto kRepairSettle = 150ms;
		// Head above feet: measured 1.62 m with the full third-person body posed, 1.52 m while
		// the body is still in its first-person form (0.3.5 log). Anything under this is not ready.
		constexpr float kStandingHeadHeight = 1.58f;
		constexpr auto kRepairRetry = 16ms;
		constexpr auto kRepairTimeout = 1800ms;
		constexpr std::uint32_t kRepairSettleFrames = 6;
		// One attempt only. 0.3.9 crashed inside the game while switching camera modes on a
		// second attempt (Linden, Ryujin HQ), so a failed rebuild now just keeps the camera on
		// the other person for that conversation.
		constexpr std::uint32_t kRepairMaxCycles = 1;

		// SFSE runs our per-frame task on whichever thread drains its queue: sometimes the
		// main thread, sometimes a job thread (the 0.3.9 crash was on "BSJobs 9"). Changing
		// camera modes or visibility is only done on ticks that land on the main thread.
		DWORD         g_mainThreadId = 0;
		std::uint64_t g_ticksMain = 0;
		std::atomic_uint64_t g_ticksOther{ 0 };

		bool                                  g_lastGameplayFirstPerson = false;
		bool                                  g_restoreFirstPersonPending = false;
		std::chrono::steady_clock::time_point g_restoreFirstPersonDeadline{};
		std::chrono::steady_clock::time_point g_restoreFirstPersonNext{};
		std::atomic_bool                      g_pilotingShip{ false };
		std::uint32_t                         g_repairMaskedFrames = 0;
		bool                                  g_viewCandidateKnown = false;
		bool                                  g_viewCandidateFirst = false;
		std::chrono::steady_clock::time_point g_viewCandidateSince{};
		// -------------------------------------------------------------------------------

		// Short pause after an NPC line ends before cutting to the player,
		// so we don't flick back and forth between consecutive NPC lines.
		constexpr auto kPlayerCutDelay = std::chrono::milliseconds(350);

		bool g_hooksSafe = false;

		// The hook runs on engine worker threads and the watchdog on the main thread.
		// The hook never waits for this lock (it skips the frame instead), so the engine's
		// worker threads can never be held up by us.
		std::mutex                            g_lock;
		std::chrono::steady_clock::time_point g_lastHookTime{};

		// Player visibility changes go through the engine's scene graph, which isn't safe to
		// touch from the dialogue camera hook's worker threads (0.2.0 froze doing that). The
		// hook only records what it wants; the main-thread watchdog applies it.
		bool g_wantRevealPlayer = false;
		bool g_needVisibilityRestore = false;

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
			g_state.started = g_state.lastFrame;
			g_state.startedFirstPerson = g_lastGameplayFirstPerson;
			g_state.needsBodyRepair = Settings::Get().fixFirstPersonBody && g_lastGameplayFirstPerson;
			g_state.bodyReady = !g_state.needsBodyRepair;
			g_repair = Repair{};
			REX::INFO("Conversation started (from {} person)", g_lastGameplayFirstPerson ? "first" : "third");
		}

		void End()
		{
			if (g_state.active) {
				REX::INFO("Conversation ended after {} camera frames", g_state.framesThisConversation);
				Camera::RestoreRender();
				if (g_wantRevealPlayer) {
					g_needVisibilityRestore = true;
				}
				if (g_state.restoreFirstPerson) {
					g_restoreFirstPersonPending = true;
					g_restoreFirstPersonDeadline = std::chrono::steady_clock::now() + 1500ms;
					g_restoreFirstPersonNext = {};
				}
			}
			g_repair = Repair{};
			g_wantRevealPlayer = false;
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
				g_state.npcEverTalked = true;
				return Subject::kNpc;
			}
			// Opening: the NPC usually speaks first but their line starts a moment after the
			// conversation does. Stay on them instead of flicking to the player and back.
			if (!g_state.npcEverTalked && a_now - g_state.started < 1200ms) {
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

			if (g_pilotingShip.load(std::memory_order_acquire)) {
				return Skip("piloting a ship; leaving the game's camera alone");
			}
			// When the conversation ends, the game keeps its dialogue camera running for most of
			// a second before returning to gameplay. Hold our last shot through that instead of
			// flashing the vanilla dialogue view; the main thread ends the conversation the
			// moment the game leaves its dialogue camera.
			if (!speaker && g_state.active && g_state.lastPose && inDialogueState && s.cameraTarget == 1) {
				const auto now = std::chrono::steady_clock::now();
				if (!g_state.holdingExit) {
					g_state.holdingExit = true;
					g_state.holdStarted = now;
					REX::INFO("Speaker gone; holding the last shot for up to {:.2f} s", s.exitHoldSeconds);
				}
				const float held = std::chrono::duration<float>(now - g_state.holdStarted).count();
				if (held < s.exitHoldSeconds && Camera::ApplyRender(camera->cameraRoot.get(), *g_state.lastPose, 0.0f)) {
					return;
				}
				// Third-person conversations: the game would now show its eye-level dialogue view
				// for about half a second before going back to third person. Show the third-person
				// view the player had before the conversation instead, until the game gets there.
				if (!g_state.startedFirstPerson && g_thirdPersonView && held < 3.0f &&
					Camera::ApplyRender(camera->cameraRoot.get(), *g_thirdPersonView, 0.0f)) {
					if (!g_state.loggedReturnPose) {
						g_state.loggedReturnPose = true;
						REX::INFO("Showing the saved third-person view until the game leaves its dialogue camera");
					}
					return;
				}
			}
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

			auto npcHead = Camera::HeadPosition(speaker);
			auto playerHead = Camera::HeadPosition(player);
			if (!npcHead || !playerHead) {
				LogFailure("head position unavailable; leaving vanilla camera");
				return;
			}

			float rawPlayerHeadHeight = 0.0f;

			// Follow the heads through a short low-pass filter instead of frame by frame. Raw
			// bone positions are sampled mid-animation on a worker thread, so tracking them
			// directly made the picture vibrate when someone walked; this also steadies the
			// shot against idle head bob. Large jumps (teleports, body rebuild) snap.
			{
				const float k = 1.0f - std::exp(-(std::clamp(delta, 0.0f, 0.1f)) / (std::max)(s.followSmoothing, 0.001f));
				auto        follow = [k](std::optional<RE::NiPoint3>& a_state, const RE::NiPoint3& a_raw) {
                    if (!a_state) {
                        a_state = a_raw;
                    } else {
                        const float dx = a_raw.x - a_state->x, dy = a_raw.y - a_state->y, dz = a_raw.z - a_state->z;
                        if (dx * dx + dy * dy + dz * dz > 4.0f) {
                            a_state = a_raw;
                        } else {
                            a_state->x += dx * k;
                            a_state->y += dy * k;
                            a_state->z += dz * k;
                        }
                    }
                    return *a_state;
				};
				if (g_state.framesThisConversation == 1) {
					REX::INFO("Head heights: npc {:.3f}, player {:.3f} (difference {:+.3f} m)", npcHead->z, playerHead->z, playerHead->z - npcHead->z);
				}
				rawPlayerHeadHeight = playerHead->z - player->GetPosition().z;
				npcHead = follow(g_state.smoothNpcHead, *npcHead);
				playerHead = follow(g_state.smoothPlayerHead, *playerHead);
			}

			if (g_repair.stage == RepairStage::kSettle) {
				++g_repair.settleFrames;
			}

			auto subject = ChooseSubject(speaker, now);
			if (!g_state.bodyReady || now < g_state.bodyReadyAt) {
				subject = Subject::kNpc;  // don't show the player until their body is rebuilt
			}
			// Never cut again within a fraction of a second of the last cut.
			if (subject != g_state.subject && g_state.subject != Subject::kNone && now - g_state.lastCut < 600ms &&
				!(subject == Subject::kNpc && !g_state.bodyReady)) {
				subject = g_state.subject;
			}
			if (subject != g_state.subject) {
				REX::INFO("Cut to {} (frame {}; player head {:.2f} m above feet)", SubjectName(subject), g_state.framesThisConversation, rawPlayerHeadHeight);
				g_state.subject = subject;
				g_state.lastCut = now;
			}

			// The game can put the player back into the first-person body form mid-conversation.
			// If we're showing them and their head drops to that height, rebuild the body again.
			if (g_state.restoreFirstPerson && g_state.bodyReady && g_state.framesThisConversation % 10 == 0) {
				g_state.lowHeadChecks = rawPlayerHeadHeight < kStandingHeadHeight ? g_state.lowHeadChecks + 1 : 0;
				if (g_state.lowHeadChecks >= 3 && g_state.reRepairs < 2) {
					++g_state.reRepairs;
					g_state.lowHeadChecks = 0;
					REX::WARN("Player body is back in first-person form (head {:.2f} m above feet); rebuilding it again", rawPlayerHeadHeight);
					g_state.bodyReady = false;
					g_state.needsBodyRepair = true;
					g_state.started = now - kRepairStartDelay;  // no opening delay this time
				}
			}

			// Over the near character's shoulder, unless the near character is clearly lower
			// (sitting): a camera behind a seated head ends up inside the chair back, so step
			// in front of and beside them instead and shoot a clean single of the other person.
			const auto& nearHeadRef = subject == Subject::kNpc ? *playerHead : *npcHead;
			const auto& farHeadRef = subject == Subject::kNpc ? *npcHead : *playerHead;
			const bool  nearSeated = farHeadRef.z - nearHeadRef.z > 0.2f;
			const auto  pose = nearSeated || s.shotStyle == 1 ?
				Camera::Shot(nearHeadRef, farHeadRef, -s.seatedForward, s.seatedSide, s.seatedHeight) :
				Camera::OverShoulder(nearHeadRef, farHeadRef);
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

			// The game's dialogue camera has just run and is aimed at the speaker. Check its axis
			// convention against our built-in one before we overwrite it (diagnostic only).
			if (g_state.framesThisConversation == 1) {
				Camera::Calibrate(root, *npcHead);
			}
			if (!Camera::IsCalibrated()) {
				LogFailure("camera axis convention not learned yet; leaving vanilla camera");
				return;
			}

			// Reveal right here so the player is visible on this very frame (doing it only from
			// the main-thread task left them hidden for the first frame of the shot, a visible
			// pop in third-person conversations). The main thread keeps it up afterwards.
			g_wantRevealPlayer = s.revealPlayer && g_state.bodyReady;
			if (g_wantRevealPlayer) {
				Visibility::RevealPlayer();
			}

			if (g_state.framesThisConversation == 1) {
				REX::INFO("frame 1: applying pose to {}", s.cameraTarget == 1 ? "render camera" : "camera root");
			}
			// Prefer moving only the rendered view; if the render camera can't be found,
			// fall back to the game camera so shots still work (NPCs may turn, as in 0.1.x).
			bool applied = s.cameraTarget == 1 && Camera::ApplyRender(root, *pose, delta);
			if (!applied) {
				applied = Camera::Apply(root, *pose, delta);
			}
			if (!applied) {
				LogFailure("could not apply pose");
				return;
			}
			g_state.lastPose = *pose;
			g_state.holdingExit = false;

			if (g_state.framesThisConversation == 1) {
				REX::INFO("frame 1: pose applied");
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
					std::unique_lock lock{ g_lock, std::try_to_lock };
					if (!lock.owns_lock()) {
						return;  // main thread is busy with our state; never stall the engine
					}
					g_lastHookTime = std::chrono::steady_clock::now();
					DirectFrame();
				}
			}

			// (see Watchdog below)
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

		// Runs once per frame on the main thread. If a conversation is active but the game
		// has stopped running its dialogue camera (load screen, scripted scene, menu that
		// closes dialogue abruptly...), give everything back instead of leaving the view stuck.
		// Also applies player visibility changes requested by the hook, on the main thread.
		// During the body repair the game is briefly in its real first- and third-person
		// cameras. These run right after each of those cameras updates (the same kind of hook
		// as the dialogue camera one) and re-apply our last shot so the cycle isn't visible.
		void MaskRepairFrame(const char* a_which)
		{
			if (!g_hooksSafe) {
				return;
			}
			std::unique_lock lock{ g_lock, std::try_to_lock };
			if (!lock.owns_lock()) {
				return;
			}
			if (!g_state.active || g_repair.stage == RepairStage::kIdle || !g_state.lastPose || Settings::Get().cameraTarget != 1) {
				return;
			}
			auto* camera = RE::PlayerCamera::GetSingleton();
			if (camera && Camera::ApplyRender(camera->cameraRoot.get(), *g_state.lastPose, 0.0f)) {
				if (g_repairMaskedFrames++ == 0) {
					REX::INFO("Body repair: holding the shot from the {} camera hook", a_which);
				}
			}
		}

		// The player's own third-person view, refreshed every frame they are in third person
		// outside a conversation. Used to return to it seamlessly when a conversation ends.
		std::optional<Camera::Pose> g_thirdPersonView;

		void RememberThirdPersonView()
		{
			if (!g_hooksSafe) {
				return;
			}
			std::unique_lock lock{ g_lock, std::try_to_lock };
			if (!lock.owns_lock() || g_state.active) {
				return;
			}
			if (auto* camera = RE::PlayerCamera::GetSingleton()) {
				g_thirdPersonView = Camera::PoseOf(camera->cameraRoot.get());
			}
		}

		struct FirstPersonUpdateHook
		{
			static void Thunk(void* a_state, void* a_arg)
			{
				Hook(a_state, a_arg);
				MaskRepairFrame("first-person");
			}
			static inline REL::THookVFT<void(void*, void*)> Hook{
				"CCC_FirstPersonUpdate", REL::EHookStep::None, RE::VTABLE::FirstPersonState[0], kDialogueUpdateVtableIndex, Thunk
			};
		};

		struct ThirdPersonUpdateHook
		{
			static void Thunk(void* a_state, void* a_arg)
			{
				Hook(a_state, a_arg);
				MaskRepairFrame("third-person");
				RememberThirdPersonView();
			}
			static inline REL::THookVFT<void(void*, void*)> Hook{
				"CCC_ThirdPersonUpdate", REL::EHookStep::None, RE::VTABLE::ThirdPersonState[0], kDialogueUpdateVtableIndex, Thunk
			};
		};

		float PlayerHeadAboveFeet()
		{
			auto* player = RE::PlayerCharacter::GetSingleton();
			const auto head = player ? Camera::HeadPosition(player) : std::nullopt;
			return head ? head->z - player->GetPosition().z : 0.0f;
		}

		// Asking the game for its dialogue camera when no conversation is live crashes it.
		bool ReturnToDialogueCamera(RE::PlayerCamera* a_camera)
		{
			static const RE::BSFixedString dialogueMenu{ "DialogueMenu" };
			auto*                          ui = RE::UI::GetSingleton();
			auto*                          speaker = Speaker();
			if (!ui || !ui->IsMenuOpen(dialogueMenu) || !speaker || !Camera::HeadPosition(speaker)) {
				REX::WARN("Conversation is no longer live; not returning to the dialogue camera");
				End();
				return false;
			}
			a_camera->SetCameraState(RE::CameraState::kDialogue);
			return true;
		}

		void FailRepair(RE::PlayerCamera* a_camera, const char* a_why)
		{
			REX::WARN("First-person body repair failed ({}); the player won't be shown this conversation", a_why);
			if (!ReturnToDialogueCamera(a_camera)) {
				return;
			}
			g_repair = Repair{};
			g_state.needsBodyRepair = false;  // don't retry; bodyReady stays false
			g_lastHookTime = std::chrono::steady_clock::now();
		}

		// One step of the body repair. Caller holds g_lock; main thread only.
		void StepRepair(RE::PlayerCamera* a_camera, std::chrono::steady_clock::time_point a_now)
		{
			const bool first = a_camera->QCameraEquals(RE::CameraState::kFirstPerson) || a_camera->QCameraEquals(RE::CameraState::kIronSights);
			const bool third = a_camera->QCameraEquals(RE::CameraState::kThirdPerson);
			const bool dialogue = a_camera->QCameraEquals(RE::CameraState::kDialogue);

			if (g_repair.stage == RepairStage::kIdle) {
				if (!dialogue || a_now - g_state.started < kRepairStartDelay) {
					return;
				}
				REX::INFO("Body repair: started in first person; cycling first -> third -> dialogue (head {:.2f} m above feet)", PlayerHeadAboveFeet());
				g_repairMaskedFrames = 0;
				g_repair.stage = RepairStage::kAcquireFirst;
				g_repair.deadline = a_now + kRepairTimeout;
				g_repair.nextRetry = a_now + kRepairRetry;
				g_state.restoreFirstPerson = true;
				a_camera->ForceFirstPerson();
				return;
			}
			if (!Speaker()) {
				REX::INFO("Body repair: conversation ended mid-repair");
				End();
				return;
			}
			if (a_now >= g_repair.deadline) {
				return FailRepair(a_camera, "timed out");
			}

			switch (g_repair.stage) {
			case RepairStage::kAcquireFirst:
				if (first) {
					g_repair.stage = RepairStage::kHoldFirst;
					g_repair.stageStart = a_now;
				} else if (a_now >= g_repair.nextRetry) {
					a_camera->ForceFirstPerson();
					g_repair.nextRetry = a_now + kRepairRetry;
				}
				return;
			case RepairStage::kHoldFirst:
				if (a_now - g_repair.stageStart < kRepairHoldFirst) {
					return;
				}
				g_repair.stage = RepairStage::kAcquireThird;
				g_repair.nextRetry = a_now + kRepairRetry;
				a_camera->ForceThirdPerson();
				return;
			case RepairStage::kAcquireThird:
				if (third) {
					g_repair.stage = RepairStage::kHoldThird;
					g_repair.stageStart = a_now;
				} else if (a_now >= g_repair.nextRetry) {
					a_camera->ForceThirdPerson();
					g_repair.nextRetry = a_now + kRepairRetry;
				}
				return;
			case RepairStage::kHoldThird:
			{
				// The third-person skeleton exists a few frames before the animation system has
				// posed it. Stay in third person until the head has stopped moving and is at
				// standing height, not just for a fixed time.
				if (a_now - g_repair.lastHeadSample >= 16ms) {
					g_repair.lastHeadSample = a_now;
					auto*      player = RE::PlayerCharacter::GetSingleton();
					const auto head = player ? Camera::HeadPosition(player) : std::nullopt;
					if (head && g_repair.lastHead) {
						const float dx = head->x - g_repair.lastHead->x, dy = head->y - g_repair.lastHead->y, dz = head->z - g_repair.lastHead->z;
						const bool  still = dx * dx + dy * dy + dz * dz < 0.0025f;
						const bool  standing = head->z - player->GetPosition().z > kStandingHeadHeight;
						g_repair.stableSamples = still && standing ? g_repair.stableSamples + 1 : 0;
					}
					g_repair.lastHead = head;
				}
				const auto held = a_now - g_repair.stageStart;
				if (held < kRepairHoldThird || (g_repair.stableSamples < 3 && held < kRepairHoldThirdMax)) {
					return;
				}
				g_repair.stage = RepairStage::kRestoreDialogue;
				g_repair.nextRetry = a_now + 32ms;
				ReturnToDialogueCamera(a_camera);
				return;
			}
			case RepairStage::kRestoreDialogue:
				if (dialogue) {
					g_repair.stage = RepairStage::kSettle;
					g_repair.stageStart = a_now;
					g_repair.settleFrames = 0;
				} else if (a_now >= g_repair.nextRetry) {
					if (!ReturnToDialogueCamera(a_camera)) {
						return;
					}
					g_repair.nextRetry = a_now + 32ms;
				}
				return;
			case RepairStage::kSettle:
				if (!dialogue) {
					g_repair.stage = RepairStage::kRestoreDialogue;
					g_repair.nextRetry = a_now;
					return;
				}
				if (a_now - g_repair.stageStart < kRepairSettle || g_repair.settleFrames < kRepairSettleFrames) {
					return;
				}
			{
				const float height = PlayerHeadAboveFeet();
				const auto  cycle = g_repair.cycle + 1;
				if (height < kStandingHeadHeight && cycle < kRepairMaxCycles) {
					REX::WARN("Body repair: head only {:.2f} m above feet after cycle {}; trying again", height, cycle);
					g_repair = Repair{};
					g_repair.cycle = cycle;
					g_repair.stage = RepairStage::kAcquireFirst;
					g_repair.deadline = a_now + kRepairTimeout;
					g_repair.nextRetry = a_now + kRepairRetry;
					a_camera->ForceFirstPerson();
					return;
				}
				if (height < kStandingHeadHeight) {
					// The game would not build the full body here. Showing it would show the
					// broken first-person form, so keep the camera on the other person for the
					// rest of this conversation instead.
					REX::WARN("Body repair: gave up after {} cycles (head still {:.2f} m above feet); the player won't be shown this conversation", cycle, height);
					g_repair = Repair{};
					g_state.needsBodyRepair = false;
					g_state.bodyReady = false;
					g_state.reRepairs = 99;
					return;
				}
				REX::INFO("Body repair: done after {} cycle(s); head {:.2f} m above feet; shot held for {} frames during the cycle", cycle, height, g_repairMaskedFrames);
				g_repair = Repair{};
				g_state.needsBodyRepair = false;
				g_state.bodyReady = true;
				g_state.bodyReadyAt = a_now + 300ms;  // a few more frames for the pose to settle
				return;
			}
			default:
				return;
			}
		}

		// After a conversation that began in first person, put first person back once the
		// game has left its dialogue camera. Caller holds g_lock; main thread only.
		void StepFirstPersonRestore(RE::PlayerCamera* a_camera, std::chrono::steady_clock::time_point a_now)
		{
			if (g_state.active) {
				g_restoreFirstPersonPending = false;  // a new conversation started; it will handle it
				return;
			}
			if (a_camera->QCameraEquals(RE::CameraState::kDialogue)) {
				g_restoreFirstPersonDeadline = a_now + 1500ms;  // dialogue still closing; keep waiting
				return;
			}
			if (a_camera->QCameraEquals(RE::CameraState::kFirstPerson)) {
				REX::INFO("First person restored");
				g_restoreFirstPersonPending = false;
				return;
			}
			if (g_pilotingShip.load(std::memory_order_acquire)) {
				g_restoreFirstPersonPending = false;
				return;
			}
			if (a_now >= g_restoreFirstPersonDeadline) {
				REX::WARN("First person restore timed out; forcing camera state");
				a_camera->SetCameraState(RE::CameraState::kFirstPerson);
				g_restoreFirstPersonPending = false;
				return;
			}
			if (a_now >= g_restoreFirstPersonNext) {
				a_camera->ForceFirstPerson();
				g_restoreFirstPersonNext = a_now + 32ms;
			}
		}

		void Watchdog()
		{
			if (::GetCurrentThreadId() != g_mainThreadId) {
				++g_ticksOther;
				return;
			}
			if (++g_ticksMain == 3000) {
				REX::INFO("Per-frame task: {} ticks on the main thread, {} on other threads so far", g_ticksMain, g_ticksOther.load());
			}

			// Diagnostic: record every camera mode change, to see exactly which modes the game
			// passes through (and for how long) when a conversation opens and closes.
			if (Settings::Get().debugLog) {
				static int  lastState = -2;
				static auto lastChange = std::chrono::steady_clock::now();
				if (auto* cam = RE::PlayerCamera::GetSingleton()) {
					const int state = CurrentCameraStateIndex(cam);
					if (state != lastState) {
						const auto t = std::chrono::steady_clock::now();
						REX::INFO("Camera mode {} -> {} (previous mode lasted {} ms)", lastState, state,
							std::chrono::duration_cast<std::chrono::milliseconds>(t - lastChange).count());
						lastState = state;
						lastChange = t;
					}
				}
			}

			bool reveal = false;
			bool restore = false;

			if (auto* ui = RE::UI::GetSingleton()) {
				static const RE::BSFixedString shipHud{ "SpaceshipHudMenu" };
				g_pilotingShip.store(ui->IsMenuOpen(shipHud), std::memory_order_release);
			}

			{
				std::scoped_lock lock{ g_lock };
				const auto       now = std::chrono::steady_clock::now();
				auto*            camera = RE::PlayerCamera::GetSingleton();
				const bool       repairing = g_repair.stage != RepairStage::kIdle;

				if (camera && !g_state.active && !g_restoreFirstPersonPending) {
					// Remember which view the player uses outside conversations.
					// Starfield can pass through third person for a moment while opening a
					// conversation from first person, so only accept a change once it has
					// lasted long enough to be real gameplay.
					const bool first = camera->QCameraEquals(RE::CameraState::kFirstPerson) || camera->QCameraEquals(RE::CameraState::kIronSights);
					const bool third = camera->QCameraEquals(RE::CameraState::kThirdPerson);
					if (first || third) {
						if (!g_viewCandidateKnown || g_viewCandidateFirst != first) {
							g_viewCandidateKnown = true;
							g_viewCandidateFirst = first;
							g_viewCandidateSince = now;
						} else if (now - g_viewCandidateSince >= 120ms) {
							g_lastGameplayFirstPerson = first;
						}
					}
				}

				// While repairing, the game is briefly out of its dialogue camera on purpose.
				if (g_state.active && !repairing && camera && !camera->QCameraEquals(RE::CameraState::kDialogue)) {
					REX::INFO("Game left its dialogue camera; restoring camera");
					End();
				}
				if (g_state.active && !repairing && now - g_lastHookTime > 250ms) {
					REX::INFO("Dialogue camera stopped updating; restoring camera");
					End();
				}
				if (camera && g_state.active && g_state.needsBodyRepair) {
					StepRepair(camera, now);
				}
				if (camera && g_restoreFirstPersonPending) {
					StepFirstPersonRestore(camera, now);
				}

				reveal = g_wantRevealPlayer;
				restore = g_needVisibilityRestore;
				g_needVisibilityRestore = false;
			}

			// Scene-graph work happens outside the lock, so the hook is never kept waiting
			// (it would skip frames instead) while the engine does its own locking.
			if (restore) {
				Visibility::Restore();
			} else if (reveal) {
				Visibility::RevealPlayer();
			}
		}
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
	if (!settings.stateHooks) {
		REX::INFO("First/third-person camera hooks disabled in INI (bStateHooks=0)");
	} else if (!CCC::FirstPersonUpdateHook::Hook.Enable() || !CCC::ThirdPersonUpdateHook::Hook.Enable()) {
		REX::WARN("Could not install the first/third-person camera hooks; the body cycle will be visible");
	}

	CCC::g_mainThreadId = ::GetCurrentThreadId();
	if (const auto* tasks = SFSE::GetTaskInterface()) {
		tasks->AddPermanentTask([]() { CCC::Watchdog(); });
	} else {
		REX::WARN("SFSE task interface unavailable; camera watchdog disabled");
	}

	CCC::g_hooksSafe = true;
	REX::INFO("Cinematic Conversation Camera SF 0.3.12 loaded (enabled={}, shotMode={}, cameraTarget={}, revealPlayer={}, poseMode={})",
		settings.enabled, static_cast<int>(settings.shotMode), settings.cameraTarget, settings.revealPlayer, settings.poseMode);
	return true;
}
