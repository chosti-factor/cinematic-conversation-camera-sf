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
#include "Director.h"
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
			std::uint64_t                         framesAtRepairEnd{ 0 };
			bool                                  startRootCaughtUp{ false };
			bool                                  smallSpeaker{ false };
			bool                                  handedBack{ false };       // quest scene: leave it to the game
			bool                                  plainShots{ false };       // seated speaker or merchant: plain over-the-shoulder shot
			bool                                  merchant{ false };         // speaker is a merchant (faction vendor flag)
			bool                                  speakerWasTalking{ false };
			bool                                  lineIntense{ false };      // current line's expression is in sCloseUpEmotions
			bool                                  backgroundScene{ false };  // speaker is in a background scene (no mid-conversation camera switching)
			std::optional<RE::NiPoint3>           rootBeforeFocus{};          // where the game put its camera before the focus move
			bool                                  focusLogged{ false };
			std::uint32_t                         lastAppliedFrame{ 0 };     // frame our shot was last placed on (main path)
			std::uint32_t                         driftBits{ 0 };            // last 32 frames: 1 = the game moved its camera after us
			int                                   driftLogged{ 0 };
			std::uint32_t                         speakerId{ 0 };
			std::optional<Camera::Pose>           lastRoot{};
			std::optional<RE::NiPoint3>           lastNpcFeet{};
			std::optional<RE::NiPoint3>           lastPlayerFeet{};
			bool                                  lateRetryDone{ false };
			bool                                  directRequested{ false };
			bool                                  directFailed{ false };
			int                                   directStable{ 0 };
			std::chrono::steady_clock::time_point directAt{};
			std::chrono::steady_clock::time_point directLastSample{};
			std::optional<RE::NiPoint3>           directLastHead{};
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
			bool                                  hidePlayer{ false };   // rebuild failed: keep the broken body out of frame
			bool                                  povNpcShot{ false };   // shoot the NPC from the player's eyes until the first cut
			std::chrono::steady_clock::time_point npcTalkStart{};
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
			std::uint32_t                         attempts{ 0 };
			bool                                  early{ false };  // started on the conversation's first frames
			std::optional<Camera::Pose>           earlyHeld{};     // what to show during an early rebuild
			// After asking for a camera mode: where its camera root will be and what to show,
			// used until that mode's own hook has run once (or briefly timed out).
			int                                   pendingMode{ -1 };
			std::optional<Camera::Pose>           pendingRoot{};
			std::optional<Camera::Pose>           pendingShot{};
			std::chrono::steady_clock::time_point pendingSince{};
			std::chrono::steady_clock::time_point nextAttemptAt{};
			std::optional<RE::NiPoint3>           lastHead{};
			std::chrono::steady_clock::time_point lastHeadSample{};
			std::uint32_t                         stableSamples{ 0 };
		} g_repair;

		using namespace std::chrono_literals;
		constexpr auto kRepairStartDelay = 250ms;  // never sooner than this after the conversation opens
		constexpr auto kRepairFallbackStart = 1200ms;  // start anyway if the NPC hasn't spoken by then
		constexpr auto kRepairAcquire = 150ms;         // how long a requested camera mode may take to appear
		constexpr auto kRepairBackoff = 900ms;         // wait between attempts
		constexpr std::uint32_t kRepairMaxAttempts = 4;
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
		std::atomic<float>                    g_shotDistance{ 0.0f };  // camera to the person in shot, this frame
		std::uint32_t                         g_repairMaskedFrames = 0;
		// The player's own third-person view, refreshed every frame they are in third person
		// outside a conversation. Used to return to it seamlessly when a conversation ends.
		std::optional<Camera::Pose> g_thirdPersonView;
		std::optional<Camera::Pose> g_firstPersonView;  // likewise for first person

		// Where the third-person camera sits relative to the first-person view (offset and
		// aim, in the first-person view's own forward/right/up axes). Learned during a body
		// rebuild and used to predict the third-person camera on the next one.
		struct RelativeView
		{
			float offset[3]{};
			float aim[3]{};
		};
		std::optional<RelativeView> g_thirdFromFirst;

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
		std::timed_mutex                      g_lock;
	std::atomic<DWORD>                    g_lockHolder{ 0 };  // main thread while it holds g_lock
	std::atomic<int>                      g_hookLockMisses{ 0 };
	void*                                 g_dialogueStateObject = nullptr;
	int                                   g_returnScanFrames = 0;

	// The camera hooks run on engine worker threads (sometimes on the main thread). When the
	// main thread has our state locked they used to skip their frame, which left one frame
	// of a stale camera on screen at mode changes. Wait briefly instead; only give up at once
	// when this very thread holds the lock (the game called the hook from inside our own call).
	std::unique_lock<std::timed_mutex> HookLock()
	{
		if (g_lockHolder.load() == ::GetCurrentThreadId()) {
			++g_hookLockMisses;
			return std::unique_lock<std::timed_mutex>{};
		}
		std::unique_lock<std::timed_mutex> lock{ g_lock, std::chrono::milliseconds(4) };
		if (!lock.owns_lock()) {
			++g_hookLockMisses;
		}
		return lock;
	}

		std::chrono::steady_clock::time_point g_lastHookTime{};

		// Player visibility changes go through the engine's scene graph, which isn't safe to
		// touch from the dialogue camera hook's worker threads (0.2.0 froze doing that). The
		// hook only records what it wants; the main-thread watchdog applies it.
		bool g_wantRevealPlayer = false;
		bool g_wantHidePlayer = false;
		float g_standingHeadHeight = 1.62f;  // the player's head above their feet with the full body; refined as observed
		bool g_needVisibilityRestore = false;
		bool g_restoreVisible = true;  // with g_needVisibilityRestore: leave the player shown
		std::chrono::steady_clock::time_point g_exitMaskUntil{};  // first-person exit: the game passes through third person
		bool g_exitMasking = false;
		int  g_exitCoverFrames = 0;  // first-person frames still to cover after a first-person exit
		int  g_exitLogFrames = 0;    // first-person frames still to log after a first-person exit
		// Where the game's dialogue camera sits relative to the player's eyes, as measured on
		// earlier conversations: metres back from the NPC, and up. Lateral varies, so 0.
		float g_learnedBack = 0.27f;
		float g_learnedUp = 0.01f;
		float g_learnedLateral = 0.18f;  // size of the sideways offset
		int   g_lateralVotes = 2;        // > = 0: offset to the negative side (seen most often)

		// Diagnostic for the frames right after we ask for the dialogue camera back: where the
	// game's camera really is, against where we assumed it would be.
	void LogRootAgainstPrediction(const char* a_when, RE::NiAVObject* a_root);

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

		// ---- Data collection for the test builds -------------------------------------------
		int  g_conversationNumber = 0;
		bool g_menusLogged = false;
		bool g_dumpedThisConversation = false;

		bool SafeWrite(void* a_to, const void* a_from, std::size_t a_bytes)
		{
			__try {
				std::memcpy(a_to, a_from, a_bytes);
				return true;
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				return false;
			}
		}

		std::size_t SafeCopy(void* a_to, const void* a_from, std::size_t a_bytes)
		{
			__try {
				std::memcpy(a_to, a_from, a_bytes);
				return a_bytes;
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				return 0;
			}
		}

		// Raw contents of a game object, 16 values per line, to compare between situations
		// (standing or seated speaker, first or third person) and find what differs.
		void DumpObject(const char* a_what, const void* a_base, std::size_t a_bytes)
		{
			if (!a_base) {
				return;
			}
			std::array<std::uint32_t, 0x400 / 4> words{};
			a_bytes = (std::min)(a_bytes, sizeof(words));
			if (SafeCopy(words.data(), a_base, a_bytes) == 0) {
				REX::INFO("Dump {}: unreadable", a_what);
				return;
			}
			for (std::size_t off = 0; off < a_bytes; off += 64) {
				std::string line;
				for (std::size_t i = off / 4; i < (std::min)(a_bytes, off + 64) / 4; ++i) {
					line += std::format(" {:08X}", words[i]);
				}
				REX::INFO("Dump {} +{:03X}:{}", a_what, off, line);
			}
		}

		struct FovSaved
		{
			RE::Setting* setting;
			float        original;
		};
		std::vector<FovSaved> g_fovSaved;
		bool                  g_stateFovWritten = false;
		float                 g_stateFovOriginal = 0.0f;
		constexpr std::size_t kDialogueStateAngleOffset = 0x2A8;  // holds 90.0 during conversations (seen in the 0.3.25 dumps)

		RE::Setting* FindSetting(std::string_view a_key)
		{
			for (auto* collection : { static_cast<RE::INISettingCollection*>(RE::INISettingCollection::GetSingleton()),
					 static_cast<RE::INISettingCollection*>(RE::INIPrefSettingCollection::GetSingleton()) }) {
				if (!collection) {
					continue;
				}
				for (const auto& setting : collection->settings) {
					if (setting && setting->GetKey() == a_key && setting->GetType() == RE::Setting::Type::kFloat) {
						return setting;
					}
				}
			}
			return nullptr;
		}

		// Main thread only.
		void ApplyFovExperiment(bool a_on)
		{
			if (!a_on) {
				for (const auto& saved : g_fovSaved) {
					saved.setting->SetFloat(saved.original);
				}
				if (!g_fovSaved.empty()) {
					REX::INFO("FOV experiment: settings restored");
				}
				g_fovSaved.clear();
				if (g_stateFovWritten) {
					g_stateFovWritten = false;
					if (g_dialogueStateObject) {
						SafeCopy(static_cast<std::uint8_t*>(g_dialogueStateObject) + kDialogueStateAngleOffset, &g_stateFovOriginal, sizeof(float));
					}
					REX::INFO("FOV experiment 4: dialogue camera value restored");
				}
				return;
			}
			if (Settings::Get().fovExperiment == 4) {
				if (g_dialogueStateObject) {
					float current = 0.0f;
					SafeCopy(&current, static_cast<std::uint8_t*>(g_dialogueStateObject) + kDialogueStateAngleOffset, sizeof(float));
					const float wanted = Settings::Get().fovExperimentValue;
					if (!g_stateFovWritten) {
						if (current < 10.0f || current > 170.0f) {
							return;  // not what we expected to find there; leave it alone
						}
						g_stateFovOriginal = current;
						g_stateFovWritten = true;
						REX::INFO("FOV experiment 4: dialogue camera value {:.2f} -> {:.2f}", current, wanted);
					}
					if (current != wanted) {
						SafeCopy(static_cast<std::uint8_t*>(g_dialogueStateObject) + kDialogueStateAngleOffset, &wanted, sizeof(float));
					}
				}
				return;
			}
			const auto& s = Settings::Get();
			if (s.fovExperiment == 0 || !g_fovSaved.empty()) {
				return;
			}
			std::vector<std::string_view> keys;
			switch (s.fovExperiment) {
			case 1:
				keys = { "fTPWorldFOV:Camera", "fFPWorldFOV:Camera" };
				break;
			case 2:
				keys = { "fDialogueCameraFailsafeFPFOV:Dialogue" };
				break;
			default:
				keys = { "fDefault1stPersonFOV:Camera", "fFPGeometryFOV:Camera" };
				break;
			}
			for (const auto key : keys) {
				if (auto* setting = FindSetting(key)) {
					g_fovSaved.push_back({ setting, setting->GetFloat() });
					setting->SetFloat(s.fovExperimentValue);
					REX::INFO("FOV experiment {}: {} {:.2f} -> {:.2f}", s.fovExperiment, key, g_fovSaved.back().original, s.fovExperimentValue);
				} else {
					REX::INFO("FOV experiment {}: setting {} not found", s.fovExperiment, key);
				}
			}
		}

		// Main thread only. Game settings listed in the INI's [GameSettings] section, applied
		// (and put back when removed from the INI) so their effect on the camera can be tried
		// without restarting the game.
		struct SavedGameSetting
		{
			RE::Setting* setting;
			double       original;
		};
		std::vector<std::pair<std::string, SavedGameSetting>> g_savedGameSettings;

		RE::Setting* FindAnySetting(const std::string& a_key)
		{
			for (auto* collection : { static_cast<RE::INISettingCollection*>(RE::INISettingCollection::GetSingleton()),
					 static_cast<RE::INISettingCollection*>(RE::INIPrefSettingCollection::GetSingleton()) }) {
				if (!collection) {
					continue;
				}
				for (const auto& setting : collection->settings) {
					if (setting && setting->GetKey() == a_key) {
						return setting;
					}
				}
			}
			if (a_key.find(':') == std::string::npos) {
				if (auto* games = RE::GameSettingCollection::GetSingleton()) {
					return games->GetSetting(a_key.c_str());
				}
			}
			return nullptr;
		}

		std::optional<double> ReadSettingValue(RE::Setting* a_setting)
		{
			switch (a_setting->GetType()) {
			case RE::Setting::Type::kFloat:
				return a_setting->GetFloat();
			case RE::Setting::Type::kInt:
				return a_setting->GetInt();
			case RE::Setting::Type::kUInt:
				return a_setting->GetUInt();
			case RE::Setting::Type::kBool:
				return a_setting->GetBool() ? 1.0 : 0.0;
			default:
				return std::nullopt;
			}
		}

		void WriteSettingValue(RE::Setting* a_setting, double a_value)
		{
			switch (a_setting->GetType()) {
			case RE::Setting::Type::kFloat:
				a_setting->SetFloat(static_cast<float>(a_value));
				break;
			case RE::Setting::Type::kInt:
				a_setting->SetInt(static_cast<std::int32_t>(a_value));
				break;
			case RE::Setting::Type::kUInt:
				a_setting->SetUInt(static_cast<std::uint32_t>(a_value));
				break;
			case RE::Setting::Type::kBool:
				a_setting->SetBool(a_value != 0.0);
				break;
			default:
				break;
			}
		}

		void ApplyGameSettings()
		{
			const auto& wanted = Settings::Get().gameSettings;
			// Put back settings no longer listed.
			std::erase_if(g_savedGameSettings, [&](auto& a_saved) {
				const bool listed = std::ranges::any_of(wanted, [&](const auto& a_w) { return a_w.first == a_saved.first; });
				if (!listed) {
					WriteSettingValue(a_saved.second.setting, a_saved.second.original);
					REX::INFO("Game setting {} put back to {}", a_saved.first, a_saved.second.original);
				}
				return !listed;
			});
			for (const auto& [key, text] : wanted) {
				double value = 0.0;
				try {
					value = std::stod(text);
				} catch (...) {
					static std::vector<std::string> warned;
					if (std::ranges::find(warned, key) == warned.end()) {
						warned.push_back(key);
						REX::WARN("Game setting {}: value '{}' is not a number", key, text);
					}
					continue;
				}
				auto it = std::ranges::find_if(g_savedGameSettings, [&](const auto& a_s) { return a_s.first == key; });
				RE::Setting* setting = it != g_savedGameSettings.end() ? it->second.setting : FindAnySetting(key);
				if (!setting) {
					static std::vector<std::string> warned;
					if (std::ranges::find(warned, key) == warned.end()) {
						warned.push_back(key);
						REX::WARN("Game setting {} not found (check the name and the :Section part)", key);
					}
					continue;
				}
				const auto current = ReadSettingValue(setting);
				if (!current) {
					continue;
				}
				if (it == g_savedGameSettings.end()) {
					g_savedGameSettings.emplace_back(key, SavedGameSetting{ setting, *current });
				}
				if (*current != value) {
					WriteSettingValue(setting, value);
					REX::INFO("Game setting {}: {} -> {}", key, *current, value);
				}
			}
		}

		// Main thread only. Which of the game's menus are open during a conversation (for
		// the letterbox bars and subtitle placement to come).
		void LogOpenMenus()
		{
			auto* ui = RE::UI::GetSingleton();
			if (!ui) {
				return;
			}
			std::string text;
			std::size_t total = 0;
			for (const auto& entry : ui->menuMap) {
				++total;
				if (ui->IsMenuOpen(entry.key)) {
					text += ' ';
					text += entry.key.c_str();
				}
			}
			REX::INFO("Open menus ({} registered):{}", total, text);
		}

		// Field of view of the conversation shots. Found in the 0.3.28 dumps: the game keeps
		// the camera's current horizontal field of view (degrees) inside the player camera
		// object, and the render camera's real frustum (tangents of the half angles) is at a
		// different place than the library assumes. Called right after the game's dialogue
		// camera update, so the game's own value (about 30) is replaced every frame.
		constexpr std::size_t kPlayerCameraFovOffset = 0x280;
		constexpr std::size_t kRenderCameraFrustumOffset = 0x1C0;
		bool                  g_fovLogged = false;

		void Begin()
		{
			g_state = State{};
			g_state.active = true;
			g_state.lastFrame = std::chrono::steady_clock::now();
			g_state.started = g_state.lastFrame;
			g_state.startedFirstPerson = g_lastGameplayFirstPerson;
			// The opening shot from the player's eyes was only needed while the camera cycle
			// rebuilt the body. The direct refresh takes about 0.1 s (player hidden meanwhile),
			// so first-person conversations open over the shoulder like third-person ones.
			g_state.povNpcShot = g_lastGameplayFirstPerson && Settings::Get().bodyRebuildMethod != 1;
			g_state.needsBodyRepair = Settings::Get().fixFirstPersonBody && g_lastGameplayFirstPerson;
			g_state.bodyReady = !g_state.needsBodyRepair;
			g_repair = Repair{};
			Settings::Get().Load();  // pick up INI edits made since the last conversation
			++g_conversationNumber;
			g_menusLogged = false;
			g_fovLogged = false;
			g_dumpedThisConversation = false;
			REX::INFO("Conversation {} started (from {} person)", g_conversationNumber, g_lastGameplayFirstPerson ? "first" : "third");
			if (Settings::Get().director) {
				Director::Reset();
			}
		}

		void End()
		{
			if (g_state.active) {
				REX::INFO("Conversation ended after {} camera frames", g_state.framesThisConversation);
				if (g_state.rootBeforeFocus) {
					if (auto* camera = RE::PlayerCamera::GetSingleton(); camera && camera->QCameraEquals(RE::CameraState::kDialogue)) {
						Camera::SetRootPosition(camera->cameraRoot.get(), *g_state.rootBeforeFocus);
					}
				}
				Camera::RestoreRender();
				if (g_wantRevealPlayer || g_wantHidePlayer) {
					g_needVisibilityRestore = true;
				}
				g_restoreVisible = !g_state.startedFirstPerson;
				if (g_state.startedFirstPerson) {
					g_exitMaskUntil = std::chrono::steady_clock::now() + 1000ms;
				}
				if (g_state.restoreFirstPerson) {
					g_restoreFirstPersonPending = true;
					g_restoreFirstPersonDeadline = std::chrono::steady_clock::now() + 1500ms;
					g_restoreFirstPersonNext = {};
				}
			}
			g_repair = Repair{};
			g_wantRevealPlayer = false;
			g_wantHidePlayer = false;
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
				if (!g_state.npcEverTalked) {
					g_state.npcEverTalked = true;
					g_state.npcTalkStart = a_now;
				}
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

		// Diagnostic: find where the game keeps its field of view. Copies plausible FOV values
		// (degrees or radians) out of a block of memory; guarded because the size of the
		// camera state objects isn't known.
		struct FloatAt
		{
			std::uint32_t offset;
			float         value;
		};

		// Floats within a_tolerance of either value (0.4.11 focus search).
		[[maybe_unused]] std::size_t ScanNear(const void* a_base, std::size_t a_bytes, float a_a, float a_b, float a_tolerance, FloatAt* a_out, std::size_t a_max)
		{
			std::size_t n = 0;
			__try {
				for (std::size_t off = 0; off + 4 <= a_bytes && n < a_max; off += 4) {
					const float v = *reinterpret_cast<const float*>(static_cast<const std::uint8_t*>(a_base) + off);
					if ((a_a > 0.0f && std::abs(v - a_a) < a_tolerance) || (a_b > 0.0f && std::abs(v - a_b) < a_tolerance)) {
						a_out[n++] = { static_cast<std::uint32_t>(off), v };
					}
				}
			} __except (EXCEPTION_EXECUTE_HANDLER) {
			}
			return n;
		}

		[[maybe_unused]] void LogNearValues(const char* a_what, const void* a_base, std::size_t a_bytes, float a_speaker, float a_you)
		{
			if (!a_base) {
				return;
			}
			std::array<FloatAt, 48> found{};
			const auto              n = ScanNear(a_base, a_bytes, a_speaker, a_you, 0.05f, found.data(), found.size());
			std::string             text;
			for (std::size_t i = 0; i < n; ++i) {
				text += std::format(" +{:X}={:.3f}", found[i].offset, found[i].value);
			}
			REX::INFO("Focus search, {} (speaker {:.3f} m, you {:.3f} m):{}", a_what, a_speaker, a_you, text.empty() ? " none" : text);
		}

		std::size_t ScanFloats(const void* a_base, std::size_t a_from, std::size_t a_to, FloatAt* a_out, std::size_t a_max)
		{
			std::size_t n = 0;
			__try {
				for (std::size_t off = a_from; off + 4 <= a_to && n < a_max; off += 4) {
					const float v = *reinterpret_cast<const float*>(static_cast<const std::uint8_t*>(a_base) + off);
					if ((v >= 15.0f && v <= 150.0f) || (v >= 0.25f && v <= 2.7f && v != 1.0f)) {
						a_out[n++] = { static_cast<std::uint32_t>(off), v };
					}
				}
			} __except (EXCEPTION_EXECUTE_HANDLER) {
			}
			return n;
		}

		void LogFovCandidates(const char* a_what, int a_mode, const void* a_base, std::size_t a_from, std::size_t a_to)
		{
			if (!a_base) {
				return;
			}
			std::array<FloatAt, 96> found{};
			const auto              n = ScanFloats(a_base, a_from, a_to, found.data(), found.size());
			std::string             text;
			for (std::size_t i = 0; i < n; ++i) {
				text += std::format(" +{:X}={:.3f}", found[i].offset, found[i].value);
			}
			REX::INFO("FOV scan, mode {}, {}:{}", a_mode, a_what, text);
		}

		// Diagnostic: look inside a game object for a stored position near a_near (within
		// a_radius metres), to find where the dialogue camera keeps its own, current position.
		std::size_t ScanPositions(const void* a_base, std::size_t a_from, std::size_t a_to, const RE::NiPoint3& a_near, float a_radius, std::uint32_t* a_offsets, float* a_values, std::size_t a_max)
		{
			std::size_t n = 0;
			__try {
				for (std::size_t off = a_from; off + 12 <= a_to && n < a_max; off += 4) {
					const float* v = reinterpret_cast<const float*>(static_cast<const std::uint8_t*>(a_base) + off);
					const float  dx = v[0] - a_near.x, dy = v[1] - a_near.y, dz = v[2] - a_near.z;
					if (dx * dx + dy * dy + dz * dz < a_radius * a_radius) {
						a_offsets[n] = static_cast<std::uint32_t>(off);
						a_values[n * 3] = v[0];
						a_values[n * 3 + 1] = v[1];
						a_values[n * 3 + 2] = v[2];
						++n;
					}
				}
			} __except (EXCEPTION_EXECUTE_HANDLER) {
			}
			return n;
		}

		void LogPositionCandidates(const char* a_what, const void* a_base, std::size_t a_from, std::size_t a_to, const RE::NiPoint3& a_near)
		{
			if (!a_base) {
				return;
			}
			std::uint32_t offsets[24]{};
			float         values[72]{};
			const auto    n = ScanPositions(a_base, a_from, a_to, a_near, 2.5f, offsets, values, 24);
			std::string   text;
			for (std::size_t i = 0; i < n; ++i) {
				text += std::format(" +{:X}=({:.2f} {:.2f} {:.2f})", offsets[i], values[i * 3] - a_near.x, values[i * 3 + 1] - a_near.y, values[i * 3 + 2] - a_near.z);
			}
			REX::INFO("Position scan, {} (offsets from the predicted spot):{}", a_what, text.empty() ? " none" : text);
		}

		// Diagnostic: list the game's own INI settings that mention field of view, the
		// dialogue camera or the camera in general, with their current values. The game's
		// dialogue zoom is most likely one of these, and they can be changed at run time.
		void LogCameraSettings(RE::INISettingCollection* a_collection, const char* a_which)
		{
			if (!a_collection) {
				REX::INFO("Settings scan: {} collection unavailable", a_which);
				return;
			}
			std::size_t total = 0;
			std::size_t shown = 0;
			for (const auto& setting : a_collection->settings) {
				if (!setting) {
					continue;
				}
				++total;
				const std::string_view key = setting->GetKey();
				std::string            lower{ key };
				std::ranges::transform(lower, lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
				if (lower.find("fov") == std::string::npos && lower.find("dialog") == std::string::npos &&
					lower.find("camera") == std::string::npos && lower.find("zoom") == std::string::npos &&
					lower.find("dof") == std::string::npos && lower.find("focus") == std::string::npos &&
					lower.find("depth") == std::string::npos && lower.find("bokeh") == std::string::npos &&
					lower.find("blur") == std::string::npos) {
					continue;
				}
				++shown;
				switch (setting->GetType()) {
				case RE::Setting::Type::kFloat:
					REX::INFO("Setting [{}] {} = {:.4f}", a_which, key, setting->GetFloat());
					break;
				case RE::Setting::Type::kInt:
					REX::INFO("Setting [{}] {} = {}", a_which, key, setting->GetInt());
					break;
				case RE::Setting::Type::kUInt:
					REX::INFO("Setting [{}] {} = {}", a_which, key, setting->GetUInt());
					break;
				case RE::Setting::Type::kBool:
					REX::INFO("Setting [{}] {} = {}", a_which, key, setting->GetBool());
					break;
				default:
					REX::INFO("Setting [{}] {} (other type)", a_which, key);
					break;
				}
			}
			REX::INFO("Settings scan: {} collection has {} settings, {} listed", a_which, total, shown);
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

		bool ReadFormInfo(std::uintptr_t a_ptr, std::uint32_t& a_type, std::uint32_t& a_id, const char*& a_editorId);
		std::string LineEmotion(RE::Actor* a_speaker);

		// Is this expression tag one of the INI's close-up emotions (case doesn't matter)?
		bool IsCloseUpEmotion(std::string_view a_emotion)
		{
			if (a_emotion.empty() || a_emotion == "none") {
				return false;
			}
			auto lower = [](std::string_view a_text) {
				std::string out;
				for (const char c : a_text) {
					if (c != ' ') {
						out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
					}
				}
				return out;
			};
			const auto        wanted = lower(a_emotion);
			const std::string list = lower(Settings::Get().closeUpEmotions);
			std::size_t       start = 0;
			while (start <= list.size()) {
				const auto end = list.find(',', start);
				if (list.substr(start, end == std::string::npos ? std::string::npos : end - start) == wanted) {
					return true;
				}
				if (end == std::string::npos) {
					break;
				}
				start = end + 1;
			}
			return false;
		}

		RE::BGSScene* CurrentSceneUnsafe(RE::TESObjectREFR* a_ref)
		{
			__try {
				return a_ref->GetCurrentScene();
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				return nullptr;
			}
		}

		// The quest scene this reference is playing a part in, if any.
		std::string SceneLabel(RE::TESObjectREFR* a_ref, bool& a_inScene)
		{
			a_inScene = false;
			auto* scene = a_ref ? CurrentSceneUnsafe(a_ref) : nullptr;
			if (!scene) {
				return "not in a scene";
			}
			std::uint32_t type = 0, id = 0;
			const char*   editorId = nullptr;
			if (!ReadFormInfo(reinterpret_cast<std::uintptr_t>(scene), type, id, editorId) || type != std::to_underlying(RE::FormType::kSCEN)) {
				return std::format("unrecognised value {:p} (not counted)", static_cast<void*>(scene));
			}
			a_inScene = true;
			// Diagnostic (0.4.14): the quest this scene belongs to, and its type, to recognise
			// story scenes by their quest rather than by the scene's name.
			std::string quest;
			std::array<std::uintptr_t, 0x40> words{};
			if (SafeCopy(words.data(), scene, sizeof(words))) {
				for (std::size_t k = 0; k < words.size() && quest.empty(); ++k) {
					std::uint32_t qType = 0, qId = 0;
					const char*   qEditorId = nullptr;
					if (ReadFormInfo(words[k], qType, qId, qEditorId) && qType == std::to_underlying(RE::FormType::kQUST)) {
						std::array<std::uint8_t, 8> data{};
						SafeCopy(data.data(), reinterpret_cast<const void*>(words[k] + 0x108), data.size());
						quest = std::format("; quest {:08X} '{}' at scene +{:X}, quest data {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X}", qId, qEditorId ? qEditorId : "", k * 8,
							data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
					}
				}
			}
			return std::format("in scene {:08X} '{}'{}", id, editorId ? editorId : "", quest.empty() ? "; quest not found" : quest);
		}

		// A merchant, judged by the speaker's factions (the game's vendor factions are named
		// for it). Every faction pointer is checked as a real form before it is read.
		std::string MerchantFaction(RE::Actor* a_actor, bool a_log)
		{
			auto* npc = a_actor ? a_actor->GetNPC() : nullptr;
			if (!npc) {
				return {};
			}
			std::string found;
			std::string all;
			// Copied out under a guard: if this game version's layout differed, a bad list
			// must not crash the game.
			const auto* first = npc->factions.data();
			const auto  count = (std::min)(static_cast<std::size_t>(npc->factions.size()), std::size_t{ 200 });
			for (std::size_t i = 0; first && i < count; ++i) {
				RE::FACTION_RANK entry{};
				if (!SafeCopy(&entry, first + i, sizeof(entry))) {
					break;
				}
				std::uint32_t type = 0, id = 0;
				const char*   editorId = nullptr;
				if (!entry.faction || !ReadFormInfo(reinterpret_cast<std::uintptr_t>(entry.faction), type, id, editorId) ||
					type != std::to_underlying(RE::FormType::kFACT)) {
					continue;
				}
				const char* name = static_cast<const RE::TESFullName*>(entry.faction)->GetFullName();
				// The faction's flags (1.16.244: +0x68) carry the vendor bit 0x4000, as in
				// Fallout 4. Found in 0.4.11 by comparing shop factions (Jemison Mercantile,
				// Viewport, Ryujin Industries) with ordinary ones.
				std::uint32_t flags = 0;
				SafeCopy(&flags, reinterpret_cast<const std::uint8_t*>(entry.faction) + 0x68, sizeof(flags));
				const bool  vendor = (flags & 0x4000) != 0;
				std::string label = std::format("{:08X} '{}' rank {}{}", id, name ? name : "", entry.rank, vendor ? " (vendor)" : "");
				if (a_log && all.size() < 1500) {
					all += (all.empty() ? "" : "; ") + label;
				}
				if (found.empty() && entry.rank > -1 && vendor) {
					found = label;
				}
			}
			if (a_log) {
				REX::INFO("Speaker factions: {}", all.empty() ? "none listed" : all);
			}
			return found;
		}

		// Seated speakers and merchants get the plain over-the-shoulder shot (0.4.10).
		void DecidePlainShots(RE::Actor* a_speaker, float a_headHeight, bool a_log)
		{
			const auto& s = Settings::Get();
			std::string reason;
			if (s.plainShotsSeated && a_headHeight < s.seatedHeadHeight) {
				reason = std::format("they're sitting (head {:.2f} m above their feet)", a_headHeight);
			}
			const auto merchant = MerchantFaction(a_speaker, a_log && s.debugLog);
			g_state.merchant = !merchant.empty();
			if (reason.empty() && s.plainShotsMerchants && !merchant.empty()) {
				reason = "merchant (faction " + merchant + ")";
			}
			if (!reason.empty() != g_state.plainShots || a_log) {
				REX::INFO("Shots: {}", reason.empty() ? "shot library" : "plain over-the-shoulder shot; " + reason);
			}
			g_state.plainShots = !reason.empty();
		}

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
				// First-person conversations: keep the last shot until the game leaves its dialogue
				// camera (the first-person camera hook then cuts straight to the player's view),
				// instead of showing the game's own close-up for the last half second.
				const bool holdToEnd = g_state.startedFirstPerson && g_firstPersonView && held < 3.0f;
				if ((held < s.exitHoldSeconds || holdToEnd) && Camera::ApplyRender(camera->cameraRoot.get(), *g_state.lastPose, 0.0f)) {
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
			// Back in the dialogue camera after the body rebuild. The camera root still reports
			// the third-person position for a couple of frames after the switch (measured in
			// 0.3.22: 1.7 m behind the eyes), while the picture is already drawn from the eyes;
			// placing the shot against the reported root put it 1.7 m too far forward for two
			// frames. Keep using the predicted root until the reported one has caught up.
			const Camera::Pose* predictedRoot = nullptr;
			if (g_repair.pendingMode == 17 && inDialogueState && g_repair.pendingRoot) {
				const auto actual = Camera::PoseOf(camera->cameraRoot.get());
				const auto& p = *g_repair.pendingRoot;
				bool        caughtUp = false;
				if (actual) {
					const float dx = actual->position.x - p.position.x, dy = actual->position.y - p.position.y, dz = actual->position.z - p.position.z;
					caughtUp = dx * dx + dy * dy + dz * dz < 0.3f * 0.3f;
				}
				const bool expired = std::chrono::steady_clock::now() - g_repair.pendingSince > 400ms;
				if (g_returnScanFrames < 60) {
					++g_returnScanFrames;
					LogRootAgainstPrediction(caughtUp ? "reported root, caught up" : "reported root, stale", camera->cameraRoot.get());
				}
				if (caughtUp && actual) {
					// What is the dialogue camera's real spot made of? Compare with everything known.
					const auto pHead = Camera::HeadPosition(player);
					const auto nHead = Camera::HeadPosition(speaker);
					const auto feet = player->GetPosition();
					const auto nFeet = speaker->GetPosition();
					const auto& a = *actual;
					REX::INFO("Dialogue root facts: root ({:.2f} {:.2f} {:.2f}) aim ({:.3f} {:.3f} {:.3f}); player feet ({:.2f} {:.2f} {:.2f}) head ({:.2f} {:.2f} {:.2f}); npc feet ({:.2f} {:.2f} {:.2f}) head ({:.2f} {:.2f} {:.2f})",
						a.position.x, a.position.y, a.position.z, (a.lookAt.x - a.position.x) / 5.0f, (a.lookAt.y - a.position.y) / 5.0f, (a.lookAt.z - a.position.z) / 5.0f,
						feet.x, feet.y, feet.z, pHead ? pHead->x : 0.0f, pHead ? pHead->y : 0.0f, pHead ? pHead->z : 0.0f,
						nFeet.x, nFeet.y, nFeet.z, nHead ? nHead->x : 0.0f, nHead ? nHead->y : 0.0f, nHead ? nHead->z : 0.0f);
					if (g_firstPersonView) {
						const auto& f = *g_firstPersonView;
						REX::INFO("Dialogue root facts: first-person view was at ({:.2f} {:.2f} {:.2f}) aim ({:.3f} {:.3f} {:.3f})",
							f.position.x, f.position.y, f.position.z, (f.lookAt.x - f.position.x) / 5.0f, (f.lookAt.y - f.position.y) / 5.0f, (f.lookAt.z - f.position.z) / 5.0f);
					}
				}
				if (caughtUp || expired) {
					LogRootAgainstPrediction(caughtUp ? "camera root caught up" : "camera root never caught up", camera->cameraRoot.get());
					g_repair.pendingMode = -1;
				} else {
					predictedRoot = &p;
				}
			}

			if (!g_state.active) {
				Begin();
			}

			const auto now = std::chrono::steady_clock::now();
			const float delta = std::chrono::duration<float>(now - g_state.lastFrame).count();
			g_state.lastFrame = now;
			++g_state.framesThisConversation;
			// The game keeps drawing with the gameplay field of view (about 94 degrees) for the
			// first frame or few of a conversation before switching to its narrow dialogue
			// one (about 30). Measured in 0.3.31: one frame from third person, three from
			// first person. A conversation shot drawn that wide flashes on screen, so on those
			// frames we show the view the player had just before instead.
			float gameFov = 0.0f;
			SafeCopy(&gameFov, reinterpret_cast<std::uint8_t*>(camera) + kPlayerCameraFovOffset, sizeof(float));
			const bool wideFrame = gameFov > 60.0f && gameFov < 170.0f;
			if (s.debugLog && g_conversationNumber <= 40 && g_state.framesThisConversation <= 12) {
				REX::INFO("FOV frame {} (+{} ms): game's field of view {:.2f}{}", g_state.framesThisConversation,
					std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - g_state.started).count(), gameFov,
					wideFrame ? "; showing the gameplay view" : "");
			}

			auto npcHead = Camera::HeadPosition(speaker);
			auto playerHead = Camera::HeadPosition(player);
			if (!npcHead || !playerHead) {
				LogFailure("head position unavailable; leaving vanilla camera");
				return;
			}
			// Small speakers (robots and the like): the shots are built around a head at
			// human height and would miss them, so leave the game's own camera on them.
			if (g_state.framesThisConversation == 1) {
				const float height = npcHead->z - speaker->GetPosition().z;
				g_state.smallSpeaker = !Camera::HasHeadBone(speaker) || height < s.minSpeakerHeight;
				if (g_state.smallSpeaker) {
					REX::INFO("Speaker {} (head bone {}, {:.2f} m tall); leaving the game's camera on this conversation",
						Camera::HasHeadBone(speaker) ? "is short" : "has no head bone", Camera::HasHeadBone(speaker) ? "found" : "missing", height);
					g_state.needsBodyRepair = false;
				}
				// Quest scenes (someone walks up and starts talking, several people taking turns):
				// the game directs its own camera there, so leave it to the game from the start.
				bool        speakerInScene = false, playerInScene = false;
				const auto  speakerScene = SceneLabel(speaker, speakerInScene);
				const auto  playerScene = SceneLabel(player, playerInScene);
				const char* name = speaker->GetDisplayFullName();
				REX::INFO("Scene check: speaker '{}' {:08X}: {}; you: {}", name ? name : "", speaker->formID, speakerScene, playerScene);
				// Only quest scenes the player takes part in. Everyday background scenes (people
				// chatting, a receptionist's greeting) are named "Dialogue..." in the game's data and
				// keep this mod's camera, as do scenes the speaker is in without you.
				// Crew recruits (CREW_Generic_Recruit and the like) are everyday chats too.
				const auto sceneName = playerScene.substr(0, playerScene.find("; quest"));  // the scene's own name, not its quest's
				const bool ambient = sceneName.find("'Dialogue") != std::string::npos || sceneName.find("'CREW_") != std::string::npos;
				if (!g_state.smallSpeaker && s.gameCameraInScenes && playerInScene && !ambient) {
					g_state.handedBack = true;
					g_state.needsBodyRepair = false;
					REX::INFO("Quest scene: leaving the game's camera on this conversation");
				}
				g_state.backgroundScene = speakerInScene && !g_state.handedBack;
				if (!g_state.smallSpeaker && !g_state.handedBack) {
					DecidePlainShots(static_cast<RE::Actor*>(speaker), height, true);
				}
			}
			// Who is speaking: in scenes the line can pass to someone else mid-conversation.
			if (speaker->formID != g_state.speakerId) {
				if (g_state.speakerId != 0) {
					const auto  feet = speaker->GetPosition();
					const auto  pf = player->GetPosition();
					const float dx = feet.x - pf.x, dy = feet.y - pf.y;
					const char* name = speaker->GetDisplayFullName();
					REX::INFO("Speaker changed (frame {}): {:08X} -> {:08X} '{}', {:.2f} m from you", g_state.framesThisConversation, g_state.speakerId, speaker->formID,
						name ? name : "", std::sqrt(dx * dx + dy * dy));
				}
				if (g_state.speakerId != 0 && !g_state.smallSpeaker && !g_state.handedBack) {
					DecidePlainShots(static_cast<RE::Actor*>(speaker), npcHead->z - speaker->GetPosition().z, false);
				}
				g_state.speakerId = speaker->formID;
			}

			if (g_state.smallSpeaker || g_state.handedBack) {
				// The game's own camera throughout. From first person, keep the player hidden (the
				// game's camera is at their eyes); from third person, leave visibility to the game,
				// so nothing pops in when it ends.
				Camera::RestoreRender();
				if (g_state.startedFirstPerson) {
					g_wantHidePlayer = true;
					Visibility::HidePlayer();
				}
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
				if (g_state.bodyReady && rawPlayerHeadHeight >= kStandingHeadHeight && rawPlayerHeadHeight < 2.2f) {
					g_standingHeadHeight = rawPlayerHeadHeight;
				}
				// Until the first-person body has been rebuilt, the head bone sits lower and
				// jumps when the rebuild happens, which made the camera (placed relative to it)
				// swing. Use where the head will be instead: standing height above the feet.
				if (g_state.startedFirstPerson && !g_state.bodyReady) {
					auto feet = player->GetPosition();
					feet.z += g_standingHeadHeight;
					playerHead = feet;
				}
				npcHead = follow(g_state.smoothNpcHead, *npcHead);
				playerHead = follow(g_state.smoothPlayerHead, *playerHead);
			}

			// Data collection: the first frames of every conversation, and one steady frame.
			if (s.debugLog && g_conversationNumber <= 12) {
				const auto frame = g_state.framesThisConversation;
				if (frame <= 6) {
					const auto  reported = Camera::PoseOf(camera->cameraRoot.get());
					const auto& view = g_state.startedFirstPerson ? g_firstPersonView : g_thirdPersonView;
					const auto  feet = player->GetPosition();
					const auto  nFeet = speaker->GetPosition();
					if (reported && view) {
						REX::INFO("Start frame {} (mode {}): reported root ({:.2f} {:.2f} {:.2f}) aim ({:.3f} {:.3f} {:.3f}); gameplay view was ({:.2f} {:.2f} {:.2f}) aim ({:.3f} {:.3f} {:.3f})",
							frame, stateIndex, reported->position.x, reported->position.y, reported->position.z,
							(reported->lookAt.x - reported->position.x) / 5.0f, (reported->lookAt.y - reported->position.y) / 5.0f, (reported->lookAt.z - reported->position.z) / 5.0f,
							view->position.x, view->position.y, view->position.z,
							(view->lookAt.x - view->position.x) / 5.0f, (view->lookAt.y - view->position.y) / 5.0f, (view->lookAt.z - view->position.z) / 5.0f);
					}
					if (frame == 1) {
						REX::INFO("Start facts: player feet ({:.2f} {:.2f} {:.2f}) head ({:.2f} {:.2f} {:.2f}); npc feet ({:.2f} {:.2f} {:.2f}) head ({:.2f} {:.2f} {:.2f}), npc head {:.2f} m above its feet",
							feet.x, feet.y, feet.z, playerHead->x, playerHead->y, playerHead->z, nFeet.x, nFeet.y, nFeet.z, npcHead->x, npcHead->y, npcHead->z, npcHead->z - nFeet.z);
					}
				}
				// Every frame until the rebuild is over and for a few frames after: what the
				// camera nodes held when this frame's update ran, and what we showed last frame.
				if (frame <= 6 || g_repair.stage != RepairStage::kIdle || frame <= g_state.framesAtRepairEnd + 6) {
					if (g_repair.stage != RepairStage::kIdle) {
						g_state.framesAtRepairEnd = frame;
					}
					REX::INFO("Frame {} (mode {}, rebuild stage {}): {}; last shot placed at ({:.2f} {:.2f} {:.2f})", frame, stateIndex, static_cast<int>(g_repair.stage),
						Camera::DescribeRoot(camera->cameraRoot.get()),
						g_state.lastPose ? g_state.lastPose->position.x : 0.0f, g_state.lastPose ? g_state.lastPose->position.y : 0.0f, g_state.lastPose ? g_state.lastPose->position.z : 0.0f);
				}
				if (frame == 1 || (frame >= 90 && !g_dumpedThisConversation && g_repair.stage == RepairStage::kIdle)) {
					if (frame > 1) {
						g_dumpedThisConversation = true;
					}
					const auto label = std::format("conversation {} frame {}", g_conversationNumber, frame);
					DumpObject((label + " player camera").c_str(), camera, 0x188);
					DumpObject((label + " player camera part 2").c_str(), reinterpret_cast<const std::uint8_t*>(camera) + 0x188, 0x400);
					DumpObject((label + " dialogue state").c_str(), g_dialogueStateObject, 0x340);
					DumpObject((label + " render camera").c_str(), Camera::RenderCameraObject(), 0x400);
				}
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
				if (subject == Subject::kPlayer) {
					g_state.povNpcShot = false;  // from the next NPC shot on, over the shoulder
				}
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
					g_state.directRequested = false;
					g_state.directFailed = false;
				}
			}

			// Over the near character's shoulder, unless the near character is clearly lower
			// (sitting): a camera behind a seated head ends up inside the chair back, so step
			// in front of and beside them instead and shoot a clean single of the other person.
			const auto& nearHeadRef = subject == Subject::kNpc ? *playerHead : *npcHead;
			const auto& farHeadRef = subject == Subject::kNpc ? *npcHead : *playerHead;
			const bool  nearSeated = farHeadRef.z - nearHeadRef.z > 0.2f;
			// First-person conversations open on the NPC from just in front of the player's
			// eyes: while the body is being rebuilt (the game passes through first and third
			// person), nothing of the player (arms, half-built body) is in front of the camera.
			const bool  pov = subject == Subject::kNpc && g_state.povNpcShot;
			const auto& gameplayView = g_state.startedFirstPerson ? g_firstPersonView : g_thirdPersonView;
			// Conversation start: for a frame or few the game draws from its dialogue camera
			// while still reporting the gameplay camera's position to us (0.3.33/0.3.34 logs and
			// clips: 1-3 frames of the game's own close-up). Detect that and place the shot
			// against where the dialogue camera is about to be reported instead.
			std::optional<Camera::Pose> startRoot;
			const int startMode = g_state.startedFirstPerson ? s.startMode : s.startModeThird;
			if ((startMode == 1 || startMode == 2) && !predictedRoot && !g_state.startRootCaughtUp && gameplayView && g_state.framesThisConversation <= 20) {
				const auto reported = Camera::PoseOf(camera->cameraRoot.get());
				bool       stale = false;
				if (reported) {
					const auto& g = *gameplayView;
					const float dx = reported->position.x - g.position.x, dy = reported->position.y - g.position.y, dz = reported->position.z - g.position.z;
					stale = dx * dx + dy * dy + dz * dz < 0.03f * 0.03f;
				}
				RE::NiPoint3 eye = g_state.startedFirstPerson ? gameplayView->position : RE::NiPoint3{ playerHead->x, playerHead->y, playerHead->z + 0.06f };
				float        hx = npcHead->x - eye.x, hy = npcHead->y - eye.y;
				const float  hl = std::sqrt(hx * hx + hy * hy);
				if (hl > 1e-3f) {
					hx /= hl;
					hy /= hl;
				}
				if (stale) {
					Camera::Pose p;
					// Sideways the game picks either side; in testing it was the same side about
					// four times in five, so predict the side seen most often.
					const float lateral = g_lateralVotes >= 0 ? -std::abs(g_learnedLateral) : std::abs(g_learnedLateral);
					p.position = { eye.x - hx * g_learnedBack + hy * lateral, eye.y - hy * g_learnedBack - hx * lateral, eye.z + g_learnedUp };
					p.lookAt = *npcHead;
					startRoot = p;
					if (s.debugLog) {
						REX::INFO("Start frame {}: game still reports the gameplay camera; placing against the predicted dialogue camera ({:.2f} {:.2f} {:.2f})",
							g_state.framesThisConversation, p.position.x, p.position.y, p.position.z);
					}
				} else {
					g_state.startRootCaughtUp = true;
					if (reported && g_state.framesThisConversation > 1) {
						const float ox = reported->position.x - eye.x, oy = reported->position.y - eye.y, oz = reported->position.z - eye.z;
						const float back = -(ox * hx + oy * hy);
						const float lateral = ox * hy - oy * hx;
						REX::INFO("Start frame {}: game now reports its dialogue camera: {:.2f} m back from the eyes, {:+.2f} m sideways, {:+.2f} m up (prediction used {:.2f} back, {:+.2f} up)",
							g_state.framesThisConversation, back, lateral, oz, g_learnedBack, g_learnedUp);
						if (back > 0.1f && back < 0.6f && std::abs(oz) < 0.3f && std::abs(lateral) < 0.4f) {
							g_learnedBack = 0.5f * g_learnedBack + 0.5f * back;
							g_learnedUp = 0.5f * g_learnedUp + 0.5f * oz;
							if (std::abs(lateral) > 0.05f) {
								g_learnedLateral = 0.5f * std::abs(g_learnedLateral) + 0.5f * std::abs(lateral);
								g_lateralVotes = std::clamp(g_lateralVotes + (lateral < 0.0f ? 1 : -1), -5, 5);
							}
						}
					} else if (g_state.framesThisConversation == 1) {
						REX::INFO("Start frame 1: game already reports its dialogue camera");
					}
				}
			}
			std::optional<Camera::Pose> waitingView;
			if (gameplayView && ((startMode == 0 && wideFrame) || (startMode == 2 && startRoot))) {
				waitingView = *gameplayView;
			}
			// The expression tag of the line the speaker is starting (read once per line), for
			// the emotion close-ups.
			{
				const bool talkingNow = speaker->IsTalking();
				if (talkingNow && !g_state.speakerWasTalking) {
					g_state.lineIntense = s.emotionCloseUps && IsCloseUpEmotion(LineEmotion(static_cast<RE::Actor*>(speaker)));
				}
				g_state.speakerWasTalking = talkingNow;
			}
			std::optional<Camera::Pose> directed;
			if (s.director && !waitingView && !pov && g_state.plainShots) {
				// Plain shots (seated people, merchants): only the emotion close-ups, from in
				// front of them, so the chair or the counter behind them stays out of the way.
				Director::Frame frame;
				frame.npcTurn = subject == Subject::kNpc;
				frame.npcTalking = speaker->IsTalking();
				frame.playerTalking = player->IsTalking();
				frame.nearSeated = nearSeated;
				frame.intenseLine = g_state.lineIntense;
				frame.npcHead = *npcHead;
				frame.playerHead = *playerHead;
				directed = Director::UpdatePlain(frame, now);
			}
			if (s.director && !waitingView && !pov && !g_state.plainShots) {
				Director::Frame frame;
				frame.npcTurn = subject == Subject::kNpc;
				frame.npcTalking = speaker->IsTalking();
				frame.playerTalking = player->IsTalking();
				frame.nearSeated = nearSeated;
				frame.intenseLine = g_state.lineIntense;
				frame.npcHead = *npcHead;
				frame.playerHead = *playerHead;
				directed = Director::Update(frame, now);
			}
			const auto  pose = waitingView ? waitingView :
			                   pov ? Camera::Shot(nearHeadRef, farHeadRef, -0.30f, 0.0f, 0.0f) :
			                   directed ? directed :
			                   nearSeated || s.shotStyle == 1 ?
			                   Camera::Shot(nearHeadRef, farHeadRef, -s.seatedForward, s.seatedSide, s.seatedHeight) :
			                   g_state.plainShots && g_state.merchant && subject == Subject::kPlayer ?
			                   Camera::Shot(nearHeadRef, farHeadRef, -s.seatedForward, s.vendorSide, s.vendorHeight) :
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
				if (!directed && camGap > 3.0f * headGap) {
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
			// While the first-person body is being rebuilt, keep the player hidden: the new
			// head appears around the camera (0.3.31 showed one frame of the eyes from inside).
			g_wantHidePlayer = g_state.hidePlayer || (g_state.startedFirstPerson && !g_state.bodyReady);
			if (g_wantRevealPlayer) {
				Visibility::RevealPlayer();
			} else if (g_wantHidePlayer) {
				Visibility::HidePlayer();
			}

			if (g_state.framesThisConversation == 1) {
				REX::INFO("frame 1: applying pose to {}", s.cameraTarget == 1 ? "render camera" : "camera root");
			}
			{
				const auto& subjectHead = subject == Subject::kNpc ? *npcHead : *playerHead;
				const float dx = pose->position.x - subjectHead.x, dy = pose->position.y - subjectHead.y, dz = pose->position.z - subjectHead.z;
				g_shotDistance.store(waitingView || pov ? 0.0f : std::sqrt(dx * dx + dy * dy + dz * dz), std::memory_order_relaxed);
			}
			// Depth of field: the game focuses at the distance from its own camera (at your eyes)
			// to the NPC, about 2 m, while the picture is drawn from further back, so the person
			// in shot was out of focus and whoever happened to be 2 m from the lens was sharp.
			// Move the game's camera straight back along the line from the NPC (so the NPC still
			// faces the same way) until its distance matches the distance to the person in shot.
			if (s.focusOnSubject && s.cameraTarget == 1 && !waitingView && !pov && !predictedRoot && !startRoot && g_repair.stage == RepairStage::kIdle) {
				const auto& subjectHead = subject == Subject::kNpc ? *npcHead : *playerHead;
				const float fx = pose->position.x - subjectHead.x, fy = pose->position.y - subjectHead.y, fz = pose->position.z - subjectHead.z;
				const float focus = std::clamp(std::sqrt(fx * fx + fy * fy + fz * fz), 0.5f, 10.0f);
				const auto& now3 = root->local.translate;
				const float gx = now3.x - npcHead->x, gy = now3.y - npcHead->y, gz = now3.z - npcHead->z;
				const float gameFocus = std::sqrt(gx * gx + gy * gy + gz * gz);
				if (std::abs(focus - gameFocus) > 0.05f) {
					if (const auto before = Camera::SetRootDistance(root, *npcHead, focus)) {
						g_state.rootBeforeFocus = before;
						if (!g_state.focusLogged && s.debugLog) {
							g_state.focusLogged = true;
							REX::INFO("Focus: game camera moved from {:.2f} m to {:.2f} m from the NPC to match the shot", gameFocus, focus);
						}
					}
				}
			}

			// Prefer moving only the rendered view; if the render camera can't be found,
			// fall back to the game camera so shots still work (NPCs may turn, as in 0.1.x).
			bool applied = s.cameraTarget == 1 && Camera::ApplyRender(root, *pose, delta, false, predictedRoot ? predictedRoot : (startRoot ? &*startRoot : nullptr));
			if (!applied) {
				applied = Camera::Apply(root, *pose, delta);
			}
			if (!applied) {
				LogFailure("could not apply pose");
				return;
			}
			g_state.lastPose = *pose;
			g_state.lastAppliedFrame = g_state.framesThisConversation;
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
				g_dialogueStateObject = a_dialogueState;
				if (g_hooksSafe) {
					auto lock = HookLock();
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
		// What to show while the game is out of its dialogue camera for the body rebuild.
		// The gameplay cameras use a much wider field of view than the dialogue camera, and we
		// can't control that yet, so holding a dialogue shot through the rebuild visibly zooms
		// out and back. An early rebuild (on the conversation's first frames) holds the player's
		// own first-person view instead: same position, same field of view as the moment before,
		// so it reads as the first-person view lingering a beat before the cut to the NPC.
		const Camera::Pose* HeldPose()
		{
			if (g_repair.early && g_repair.earlyHeld) {
				return &*g_repair.earlyHeld;
			}
			return g_state.lastPose ? &*g_state.lastPose : nullptr;
		}

		struct ViewBasis
		{
			RE::NiPoint3 f, r, u;
			bool         ok{ false };
		};

		ViewBasis BasisOf(const Camera::Pose& a_view)
		{
			ViewBasis    b;
			RE::NiPoint3 f{ a_view.lookAt.x - a_view.position.x, a_view.lookAt.y - a_view.position.y, a_view.lookAt.z - a_view.position.z };
			const float  fl = std::sqrt(f.x * f.x + f.y * f.y + f.z * f.z);
			if (fl < 1e-4f) {
				return b;
			}
			f = { f.x / fl, f.y / fl, f.z / fl };
			RE::NiPoint3 r{ f.y, -f.x, 0.0f };  // f x up
			const float  rl = std::sqrt(r.x * r.x + r.y * r.y);
			if (rl < 1e-4f) {
				return b;
			}
			r = { r.x / rl, r.y / rl, 0.0f };
			b.f = f;
			b.r = r;
			b.u = { r.y * f.z - r.z * f.y, r.z * f.x - r.x * f.z, r.x * f.y - r.y * f.x };  // r x f
			b.ok = true;
			return b;
		}

		void LearnThirdFromFirst(const Camera::Pose& a_third)
		{
			if (!g_firstPersonView) {
				return;
			}
			const auto b = BasisOf(*g_firstPersonView);
			const auto t = BasisOf(a_third);
			if (!b.ok || !t.ok) {
				return;
			}
			const RE::NiPoint3 d{ a_third.position.x - g_firstPersonView->position.x, a_third.position.y - g_firstPersonView->position.y,
				a_third.position.z - g_firstPersonView->position.z };
			auto dot = [](const RE::NiPoint3& a, const RE::NiPoint3& c) { return a.x * c.x + a.y * c.y + a.z * c.z; };
			RelativeView rel;
			rel.offset[0] = dot(d, b.f);
			rel.offset[1] = dot(d, b.r);
			rel.offset[2] = dot(d, b.u);
			rel.aim[0] = dot(t.f, b.f);
			rel.aim[1] = dot(t.f, b.r);
			rel.aim[2] = dot(t.f, b.u);
			if (!g_thirdFromFirst) {
				REX::INFO("Learned the third-person camera position relative to first person: {:.2f} m forward, {:.2f} m right, {:.2f} m up",
					rel.offset[0], rel.offset[1], rel.offset[2]);
			}
			g_thirdFromFirst = rel;
		}

		std::optional<Camera::Pose> PredictThirdPersonRoot()
		{
			if (!g_thirdFromFirst || !g_firstPersonView) {
				return std::nullopt;
			}
			const auto b = BasisOf(*g_firstPersonView);
			if (!b.ok) {
				return std::nullopt;
			}
			const auto& rel = *g_thirdFromFirst;
			auto        mix = [&](const float* c) {
                return RE::NiPoint3{ b.f.x * c[0] + b.r.x * c[1] + b.u.x * c[2], b.f.y * c[0] + b.r.y * c[1] + b.u.y * c[2],
	                b.f.z * c[0] + b.r.z * c[1] + b.u.z * c[2] };
			};
			const auto   off = mix(rel.offset);
			const auto   aim = mix(rel.aim);
			Camera::Pose pose;
			pose.position = { g_firstPersonView->position.x + off.x, g_firstPersonView->position.y + off.y, g_firstPersonView->position.z + off.z };
			pose.lookAt = { pose.position.x + aim.x * 5.0f, pose.position.y + aim.y * 5.0f, pose.position.z + aim.z * 5.0f };
			return pose;
		}

		void MaskRepairFrame(const char* a_which)
		{
			if (!g_hooksSafe) {
				return;
			}
			auto lock = HookLock();
			if (!lock.owns_lock()) {
				return;
			}
			const auto* held = HeldPose();
			if (!g_state.active || g_repair.stage == RepairStage::kIdle || !held || Settings::Get().cameraTarget != 1) {
				return;
			}
			auto* camera = RE::PlayerCamera::GetSingleton();
			if (!camera) {
				return;
			}
			// This mode's own update has now run, so the root is where it will be drawn.
			const bool third = camera->QCameraEquals(RE::CameraState::kThirdPerson);
			if (g_repair.pendingMode == (third ? 20 : 0)) {
				g_repair.pendingMode = -1;
			}
			if (third) {
				if (const auto pose = Camera::PoseOf(camera->cameraRoot.get())) {
					LearnThirdFromFirst(*pose);
				}
			}
			// Once the dialogue camera has been asked for, a late update of the mode being left
			// must not put the held view back: it would be drawn against the dialogue camera.
			if (g_repair.pendingMode == 17 || camera->QCameraEquals(RE::CameraState::kDialogue)) {
				return;
			}
			if (Camera::ApplyRender(camera->cameraRoot.get(), *held, 0.0f, true)) {
				if (g_repairMaskedFrames++ == 0) {
					REX::INFO("Body repair: holding the shot from the {} camera hook", a_which);
				}
			}
		}

		void RememberGameplayView(std::optional<Camera::Pose>& a_view)
		{
			if (!g_hooksSafe) {
				return;
			}
			auto lock = HookLock();
			if (!lock.owns_lock() || g_state.active) {
				return;
			}
			if (auto* camera = RE::PlayerCamera::GetSingleton()) {
				a_view = Camera::PoseOf(camera->cameraRoot.get());
			}
		}

		void LogExitFrame(RE::PlayerCamera* a_camera, int a_frame)
		{
			const auto reported = Camera::PoseOf(a_camera->cameraRoot.get());
			if (!reported || !g_firstPersonView) {
				return;
			}
			const auto& v = *g_firstPersonView;
			const auto  aim = [](const Camera::Pose& a_p) {
                return RE::NiPoint3{ (a_p.lookAt.x - a_p.position.x) / 5.0f, (a_p.lookAt.y - a_p.position.y) / 5.0f, (a_p.lookAt.z - a_p.position.z) / 5.0f };
			};
			const auto ra = aim(*reported), va = aim(v);
			REX::INFO("Exit first-person frame {}: reported camera ({:.2f} {:.2f} {:.2f}) aim ({:.2f} {:.2f} {:.2f}); first-person view before the conversation ({:.2f} {:.2f} {:.2f}) aim ({:.2f} {:.2f} {:.2f}); field of view {:.1f}",
				a_frame, reported->position.x, reported->position.y, reported->position.z, ra.x, ra.y, ra.z, v.position.x, v.position.y, v.position.z, va.x, va.y, va.z,
				[&] { float f = 0.0f; SafeCopy(&f, reinterpret_cast<std::uint8_t*>(a_camera) + kPlayerCameraFovOffset, sizeof(float)); return f; }());
		}

		bool ExitToFirstPerson()
		{
			if (!g_hooksSafe) {
				return false;
			}
			auto lock = HookLock();
			if (!lock.owns_lock() || !g_state.active || !g_state.startedFirstPerson || !g_state.holdingExit || !g_firstPersonView) {
				return false;
			}
			auto* camera = RE::PlayerCamera::GetSingleton();
			if (!camera) {
				return false;
			}
			const int mode = Settings::Get().exitMode;
			REX::INFO("Exit: cutting from the last shot straight to first person (exit mode {})", mode);
			const auto view = *g_firstPersonView;
			End();
			g_needVisibilityRestore = false;
			Visibility::Restore(false);
			LogExitFrame(camera, 0);
			g_exitLogFrames = 6;
			if (mode != 1 && Camera::ApplyRender(camera->cameraRoot.get(), view, 0.0f, true)) {
				g_exitMasking = true;
				g_exitCoverFrames = mode == 2 ? 3 : 1;
			}
			return true;
		}

		void ExitMaskRelease()
		{
			if (!g_hooksSafe) {
				return;
			}
			auto lock = HookLock();
			if (!lock.owns_lock() || !g_exitMasking) {
				return;
			}
			g_exitMasking = false;
			g_exitCoverFrames = 0;
			Camera::RestoreRender();
			REX::INFO("Exit cover released (left first person)");
		}

		void ExitMaskEnd()
		{
			if (!g_hooksSafe) {
				return;
			}
			auto lock = HookLock();
			if (!lock.owns_lock()) {
				return;
			}
			auto* camera = RE::PlayerCamera::GetSingleton();
			if (g_exitLogFrames > 0 && camera) {
				LogExitFrame(camera, 7 - g_exitLogFrames);
				--g_exitLogFrames;
			}
			if (!g_exitMasking) {
				return;
			}
			if (--g_exitCoverFrames > 0 && camera && g_firstPersonView &&
				Camera::ApplyRender(camera->cameraRoot.get(), *g_firstPersonView, 0.0f, true)) {
				return;  // keep covering
			}
			g_exitMasking = false;
			g_exitMaskUntil = {};
			Camera::RestoreRender();
		}

		struct FirstPersonUpdateHook
		{
			static void Thunk(void* a_state, void* a_arg)
			{
				Hook(a_state, a_arg);
				if (ExitToFirstPerson()) {
					return;  // this frame is drawn from the saved view; don't learn from it
				}
				ExitMaskEnd();
				if (g_exitMasking) {
					return;
				}
				MaskRepairFrame("first-person");
				RememberGameplayView(g_firstPersonView);
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
				// A first-person exit cover still in place when the player switches to third
				// person (0.3.35: the view stayed behind the head until the next conversation).
				if (g_exitMasking) {
					ExitMaskRelease();
				}
				RememberGameplayView(g_thirdPersonView);
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

		// Right after we ask the game to change camera mode, it can render a frame before the
		// new mode's own update (and our hook on it) has run; the recording showed one wildly
		// aimed frame at that point. Re-apply the held shot immediately.
		void HoldShotNow(RE::PlayerCamera* a_camera)
		{
			if (Settings::Get().cameraTarget != 1) {
				return;
			}
			auto* root = a_camera->cameraRoot.get();
			if (g_repair.pendingMode >= 0 && std::chrono::steady_clock::now() - g_repair.pendingSince > (g_repair.pendingMode == 17 ? 400ms : 120ms)) {
				g_repair.pendingMode = -1;
			}
			if (g_repair.pendingMode >= 0 && g_repair.pendingRoot && g_repair.pendingShot) {
				// A mode change is in flight: place the shot against where the root is going.
				Camera::ApplyRender(root, *g_repair.pendingShot, 0.0f, g_repair.pendingMode != 17, &*g_repair.pendingRoot);
				return;
			}
			if (a_camera->QCameraEquals(RE::CameraState::kDialogue)) {
				return;  // dialogue frames are placed by the dialogue camera hook
			}
			if (const auto* held = HeldPose()) {
				Camera::ApplyRender(root, *held, 0.0f, true);
			}
		}

		void LogRootAgainstPrediction(const char* a_when, RE::NiAVObject* a_root)
		{
			const auto actual = Camera::PoseOf(a_root);
			if (!actual || !g_repair.pendingRoot) {
				return;
			}
			auto dir = [](const Camera::Pose& a_p) {
				RE::NiPoint3 d{ a_p.lookAt.x - a_p.position.x, a_p.lookAt.y - a_p.position.y, a_p.lookAt.z - a_p.position.z };
				const float  l = (std::max)(std::sqrt(d.x * d.x + d.y * d.y + d.z * d.z), 1e-5f);
				return RE::NiPoint3{ d.x / l, d.y / l, d.z / l };
			};
			const auto& p = *g_repair.pendingRoot;
			const auto  da = dir(*actual), dp = dir(p);
			const float dot = std::clamp(da.x * dp.x + da.y * dp.y + da.z * dp.z, -1.0f, 1.0f);
			const auto  ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - g_repair.pendingSince).count();
			REX::INFO("Dialogue return +{} ms ({}): camera root is ({:.2f}, {:.2f}, {:.2f}) m from the predicted spot, aimed {:.1f} degrees off (actual dir {:.2f} {:.2f} {:.2f}, predicted {:.2f} {:.2f} {:.2f})",
				ms, a_when, actual->position.x - p.position.x, actual->position.y - p.position.y, actual->position.z - p.position.z,
				std::acos(dot) * 57.2958f, da.x, da.y, da.z, dp.x, dp.y, dp.z);
		}

		void ExpectMode(int a_mode, std::optional<Camera::Pose> a_root, std::optional<Camera::Pose> a_shot)
		{
			g_repair.pendingMode = a_root && a_shot ? a_mode : -1;
			g_repair.pendingRoot = a_root;
			g_repair.pendingShot = a_shot;
			g_repair.pendingSince = std::chrono::steady_clock::now();
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
			if (g_firstPersonView && g_state.smoothNpcHead && g_repair.stage != RepairStage::kIdle) {
				// The dialogue camera's root sits at the player's eyes looking at the speaker.
				Camera::Pose root{ g_firstPersonView->position, *g_state.smoothNpcHead };
				auto         playerHead = RE::PlayerCharacter::GetSingleton()->GetPosition();
				playerHead.z += g_standingHeadHeight;
				const auto shot = g_state.povNpcShot ? Camera::Shot(playerHead, *g_state.smoothNpcHead, -0.30f, 0.0f, 0.0f) :
				                                       Camera::OverShoulder(playerHead, *g_state.smoothNpcHead);
				ExpectMode(17, root, shot);
			}
			HoldShotNow(a_camera);
			return true;
		}

		// An attempt didn't work. Make sure the game is back in its dialogue camera, then try
		// again later (not immediately: 0.3.12 kept re-requesting first person every 16 ms for
		// 1.8 s, which left the body stuck in a T-pose). After the last attempt, give up and
		// keep the player out of frame for this conversation.
		void AbortAttempt(RE::PlayerCamera* a_camera, std::chrono::steady_clock::time_point a_now, const char* a_why, bool a_refused = false)
		{
			if (!a_camera->QCameraEquals(RE::CameraState::kDialogue) && !ReturnToDialogueCamera(a_camera)) {
				return;  // conversation is over; End() already ran
			}
			if (a_refused && Settings::Get().debugLog && g_conversationNumber <= 12) {
				const auto label = std::format("conversation {} rebuild refused", g_conversationNumber);
				DumpObject((label + " player camera").c_str(), a_camera, 0x188);
				DumpObject((label + " dialogue state").c_str(), g_dialogueStateObject, 0x340);
			}
			const auto attempts = g_repair.attempts + 1;
			g_repair = Repair{};
			g_repair.attempts = attempts;
			g_state.bodyReady = false;  // whatever we assumed mid-attempt, the body isn't confirmed
			g_lastHookTime = a_now;
			// When the game refuses to leave its dialogue camera (seated NPCs, in every log so
			// far), it refuses every time; further attempts only make the picture flicker.
			// Test: when refused on the opening frames, ask once more a few seconds in, to
			// learn whether the game only refuses while it is setting the conversation up.
			if (a_refused && !g_state.lateRetryDone) {
				g_state.lateRetryDone = true;
				g_state.hidePlayer = true;
				g_repair.attempts = 0;
				g_repair.nextAttemptAt = a_now + 2500ms;
				REX::WARN("Body repair: refused ({}); the player stays out of frame for now, one more attempt in 2.5 s", a_why);
				return;
			}
			if (attempts >= kRepairMaxAttempts || a_refused) {
				REX::WARN("Body repair: attempt {} failed ({}); giving up. The player will be kept out of frame this conversation", attempts, a_why);
				g_state.needsBodyRepair = false;
				g_state.bodyReady = false;
				g_state.hidePlayer = true;
				g_state.reRepairs = 99;
				return;
			}
			REX::INFO("Body repair: attempt {} failed ({}); will try again shortly", attempts, a_why);
			g_repair.nextAttemptAt = a_now + kRepairBackoff;
		}

		// One step of the body repair. Caller holds g_lock; main thread only.
		void StepRepair(RE::PlayerCamera* a_camera, std::chrono::steady_clock::time_point a_now)
		{
			const bool first = a_camera->QCameraEquals(RE::CameraState::kFirstPerson) || a_camera->QCameraEquals(RE::CameraState::kIronSights);
			const bool third = a_camera->QCameraEquals(RE::CameraState::kThirdPerson);
			const bool dialogue = a_camera->QCameraEquals(RE::CameraState::kDialogue);

			if (g_repair.stage == RepairStage::kIdle) {
				// Wait for a quiet moment: after the game's own opening has settled and the NPC's
				// first line is under way. Asking for first person while the game is still setting
				// up the conversation gets overridden straight away.
				const auto elapsed = a_now - g_state.started;
				const bool npcSpeaking = g_state.npcEverTalked && a_now - g_state.npcTalkStart >= 200ms;
				// First attempt: straight away, on the conversation's first frames (see HeldPose).
				const bool early = Settings::Get().earlyBodyRebuild && g_repair.attempts == 0 && g_state.reRepairs == 0 &&
				                   g_firstPersonView && elapsed < kRepairStartDelay;
				if (!dialogue) {
					return;
				}
				// Direct method: ask the game to refresh the player's 3D model. With the dialogue
				// camera active it builds the full body, with no camera switching at all.
				if (Settings::Get().bodyRebuildMethod == 1 && !g_state.directFailed) {
					auto* player = RE::PlayerCharacter::GetSingleton();
					if (!player) {
						return;
					}
					if (!g_state.directRequested) {
						g_state.directRequested = true;
						g_state.directAt = a_now;
						g_state.directStable = 0;
						g_state.directLastHead.reset();
						g_state.directLastSample = a_now;
						REX::INFO("Body rebuild (direct): 3D update requested with flags {} (head {:.2f} m above feet, {} ms into the conversation)",
							Settings::Get().bodyUpdateFlags, PlayerHeadAboveFeet(), std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
						player->Set3DUpdateFlag(static_cast<std::uint32_t>(Settings::Get().bodyUpdateFlags));
						return;
					}
					if (a_now - g_state.directLastSample >= 12ms) {
						g_state.directLastSample = a_now;
						const auto head = Camera::HeadPosition(player);
						if (head && g_state.directLastHead) {
							const float dx = head->x - g_state.directLastHead->x, dy = head->y - g_state.directLastHead->y, dz = head->z - g_state.directLastHead->z;
							const bool  still = dx * dx + dy * dy + dz * dz < 0.0025f;
							const bool  standing = head->z - player->GetPosition().z >= kStandingHeadHeight;
							g_state.directStable = still && standing ? g_state.directStable + 1 : 0;
						}
						g_state.directLastHead = head;
					}
					const auto waited = std::chrono::duration_cast<std::chrono::milliseconds>(a_now - g_state.directAt).count();
					if (g_state.directStable >= 3) {
						REX::INFO("Body rebuild (direct): full body ready after {} ms (head {:.2f} m above feet)", waited, PlayerHeadAboveFeet());
						g_state.needsBodyRepair = false;
						g_state.bodyReady = true;
						g_state.bodyReadyAt = a_now;
						g_state.hidePlayer = false;
						// Keep the opening shot (from the player's eyes) until the conversation's
						// first real cut; cutting to over the shoulder as soon as the body was
						// ready made a 0.1 s shot that read as a flicker.
						g_state.restoreFirstPerson = false;
						return;
					}
					if (waited < 700) {
						return;
					}
					if (g_state.backgroundScene) {
						// The fallback switches the game's camera to first person and back while
						// the conversation runs. With the speaker in a background scene (0.4.10:
						// Jupiter, whose lines then played on top of each other) don't risk it.
						REX::WARN("Body rebuild (direct): no full body after {} ms (head {:.2f} m above feet); the speaker is in a background scene, so no camera switching: you stay out of frame this conversation", waited, PlayerHeadAboveFeet());
						g_state.needsBodyRepair = false;
						g_state.bodyReady = false;
						g_state.hidePlayer = true;
						g_state.reRepairs = 99;
						return;
					}
					REX::WARN("Body rebuild (direct): no full body after {} ms (head {:.2f} m above feet); falling back to the camera cycle", waited, PlayerHeadAboveFeet());
					g_state.directFailed = true;
					g_state.povNpcShot = true;  // the camera cycle needs its opening shot
				}
				if (!early && (a_now < g_repair.nextAttemptAt || elapsed < kRepairStartDelay ||
								  (!npcSpeaking && elapsed < kRepairFallbackStart))) {
					return;
				}
				REX::INFO("Body repair: attempt {} (head {:.2f} m above feet, {} ms into the conversation)", g_repair.attempts + 1, PlayerHeadAboveFeet(),
					std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
				g_repairMaskedFrames = 0;
				g_repair.early = early;
				if (early) {
					// The player's first-person view, moved a little forward along its own line
					// of sight: during the rebuild the full body appears at the player's position,
					// and a camera exactly at the eyes ends up inside the head.
					auto        held = *g_firstPersonView;
					const float dx = held.lookAt.x - held.position.x, dy = held.lookAt.y - held.position.y, dz = held.lookAt.z - held.position.z;
					const float len = std::sqrt(dx * dx + dy * dy + dz * dz);
					if (len > 1e-3f) {
						const float k = 0.25f / len;
						held.position.x += dx * k;
						held.position.y += dy * k;
						held.position.z += dz * k;
						held.lookAt.x += dx * k;
						held.lookAt.y += dy * k;
						held.lookAt.z += dz * k;
					}
					g_repair.earlyHeld = held;
				}
				g_repair.stageStart = a_now;
				g_repair.deadline = a_now + kRepairTimeout;
				g_state.restoreFirstPerson = true;
				// With some NPCs (seated or busy ones, in the 0.3.13 log) the game refuses to
				// leave its dialogue camera for first person. On every second attempt, ask for
				// third person directly instead of going through first person.
				if (g_repair.attempts % 2 == 1) {
					REX::INFO("Body repair: this attempt asks for third person directly");
					g_repair.stage = RepairStage::kAcquireThird;
					a_camera->ForceThirdPerson();
				} else {
					g_repair.stage = RepairStage::kAcquireFirst;
					a_camera->ForceFirstPerson();  // once; never repeated within an attempt
				}
				HoldShotNow(a_camera);
				return;
			}
			if (!Speaker()) {
				REX::INFO("Body repair: conversation ended mid-repair");
				End();
				return;
			}
			if (a_now >= g_repair.deadline) {
				return AbortAttempt(a_camera, a_now, "timed out");
			}

			switch (g_repair.stage) {
			case RepairStage::kAcquireFirst:
				if (first) {
					g_repair.stage = RepairStage::kHoldFirst;
					g_repair.stageStart = a_now;
				} else if (a_now - g_repair.stageStart > kRepairAcquire) {
					return AbortAttempt(a_camera, a_now, "the game did not switch to first person", true);
				}
				return;
			case RepairStage::kHoldFirst:
				if (a_now - g_repair.stageStart < kRepairHoldFirst) {
					return;
				}
				g_repair.stage = RepairStage::kAcquireThird;
				g_repair.stageStart = a_now;
				a_camera->ForceThirdPerson();
				if (const auto* held = HeldPose()) {
					ExpectMode(20, PredictThirdPersonRoot(), *held);
				}
				HoldShotNow(a_camera);
				return;
			case RepairStage::kAcquireThird:
				if (third) {
					g_repair.stage = RepairStage::kHoldThird;
					g_repair.stageStart = a_now;
				} else if (a_now - g_repair.stageStart > kRepairAcquire) {
					return AbortAttempt(a_camera, a_now, "the game did not switch to third person", true);
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
				// An early rebuild whose body was seen standing and still: the conversation's
				// first real shot can be the over-the-shoulder one, with the player in it.
				if (g_repair.early && g_repair.stableSamples >= 3) {
					g_state.povNpcShot = false;
					g_state.bodyReady = true;
					g_state.bodyReadyAt = a_now + 300ms;
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
				(void)cycle;
				if (height < kStandingHeadHeight) {
					return AbortAttempt(a_camera, a_now, "the full body was not built");
				}
				REX::INFO("Body repair: done on attempt {}; head {:.2f} m above feet; shot held for {} frames during the cycle; {} camera updates missed so far this session", g_repair.attempts + 1, height, g_repairMaskedFrames, g_hookLockMisses.load());
				if (g_repair.early || g_state.directFailed) {
					// Nothing of the conversation has been shown yet, so the first shot of the NPC
					// can be the normal over-the-shoulder one. After a late fallback (the direct
					// rebuild failed), the conversation was already on the director's shot: go
					// back to it rather than staying at your eyes until the next cut (0.4.9 log,
					// 5 s of the NPC from your eyes).
					g_state.povNpcShot = false;
				}
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

		// Diagnostic for using the lines' emotions: the subtitle of the line the speaker has
		// just started, the dialogue record it comes from, and what the record's response
		// holds around where Cassiopeia reads its facial "archetype" keyword.
		// SEH can't live in a function with C++ objects to unwind, so the raw reads are here.
		// Only the type and ID are read straight from memory (guarded). The editor ID needs a
		// virtual call, which on something that isn't really a form would run an unrelated
		// function of that object (0.4.9 crash). So the pointer must first be confirmed as a
		// real form: the game's form table has to return this exact object for its ID.
		bool ReadFormFields(std::uintptr_t a_ptr, std::uint32_t& a_type, std::uint32_t& a_id)
		{
			__try {
				a_type = *reinterpret_cast<const std::uint8_t*>(a_ptr + 0x2E);
				a_id = *reinterpret_cast<const std::uint32_t*>(a_ptr + 0x28);
				return true;
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				return false;
			}
		}

		const char* EditorIdUnsafe(const RE::TESForm* a_form)
		{
			__try {
				const char* editorId = a_form->GetFormEditorID();
				if (editorId) {
					volatile char probe = editorId[0];
					(void)probe;
				}
				return editorId;
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				return nullptr;
			}
		}

		bool ReadFormInfo(std::uintptr_t a_ptr, std::uint32_t& a_type, std::uint32_t& a_id, const char*& a_editorId)
		{
			a_editorId = nullptr;
			if (a_ptr < 0x10000 || a_ptr > 0x7FFFFFFFFFFF || (a_ptr & 7) != 0 || !ReadFormFields(a_ptr, a_type, a_id)) {
				return false;
			}
			if (a_id == 0 || a_type >= 0xB0) {
				return false;
			}
			auto* form = RE::TESForm::LookupByID(a_id);
			if (reinterpret_cast<std::uintptr_t>(form) != a_ptr) {
				return false;  // not a registered form: never call into it
			}
			a_editorId = EditorIdUnsafe(form);
			return true;
		}

		std::string FormLabel(std::uintptr_t a_ptr)
		{
			std::uint32_t type = 0, id = 0;
			const char*   editorId = nullptr;
			if (!ReadFormInfo(a_ptr, type, id, editorId)) {
				return "not a form";
			}
			return std::format("form type {} id {:08X} '{}'", type, id, editorId ? editorId : "");
		}

		// ---- Line emotion (0.4.8 diagnostic) ------------------------------------------------
		// The speaker's voice data holds the subtitle of the line being spoken; look around it
		// for the dialogue record (INFO) that line came from, whose responses carry the emotion.
		[[maybe_unused]] void LogInfo(const std::string& a_where, std::uintptr_t a_info)
		{
			std::uint32_t type = 0, id = 0;
			const char*   editorId = nullptr;
			ReadFormInfo(a_info, type, id, editorId);
			REX::INFO("Line emotion:   {}: INFO {:08X} '{}'", a_where, id, editorId ? editorId : "");
			std::uintptr_t response = 0;
			// 0.4.10 dumps: in 1.16.244 the responses list is at +0x70 (CommonLibSF says +0x68;
			// +0x68 holds two 32-bit values instead).
			SafeCopy(&response, reinterpret_cast<const void*>(a_info + 0x70), sizeof(response));
			static int dumped = 0;
			if (response > 0x10000 && response < 0x7FFFFFFFFFFF && dumped < 8) {
				++dumped;
				DumpObject(std::format("INFO {:08X} response", id).c_str(), reinterpret_cast<const void*>(response), 0xC0);
			}
			for (int i = 0; response && i < 4; ++i) {
				std::array<std::uint64_t, 8> words{};
				if (!SafeCopy(words.data(), reinterpret_cast<const void*>(response), sizeof(words))) {
					break;
				}
				std::string forms;
				for (int k = 0; k < 8; ++k) {
					std::uint32_t t = 0, fid = 0;
					const char*   ed = nullptr;
					if (words[k] > 0x10000 && words[k] < 0x7FFFFFFFFFFF && ReadFormInfo(static_cast<std::uintptr_t>(words[k]), t, fid, ed) && t < 0xB0 && fid != 0) {
						forms += std::format(" +{:X}=type {} {:08X} '{}'", k * 8, t, fid, ed ? ed : "");
					}
				}
				REX::INFO("Line emotion:     response {}: words {:016X} {:016X} {:016X} {:016X} {:016X} {:016X};{}", i, words[0], words[1], words[2], words[3], words[4], words[5], forms);
				response = static_cast<std::uintptr_t>(words[0]);  // next response, if the list starts with it
				if (response < 0x10000 || response > 0x7FFFFFFFFFFF) {
					break;
				}
			}
		}

		// 0.4.9 found the line's dialogue record (INFO) at the same place for every line:
		// the speaker's voice data +0x440 points to an object that holds it at +0x10. Read only
		// that path now (the wide scan of 0.4.8/0.4.9 is gone), and dump the record's raw
		// contents: its responses list was empty in memory, so look at what else it carries.
		// ---- Depth of field through the game's image spaces (0.4.12) --------------------------
		// Every image space record (TESImageSpace) carries depth-of-field settings at
		// +0x150 (settings) +0x220 (DOF): mode, strength, far/near start and plane, and a
		// "focus on the target" switch. List the ones with blur, and for the experiment set
		// their strength to zero while a conversation runs (restored afterwards).
		struct DofBlock
		{
			std::uint32_t mode;
			float         strength, farStart, farPlane, nearStart, nearPlane;
			bool          useTarget;
		};

		constexpr std::size_t kImageSpaceDof = 0x150 + 0x220;

		bool ReadDof(std::uintptr_t a_form, DofBlock& a_out)
		{
			std::array<std::uint8_t, 0x60> raw{};
			if (!SafeCopy(raw.data(), reinterpret_cast<const void*>(a_form + kImageSpaceDof), raw.size())) {
				return false;
			}
			auto f = [&](std::size_t a_off) { float v; std::memcpy(&v, raw.data() + a_off, 4); return v; };
			std::memcpy(&a_out.mode, raw.data(), 4);
			a_out.strength = f(0x0C);
			a_out.farStart = f(0x18);
			a_out.farPlane = f(0x24);
			a_out.nearStart = f(0x3C);
			a_out.nearPlane = f(0x48);
			a_out.useTarget = raw[0x5C] != 0;
			auto sane = [](float v) { return std::isfinite(v) && v > -1.0e6f && v < 1.0e7f; };
			return a_out.mode < 16 && sane(a_out.strength) && sane(a_out.farStart) && sane(a_out.farPlane) && sane(a_out.nearStart) && sane(a_out.nearPlane);
		}

		struct BlurBackup
		{
			std::uintptr_t form;
			float          strength;
		};
		std::vector<BlurBackup> g_blurSpaces;
		bool                    g_blurListed = false;
		bool                    g_blurOff = false;

		// The documented position of the record lists (formArrays at +0x70) gave empty lists
		// in 1.16.244 (0.4.12 log, and the dialogue records in 0.4.8), so find the list whose
		// entries really are of the wanted type: scan the data handler for array headers
		// (data pointer, then 32-bit size and capacity) and check the first entry's type.
		std::pair<const RE::NiPointer<RE::TESForm>*, std::size_t> FindRecordList(RE::TESDataHandler* a_data, RE::FormType a_type)
		{
			const auto  base = reinterpret_cast<std::uintptr_t>(a_data);
			static bool listedAll = false;
			int         listed = 0;
			for (std::size_t off = 0x28; off < 0x3000; off += 8) {
				// BSTArray: 32-bit size, 32-bit capacity, then the data pointer (0.4.13 read the
				// pointer first and found no lists at all).
				struct Header
				{
					std::uint32_t  a, b;
					std::uintptr_t data;
				} h{};
				if (!SafeCopy(&h, reinterpret_cast<const void*>(base + off), sizeof(h)) || h.data < 0x10000 || h.data > 0x7FFFFFFFFFFF) {
					continue;
				}
				for (const auto [size, capacity] : { std::pair{ h.a, h.b }, std::pair{ h.b, h.a } }) {
					if (size == 0 || size > 200000 || size > capacity || capacity > 400000) {
						continue;
					}
					std::uintptr_t entry = 0;
					std::uint32_t  type = 0, id = 0;
					const char*    editorId = nullptr;
					if (!SafeCopy(&entry, reinterpret_cast<const void*>(h.data), sizeof(entry)) || !ReadFormInfo(entry, type, id, editorId)) {
						continue;
					}
					if (!listedAll && listed++ < 8) {
						REX::INFO("Record list at data handler +{:X}: {} entries of form type {:#x}", off, size, type);
					}
					if (type == std::to_underlying(a_type)) {
						REX::INFO("Record list for form type {:#x} found at data handler +{:X}: {} entries", type, off, size);
						listedAll = true;
						return { reinterpret_cast<const RE::NiPointer<RE::TESForm>*>(h.data), static_cast<std::size_t>(size) };
					}
					break;
				}
			}
			listedAll = true;
			REX::INFO("Record list for form type {:#x}: not found", std::to_underlying(a_type));
			return { nullptr, 0 };
		}

		// Diagnostic (0.4.13): the game's image-space modifiers (what the console's imod and
		// Papyrus ImageSpaceModifier.Apply use; Starvegas applies its own depth-of-field ones).
		// Names, and the raw data of those that look like depth of field or conversation ones.
		void ListModifiers(RE::TESDataHandler* a_data)
		{
			auto [first, count] = FindRecordList(a_data, RE::FormType::kIMAD);
			std::string names;
			int         dumped = 0;
			for (std::size_t i = 0; first && i < (std::min)(count, std::size_t{ 5000 }); ++i) {
				std::uintptr_t form = 0;
				if (!SafeCopy(&form, first + i, sizeof(form))) {
					break;
				}
				std::uint32_t type = 0, id = 0;
				const char*   editorId = nullptr;
				if (!ReadFormInfo(form, type, id, editorId) || type != std::to_underlying(RE::FormType::kIMAD)) {
					continue;
				}
				const std::string name{ editorId ? editorId : "" };
				names += std::format(" {:08X}:{}", id, name);
				if (names.size() > 1500) {
					REX::INFO("Image space modifiers:{}", names);
					names.clear();
				}
				std::string lower = name;
				std::ranges::transform(lower, lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
				if (dumped < 12 && (lower.find("dof") != std::string::npos || lower.find("dialog") != std::string::npos || lower.find("depth") != std::string::npos)) {
					++dumped;
					DumpObject(std::format("modifier {:08X} '{}'", id, name).c_str(), reinterpret_cast<const void*>(form), 0x300);
				}
			}
			if (!names.empty()) {
				REX::INFO("Image space modifiers:{}", names);
			}
			REX::INFO("Image space modifiers: {} listed, {} dumped", count, dumped);
		}

		void ListBlurImageSpaces()
		{
			g_blurListed = true;
			auto* data = RE::TESDataHandler::GetSingleton();
			if (!data) {
				return;
			}
			auto [first, count] = FindRecordList(data, RE::FormType::kIMGS);
			std::size_t withBlur = 0;
			for (std::size_t i = 0; first && i < count; ++i) {
				std::uintptr_t form = 0;
				if (!SafeCopy(&form, first + i, sizeof(form))) {
					break;
				}
				std::uint32_t type = 0, id = 0;
				const char*   editorId = nullptr;
				DofBlock      dof{};
				if (!ReadFormInfo(form, type, id, editorId) || type != std::to_underlying(RE::FormType::kIMGS) || !ReadDof(form, dof)) {
					continue;
				}
				if (dof.strength <= 0.0f) {
					continue;
				}
				++withBlur;
				g_blurSpaces.push_back({ form, dof.strength });
				if (withBlur <= 80) {
					REX::INFO("Image space {:08X} '{}': blur mode {} strength {:.2f}, far {:.2f}-{:.2f}, near {:.2f}-{:.2f}, focus on target {}", id, editorId ? editorId : "",
						dof.mode, dof.strength, dof.farStart, dof.farPlane, dof.nearStart, dof.nearPlane, dof.useTarget ? "yes" : "no");
				}
			}
			REX::INFO("Image spaces: {} in the game, {} with depth-of-field blur", count, withBlur);
			ListModifiers(data);
		}

		// ---- Image-space modifier test (0.4.14) -----------------------------------------
		// The game's own apply/remove for image-space modifiers: the Papyrus natives behind
		// ImageSpaceModifier.Apply() and .Remove() (Address Library IDs 171562 and 105038, from
		// Cassiopeia Papyrus Extender by LarannKiar, which calls them the same way). Resolved
		// only when the test is switched on, so a missing ID can't affect anyone else.
		RE::TESForm* g_testModifierForm = nullptr;
		bool         g_testModifierOn = false;

		void ApplyModifier(RE::TESForm* a_modifier, float a_strength)
		{
			static REL::Relocation<std::uintptr_t> apply{ REL::ID(171562) };
			reinterpret_cast<void (*)(std::uintptr_t, std::uintptr_t, RE::TESForm*, float)>(apply.address())(0, 0, a_modifier, a_strength);
		}

		void RemoveModifier(RE::TESForm* a_modifier)
		{
			static REL::Relocation<std::uintptr_t> remove{ REL::ID(105038) };
			reinterpret_cast<void (*)(RE::TESForm*)>(remove.address())(a_modifier);
		}

		void SetTestModifier(bool a_on, const char* a_why)
		{
			if (a_on == g_testModifierOn) {
				return;
			}
			const auto wanted = Settings::Get().testModifier;
			if (a_on) {
				auto* form = wanted ? RE::TESForm::LookupByID(wanted) : nullptr;
				std::uint32_t type = 0, id = 0;
				const char*   editorId = nullptr;
				if (!form || !ReadFormInfo(reinterpret_cast<std::uintptr_t>(form), type, id, editorId) || type != std::to_underlying(RE::FormType::kIMAD)) {
					static std::uint32_t warned = 0;
					if (warned != wanted) {
						warned = wanted;
						REX::WARN("Modifier test: {:08X} is not an image-space modifier; nothing applied", wanted);
					}
					return;
				}
				// Calling the Papyrus native directly (Cassiopeia's way) crashed in 1.16.244 (0.4.16
				// crash log: null pointer inside the game's apply). Not called until a safe route
				// is in place.
				(void)ApplyModifier;
				static bool warnedUnsafe = false;
				if (!warnedUnsafe) {
					warnedUnsafe = true;
					REX::WARN("Modifier test: applying {:08X} is disabled in this build (the direct call crashed in 0.4.16)", id);
				}
				g_testModifierOn = true;
				return;
				REX::INFO("Modifier test: applied {:08X} '{}' ({})", id, editorId ? editorId : "", a_why);
			} else if (g_testModifierForm) {
				RemoveModifier(g_testModifierForm);
				REX::INFO("Modifier test: removed {:08X} ({})", g_testModifierForm->formID, a_why);
				g_testModifierForm = nullptr;
			}
			g_testModifierOn = a_on;
		}

		void SetConversationBlur(bool a_off, const char* a_why)
		{
			if (a_off == g_blurOff) {
				return;
			}
			g_blurOff = a_off;
			for (const auto& space : g_blurSpaces) {
				const float value = a_off ? 0.0f : space.strength;
				std::uint8_t* where = reinterpret_cast<std::uint8_t*>(space.form + kImageSpaceDof + 0x0C);
				SafeWrite(where, &value, sizeof(value));
			}
			static int logged = 0;
			if (logged++ < 60) {
				REX::INFO("Blur experiment: depth of field {} ({}; {} image spaces)", a_off ? "off" : "back on", a_why, g_blurSpaces.size());
			}
		}

		// The line's facial expression ("emotion"), as the game's animators tagged it:
		// speaker's voice data +0x440 -> +0x10 = the dialogue record (INFO); its responses
		// list at +0x70; the first response holds the expression keyword at +0x18, e.g.
		// AnimFaceArchetypeFriendly. Found in 0.4.8-0.4.11. Every read is guarded and every
		// pointer confirmed as a real game record before it's used.
		std::string LineEmotion(RE::Actor* a_speaker)
		{
			if (!a_speaker || !a_speaker->currentProcess || !a_speaker->currentProcess->high) {
				return {};
			}
			const auto     base = reinterpret_cast<std::uintptr_t>(a_speaker->currentProcess->high);
			std::uintptr_t holder = 0, info = 0, response = 0, keyword = 0;
			auto           ok = [](std::uintptr_t a_p) { return a_p > 0x10000 && a_p < 0x7FFFFFFFFFFF; };
			if (!SafeCopy(&holder, reinterpret_cast<const void*>(base + 0x440), sizeof(holder)) || !ok(holder) ||
				!SafeCopy(&info, reinterpret_cast<const void*>(holder + 0x10), sizeof(info))) {
				return {};
			}
			std::uint32_t type = 0, id = 0;
			const char*   editorId = nullptr;
			if (!ReadFormInfo(info, type, id, editorId) || type != std::to_underlying(RE::FormType::kINFO)) {
				return {};
			}
			if (!SafeCopy(&response, reinterpret_cast<const void*>(info + 0x70), sizeof(response)) || !ok(response) ||
				!SafeCopy(&keyword, reinterpret_cast<const void*>(response + 0x18), sizeof(keyword)) || !ok(keyword)) {
				return "none";
			}
			if (!ReadFormInfo(keyword, type, id, editorId) || type != std::to_underlying(RE::FormType::kKYWD) || !editorId) {
				return "none";
			}
			std::string_view name{ editorId };
			constexpr std::string_view prefix{ "AnimFaceArchetype" };
			if (name.starts_with(prefix)) {
				name.remove_prefix(prefix.size());
			}
			return std::string{ name };
		}

		std::string                           g_emotionMessage;
		int                                   g_emotionCount = 0;
		bool                                  g_emotionWaiting = false;
		std::chrono::steady_clock::time_point g_emotionLineAt{};
		std::chrono::steady_clock::time_point g_emotionDeadline{};

		void ShowHudMessage(const std::string& a_text)
		{
			if (auto* source = RE::ShowHUDMessageEvent::GetEventSource()) {
				RE::ShowHUDMessageEvent event{ RE::BSFixedString{ a_text.c_str() }, RE::BSFixedString{ "" }, false, false };
				source->Notify(event);
			}
		}

		// Called every main-thread tick: posts a close-up emotion's message once the camera
		// has cut (or not) for it.
		void PostEmotionMessage(std::chrono::steady_clock::time_point a_now)
		{
			if (!g_emotionWaiting) {
				return;
			}
			std::chrono::steady_clock::time_point cutAt{};
			const int                             kind = Director::LastEmotionCut(cutAt);
			// The camera may decide a frame before or after this line was noticed here.
			if (kind != 0 && cutAt >= g_emotionLineAt - 300ms) {
				g_emotionWaiting = false;
				ShowHudMessage(std::format("{} - {} (#{})", g_emotionMessage, kind == 2 ? "extreme close-up" : "close-up", g_emotionCount));
			} else if (a_now >= g_emotionDeadline) {
				g_emotionWaiting = false;
				ShowHudMessage(std::format("{} - no cut (#{})", g_emotionMessage, g_emotionCount));
			}
		}

		void LogLineRecord(RE::Actor* a_speaker)
		{
			if (!a_speaker || !a_speaker->currentProcess || !a_speaker->currentProcess->high) {
				return;
			}
			const char* raw = a_speaker->currentProcess->high->strVoiceSubtitle.c_str();
			const auto  emotion = LineEmotion(a_speaker);
			const bool  closeUp = Settings::Get().emotionCloseUps && IsCloseUpEmotion(emotion);
			REX::INFO("Line emotion: {} for '{}'{}", emotion.empty() ? "unknown" : emotion, std::string{ raw ? raw : "" }.substr(0, 60), closeUp ? " (close-up emotion)" : "");
			// On-screen message (testing aid), through the game's own HUD message event. For a
			// close-up emotion, wait for the camera's decision (posted by PostEmotionMessage).
			if (Settings::Get().showEmotions) {
				static std::unordered_map<std::string, int> seen;
				const auto  key = emotion.empty() ? std::string{ "unknown" } : emotion;
				const int   count = ++seen[key];
				const char* name = a_speaker->GetDisplayFullName();
				g_emotionMessage = std::format("{}: {}", name ? name : "Speaker", key);
				g_emotionCount = count;
				g_emotionWaiting = closeUp;
				g_emotionLineAt = std::chrono::steady_clock::now();
				g_emotionDeadline = g_emotionLineAt + 350ms;
				if (!closeUp) {
					ShowHudMessage(std::format("{} (#{})", g_emotionMessage, g_emotionCount));
				}
			}
		}

		// ---- Depth of field (0.4.7 diagnostic) ------------------------------------------------
		// Outdoors the conversation's depth of field focuses on the listener. The game most
		// likely focuses at the distance from its own camera to whoever it is aimed at, while
		// the picture is drawn from our camera further back. Look for the game's depth-of-field
		// effect object (found by its type) near the camera, and log its values over time.
		std::uintptr_t DofVtable()
		{
			static const std::uintptr_t vtable = [] {
				try {
					return RE::VTABLE::ImageSpaceModifierInstanceDOF[0].address();
				} catch (...) {
					return std::uintptr_t{ 0 };
				}
			}();
			return vtable;
		}

		struct DofHit
		{
			std::uintptr_t address;
			std::string    path;
		};
		std::vector<DofHit> g_dofHits;
		int                 g_dofConversation = -1;
		int                 g_dofSamples = 0;
		std::chrono::steady_clock::time_point g_dofNext{};

		[[maybe_unused]] void FindDofObjects()
		{
			g_dofHits.clear();
			const auto vtable = DofVtable();
			if (!vtable) {
				REX::INFO("Depth of field: the effect's type isn't in the address library");
				return;
			}
			struct Node
			{
				std::uintptr_t address;
				std::string    path;
				int            depth;
			};
			std::vector<Node>                  queue;
			std::unordered_set<std::uintptr_t> seen;
			if (auto* camera = RE::PlayerCamera::GetSingleton()) {
				queue.push_back({ reinterpret_cast<std::uintptr_t>(camera), "PlayerCamera", 0 });
			}
			if (g_dialogueStateObject) {
				queue.push_back({ reinterpret_cast<std::uintptr_t>(g_dialogueStateObject), "dialogue camera", 0 });
			}
			if (auto* render = Camera::RenderCameraObject()) {
				queue.push_back({ reinterpret_cast<std::uintptr_t>(render), "render camera", 0 });
			}
			if (auto* player = RE::PlayerCharacter::GetSingleton()) {
				queue.push_back({ reinterpret_cast<std::uintptr_t>(player), "player", 1 });
			}
			std::size_t visited = 0;
			for (std::size_t i = 0; i < queue.size() && visited < 15000 && g_dofHits.size() < 12; ++i) {
				const Node node = queue[i];
				if (!seen.insert(node.address).second) {
					continue;
				}
				++visited;
				std::array<std::uint64_t, 64> words{};
				if (!SafeCopy(words.data(), reinterpret_cast<const void*>(node.address), sizeof(words))) {
					continue;
				}
				for (std::size_t k = 0; k < words.size(); ++k) {
					const auto w = words[k];
					if (w == vtable) {
						g_dofHits.push_back({ node.address + k * 8, std::format("{}+{:X}", node.path, k * 8) });
					} else if (node.depth < 3 && w > 0x10000 && w < 0x7FFFFFFFFFFF && (w & 7) == 0 && queue.size() < 60000) {
						queue.push_back({ static_cast<std::uintptr_t>(w), std::format("{}+{:X}", node.path, k * 8), node.depth + 1 });
					}
				}
			}
			REX::INFO("Depth of field: searched {} objects near the camera; {} depth-of-field effect objects found", visited, g_dofHits.size());
			for (const auto& hit : g_dofHits) {
				REX::INFO("Depth of field:   {:#x} at {}", hit.address, hit.path);
			}
		}

		[[maybe_unused]] void LogDofSample()
		{
			auto* camera = RE::PlayerCamera::GetSingleton();
			auto* speaker = Speaker();
			auto* player = RE::PlayerCharacter::GetSingleton();
			if (!camera || !speaker || !player) {
				return;
			}
			const auto root = Camera::PoseOf(camera->cameraRoot.get());
			const auto npc = Camera::HeadPosition(speaker);
			const auto you = Camera::HeadPosition(player);
			auto*      render = static_cast<RE::NiAVObject*>(Camera::RenderCameraObject());
			auto dist = [](const RE::NiPoint3& a, const std::optional<RE::NiPoint3>& b) {
				if (!b) {
					return -1.0f;
				}
				const float dx = a.x - b->x, dy = a.y - b->y, dz = a.z - b->z;
				return std::sqrt(dx * dx + dy * dy + dz * dz);
			};
			REX::INFO("Depth of field sample {}: {} talking; game camera to speaker {:.2f} m, to you {:.2f} m; picture to speaker {:.2f} m, to you {:.2f} m",
				g_dofSamples, speaker->IsTalking() ? "they're" : "you're (or nobody)",
				root ? dist(root->position, npc) : -1.0f, root ? dist(root->position, you) : -1.0f,
				render ? dist(render->world.translate, npc) : -1.0f, render ? dist(render->world.translate, you) : -1.0f);
			// Where the game keeps its focus distance: a value equal to the distance from its
			// camera to the speaker (or to you, on your lines) that follows it between samples.
			if (root) {
				const float toSpeaker = dist(root->position, npc);
				const float toYou = speaker->IsTalking() ? -1.0f : dist(root->position, you);
				LogNearValues("dialogue camera", g_dialogueStateObject, 0x600, toSpeaker, toYou > 0.5f ? toYou : -1.0f);
				LogNearValues("PlayerCamera", camera, 0x800, toSpeaker, toYou > 0.5f ? toYou : -1.0f);
				LogNearValues("render camera", render, 0x500, toSpeaker, toYou > 0.5f ? toYou : -1.0f);
			}
			int n = 0;
			for (const auto& hit : g_dofHits) {
				if (++n > 4) {
					break;
				}
				std::array<float, 40> values{};
				if (!SafeCopy(values.data(), reinterpret_cast<const void*>(hit.address), sizeof(values))) {
					REX::INFO("Depth of field:   {} gone", hit.path);
					continue;
				}
				std::string text;
				for (std::size_t k = 2; k < values.size(); ++k) {  // skip the type pointer
					const float v = values[k];
					text += std::isfinite(v) && std::abs(v) > 1e-4f && std::abs(v) < 1e6f ? std::format(" +{:X}={:.3f}", k * 4, v) : "";
				}
				REX::INFO("Depth of field:   {}:{}", hit.path, text);
			}
		}

		[[maybe_unused]] void DofDiagnostics(std::chrono::steady_clock::time_point a_now)
		{
			if (!g_state.active) {
				return;
			}
			if (g_dofConversation != g_conversationNumber) {
				if (g_state.framesThisConversation < 30) {
					return;  // let the conversation settle first
				}
				g_dofConversation = g_conversationNumber;
				g_dofSamples = 0;
				g_dofHits.clear();  // the effect-object search (0.4.7-0.4.10) never found one; retired
				g_dofNext = a_now;
			}
			if (a_now >= g_dofNext && g_dofSamples < 12) {
				g_dofNext = a_now + 1500ms;
				++g_dofSamples;
				LogDofSample();
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
						// A few samples of each of first person (0), dialogue (17) and third person (20).
						static std::array<int, RE::CameraState::kTotal> dumps{};
						if (state >= 0 && (state == 0 || state == 17 || state == 20) && dumps[state] < 3) {
							++dumps[state];
							LogFovCandidates("PlayerCamera", state, cam, 0x20, 0x188);
							LogFovCandidates("camera state object", state, cam->currentState, 0x8, 0x340);
						}
					}
				}
			}

			{
				static bool scanned = false;
				if (!scanned && Settings::Get().debugLog && g_hookCalls > 0) {
					scanned = true;
					LogCameraSettings(RE::INISettingCollection::GetSingleton(), "ini");
					LogCameraSettings(RE::INIPrefSettingCollection::GetSingleton(), "prefs");
				}
			}

			bool reveal = false;
			bool hide = false;
			bool restore = false;

			if (auto* ui = RE::UI::GetSingleton()) {
				static const RE::BSFixedString shipHud{ "SpaceshipHudMenu" };
				g_pilotingShip.store(ui->IsMenuOpen(shipHud), std::memory_order_release);
			}

			{
				std::scoped_lock lock{ g_lock };
			struct Holder
			{
				Holder() { g_lockHolder = ::GetCurrentThreadId(); }
				~Holder() { g_lockHolder = 0; }
			} holder;
				const auto       now = std::chrono::steady_clock::now();
				auto*            camera = RE::PlayerCamera::GetSingleton();
				const bool       repairing = g_repair.stage != RepairStage::kIdle;

				// Conversation field of view: keep the game's setting at the value asked for, so
				// it is in place whenever the dialogue camera starts.
				{
					static RE::Setting* fovSetting = nullptr;
					static bool         looked = false;
					static float        original = 0.0f;
					static float        lastLogged = -1.0f;
					static std::uint32_t ticks = 0;
					if (!fovSetting && (ticks++ % 240) == 0) {
						looked = true;
						fovSetting = FindSetting("fDialogueCameraFailsafeFPFOV:Dialogue");
						if (fovSetting) {
							original = fovSetting->GetFloat();
						}
					}
					if (fovSetting && Settings::Get().fovExperiment != 2) {
						const float wanted = original;  // changing this setting had no visible effect (0.3.28 test); keep the game's value
						if (fovSetting->GetFloat() != wanted) {
							fovSetting->SetFloat(wanted);
						}
						if (wanted != lastLogged) {
							lastLogged = wanted;
							REX::INFO("Conversation field of view: fDialogueCameraFailsafeFPFOV set to {:.1f} (game's own value {:.1f})", wanted, original);
						}
					}
				}

				// Re-read the INI every couple of seconds outside conversations, so edits (game
				// settings in particular) are in place before the next conversation starts.
				{
					static std::chrono::steady_clock::time_point nextReload{};
					static bool appliedOnce = false;
					if (!g_state.active && now >= nextReload) {
						nextReload = now + 2s;
						Settings::Get().Load();
						ApplyGameSettings();
						appliedOnce = true;
					} else if (!appliedOnce) {
						ApplyGameSettings();
						appliedOnce = true;
					}
				}

				// Diagnostic: the dialogue record of each line the speaker starts.
				{
					static bool wasTalking = false;
					auto*       speaker = g_state.active && (Settings::Get().debugLog || Settings::Get().showEmotions) ? Speaker() : nullptr;
					const bool  talking = speaker && speaker->IsTalking();
					if (talking && !wasTalking) {
						LogLineRecord(static_cast<RE::Actor*>(speaker));
					}
					PostEmotionMessage(now);
					wasTalking = talking;
				}

				// Diagnostic: where the game keeps the conversation's depth of field.
				// Depth-of-field experiment through the game's image spaces (0.4.12).
				{
					const auto& st = Settings::Get();
					const bool  live = g_state.active && !g_state.handedBack && !g_state.smallSpeaker;
					if (live && !g_blurListed && (st.blurExperiment != 0 || st.debugLog)) {
						ListBlurImageSpaces();
					}
					if (live && st.blurExperiment != 0) {
						const float d = g_shotDistance.load(std::memory_order_relaxed);
						bool        off = g_blurOff;
						if (st.blurExperiment == 1) {
							off = true;
						} else if (d > st.noBlurBeyond + 0.15f) {
							off = true;
						} else if (d < st.noBlurBeyond - 0.15f) {
							off = false;
						}
						SetConversationBlur(off, st.blurExperiment == 1 ? "whole conversation" : std::format("shot {:.1f} m from the person in shot", d).c_str());
					} else if (g_blurOff) {
						SetConversationBlur(false, "conversation over");
					}
				}
				// Image-space modifier test (0.4.14).
				{
					const auto& st = Settings::Get();
					const bool  live = g_state.active && !g_state.handedBack && !g_state.smallSpeaker;
					if (live && st.testModifier != 0) {
						const float d = g_shotDistance.load(std::memory_order_relaxed);
						bool        on = g_testModifierOn;
						if (st.testModifierMode == 1) {
							on = true;
						} else if (d > st.noBlurBeyond + 0.15f) {
							on = true;
						} else if (d < st.noBlurBeyond - 0.15f) {
							on = false;
						}
						SetTestModifier(on, st.testModifierMode == 1 ? "whole conversation" : std::format("shot {:.1f} m from the person in shot", d).c_str());
					} else if (g_testModifierOn) {
						SetTestModifier(false, "conversation over");
					}
				}

				// Data collection and experiments (main thread).
				{
					static bool dumpedFirst = false, dumpedThird = false;
					if (camera && !g_state.active && Settings::Get().debugLog && Camera::RenderCameraObject()) {
						const bool fp = camera->QCameraEquals(RE::CameraState::kFirstPerson);
						const bool tp = camera->QCameraEquals(RE::CameraState::kThirdPerson);
						if ((fp && !dumpedFirst) || (tp && !dumpedThird)) {
							(fp ? dumpedFirst : dumpedThird) = true;
							const std::string label = fp ? "gameplay first person" : "gameplay third person";
							DumpObject((label + " player camera").c_str(), camera, 0x188);
							DumpObject((label + " player camera part 2").c_str(), reinterpret_cast<const std::uint8_t*>(camera) + 0x188, 0x400);
							DumpObject((label + " camera state").c_str(), camera->currentState, 0x340);
							DumpObject((label + " render camera").c_str(), Camera::RenderCameraObject(), 0x400);
						}
					}

					const bool live = g_state.active && camera && camera->QCameraEquals(RE::CameraState::kDialogue) && !repairing &&
					                  g_state.framesThisConversation > 60;
					if (live) {
						ApplyFovExperiment(true);
						if (!g_menusLogged) {
							g_menusLogged = true;
							LogOpenMenus();
						}
					} else if (!g_state.active || repairing) {
						ApplyFovExperiment(false);
					}
				}

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
				static std::chrono::steady_clock::time_point leftDialogueAt{};
				const bool leftDialogue = g_state.active && !repairing && camera && !camera->QCameraEquals(RE::CameraState::kDialogue);
				if (!leftDialogue) {
					leftDialogueAt = {};
				} else if (leftDialogueAt == std::chrono::steady_clock::time_point{}) {
					leftDialogueAt = now;
				}
				// A first-person exit is finished by the first-person camera hook on the game's
				// first first-person frame; only step in if that hasn't happened shortly.
				const bool hookFinishes = g_state.startedFirstPerson && g_state.holdingExit && now - leftDialogueAt < 150ms;
				if (leftDialogue && !hookFinishes) {
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
				// During the rebuild, also re-apply the held shot on every main-thread tick. The
				// recordings show a frame slipping through right after each camera-mode change,
				// before that mode's own update (and our hook on it) has run.
				if (camera && g_state.active && g_repair.stage != RepairStage::kIdle) {
					HoldShotNow(camera);
				}
				if (camera && g_restoreFirstPersonPending) {
					StepFirstPersonRestore(camera, now);
				}

				reveal = g_wantRevealPlayer;
				hide = g_wantHidePlayer;
				restore = g_needVisibilityRestore;
				g_needVisibilityRestore = false;
			}

			// Scene-graph work happens outside the lock, so the hook is never kept waiting
			// (it would skip frames instead) while the engine does its own locking.
			if (restore) {
				Visibility::Restore(g_restoreVisible);
			} else if (reveal) {
				Visibility::RevealPlayer();
			} else if (hide) {
				Visibility::HidePlayer();
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
	REX::INFO("Cinematic Conversation Camera SF 0.4.16 loaded (enabled={}, shotMode={}, cameraTarget={}, revealPlayer={}, poseMode={})",
		settings.enabled, static_cast<int>(settings.shotMode), settings.cameraTarget, settings.revealPlayer, settings.poseMode);
	return true;
}
