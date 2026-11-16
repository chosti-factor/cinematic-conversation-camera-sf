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
#include "Face.h"
#include "Hud.h"
#include "Settings.h"
#include "Visibility.h"

#include <deque>
#include <numbers>
#include <random>

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
			std::chrono::steady_clock::time_point lineStart{};
			bool                                  lineBeat{ false };         // this line answers a persuasion: held close-up
			bool                                  replyBeat{ false };        // ... and so does the rest of their reply, until your turn (0.8.5)
			std::int64_t                          beatConsumed{ 0 };         // the persuasion outcome already used
			bool                                  reactionOwed{ false };     // the roll said yes: the next line that can take it (0.8.3, as CCC)
			bool                                  reactionActive{ false };   // this NPC line is shown on the player
			std::chrono::steady_clock::time_point reactionFrom{};            // on you from then (0.8.3: after an emotion close-up's minimum)
			std::chrono::steady_clock::time_point reactionOnAt{};            // when the camera cut to you (0.8.3: the time counts from the cut)
			float                                 reactionHold{ 2.5f };      // seconds on you from the cut; < 0 = the rest of the line
			bool                                  reactionShownLine{ false };  // a reaction was on screen during this line (0.8.10)
			bool                                  reactionCloseUp{ false };  // ...as your emotion close-up
			bool                                  reactionPending{ false };  // waiting for this line's subtitle to decide
			float                                 monoSeconds{ 0.0f };       // their talking since the camera last showed you (0.8.5)
			int                                   npcLinesStarted{ 0 };      // their lines so far in this conversation (0.8.6)
			bool                                  lineExtreme{ false };      // this emotion line's close-up is an extreme one (0.8.3)
			bool                                  closeUpGoesOn{ false };    // this emotion line carries on the last line's close-up, no new cut (0.8.8)
			const char*                           msgShot{ nullptr };        // shot messages: the last one shown
			int                                   msgSubject{ -1 };
			std::uint32_t                         msgCuts{ 0 };
			std::string                           lastLineText{};
			std::uintptr_t                        playerLineInfo{ 0 };       // diagnostic: your last dialogue record
			std::string                           playerLineText{};
			std::optional<RE::NiPoint3>           lastRawNpcHead{};
			std::optional<RE::NiPoint3>           slackNpcHead{};    // head point after the dead band
			std::optional<RE::NiPoint3>           slackPlayerHead{};
			Camera::FaceTrack                     npcFace{};         // which way each face points
			Camera::FaceTrack                     playerFace{};
			float                                 side{ 1.0f };      // side of the eyeline this conversation is staged on
			bool                                  sideChosen{ false };
			float                                 npcHeadSpeed{ 0.0f };      // smoothed, m/s
			bool                                  npcHeadBusy{ false };      // walking: steadier following
			const char*                           castShot{ nullptr };       // obstacle check: last shot checked
			int                                   castSubject{ -1 };
			bool                                  roomMeasured{ false };     // room measurement (room shots, 0.8)
			float                                 roomSide{ 0.0f };          // the side it was measured for
			std::array<float, 3>                  roomAngle{ std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN() };
			std::array<float, 3>                  roomDistance{ 0.0f, 0.0f, 0.0f };
			float                                 roomRise{ 10.0f };
			RE::NiPoint3                          roomAt{};
			std::chrono::steady_clock::time_point roomTime{};
			int                                   plainCheckSubject{ -1 };   // obstacle check of the plain shot: whose turn it was done for
			float                                 plainLimit{ std::numeric_limits<float>::infinity() };
			bool                                  plainEyeLine{ false };     // plain shot blocked: shoot from the other person's eyes
			int                                   busySwitches{ 0 };
			float                                 busySeconds{ 0.0f };
			// Head motion of the speaker during the current shot (0.4.19 measurement, for the
			// jitter seen with NPCs posing or working at a crafting table).
			struct HeadMotion
			{
				std::optional<RE::NiPoint3> last;
				std::uint32_t               frames{ 0 };
				float                       seconds{ 0.0f };
				float                       path{ 0.0f };       // metres travelled
				float                       peakSpeed{ 0.0f };  // m/s
				float                       gapSum{ 0.0f };     // raw head to smoothed head, summed
				float                       gapMax{ 0.0f };
				const char*                 shot{ nullptr };
				int                         subject{ -1 };
			} head;
			bool                                  backgroundScene{ false };  // speaker is in a background scene (no mid-conversation camera switching)
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
			int                                   directRetries{ 0 };    // 3D update requests repeated while waiting
			std::optional<RE::NiPoint3>           smoothNpcFeet{};       // where each stands, steadied (the eyeline's bearing)
			std::optional<RE::NiPoint3>           smoothPlayerFeet{};
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
		bool                                  g_filmThirdPerson = false;  // filming from first person: the game is in third person for it (0.8.8)
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
		std::mt19937 g_reactionRng{ std::random_device{}() };

		Director::Clearance TestShot(const RE::NiPoint3& a_subjectHead, const RE::NiPoint3& a_camera, const RE::NiPoint3& a_nearHead, bool a_overShoulder);
		void                OnOptionChosen(const std::string& a_text);
		void                FilmFrame();
		void                FocusOnShot(float a_distance);
		void                LensNow();
		std::atomic<float>  g_filmLens{ 50.0f };  // the lens of the shot filming people talking (0.8.15)

		// Filming people talking (0.8.15): each shot on Standard's lens for it (CCC's numbers).
		float FilmLens(Shots::ShotType a_type, bool a_group)
		{
			if (a_group) {
				return 72.0f;
			}
			switch (a_type) {
			case Shots::ShotType::kCloseUp: return 50.0f;
			case Shots::ShotType::kOverPlayerShoulder: return 55.0f;
			case Shots::ShotType::kThreeQuarterNpc: return 58.0f;
			case Shots::ShotType::kMediumNpc: return 62.0f;
			case Shots::ShotType::kTwoShot: return 72.0f;
			default: return 50.0f;
			}
		}
		bool                FilmingActive();
		std::atomic<bool>   g_playerFaceShown{ false };  // your face answers the NPC's current line (set on the main thread)
		std::atomic<int>    g_lineSerial{ 0 };          // counts their lines, parts included (0.8.3)
		std::atomic<bool>   g_remeasureRoom{ false };    // the preset key changed the preset: measure the room again (0.8.3)
		std::atomic<bool>   g_reactionStarted{ false };  // a reaction shot began (worker -> main: give a blank face a look)
		std::atomic<std::int64_t> g_faceLineAt{ 0 };    // when the main thread last set your face for a line (steady_clock ticks)
		void                QueueHudMessage(std::string a_text);
		void                ShowHudMessage(const std::string& a_text);

		// Bystanders (0.7.5): people standing still near the conversation, other than the two
		// of you. Gathered on the main thread a few times a second; the camera's shot test
		// (worker threads) reads the list.
		struct Bystander
		{
			RE::NiPoint3 feet{};
			RE::NiPoint3 head{};
		};
		std::mutex             g_bystanderLock;
		std::vector<Bystander> g_bystanders;
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
				Director::SetObstacleTest(&TestShot);
				Hud::SetChoiceListener(&OnOptionChosen);
				Director::Reset();
			}
		}

		void LogHeadMotion(const char* a_why);

		void End()
		{
			if (g_state.active) {
				if (Settings::Get().debugLog) {
					LogHeadMotion("conversation end");
					REX::INFO("Steadier following (speaker's head above {:.2f} m/s): switched on {} times, {:.1f} s in all", Settings::Get().busyHeadSpeed, g_state.busySwitches,
						g_state.busySeconds);
				}
				REX::INFO("Conversation ended after {} camera frames", g_state.framesThisConversation);
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
			if (talking && g_state.reactionActive && a_now >= g_state.reactionFrom) {
				// A reaction shot: you, listening, for reactionHold seconds from the cut to you.
				if (g_state.reactionOnAt == std::chrono::steady_clock::time_point{}) {
					g_state.reactionOnAt = a_now;
				}
				g_state.reactionShownLine = true;
				const float on = std::chrono::duration<float>(a_now - g_state.reactionOnAt).count();
				if (g_state.reactionHold < 0.0f || on < g_state.reactionHold) {
					return Subject::kPlayer;
				}
				g_state.reactionActive = false;  // the reaction's done: back to the speaker
				g_state.reactionCloseUp = false;
			}
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
			if (g_state.subject == Subject::kNone) {
				return Subject::kNpc;
			}
			g_state.monoSeconds = 0.0f;  // your turn: the camera is on you (0.8.5)
			g_state.reactionOwed = false;  // 0.8.6 test: one owed from before your turn cut to you 0.6 s into their reply
			g_state.replyBeat = false;   // their answer to a persuasion is over
			return Subject::kPlayer;
		}

		// Diagnostic: find where the game keeps its field of view. Copies plausible FOV values
		// (degrees or radians) out of a block of memory; guarded because the size of the
		// camera state objects isn't known.
		struct FloatAt
		{
			std::uint32_t offset;
			float         value;
		};

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
		std::uintptr_t LineInfo(RE::Actor* a_speaker);

		void LogHeadMotion(const char* a_why)
		{
			auto& m = g_state.head;
			if (m.frames >= 20 && m.seconds > 0.3f) {
				auto* speaker = Speaker();
				const char* name = speaker ? speaker->GetDisplayFullName() : nullptr;
				REX::INFO("Head motion ({}; {} on {}, {:.1f} s): average {:.2f} m/s, peak {:.2f} m/s; camera follows {:.1f} cm behind on average, {:.1f} cm at most",
					a_why, name ? name : "speaker", m.shot ? m.shot : "plain shot", m.seconds, m.path / m.seconds, m.peakSpeed, 100.0f * m.gapSum / m.frames, 100.0f * m.gapMax);
			}
			m = {};
		}

		// Sentences in a subtitle, for the reaction count: each run of '.', '!', '?' or '...'
		// (a pause) followed by a space or the end. A line with none counts as one.
		int CountSentences(std::string_view a_text)
		{
			int  count = 0;
			bool inRun = false;
			for (std::size_t i = 0; i < a_text.size(); ++i) {
				const char c = a_text[i];
				const bool ellipsis = static_cast<unsigned char>(c) == 0xE2 && i + 2 < a_text.size() && static_cast<unsigned char>(a_text[i + 1]) == 0x80 &&
				                      static_cast<unsigned char>(a_text[i + 2]) == 0xA6;  // U+2026 in UTF-8
				if (c == '.' || c == '!' || c == '?' || ellipsis) {
					inRun = true;
					if (ellipsis) {
						i += 2;
					}
					continue;
				}
				if (inRun) {
					inRun = false;
					if (c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '"' || c == '\'' || c == ')') {
						++count;
					}
				}
			}
			if (inRun) {
				++count;
			}
			return (std::max)(count, a_text.empty() ? 0 : 1);
		}

		// Is this expression tag one of the INI's close-up emotions (case doesn't matter)?
		// Which emotion close-up tier a line's expression is in (0.8.6): 0 none, 1 mild, 2 medium,
		// 3 strong; with the chance for it.
		int CloseUpTier(std::string_view a_emotion, int* a_chance = nullptr, bool a_player = false)
		{
			if (a_emotion.empty() || a_emotion == "none") {
				return 0;
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
			const auto wanted = lower(a_emotion);
			const auto inList = [&](const std::string& a_list) {
				const std::string list = lower(a_list);
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
			};
			const auto& s = Settings::Get();
			const std::array<std::pair<const std::string*, int>, 3> tiers{
				std::pair{ &s.closeUpEmotionsStrong, a_player ? s.closeUpChanceStrongPlayer : s.closeUpChanceStrong },
				std::pair{ &s.closeUpEmotionsMedium, a_player ? s.closeUpChanceMediumPlayer : s.closeUpChanceMedium },
				std::pair{ &s.closeUpEmotionsMild, a_player ? s.closeUpChanceMildPlayer : s.closeUpChanceMild } };
			for (int i = 0; i < 3; ++i) {
				if (inList(*tiers[i].first)) {
					if (a_chance) {
						*a_chance = tiers[i].second;
					}
					return 3 - i;
				}
			}
			return 0;
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

		// ---- Obstacle detection ---------------------------------------------------------------
		// The game's dialogue camera checks for obstacles with a sphere cast (found in 0.5.1):
		//   Starfield.exe+1E8DC20(query, &from, &to, radius, &outHit, nullptr, 0.0f) -> bool hit
		// where query = [dialogue camera state + 0x48], and the hit's fraction along the line
		// is then at [query + 0x2B4]. The cast also moves `to` back to the last safe spot
		// (0.5.2 logs: the lengths measured after the call were the blocked lengths), so it is
		// always given copies. The game casts from the speaker to its camera; we cast from the
		// person in shot to each candidate camera spot (0.5.3: the Director skips or pulls in
		// blocked shots), and once more per shot for the log.
		constexpr std::uintptr_t kSphereCastRva = 0x1E8DC20;
		bool                     g_castBroken = false;

		bool SphereCastUnsafe(void* a_query, const RE::NiPoint3* a_from, const RE::NiPoint3* a_to, float a_radius, bool& a_hit, float& a_fraction, void*& a_what)
		{
			__try {
				using func_t = bool (*)(void*, const RE::NiPoint3*, const RE::NiPoint3*, float, void**, void*, float);
				const auto fn = reinterpret_cast<func_t>(reinterpret_cast<std::uintptr_t>(::GetModuleHandleW(nullptr)) + kSphereCastRva);
				void*      out = nullptr;
				a_hit = fn(a_query, a_from, a_to, a_radius, &out, nullptr, 0.0f);
				a_fraction = *reinterpret_cast<const float*>(reinterpret_cast<std::uintptr_t>(a_query) + 0x2B4);
				a_what = out;
				return true;
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				return false;
			}
		}

		void* CollisionQuery()
		{
			if (g_castBroken || !g_dialogueStateObject) {
				return nullptr;
			}
			std::uintptr_t query = 0;
			if (!SafeCopy(&query, static_cast<const std::uint8_t*>(g_dialogueStateObject) + 0x48, sizeof(query)) || query < 0x10000 || query > 0x7FFFFFFFFFFF) {
				static bool warned = false;
				if (!warned) {
					warned = true;
					REX::WARN("Obstacle check: the dialogue camera's collision query isn't there ({:#x}); not checking", query);
				}
				return nullptr;
			}
			return reinterpret_cast<void*>(query);
		}

		// One cast from a_from towards a_to. Returns false when it couldn't be done; a_fraction
		// is how far along the line the first hit is (1 = clear).
		bool Cast(const RE::NiPoint3& a_from, const RE::NiPoint3& a_to, float& a_fraction)
		{
			void* query = CollisionQuery();
			if (!query) {
				return false;
			}
			RE::NiPoint3 from = a_from, to = a_to;  // the cast writes into them
			bool         hit = false;
			void*        what = nullptr;
			a_fraction = 1.0f;
			if (!SphereCastUnsafe(query, &from, &to, Settings::Get().obstacleRadius, hit, a_fraction, what)) {
				g_castBroken = true;
				REX::WARN("Obstacle check: the game's sphere cast failed; obstacle checks are off for this session");
				return false;
			}
			if (!hit || !std::isfinite(a_fraction)) {
				a_fraction = 1.0f;
			}
			a_fraction = std::clamp(a_fraction, 0.0f, 1.0f);
			return true;
		}

		// As Cast, plus what the cast reports beside the fraction (room measurement diagnostics):
		// whether it hit, and the safe point it writes back into the end of the line.
		bool CastDetail(const RE::NiPoint3& a_from, const RE::NiPoint3& a_to, float& a_fraction, bool& a_hit, RE::NiPoint3& a_safe)
		{
			void* query = CollisionQuery();
			if (!query) {
				return false;
			}
			RE::NiPoint3 from = a_from, to = a_to;
			void*        what = nullptr;
			a_fraction = 1.0f;
			if (!SphereCastUnsafe(query, &from, &to, Settings::Get().obstacleRadius, a_hit, a_fraction, what)) {
				return false;
			}
			a_safe = to;
			return true;
		}

		float Distance(const RE::NiPoint3& a, const RE::NiPoint3& b)
		{
			const float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
			return std::sqrt(dx * dx + dy * dy + dz * dz);
		}

		// The Director's question: can the camera stand at a_camera to shoot a_subjectHead?
		//  - clear: yes;
		//  - the line touches the near person (an over-the-shoulder shot shows their shoulder
		//    on purpose): check again from their head to the camera, past them;
		//  - blocked near the camera (a wall behind it): pull the camera in to just before the
		//    obstacle, if that keeps at least 70% of the distance (and, over a shoulder, keeps
		//    the camera behind that shoulder);
		//  - blocked closer to the subject (a ladder in front of their face): skip the shot.
		// Shortest distance between segments P0-P1 and Q0-Q1; a_t: where on P (0-1).
		float SegmentDistance(const RE::NiPoint3& a_p0, const RE::NiPoint3& a_p1, const RE::NiPoint3& a_q0, const RE::NiPoint3& a_q1, float& a_t)
		{
			const RE::NiPoint3 d1{ a_p1.x - a_p0.x, a_p1.y - a_p0.y, a_p1.z - a_p0.z };
			const RE::NiPoint3 d2{ a_q1.x - a_q0.x, a_q1.y - a_q0.y, a_q1.z - a_q0.z };
			const RE::NiPoint3 r{ a_p0.x - a_q0.x, a_p0.y - a_q0.y, a_p0.z - a_q0.z };
			auto        dot = [](const RE::NiPoint3& a, const RE::NiPoint3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; };
			const float a = dot(d1, d1), e = dot(d2, d2), f = dot(d2, r);
			float       t = 0.0f, u = 0.0f;
			if (a <= 1e-6f && e <= 1e-6f) {
				a_t = 0.0f;
				return std::sqrt(dot(r, r));
			}
			if (a <= 1e-6f) {
				u = std::clamp(f / e, 0.0f, 1.0f);
			} else {
				const float c = dot(d1, r);
				if (e <= 1e-6f) {
					t = std::clamp(-c / a, 0.0f, 1.0f);
				} else {
					const float b = dot(d1, d2), denom = a * e - b * b;
					t = denom > 1e-6f ? std::clamp((b * f - c * e) / denom, 0.0f, 1.0f) : 0.0f;
					u = (b * t + f) / e;
					if (u < 0.0f) {
						u = 0.0f;
						t = std::clamp(-c / a, 0.0f, 1.0f);
					} else if (u > 1.0f) {
						u = 1.0f;
						t = std::clamp((b - c) / a, 0.0f, 1.0f);
					}
				}
			}
			a_t = t;
			const RE::NiPoint3 cp{ a_p0.x + d1.x * t - (a_q0.x + d2.x * u), a_p0.y + d1.y * t - (a_q0.y + d2.y * u), a_p0.z + d1.z * t - (a_q0.z + d2.z * u) };
			return std::sqrt(dot(cp, cp));
		}

		// Where a still bystander blocks the line from the subject to the camera, in metres from
		// the subject, or -1 for none.
		float BystanderBlock(const RE::NiPoint3& a_subjectHead, const RE::NiPoint3& a_camera, const RE::NiPoint3& a_dir, float a_length)
		{
			if (!Settings::Get().avoidBystanders) {
				return -1.0f;
			}
			std::vector<Bystander> people;
			{
				std::scoped_lock lock{ g_bystanderLock };
				people = g_bystanders;
			}
			float closest = -1.0f;
			for (const auto& p : people) {
				const RE::NiPoint3 top{ p.head.x, p.head.y, p.head.z + 0.12f };
				const RE::NiPoint3 bottom{ p.feet.x, p.feet.y, p.feet.z + 0.3f };
				// Along the line from the subject, between them and the camera?
				const RE::NiPoint3 mid{ (top.x + bottom.x) * 0.5f, (top.y + bottom.y) * 0.5f, (top.z + bottom.z) * 0.5f };
				const float along = (mid.x - a_subjectHead.x) * a_dir.x + (mid.y - a_subjectHead.y) * a_dir.y + (mid.z - a_subjectHead.z) * a_dir.z;
				if (along < 0.4f || along > a_length + 0.3f) {
					continue;
				}
				bool  blocks = false;
				float t = 0.0f;
				// Their body (a capsule about 0.3 m wide) across the line to the face in shot.
				if (SegmentDistance(a_subjectHead, a_camera, bottom, top, t) < 0.32f) {
					blocks = true;
				} else {
					// Or close enough to the camera to fill most of the picture (the lens is about
					// 30 degrees wide: someone 0.6 m wide within 1.7 m of it takes up most of it).
					const RE::NiPoint3 toMid{ mid.x - a_camera.x, mid.y - a_camera.y, mid.z - a_camera.z };
					const float        d = std::sqrt(toMid.x * toMid.x + toMid.y * toMid.y + toMid.z * toMid.z);
					const float        cosine = d > 1e-3f ? -(toMid.x * a_dir.x + toMid.y * a_dir.y + toMid.z * a_dir.z) / d : 0.0f;
					blocks = d < 1.7f && cosine > std::cos(0.30f);
				}
				const float at = (std::max)(0.0f, along - 0.35f);  // just short of their body
				if (blocks && (closest < 0.0f || at < closest)) {
					closest = at;
				}
			}
			return closest;
		}

		Director::Clearance TestShot(const RE::NiPoint3& a_subjectHead, const RE::NiPoint3& a_camera, const RE::NiPoint3& a_nearHead, bool a_overShoulder)
		{
			constexpr float kMargin = 0.25f;  // stop this far short of the obstacle
			Director::Clearance result;
			result.length = Distance(a_subjectHead, a_camera);
			if (!Settings::Get().obstacleCheck || result.length < 0.05f) {
				return result;
			}
			float              fraction = 1.0f;
			const bool         cast = Cast(a_subjectHead, a_camera, fraction);
			const float        length = result.length;
			const RE::NiPoint3 dir{ (a_camera.x - a_subjectHead.x) / length, (a_camera.y - a_subjectHead.y) / length, (a_camera.z - a_subjectHead.z) / length };
			// Bystanders standing still: one whose body crosses the line to the person in shot, or
			// who fills most of the picture from close to the camera, blocks like a wall at their
			// distance (0.7.4: Sarah standing still covered most of a shot in the Viewport).
			const float bystanderAt = BystanderBlock(a_subjectHead, a_camera, dir, length);
			if ((!cast || fraction >= 0.999f) && bystanderAt < 0.0f) {
				return result;
			}
			const bool hit = cast && fraction < 0.999f;
			float      blockedAt = hit ? fraction * length : length + 1.0f;  // past the camera = clear
			if (hit) {
				const RE::NiPoint3 hitPoint{ a_subjectHead.x + dir.x * blockedAt, a_subjectHead.y + dir.y * blockedAt, a_subjectHead.z + dir.z * blockedAt };
				const float        fromNear = std::sqrt((hitPoint.x - a_nearHead.x) * (hitPoint.x - a_nearHead.x) + (hitPoint.y - a_nearHead.y) * (hitPoint.y - a_nearHead.y));
				if (fromNear < 0.45f && hitPoint.z < a_nearHead.z + 0.3f && hitPoint.z > a_nearHead.z - 1.8f) {
					// The near person's body. What matters is whether the rest of the way is clear.
					result.pastNear = true;
					float rest = 1.0f;
					if (!Cast(a_nearHead, a_camera, rest) || rest >= 0.999f) {
						blockedAt = length + 1.0f;
					} else {
						const RE::NiPoint3 p{ a_nearHead.x + (a_camera.x - a_nearHead.x) * rest, a_nearHead.y + (a_camera.y - a_nearHead.y) * rest,
							a_nearHead.z + (a_camera.z - a_nearHead.z) * rest };
						blockedAt = (p.x - a_subjectHead.x) * dir.x + (p.y - a_subjectHead.y) * dir.y + (p.z - a_subjectHead.z) * dir.z;
					}
				}
			}
			if (bystanderAt >= 0.0f && bystanderAt < blockedAt) {
				blockedAt = bystanderAt;
				result.pastNear = false;
				result.bystander = true;
			}
			if (blockedAt > length) {
				return result;  // clear (past the near person, if it touched them)
			}
			result.blockedAt = blockedAt;
			const float safe = blockedAt - kMargin;
			float       keep = (std::max)(1.0f, 0.7f * length);
			if (a_overShoulder) {
				keep = (std::max)(keep, Distance(a_subjectHead, a_nearHead) + 0.6f);
			}
			result.limit = safe;
			result.usable = safe >= keep;
			// When nothing in the preset passes, the Director takes the blocked shot that still
			// has the most room, pulled in (0.5.3 test: Barrett beside the ladder got a shot
			// from his eyes through the ladder while the side-on Profile had 3.8 m clear).
			result.fallbackOk = safe >= (std::max)(1.5f, 0.4f * length) && (!a_overShoulder || safe >= Distance(a_subjectHead, a_nearHead) + 0.6f);
			return result;
		}

		// The shot on screen, once per shot, for the log: what the check says about it now.
		void LogShotObstacles(const char* a_shot, const RE::NiPoint3& a_subjectHead, const RE::NiPoint3& a_camera, const RE::NiPoint3& a_nearHead, bool a_overShoulder, const char* a_who)
		{
			if (!Settings::Get().obstacleCheck || !CollisionQuery()) {
				return;
			}
			const auto c = TestShot(a_subjectHead, a_camera, a_nearHead, a_overShoulder);
			if (c.blockedAt < 0.0f) {
				REX::INFO("Obstacle check ({}, {} in shot, {:.1f} m): clear{}", a_shot, a_who, c.length, c.pastNear ? " (past the near person)" : "");
			} else {
				REX::INFO("Obstacle check ({}, {} in shot, {:.1f} m): blocked {:.1f} m from them{}, {}", a_shot, a_who, c.length, c.blockedAt,
					c.bystander ? " (someone standing there)" : c.pastNear ? " (beyond the near person)" : "", c.usable ? std::format("could pull in to {:.1f} m", c.limit) : std::string{ "blocked" });
			}
		}

		// Room measurement (groundwork for CCC's room shots, Wide, Master and Distant; log
		// only, nothing is filmed differently yet). From between the two people: the ceiling,
		// and how far it's open in 16 directions at eye height and 1.5 m higher, with the
		// game's collision check. Then, for each room shot, how far back it wants to be (at
		// this lens), the best place it could actually stand, how big the people would be from
		// there, and whether both faces are in clear view. Measured once per conversation and
		// again when the two move more than 1.5 m.
		void MeasureRoom(const RE::NiPoint3& a_npcHead, const RE::NiPoint3& a_playerHead, float a_ground, float a_side, bool a_log)
		{
			constexpr int   kBearings = 16;
			constexpr float kReach = 25.0f;
			constexpr float kSky = 15.0f;
			constexpr float kWallMargin = 0.5f;
			constexpr float kPeople = 1.8f;                  // height of a standing person
			// Each room shot at its own lens (0.8.12: CCC's lens per angle): the one its distance is
			// worked out for, and the one drawn.
			const auto verticalOf = [](float a_lens) { return 2.0f * std::atan(std::tan(a_lens * 0.5f * std::numbers::pi_v<float> / 180.0f) / 1.7778f); };
			Shots::ShotType current = Shots::ShotType::kWide;  // (set for each shot below)
			const auto      share = [&](float a_distance) {  // of the frame's height, a person standing there
                const float vertical = verticalOf(Director::LensFor(current));
                return a_distance > 0.1f ? kPeople / (2.0f * a_distance * std::tan(vertical * 0.5f)) : 1.0f;
			};
			const auto wants = [&](float a_fill) {  // distance for a 0.6 m extent filling a_fill (as Shots)
				return 0.3f / std::tan(verticalOf(Director::LensFor(current)) * a_fill * 0.5f);
			};

			const RE::NiPoint3 mid{ (a_npcHead.x + a_playerHead.x) * 0.5f, (a_npcHead.y + a_playerHead.y) * 0.5f, (a_npcHead.z + a_playerHead.z) * 0.5f };
			const float        eyeline = std::atan2(a_npcHead.y - a_playerHead.y, a_npcHead.x - a_playerHead.x);  // towards them

			// The ceiling. 0.7.5 test: every roofed place measured 1.6 m (the cast's fraction
			// came back 0 going straight up). Now from the safe point the cast writes back,
			// cross-checked by casts from higher up, and the floor straight down.
			float        up = 1.0f;
			bool         upHit = false;
			RE::NiPoint3 upSafe{};
			if (!CastDetail(mid, { mid.x, mid.y, mid.z + kSky }, up, upHit, upSafe)) {
				return;
			}
			float        down = 1.0f, up2 = 1.0f;
			bool         downHit = false, up2Hit = false;
			RE::NiPoint3 downSafe{}, up2Safe{};
			CastDetail(mid, { mid.x, mid.y, mid.z - 4.0f }, down, downHit, downSafe);
			const RE::NiPoint3 higher{ mid.x, mid.y, mid.z + 0.8f };
			CastDetail(higher, { mid.x, mid.y, mid.z + kSky }, up2, up2Hit, up2Safe);
			if (a_log) REX::INFO("Room: straight up from between you: hit {} at fraction {:.3f}, safe point {:.2f} m up; from 0.8 m higher: hit {} at {:.3f}, safe point {:.2f} m up; "
					  "straight down: hit {} at {:.3f}, safe point {:.2f} m down (your feet {:.2f} m down)",
				upHit, up, upSafe.z - mid.z, up2Hit, up2, up2Safe.z - mid.z, downHit, down, mid.z - downSafe.z, mid.z - a_ground);
			// The safe point is the better reading when it's sensible (the fraction may be
			// measured differently going up).
			float rise = kSky;
			if (upHit) {
				rise = upSafe.z - mid.z > 0.05f && upSafe.z - mid.z < kSky ? upSafe.z - mid.z : up * kSky;
				if (rise < 0.3f && up2Hit && up2Safe.z - mid.z > 0.85f) {
					rise = up2Safe.z - mid.z;
				}
			}
			const bool  roofed = upHit && rise < kSky - 0.01f;
			const float ceiling = roofed ? mid.z + rise - a_ground : 0.0f;
			const float highRise = roofed ? std::clamp(ceiling - (mid.z - a_ground) - 0.4f, 0.0f, 1.5f) : 1.5f;

			// Towards either person the cast meets their body first; past them it starts again.
			const float personZone = Distance(a_npcHead, a_playerHead) * 0.5f + 0.45f;
			const auto  open = [&](const RE::NiPoint3& a_from, float a_dx, float a_dy, bool a_nearLine) {
                float f = 1.0f;
                Cast(a_from, { a_from.x + a_dx * kReach, a_from.y + a_dy * kReach, a_from.z }, f);
                if (a_nearLine && f * kReach < personZone) {
                    const RE::NiPoint3 past{ a_from.x + a_dx * personZone, a_from.y + a_dy * personZone, a_from.z };
                    float              g = 1.0f;
                    Cast(past, { a_from.x + a_dx * kReach, a_from.y + a_dy * kReach, a_from.z }, g);
                    return personZone + g * (kReach - personZone);
                }
                return f * kReach;
			};
			std::array<float, kBearings> low{}, high{};
			for (int i = 0; i < kBearings; ++i) {
				const float a = eyeline + i * 2.0f * std::numbers::pi_v<float> / kBearings;
				const bool  nearLine = i <= 2 || i >= kBearings - 2 || std::abs(i - kBearings / 2) <= 2;  // within 45 degrees of the line
				low[i] = open(mid, std::cos(a), std::sin(a), nearLine);
				high[i] = open({ mid.x, mid.y, mid.z + highRise }, std::cos(a), std::sin(a), nearLine);
			}
			std::string around, above;
			for (int i = 0; i < kBearings; ++i) {
				around += std::format("{}{}:{:.1f}", i ? " " : "", i * 360 / kBearings, low[i]);
				above += std::format("{}{}:{:.1f}", i ? " " : "", i * 360 / kBearings, high[i]);
			}
			const float along = low[0] + low[kBearings / 2];
			const float across = low[kBearings / 4] + low[3 * kBearings / 4];
			if (a_log) REX::INFO("Room: {}; {:.1f} m along the eyeline, {:.1f} m across it (wall to wall through the pair)", roofed ? std::format("ceiling {:.1f} m", ceiling) : "no ceiling within 15 m",
				along, across);
			if (a_log) REX::INFO("Room: open from the pair (degrees from the line towards them, anticlockwise: metres) at eye height {}; {:.1f} m higher {}", around, highRise, above);

			// Each room shot: its setup's fill and height (Shots' SpecFor: Wide 0.11 and 24
			// units up, Master 0.075 and 76 up, Distant 0.09 and 28 up), placed (0.8) at the
			// direction with the most room on the conversation's side of the line (0 to 165
			// degrees off it, as the solver counts), both faces in clear view, as far back as
			// CCC's framing wants or the room allows; no room shot where the room gives less than
			// iRoomShotMinRoom percent of that.
			struct RoomShot
			{
				const char* name;
				float       fill;
				float       rise;
				float       angle;  // CCC's own, preferred where the room allows
			};
			constexpr std::array kShots{ RoomShot{ "Wide", 0.11f, 24.0f / 70.0f, 0.0f }, RoomShot{ "Master", 0.075f, 76.0f / 70.0f, 24.0f },
				RoomShot{ "Distant", 0.09f, 28.0f / 70.0f, 0.0f } };
			constexpr std::array kShotTypes{ Shots::ShotType::kWide, Shots::ShotType::kMaster, Shots::ShotType::kDistant };
			const float lineSide = a_side < 0.0f ? -1.0f : 1.0f;
			const float minShare = static_cast<float>(Settings::Get().roomShotMinRoom) / 100.0f;
			g_state.roomAngle.fill(std::numeric_limits<float>::quiet_NaN());
			g_state.roomDistance.fill(0.0f);
			g_state.roomRise = roofed ? (std::max)(0.2f, ceiling - (mid.z - a_ground) - 0.4f) : 10.0f;
			std::string summary;
			// No further back than the preset allows (0.8.1: CCC's presets as far as CCC's
			// framing wants, 12 to 18 m at this lens; CCC SF's 9 m indoors, 12 m outdoors), or
			// fRoomShotMaxIndoors / fRoomShotMaxOutdoors.
			auto*       roomPlayer = RE::PlayerCharacter::GetSingleton();
			const bool  interior = roomPlayer && roomPlayer->parentCell && roomPlayer->parentCell->IsInterior();
			const float roomCap = Director::RoomShotLimit(interior);
			// The limit applies to the furthest of them (Master), and the others keep their
			// distance from it in proportion (0.8.1 test: capped one by one, Wide and Distant
			// came out the same, 12 m on the line).
			float furthest = 0.0f;
			for (std::size_t n = 0; n < kShots.size(); ++n) {
				current = kShotTypes[n];
				furthest = (std::max)(furthest, wants(kShots[n].fill));
			}
			const float roomScale = roomCap > 0.0f && furthest > roomCap ? roomCap / furthest : 1.0f;
			for (std::size_t n = 0; n < kShots.size(); ++n) {
				const auto& shot = kShots[n];
				current = kShotTypes[n];
				const float wanted = wants(shot.fill) * roomScale;
				const auto  room = [&](int i) { return (shot.rise > 0.7f ? (std::min)(low[i], high[i]) : low[i]) - kWallMargin; };
				// The solver's angle for a bearing (bearings from the line towards the NPC,
				// anticlockwise; the solver turns from the line towards you, by side).
				const auto angleOf = [&](int i) {
					float a = (static_cast<float>(i) * 360.0f / kBearings - 180.0f) * lineSide;
					while (a <= -180.0f) {
						a += 360.0f;
					}
					while (a > 180.0f) {
						a -= 360.0f;
					}
					return a;
				};
				std::vector<int> order;
				for (int i = 0; i < kBearings; ++i) {
					const float a = angleOf(i);
					if (a >= 0.0f && a <= 165.0f) {
						order.push_back(i);
					}
				}
				// Most room first (as far as the shot wants: past that, more doesn't help), then
				// nearest CCC's own angle for the shot, then away from where the other room shots
				// already stand (0.8.0 test: outdoors all three stood at the same place).
				std::array<int, kBearings> used{};
				for (std::size_t m = 0; m < n; ++m) {
					if (!std::isnan(g_state.roomAngle[m])) {
						for (int i = 0; i < kBearings; ++i) {
							if (std::abs(angleOf(i) - g_state.roomAngle[m]) < 1.0f) {
								++used[i];
							}
						}
					}
				}
				const auto score = [&](int i) {
					const float space = (std::min)(room(i), wanted);
					return space - 0.02f * std::abs(angleOf(i) - shot.angle) - (used[i] > 0 ? 3.0f : 0.0f);
				};
				std::ranges::sort(order, [&](int a, int b) { return score(a) > score(b); });
				float bestDistance = 0.0f;
				int   bestBearing = -1;
				bool  bestClear = false;
				for (std::size_t k = 0; k < order.size() && k < 4; ++k) {
					const int   i = order[k];
					const float d = std::clamp(room(i), 0.0f, wanted);
					if (d < 1.0f) {
						continue;  // ordered by score now, not by room: a later one can still have room
					}
					const float        a = eyeline + i * 2.0f * std::numbers::pi_v<float> / kBearings;
					const RE::NiPoint3 cam{ mid.x + std::cos(a) * d, mid.y + std::sin(a) * d, mid.z + (std::min)(shot.rise, g_state.roomRise) };
					const auto         toNpc = TestShot(a_npcHead, cam, a_playerHead, false);
					const auto         toYou = TestShot(a_playerHead, cam, a_npcHead, false);
					const bool         clear = toNpc.blockedAt < 0.0f && toYou.blockedAt < 0.0f;
					if ((clear && !bestClear) || (clear == bestClear && (bestBearing < 0 || score(i) > score(bestBearing)))) {
						bestDistance = d;
						bestBearing = i;
						bestClear = clear;
					}
				}
				const bool usable = bestBearing >= 0 && bestClear && bestDistance >= minShare * wanted;
				if (usable) {
					g_state.roomAngle[n] = angleOf(bestBearing);
					g_state.roomDistance[n] = bestDistance;
				}
				if (!a_log) {
					continue;
				}
				if (bestBearing < 0) {
					REX::INFO("Room shot {}: wants {:.1f} m back (people {:.0f}% of the picture's height); no room for it on this side", shot.name, wanted, 100.0f * share(wanted));
					summary += std::format("{}{} no room", summary.empty() ? "" : ", ", shot.name);
					continue;
				}
				REX::INFO("Room shot {}: wants {:.1f} m back (people {:.0f}% of the picture's height); best {:.1f} m at {:.0f} degrees off the line (people {:.0f}%), {}; {}",
					shot.name, wanted, 100.0f * share(wanted), bestDistance, angleOf(bestBearing), 100.0f * share(bestDistance),
					bestClear ? "both faces in clear view" : "a face is blocked from there", usable ? "available" : "not used");
				summary += std::format("{}{} {:.1f}/{:.0f} m{}", summary.empty() ? "" : ", ", shot.name, bestDistance, wanted, usable ? "" : " (no)");
			}
			if (a_log && Settings::Get().showMessages) {
				QueueHudMessage(std::format("Room: {:.0f} x {:.0f} m, {} | {}", along, across, roofed ? std::format("ceiling {:.1f} m", ceiling) : std::string{ "open sky" }, summary));
			}
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
				// Keep the last shot until the game leaves its dialogue camera, then cut straight
				// to the player's view: first person (the first-person hook cuts there) and, since
				// 0.7.6, third person too (0.7.5 test: the saved third-person view shown meanwhile
				// rested on the NPC's middle for a second before the game's camera took over).
				const bool holdToEnd = held < 3.0f && (g_state.startedFirstPerson ? g_firstPersonView.has_value() : true);
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

			// The heads as smoothed but before the dead band: what iCameraSway lets the camera
			// follow (the "handheld" part of the motion).
			RE::NiPoint3 npcSwayHead{}, playerSwayHead{};
			// Follow the heads through a short low-pass filter instead of frame by frame. Raw
			// bone positions are sampled mid-animation on a worker thread, so tracking them
			// directly made the picture vibrate when someone walked; this also steadies the
			// shot against idle head bob. Large jumps (teleports, body rebuild) snap.
			{
				const float k = 1.0f - std::exp(-(std::clamp(delta, 0.0f, 0.1f)) / (std::max)(s.followSmoothing, 0.001f));
				// How fast the speaker's head moves (smoothed over about 0.3 s). Walking makes the
				// head bob with each step; follow it more steadily then (0.4.20).
				{
					const float dt = std::clamp(delta, 0.0f, 0.1f);
					if (g_state.lastRawNpcHead && dt > 0.0f) {
						const float mx = npcHead->x - g_state.lastRawNpcHead->x, my = npcHead->y - g_state.lastRawNpcHead->y, mz = npcHead->z - g_state.lastRawNpcHead->z;
						const float step = std::sqrt(mx * mx + my * my + mz * mz);
						if (step < 0.5f) {
							const float a = 1.0f - std::exp(-dt / 0.3f);
							g_state.npcHeadSpeed += (step / dt - g_state.npcHeadSpeed) * a;
						}
					}
					g_state.lastRawNpcHead = *npcHead;
					const bool busy = g_state.npcHeadBusy ? g_state.npcHeadSpeed > s.busyHeadSpeed * 0.6f : g_state.npcHeadSpeed > s.busyHeadSpeed;
					if (g_state.npcHeadBusy) {
						g_state.busySeconds += dt;
					}
					if (busy != g_state.npcHeadBusy) {
						g_state.npcHeadBusy = busy;
						if (busy) {
							++g_state.busySwitches;
						}
						if (s.debugLog) {
							REX::INFO("Speaker's head {} ({:.2f} m/s): following with {:.2f} s of smoothing", busy ? "moving fast" : "settled", g_state.npcHeadSpeed,
								busy ? s.busyFollowSmoothing : s.followSmoothing);
						}
					}
				}
				const float kNpc = g_state.npcHeadBusy ?
				                       1.0f - std::exp(-(std::clamp(delta, 0.0f, 0.1f)) / (std::max)(s.busyFollowSmoothing, 0.001f)) :
				                       k;
				auto follow = [](std::optional<RE::NiPoint3>& a_state, const RE::NiPoint3& a_raw, float a_k) {
                    if (!a_state) {
                        a_state = a_raw;
                    } else {
                        const float dx = a_raw.x - a_state->x, dy = a_raw.y - a_state->y, dz = a_raw.z - a_state->z;
                        if (dx * dx + dy * dy + dz * dz > 4.0f) {
                            a_state = a_raw;
                        } else {
                            a_state->x += dx * a_k;
                            a_state->y += dy * a_k;
                            a_state->z += dz * a_k;
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
				const auto rawNpcHead = *npcHead;
				npcHead = follow(g_state.smoothNpcHead, *npcHead, kNpc);
				playerHead = follow(g_state.smoothPlayerHead, *playerHead, k);
				// Dead band (0.6.1, CCC's face framing): the framed point only moves once the head
				// has gone further than fFaceDeadband from it, so idle sway and the bob of
				// someone working at a bench don't move the camera; leaning and walking do.
				npcSwayHead = *npcHead;
				playerSwayHead = *playerHead;
				if (s.faceDeadband > 0.0f) {
					auto slack = [band = s.faceDeadband](std::optional<RE::NiPoint3>& a_held, const RE::NiPoint3& a_live) {
						if (!a_held) {
							a_held = a_live;
						} else {
							const float dx = a_live.x - a_held->x, dy = a_live.y - a_held->y, dz = a_live.z - a_held->z;
							const float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
							if (dist > band) {
								const float f = (dist - band) / dist;
								a_held->x += dx * f;
								a_held->y += dy * f;
								a_held->z += dz * f;
							}
						}
						return *a_held;
					};
					npcHead = slack(g_state.slackNpcHead, *npcHead);
					playerHead = slack(g_state.slackPlayerHead, *playerHead);
					// iCameraSway: put that share of the small head motion back (0 = steady, 100 =
					// the camera follows the heads exactly, as in 0.6.0). 0.6.1-0.6.2 applied the
					// percentage only to the angle, after the dead band had already removed most
					// of the motion, so even 100% hardly swayed.
					const float w = std::clamp(Director::CameraSway(), 0, 100) / 100.0f;
					npcHead = RE::NiPoint3{ npcHead->x + (npcSwayHead.x - npcHead->x) * w, npcHead->y + (npcSwayHead.y - npcHead->y) * w,
						npcHead->z + (npcSwayHead.z - npcHead->z) * w };
					playerHead = RE::NiPoint3{ playerHead->x + (playerSwayHead.x - playerHead->x) * w, playerHead->y + (playerSwayHead.y - playerHead->y) * w,
						playerHead->z + (playerSwayHead.z - playerHead->z) * w };
				}
				// Measurement: how much the speaker's head moves, and how far the smoothed head
				// the camera follows lags behind it.
				if (s.debugLog) {
					auto&       m = g_state.head;
					const float gx = rawNpcHead.x - npcHead->x, gy = rawNpcHead.y - npcHead->y, gz = rawNpcHead.z - npcHead->z;
					const float gap = std::sqrt(gx * gx + gy * gy + gz * gz);
					if (m.last && delta > 0.0f && delta < 0.1f) {
						const float mx = rawNpcHead.x - m.last->x, my = rawNpcHead.y - m.last->y, mz = rawNpcHead.z - m.last->z;
						const float step = std::sqrt(mx * mx + my * my + mz * mz);
						if (step < 0.5f) {
							m.path += step;
							m.seconds += delta;
							m.peakSpeed = (std::max)(m.peakSpeed, step / delta);
						}
					}
					m.last = rawNpcHead;
					++m.frames;
					m.gapSum += gap;
					m.gapMax = (std::max)(m.gapMax, gap);
				}
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

			// Each line the NPC starts (read once per line): its expression tag (emotion
			// close-ups), whether it answers a persuasion's outcome or a skill check (the
			// persuasion beat), and whether it's shown on you listening (a reaction shot).
			{
				const bool talkingNow = speaker->IsTalking();
				// The end of one of their lines (or of a part of it: below).
				const auto endLine = [&]() {
						g_state.lineBeat = false;
						// How long the line really lasted, to check the 14-characters-a-second guess
						// reactions are planned with (0.8.3).
						if (s.debugLog && !g_state.lastLineText.empty()) {
							const float lasted = std::chrono::duration<float>(now - g_state.lineStart).count();
							if (lasted > 0.3f) {
								REX::INFO("Line length: {} characters, {:.1f} s ({:.1f} characters a second; the guess is 14)", g_state.lastLineText.size(), lasted,
									static_cast<float>(g_state.lastLineText.size()) / lasted);
							}
						}
						// 0.8.5: their talking adds to the time the camera has been on them without a
						// cut to you; a reaction (it lasts the line) ends here, with the line.
						const float lastedNow = std::chrono::duration<float>(now - g_state.lineStart).count();
						// 0.8.9 test: a long line's reaction (on you for its first 4 s) had ended by the
						// line's end, so it didn't count as shown: the time on them went on adding
						// up (16, 24, 36 s) and the next four lines were all reactions.
						const bool shown = g_state.reactionShownLine;
						g_state.reactionShownLine = false;
						if (shown || g_state.replyBeat) {
							g_state.monoSeconds = 0.0f;  // (their answer to a persuasion doesn't count towards a reaction)
						} else if (lastedNow > 0.0f && lastedNow < 60.0f) {
							g_state.monoSeconds += lastedNow;  // (a reaction planned for later in the line that never came counts as time on them)
						}
						g_state.reactionActive = false;
						g_state.reactionCloseUp = false;
				};
				// 0.8.3 test: several of their lines can play back to back without the game ever
				// reporting a pause (one record, several responses: 74 characters "lasting" 38 s).
				// A new subtitle while they keep talking is a new line too.
				bool newPart = false;
				if (talkingNow && g_state.speakerWasTalking && !g_state.reactionPending && now - g_state.lineStart > 800ms && !g_state.lastLineText.empty()) {
					if (auto* actor = static_cast<RE::Actor*>(speaker); actor->currentProcess && actor->currentProcess->high) {
						const char* raw = actor->currentProcess->high->strVoiceSubtitle.c_str();
						newPart = raw && *raw && g_state.lastLineText != raw;
					}
					if (newPart) {
						if (s.debugLog) {
							REX::INFO("Line: a new line while they keep talking (a new subtitle)");
						}
						endLine();
					}
				}
				if (talkingNow && (!g_state.speakerWasTalking || newPart)) {
					g_lineSerial.fetch_add(1, std::memory_order_acq_rel);
					++g_state.npcLinesStarted;
					// An emotion close-up: a line tagged with one of sCloseUpEmotions, iCloseUpChance
					// of the time (0.8.3: every expression but Neutral and Friendly, at 67%). A line
					// that rolls no is an ordinary line (it can be a reaction).
					{
						const auto emotion = LineEmotion(static_cast<RE::Actor*>(speaker));
						int        chance = 0;
						// The preset's own (0.8.13): none in CCC's Standard and Room, strong lines
						// only in Close (CCC's "full intensity"), every tier in CCC SF's.
						const int  mode = Director::EmotionCloseUps();
						int        tier = mode != 0 ? CloseUpTier(emotion, &chance) : 0;
						if (mode == 3 && tier != 3) {
							tier = 0;
						}
						const bool listed = tier > 0;
						const bool rolled = listed && std::uniform_int_distribution<int>{ 0, 99 }(g_reactionRng) < chance;
						// 0.8.7 test: two emotion lines in a row held one close-up for 17 s, the due
						// reaction waiting at the second. When the last line was a regular close-up
						// on screen at least fShortestHold, this one carries it on with no new cut,
						// so a due reaction can take it like any line.
						// (monoSeconds > 0: no turn of yours or reaction since, which cut away.)
						const bool wasCloseUp = g_state.lineIntense && !g_state.lineExtreme && g_state.monoSeconds > 0.0f &&
						                        std::chrono::duration<float>(now - g_state.lineStart).count() >= s.shortestHold;
						// An extreme close-up holds one line only (0.8.9): the line after one, on the
						// camera without a cut since, is never an extreme one too.
						const bool afterExtreme = g_state.lineIntense && g_state.lineExtreme && g_state.monoSeconds > 0.0f;
						g_state.lineIntense = rolled;
						// Extreme or not decided here (0.8.3), so a reaction knows not to interrupt it.
						g_state.lineExtreme = rolled && !afterExtreme && std::uniform_int_distribution<int>{ 0, 99 }(g_reactionRng) < s.extremeCloseUpChance;
						g_state.closeUpGoesOn = rolled && wasCloseUp && !g_state.lineExtreme;
						if (listed && !rolled && s.debugLog) {
							REX::INFO("Emotion close-up: '{}' line ({}), the {}% roll said no", emotion, tier == 3 ? "strong" : tier == 2 ? "medium" : "mild", chance);
						}
					}
					g_state.lineStart = now;
					g_state.lineBeat = false;
					g_state.reactionActive = false;
					g_state.reactionCloseUp = false;
					g_state.reactionPending = true;
				}
				// Reaction shots (CCC; rules since 0.8.5 below). The subtitle text is only there a
				// moment after the line starts (0.7.1 never saw it at the start), so this waits up
				// to half a second for it to change.
				if (talkingNow && g_state.reactionPending) {
					std::string text;
					if (auto* actor = static_cast<RE::Actor*>(speaker); actor->currentProcess && actor->currentProcess->high) {
						if (const char* raw = actor->currentProcess->high->strVoiceSubtitle.c_str()) {
							text = raw;
						}
					}
					const bool fresh = !text.empty() && text != g_state.lastLineText;
					// 0.7.11 test: the decision came before your face was set for this line, so a close-up
					// of you followed the previous line's face. Wait for it (at most 0.8 s).
					const auto faceAt = std::chrono::steady_clock::time_point{ std::chrono::steady_clock::duration{ g_faceLineAt.load(std::memory_order_acquire) } };
					const bool faceReady = !s.playerFace || faceAt >= g_state.lineStart - 150ms || now - g_state.lineStart > 800ms;
					if ((fresh || now - g_state.lineStart > 500ms) && faceReady) {
						g_state.reactionPending = false;
						if (fresh) {
							g_state.lastLineText = text;
						}
						if (fresh && Director::ReactionShots() && s.director && s.shotMode == ShotMode::kFollowSpeaker && g_state.bodyReady &&
							!g_state.lineBeat && !g_state.replyBeat) {
							// 0.8.5, reactions break the monotony of long stretches: once the camera
							// has been on them for fReactionAfterSecondsPlayer of their talking without
							// a cut to you, a reaction is due (iReactionChancePlayer of the time; a no
							// waits another stretch). It's taken by the next line that suits it: one
							// that starts here and lasts fReactionLineMinSecondsPlayer to
							// fReactionLineMaxSecondsPlayer (guessed from the subtitle, about 14
							// characters a second), not an emotion close-up line. It lasts the whole
							// line, so the cuts are at the line's edges, never in the middle (0.8.4
							// test: some reactions were too abrupt). Then the time starts again.
							const float lineSeconds = static_cast<float>(text.size()) / 14.0f;
							const float reactionAfter = Director::ReactionAfterSeconds();  // the preset's own (0.8.15)
							const float onThem = g_state.monoSeconds;
							const auto  secs = [](float a_seconds) {
								return std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<float>(a_seconds));
							};
							const auto take = [&](bool a_intoCloseUp, float a_from, float a_hold = -1.0f) {
								// 0.8.6: a close-up of you by your expression's tier (the same lists as theirs):
								// iCloseUpChanceMildPlayer 25%, ...Medium 40%, ...Strong 100% (0.8.9; were 30 and 50).
								const std::string face = s.playerFace && g_playerFaceShown.load(std::memory_order_acquire) ? Face::Shown() : std::string{};
								int               faceChance = 0;
								const int         faceTier = face.empty() ? 0 : CloseUpTier(face, &faceChance, true);
								const bool        closeUp = faceTier > 0 && std::uniform_int_distribution<int>{ 0, 99 }(g_reactionRng) < faceChance;
								REX::INFO("Reaction shot: '{}' ({:.1f} s) shown on you {}{}", text.substr(0, 60), lineSeconds,
									closeUp ? std::format("close (your {} expression, {})", faceTier == 3 ? "strong" : faceTier == 2 ? "medium" : "mild", face) :
									          face.empty() ? std::string{ "listening" } : std::format("listening ({}, the {}% roll said no or no tier)", face, faceChance),
									a_intoCloseUp ? std::format(", {:.1f} s into their emotion close-up", a_from) :
									a_hold > 0.0f ? std::format(", its first {:.1f} s (a long line), then back to them", a_hold) : std::string{});
								g_state.reactionOwed = false;
								g_state.reactionActive = true;
								g_state.reactionCloseUp = closeUp;
								g_state.reactionFrom = g_state.lineStart + secs(a_from);
								g_state.reactionOnAt = {};
								g_state.reactionHold = a_hold;  // < 0: to the end of the line
								g_reactionStarted.store(true, std::memory_order_release);
							};
							if (g_state.lineIntense && !g_state.closeUpGoesOn) {
								// An emotion close-up is part of the stretch on them (0.8.5): the
								// reaction cuts into a regular one when the time on them reaches
								// fReactionAfterSecondsPlayer during it, and stays to the end of the line
								// (the rest has to suit). Never an extreme close-up; and one that was
								// already due before the line waits for the next line, so the close-up
								// isn't cut away from at once.
								if (!g_state.lineExtreme && !g_state.reactionOwed && onThem < reactionAfter) {
									const float crossAt = reactionAfter - onThem;
									const float onYou = lineSeconds - crossAt;
									// 0.8.5 test: the 7 s came 0.4 s into a close-up, a cut away at once.
									// Within its minimum time on screen (fShortestHold), the reaction
									// waits for the next line instead, as one already due does.
									if (crossAt >= s.shortestHold && onYou >= s.reactionLineMin && onYou <= s.reactionLineMax &&
										std::uniform_int_distribution<int>{ 0, 99 }(g_reactionRng) < s.reactionChance) {
										take(true, crossAt);
									}
								}
							} else {
								if (!g_state.reactionOwed && onThem >= reactionAfter) {
									g_state.reactionOwed = std::uniform_int_distribution<int>{ 0, 99 }(g_reactionRng) < s.reactionChance;
									if (!g_state.reactionOwed) {
										g_state.monoSeconds = 0.0f;
									}
									if (s.debugLog) {
										REX::INFO("Reaction: {:.1f} s on them without a cut to you; {}", onThem,
											g_state.reactionOwed ? "one is due" : "the chance roll said no, another stretch first");
									}
								}
								// 0.8.7 test: a line guessed at 7.4 s lasted 6.0, and its split left 2 s on
								// them between you and your turn. Up to a second over the longest, a line
								// is shown on you whole; longer ones are split.
								const float wholeMax = s.reactionLongLineHold > 0.0f ? s.reactionLineMax + 1.0f : s.reactionLineMax;
								if (g_state.reactionOwed && lineSeconds >= s.reactionLineMin && lineSeconds <= wholeMax) {
									take(false, 0.0f);
								} else if (g_state.reactionOwed && lineSeconds > wholeMax && s.reactionLongLineHold > 0.0f) {
									// 0.8.6 test: Sarah's lines run 8 to 12 s, none suited, and she went on for
									// 49 s without one. A long line takes it too: on you from its start for
									// fReactionLongLineSecondsPlayer, then back to them for the rest.
									take(false, 0.0f, s.reactionLongLineHold);
								} else if (g_state.reactionOwed && s.debugLog) {
									REX::INFO("Reaction: due, but this line doesn't suit it ({:.1f} s); waiting for the next", lineSeconds);
								}
							}
						}
					}
				}
				// The persuasion beat: a persuasion's outcome or a chosen skill check ([Intimidate],
				// [Manipulation]...) noticed by the HUD just before or as this line starts.
				if (talkingNow && s.persuasionBeat && !g_state.lineBeat && now - g_state.lineStart < 1500ms) {
					const auto outcome = Hud::PersuasionOutcomeAt();
					const auto age = now.time_since_epoch().count() - outcome;
					// Only outcomes from this conversation (0.7.1: a hire at the end of one carried
					// into the next).
					if (outcome != 0 && outcome != g_state.beatConsumed && age >= 0 && outcome > g_state.started.time_since_epoch().count() &&
						age < std::chrono::duration_cast<std::chrono::steady_clock::duration>(8s).count()) {
						g_state.beatConsumed = outcome;
						g_state.lineBeat = true;
						g_state.replyBeat = true;
						g_state.monoSeconds = 0.0f;
						g_state.reactionActive = false;
						g_state.reactionCloseUp = false;
						REX::INFO("Persuasion beat: this answer is held on a close-up of the NPC");
					}
				}
				if (!talkingNow && g_state.speakerWasTalking) {
					endLine();
				}
				g_state.speakerWasTalking = talkingNow;
			}
			// Diagnostic: do your own lines carry an expression tag too? The game never reports
			// the player as talking (0.7.1 waited for that and logged nothing), so watch the
			// player's voice data instead: a new dialogue record or a new subtitle there is a
			// new line of yours.
			if (s.debugLog) {
				const auto  info = LineInfo(player);
				std::string text;
				if (player->currentProcess && player->currentProcess->high) {
					if (const char* raw = player->currentProcess->high->strVoiceSubtitle.c_str()) {
						text = raw;
					}
				}
				if ((info != 0 && info != g_state.playerLineInfo) || (!text.empty() && text != g_state.playerLineText)) {
					std::uint32_t type = 0, id = 0;
					const char*   editorId = nullptr;
					if (info) {
						ReadFormInfo(info, type, id, editorId);
					}
					const auto emotion = info ? LineEmotion(player) : std::string{ "no record" };
					REX::INFO("Your line emotion: {} for '{}' (record {:08X}{}, talking {})", emotion.empty() ? "unknown" : emotion, text.substr(0, 60), id,
						info == g_state.playerLineInfo ? ", same record" : "", player->IsTalking() ? "yes" : "no");
					g_state.playerLineInfo = info;
					g_state.playerLineText = text;
				}
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
					g_state.directRetries = 0;
				}
			}

			// Which side of the line between you the conversation is staged from (the 180-degree
			// rule): the side with more room, measured once with the game's collision check from
			// the midpoint between your heads; near-equal room is a coin toss (CCC's ChooseSide).
			if (!g_state.sideChosen) {
				g_state.sideChosen = true;
				g_state.side = 1.0f;
				const float ax = npcHead->x - playerHead->x, ay = npcHead->y - playerHead->y;
				const float al = std::sqrt(ax * ax + ay * ay);
				if (al > 0.05f) {
					// +1 = to the right of the line from you to them (the camera over your right shoulder).
					const RE::NiPoint3 right{ ay / al, -ax / al, 0.0f };
					const RE::NiPoint3 mid{ (npcHead->x + playerHead->x) * 0.5f, (npcHead->y + playerHead->y) * 0.5f, (npcHead->z + playerHead->z) * 0.5f };
					constexpr float    kProbe = 3.7f;  // CCC: 260 units
					float              fr = 1.0f, fl = 1.0f;
					const bool         measured = s.obstacleCheck && Cast(mid, { mid.x + right.x * kProbe, mid.y + right.y * kProbe, mid.z }, fr) &&
					                      Cast(mid, { mid.x - right.x * kProbe, mid.y - right.y * kProbe, mid.z }, fl);
					const float        roomRight = fr * kProbe, roomLeft = fl * kProbe;
					const char*        how = "coin toss";
					if (measured && roomRight > roomLeft + 0.5f) {
						how = "more room";
					} else if (measured && roomLeft > roomRight + 0.5f) {
						g_state.side = -1.0f;
						how = "more room";
					} else {
						g_state.side = (std::chrono::steady_clock::now().time_since_epoch().count() & 1) ? 1.0f : -1.0f;
					}
					if (measured) {
						REX::INFO("Side: {} of the line from you to them ({}; room left {:.1f} m, right {:.1f} m)", g_state.side > 0.0f ? "right" : "left", how, roomLeft, roomRight);
					} else {
						REX::INFO("Side: {} of the line from you to them (coin toss; room not measured)", g_state.side > 0.0f ? "right" : "left");
					}
				}
			}
			// Where each stands, steadied: the camera's angles are measured off the line between
			// these, and at 10 m a centimetre of the character controller settling on the
			// ground turned into visible horizontal jitter on wide shots (0.6.2, Dirk). Moves
			// under 3 cm are ignored and the rest eased over about a third of a second; a step of
			// a metre or more (a teleport) snaps.
			RE::NiPoint3 npcFeet, playerFeet;
			{
				const float kFeet = 1.0f - std::exp(-(std::clamp(delta, 0.0f, 0.1f)) / 0.3f);
				auto        steady = [kFeet](std::optional<RE::NiPoint3>& a_held, const RE::NiPoint3& a_raw) {
                    if (!a_held) {
                        a_held = a_raw;
                        return a_raw;
                    }
                    const float dx = a_raw.x - a_held->x, dy = a_raw.y - a_held->y, dz = a_raw.z - a_held->z;
                    const float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
                    if (dist > 1.0f) {
                        a_held = a_raw;
                    } else if (dist > 0.03f) {
                        const float f = (dist - 0.03f) / dist * kFeet;
                        a_held->x += dx * f;
                        a_held->y += dy * f;
                        a_held->z += dz * f;
                    }
                    return *a_held;
				};
				npcFeet = steady(g_state.smoothNpcFeet, speaker->GetPosition());
				playerFeet = steady(g_state.smoothPlayerFeet, player->GetPosition());
			}
			// Which way each face points, for close-ups that follow the face.
			const char* speakerName = speaker->GetDisplayFullName();
			const bool  npcFaceValid = s.followFace && Camera::TrackFacing(speaker, *playerHead, g_state.npcFace, delta, speakerName ? speakerName : "speaker");
			const bool  playerFaceValid = s.followFace && Camera::TrackFacing(player, *npcHead, g_state.playerFace, delta, "you");
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
			// From first person, until your full body is ready (about 0.15 s), your own view,
			// with you hidden (0.8.14 test, the Ranger at Neon: the first shot began with you
			// missing and you popped in a moment later). At most 0.6 s.
			const bool bodyPending = g_state.startedFirstPerson && !g_state.bodyReady && now - g_state.started < 600ms;
			if (gameplayView && (bodyPending || (startMode == 0 && wideFrame) || (startMode == 2 && startRoot))) {
				waitingView = *gameplayView;
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
				frame.intenseExtreme = g_state.lineExtreme;
				frame.npcLineSerial = g_lineSerial.load(std::memory_order_acquire);
				frame.reaction = g_state.reactionActive && !frame.npcTurn;
				frame.persuasionBeat = g_state.lineBeat || g_state.replyBeat;
				frame.playerCloseUp = g_state.reactionActive && g_state.reactionCloseUp;
				frame.roomAngle = g_state.roomAngle;
				frame.roomDistance = g_state.roomDistance;
				frame.roomRise = g_state.roomRise;
				frame.npcHead = *npcHead;
				frame.playerHead = *playerHead;
				frame.side = g_state.side;
				frame.npcFaceValid = npcFaceValid;
				frame.playerFaceValid = playerFaceValid;
				frame.npcFacing = g_state.npcFace.facing;
				frame.playerFacing = g_state.playerFace.facing;
				frame.npcGround = npcFeet.z;
				frame.playerGround = playerFeet.z;
				frame.feetValid = true;
				frame.npcFeet = npcFeet;
				frame.playerFeet = playerFeet;
				frame.npcSwayHead = npcSwayHead;
				frame.playerSwayHead = playerSwayHead;
				directed = Director::UpdatePlain(frame, now);
			}
			if (s.director && !waitingView && !pov && !g_state.plainShots) {
				Director::Frame frame;
				frame.npcTurn = subject == Subject::kNpc;
				frame.npcTalking = speaker->IsTalking();
				frame.playerTalking = player->IsTalking();
				frame.nearSeated = nearSeated;
				frame.intenseLine = g_state.lineIntense;
				frame.intenseExtreme = g_state.lineExtreme;
				frame.npcLineSerial = g_lineSerial.load(std::memory_order_acquire);
				frame.reaction = g_state.reactionActive && !frame.npcTurn;
				frame.persuasionBeat = g_state.lineBeat || g_state.replyBeat;
				frame.playerCloseUp = g_state.reactionActive && g_state.reactionCloseUp;
				frame.roomAngle = g_state.roomAngle;
				frame.roomDistance = g_state.roomDistance;
				frame.roomRise = g_state.roomRise;
				frame.npcHead = *npcHead;
				frame.playerHead = *playerHead;
				frame.side = g_state.side;
				frame.npcFaceValid = npcFaceValid;
				frame.playerFaceValid = playerFaceValid;
				frame.npcFacing = g_state.npcFace.facing;
				frame.playerFacing = g_state.playerFace.facing;
				frame.npcGround = npcFeet.z;
				frame.playerGround = playerFeet.z;
				frame.feetValid = true;
				frame.npcFeet = npcFeet;
				frame.playerFeet = playerFeet;
				frame.npcSwayHead = npcSwayHead;
				frame.playerSwayHead = playerSwayHead;
				directed = Director::Update(frame, now);
				LensNow();  // a cut's lens on the cut's own frame (0.8.12: each angle has its own)
			}
			// The plain shots stage on the same side of the line as the shot library: over your
			// right shoulder on the right side; your own shots over their matching shoulder, or
			// with bTrue180 over the opposite one.
			const float plainSign = subject == Subject::kNpc ? g_state.side : (s.true180 ? -g_state.side : g_state.side);
			auto        pose = waitingView ? waitingView :
			                   pov ? Camera::Shot(nearHeadRef, farHeadRef, -0.30f, 0.0f, 0.0f) :
			                   directed ? directed :
			                   nearSeated || s.shotStyle == 1 ?
			                   Camera::Shot(nearHeadRef, farHeadRef, -s.seatedForward, s.seatedSide * plainSign, s.seatedHeight) :
			                   g_state.plainShots && g_state.merchant && subject == Subject::kPlayer ?
			                   Camera::Shot(nearHeadRef, farHeadRef, -s.seatedForward, s.vendorSide * plainSign, s.vendorHeight) :
			                   Camera::OverShoulder(nearHeadRef, farHeadRef, plainSign);
			// Obstacles on the plain shot (seated people, merchants, or nothing in the preset
			// fits): checked when the camera turns to the other person. A wall behind the
			// camera pulls it in; anything closer to the person in shot (a ladder in front of
			// their face) swaps to the eye-line shot from just in front of the other one's eyes.
			const bool plainPath = pose && !waitingView && !pov && !directed;
			if (plainPath && s.obstacleCheck) {
				const auto& subjectHead = subject == Subject::kNpc ? *npcHead : *playerHead;
				const bool  plainOverShoulder = !(nearSeated || s.shotStyle == 1 || (g_state.plainShots && g_state.merchant && subject == Subject::kPlayer));
				const int   key = static_cast<int>(subject);
				if (key != g_state.plainCheckSubject) {
					g_state.plainCheckSubject = key;
					const auto c = TestShot(subjectHead, pose->position, nearHeadRef, plainOverShoulder);
					g_state.plainLimit = c.usable || c.fallbackOk ? c.limit : std::numeric_limits<float>::infinity();
					g_state.plainEyeLine = !c.usable && !c.fallbackOk;
					if (c.blockedAt >= 0.0f) {
						REX::INFO("Obstacles: plain shot of {} blocked {:.1f} m from them{}; {}", subject == Subject::kNpc ? "them" : "you", c.blockedAt,
							c.pastNear ? " (beyond the near person)" : "",
							c.usable || c.fallbackOk ? std::format("camera pulled in to {:.1f} m", c.limit) : std::string{ "shooting from the other person's eyes instead" });
					}
				}
				if (g_state.plainEyeLine) {
					pose = Camera::Shot(nearHeadRef, farHeadRef, -0.30f, 0.0f, 0.0f);
				} else {
					Camera::PullIn(*pose, subjectHead, g_state.plainLimit);
				}
			} else if (!plainPath) {
				g_state.plainCheckSubject = -1;
			}
			if (s.debugLog) {
				// Head-motion measurement: one line per shot (the shot name changes, or the
				// camera turns to the other person).
				const char* shotName = directed ? Director::Current() : nullptr;
				const int   who = static_cast<int>(subject);
				auto&       m = g_state.head;
				if (m.frames > 0 && (shotName != m.shot || who != m.subject)) {
					LogHeadMotion("shot changed");
				}
				g_state.head.shot = shotName;
				g_state.head.subject = who;
			}
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
				const float distance = waitingView || pov ? 0.0f : std::sqrt(dx * dx + dy * dy + dz * dz);
				g_shotDistance.store(distance, std::memory_order_relaxed);
				FocusOnShot(distance);  // on this very frame (0.8.10)
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
			// On-screen message for each shot as it's used (bShowMessages, a testing aid): the
			// setup's name as written in [Shots], and why. Paired with the line it cut for.
			if (s.showMessages && !waitingView && !pov) {
				const char*         name = directed ? Director::Current() : g_state.plainEyeLine ? "eye-line shot" : "plain over-the-shoulder shot";
				const int           who = static_cast<int>(subject);
				const std::uint32_t cuts = directed ? Director::Cuts() : 0;
				if (name != g_state.msgShot || who != g_state.msgSubject || cuts != g_state.msgCuts) {
					g_state.msgShot = name;
					g_state.msgSubject = who;
					g_state.msgCuts = cuts;
					std::string why;
					if (g_state.reactionActive && subject == Subject::kPlayer) {
						// With your face's expression, so a blank one is plain to see (0.7.12 test).
						const auto face = Face::Shown();
						why = std::format("{}, {}", g_state.reactionCloseUp ? "reaction: your expression" : "reaction: you listening",
							face.empty() ? std::string{ "no expression" } : face);
					} else if (directed) {
						why = Director::Why();
					} else if (g_state.plainEyeLine) {
						why = "plain shot blocked";
					} else {
						why = nearSeated ? "seated" : g_state.plainShots && g_state.merchant ? "merchant" : g_state.plainShots ? "plain shots" : "nothing in the preset fits";
					}
					if (directed && Director::PulledIn()) {
						why += ", pulled in";
					}
					QueueHudMessage(std::format("Camera: {} ({})", name, why));
				}
			}
			// Measure the room around the two (0.8: where the room shots can stand): once, and
			// again when they've moved more than 1.5 m (at most every 2 s) or the conversation's
			// side of the line changes.
			if (s.obstacleCheck && s.director && !waitingView && !pov) {
				const RE::NiPoint3 mid{ (npcHead->x + playerHead->x) * 0.5f, (npcHead->y + playerHead->y) * 0.5f, (npcHead->z + playerHead->z) * 0.5f };
				const auto         now = std::chrono::steady_clock::now();
				const bool again = g_remeasureRoom.exchange(false);  // the preset changed: its room shot limits too
				if (again || !g_state.roomMeasured || g_state.roomSide != g_state.side || (Distance(mid, g_state.roomAt) > 1.5f && now - g_state.roomTime > 2s)) {
					g_state.roomSide = g_state.side;
					g_state.roomMeasured = true;
					g_state.roomAt = mid;
					g_state.roomTime = now;
					float ground = mid.z - 1.6f;
					if (g_state.smoothNpcFeet && g_state.smoothPlayerFeet) {
						ground = (std::min)(g_state.smoothNpcFeet->z, g_state.smoothPlayerFeet->z);
					}
					MeasureRoom(*npcHead, *playerHead, ground, g_state.side, s.debugLog || s.showMessages);
				}
			}
			// Obstacle check of the shot on screen, for the log: once per shot.
			if (s.debugLog && !waitingView && !pov) {
				const char* shotName = directed ? Director::Current() : g_state.plainEyeLine ? "eye-line shot" : "plain shot";
				const int   who = static_cast<int>(subject);
				if (shotName != g_state.castShot || who != g_state.castSubject) {
					g_state.castShot = shotName;
					g_state.castSubject = who;
					const auto& subjectHead = subject == Subject::kNpc ? *npcHead : *playerHead;
					const auto  type = directed ? Shots::FromKey(shotName) : std::optional<Shots::ShotType>{};
					const bool  overShoulder = type ? Shots::OverShoulder(*type) : !g_state.plainEyeLine && !nearSeated && s.shotStyle != 1;
					LogShotObstacles(shotName, subjectHead, pose->position, nearHeadRef, overShoulder, subject == Subject::kNpc ? "them" : "you");
				}
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
				if (FilmingActive()) {
					FilmFrame();  // people talking near you, filmed (0.8)
					return;
				}
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

		// ---- The conversation lens (0.8) ------------------------------------------------------
		// Starfield draws conversations through a 30 degree lens (fDialogueCameraFailsafeFPFOV),
		// put in place every frame by the camera's field-of-view update (Starfield.exe 1.16.244
		// +BCB990, Address Library 64682), which hands it to the player camera's set-FOV
		// function (+1E92650, ID 113443; writes PlayerCamera +280 and the render frustum). 0.3.x
		// wrote that value and the frustum directly, and the next frame's update put 30 back.
		// Here the value is changed on its way into the set-FOV call, at all three of its
		// callers, while this mod films a conversation (fConversationLens; 0 = the game's).
		std::atomic<bool>  g_lensActive{ false };  // set on the main thread
		std::atomic<float> g_lensWanted{ 0.0f };
		std::atomic<int>   g_lensCalls{ 0 };

		// The lens wanted, set from the camera thread right after the director's choice (the
		// main thread keeps it current too).
		void LensNow()
		{
			const float lens = Director::WantedLens();
			if (lens >= 10.0f) {
				g_lensWanted.store(lens, std::memory_order_relaxed);
			}
		}

		float LensFor(float a_fov)
		{
			const float wanted = g_lensWanted.load(std::memory_order_relaxed);
			if (!g_lensActive.load(std::memory_order_relaxed) || wanted < 10.0f) {
				return a_fov;
			}
			if (g_lensCalls.fetch_add(1, std::memory_order_relaxed) == 0) {
				REX::INFO("Conversation lens: the game asked for {:.1f} degrees; {:.1f} instead", a_fov, wanted);
			}
			return wanted;
		}

		struct LensHookMain
		{
			static void Thunk(void* a_camera, float a_fov) { Hook(a_camera, LensFor(a_fov)); }
			static inline REL::THook<void(void*, float)> Hook{ "CCC_LensMain", REL::EHookStep::None, REL::ID(64682), 0xF43, Thunk };
		};
		struct LensHook2
		{
			static void Thunk(void* a_camera, float a_fov) { Hook(a_camera, LensFor(a_fov)); }
			static inline REL::THook<void(void*, float)> Hook{ "CCC_Lens2", REL::EHookStep::None, REL::ID(113316), 0x1FD, Thunk };
		};
		struct LensHook3
		{
			static void Thunk(void* a_camera, float a_fov) { Hook(a_camera, LensFor(a_fov)); }
			static inline REL::THook<void(void*, float)> Hook{ "CCC_Lens3", REL::EHookStep::None, REL::ID(113488), 0x498, Thunk };
		};

		struct ThirdPersonUpdateHook
		{
			static void Thunk(void* a_state, void* a_arg)
			{
				Hook(a_state, a_arg);
				if (FilmingActive()) {
					FilmFrame();  // people talking near you, filmed (0.8)
					return;
				}
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
					// 0.8.4 test: asking again at 0.7 s, with the first answer arriving at 0.83 s,
					// left your head missing in a conversation with Vasco; the game usually answers
					// within about a second.
					if (waited < 1200) {
						return;
					}
					// 0.6.2 logs: the game sometimes takes 1.5-3 s to act on the request (the new
					// model arrived after we had given up, so a conversation with Violet showed no
					// player, and Dirk's and a merchant's fell back to the camera cycle, which
					// flickers). Ask again, keep the camera off the player meanwhile (their shots
					// of the NPC from the player's eyes), and wait longer before the fallback.
					const int dueRetries = waited < 1500 ? 1 : waited < 2500 ? 2 : 3;
					if (g_state.directRetries < dueRetries && waited < 4000) {
						++g_state.directRetries;
						g_state.povNpcShot = true;
						REX::INFO("Body rebuild (direct): no full body yet after {} ms (head {:.2f} m above feet); asking again ({})", waited, PlayerHeadAboveFeet(),
							g_state.directRetries);
						player->Set3DUpdateFlag(static_cast<std::uint32_t>(Settings::Get().bodyUpdateFlags));
						return;
					}
					if (waited < (g_state.backgroundScene ? 4000 : 2000)) {
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

		// ---- Focus on the person in shot (0.5.0) --------------------------------------------
		// The game's conversation blur is one of its Dialogue_Close/Medium/Long_DOF image-space
		// modifiers, and it reads their settings live while it runs (0.4.22 tests). In 1.16.244:
		// +0x38C blur strength (2 / 2 / 0.7), +0x398 focus distance (1.5 / 3 / 8 m), +0x3A4
		// sharp-zone depth (6 / 12 / 20). The game focuses as if the camera were at your eyes;
		// setting the focus distance to the distance from our camera to the person in shot puts
		// the focus on them.
		struct BlurRecord
		{
			std::uintptr_t form{ 0 };
			float          strength{ 0.0f }, distance{ 0.0f }, range{ 0.0f };
		};
		std::array<BlurRecord, 3> g_blurRecords{};
		bool                      g_blurRecordsRead = false;
		std::atomic<bool>         g_blurRecordsChanged{ false };
		std::atomic<bool>         g_blurReady{ false };  // read and usable (main thread sets it)
		std::atomic<float>        g_blurWritten{ -1.0f };  // the shot distance last written (-1: the game's own)
		constexpr std::size_t     kBlurStrength = 0x38C, kBlurDistance = 0x398, kBlurRange = 0x3A4;
		constexpr float           kNoBlurBeyond = 8.0f;  // metres from the camera to the person in shot

		bool ReadBlurRecords()
		{
			if (g_blurRecordsRead) {
				return g_blurRecords[0].form != 0;
			}
			g_blurRecordsRead = true;
			const std::array<std::uint32_t, 3> ids{ 0x10A27, 0x10A4C, 0x10E34 };
			for (std::size_t i = 0; i < ids.size(); ++i) {
				const auto    form = reinterpret_cast<std::uintptr_t>(RE::TESForm::LookupByID(ids[i]));
				std::uint32_t type = 0, id = 0;
				const char*   editorId = nullptr;
				BlurRecord    r{};
				if (!form || !ReadFormInfo(form, type, id, editorId) || type != std::to_underlying(RE::FormType::kIMAD) ||
					!SafeCopy(&r.strength, reinterpret_cast<const void*>(form + kBlurStrength), 4) ||
					!SafeCopy(&r.distance, reinterpret_cast<const void*>(form + kBlurDistance), 4) ||
					!SafeCopy(&r.range, reinterpret_cast<const void*>(form + kBlurRange), 4)) {
					REX::WARN("Focus: conversation blur record {:08X} not readable; focus on the person in shot is off", ids[i]);
					g_blurRecords = {};
					return false;
				}
				// Only if the values look like blur settings (another game version, or a mod that
				// changes these records' layout, would put them elsewhere).
				if (!(r.strength >= 0.0f && r.strength < 20.0f && r.distance > 0.1f && r.distance < 100.0f && r.range > 0.1f && r.range < 1000.0f)) {
					REX::WARN("Focus: {} holds ({}, {}, {}), which don't look like blur settings; focus on the person in shot is off", editorId ? editorId : "",
						r.strength, r.distance, r.range);
					g_blurRecords = {};
					return false;
				}
				r.form = form;
				g_blurRecords[i] = r;
				REX::INFO("Focus: conversation blur {:08X} '{}': strength {:.2f}, focus {:.2f} m, sharp zone {:.2f}", id, editorId ? editorId : "", r.strength,
					r.distance, r.range);
			}
			return true;
		}

		void WriteBlurRecords(float a_shotDistance, bool a_deepLong, float a_beyond)
		{
			for (const auto& r : g_blurRecords) {
				if (!r.form) {
					continue;
				}
				const float distance = a_shotDistance > 0.3f ? a_shotDistance : r.distance;
				const float range = a_deepLong && a_shotDistance > a_beyond ? (std::max)(r.range * 4.0f, 50.0f) : r.range;
				// No blur at all on the far (room) shots: past about 8 m the game's blur kept
				// things near the camera sharp and the people soft (0.8.0 test, the Lodge).
				// 0.8.10: none either before the first shot (no distance yet): the game's own
				// focus, 1.5 m, blurred the opening frame of a shot 4 m away.
				const float strength = a_shotDistance > kNoBlurBeyond || a_shotDistance <= 0.3f ? 0.0f : r.strength;
				SafeWrite(reinterpret_cast<void*>(r.form + kBlurStrength), &strength, 4);
				SafeWrite(reinterpret_cast<void*>(r.form + kBlurDistance), &distance, 4);
				SafeWrite(reinterpret_cast<void*>(r.form + kBlurRange), &range, 4);
			}
			g_blurRecordsChanged = true;
		}

		void PutBackBlurRecords()
		{
			for (const auto& r : g_blurRecords) {
				if (r.form) {
					SafeWrite(reinterpret_cast<void*>(r.form + kBlurStrength), &r.strength, 4);
					SafeWrite(reinterpret_cast<void*>(r.form + kBlurDistance), &r.distance, 4);
					SafeWrite(reinterpret_cast<void*>(r.form + kBlurRange), &r.range, 4);
				}
			}
			g_blurRecordsChanged = false;
			g_blurWritten.store(-1.0f, std::memory_order_relaxed);
		}

		// 0.8.10: the focus follows a cut on the same frame. It was set from the main thread, a
		// frame or two after the camera moved, so the first frame of a shot could be focused for
		// the last one (a blurred flash, most visible at the start of a conversation).
		void FocusOnShot(float a_distance)
		{
			const auto& st = Settings::Get();
			if (!st.focusOnSubject || !g_blurReady.load(std::memory_order_acquire) || g_state.handedBack || g_state.smallSpeaker) {
				return;
			}
			if (std::abs(a_distance - g_blurWritten.load(std::memory_order_relaxed)) < 0.05f) {
				return;
			}
			g_blurWritten.store(a_distance, std::memory_order_relaxed);
			WriteBlurRecords(a_distance, st.deepFocusLongShots, st.deepFocusBeyond);
		}

		// The line's facial expression ("emotion"), as the game's animators tagged it:
		// speaker's voice data +0x440 -> +0x10 = the dialogue record (INFO); its responses
		// list at +0x70; the first response holds the expression keyword at +0x18, e.g.
		// AnimFaceArchetypeFriendly. Found in 0.4.8-0.4.11. Every read is guarded and every
		// pointer confirmed as a real game record before it's used.
		// The dialogue record (INFO) the actor's voice data points at, or 0.
		std::uintptr_t LineInfo(RE::Actor* a_speaker)
		{
			if (!a_speaker || !a_speaker->currentProcess || !a_speaker->currentProcess->high) {
				return 0;
			}
			const auto     base = reinterpret_cast<std::uintptr_t>(a_speaker->currentProcess->high);
			std::uintptr_t holder = 0, info = 0;
			if (!SafeCopy(&holder, reinterpret_cast<const void*>(base + 0x440), sizeof(holder)) || holder < 0x10000 || holder > 0x7FFFFFFFFFFF ||
				!SafeCopy(&info, reinterpret_cast<const void*>(holder + 0x10), sizeof(info))) {
				return 0;
			}
			std::uint32_t type = 0, id = 0;
			const char*   editorId = nullptr;
			return ReadFormInfo(info, type, id, editorId) && type == std::to_underlying(RE::FormType::kINFO) ? info : 0;
		}

		// The expression tag on a dialogue record's first response ("none" if it has none).
		std::string InfoEmotion(std::uintptr_t a_info)
		{
			std::uintptr_t response = 0, keyword = 0;
			auto           ok = [](std::uintptr_t a_p) { return a_p > 0x10000 && a_p < 0x7FFFFFFFFFFF; };
			if (!SafeCopy(&response, reinterpret_cast<const void*>(a_info + 0x70), sizeof(response)) || !ok(response) ||
				!SafeCopy(&keyword, reinterpret_cast<const void*>(response + 0x18), sizeof(keyword)) || !ok(keyword)) {
				return "none";
			}
			std::uint32_t type = 0, id = 0;
			const char*   editorId = nullptr;
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

		// Diagnostic (0.7.3): where the game keeps the line you just chose. Your own voice data
		// never shows it (0.7.2), so look through the dialogue menu's topic manager for
		// dialogue records, directly or one pointer away, and log each with its tag.
		void OnOptionChosen(const std::string& a_text)
		{
			// Your face for an option with a skill tag (0.8).
			if (Settings::Get().playerFace && g_state.active && !g_state.handedBack) {
				auto        expression = Face::ForChoice(a_text);
				std::string reason;
				if (!expression.empty()) {
					const auto close = a_text.find(']');
					reason = close != std::string::npos ? a_text.substr(0, close + 1) : std::string{ "a question" };
				} else if (expression = Face::MaybeSignature(); !expression.empty()) {
					// An option without a skill tag: now and then you say it in character.
					std::string background;
					(void)Face::Signature(&background);
					reason = background;
				}
				if (!expression.empty()) {
					Face::Choose(expression, std::format("you chose '{}'", a_text.substr(0, 40)));
					if (Settings::Get().showMessages && Face::Current() == expression) {
						ShowHudMessage(std::format("You: {} ({})", expression, reason));
					}
				}
			}
			if (!Settings::Get().debugLog) {
				return;
			}
			static int calls = 0;
			if (++calls > 12) {
				return;
			}
			constexpr std::uint64_t kTopicManagerSingleton = 938421;
			if (!Hud::AddressKnown(kTopicManagerSingleton)) {
				REX::INFO("Your choice: topic manager not in the Address Library");
				return;
			}
			std::uintptr_t manager = 0;
			const auto     where = REL::ID(kTopicManagerSingleton).address();
			if (!SafeCopy(&manager, reinterpret_cast<const void*>(where), sizeof(manager)) || manager < 0x10000 || manager > 0x7FFFFFFFFFFF) {
				REX::INFO("Your choice: topic manager not there");
				return;
			}
			auto infoAt = [](std::uintptr_t a_p, std::uint32_t& a_id) {
				std::uint32_t type = 0;
				const char*   editorId = nullptr;
				return a_p > 0x10000 && a_p < 0x7FFFFFFFFFFF && ReadFormInfo(a_p, type, a_id, editorId) && type == std::to_underlying(RE::FormType::kINFO);
			};
			std::array<std::uintptr_t, 128> words{};
			if (!SafeCopy(words.data(), reinterpret_cast<const void*>(manager), sizeof(words))) {
				return;
			}
			std::string found;
			for (std::size_t i = 0; i < words.size(); ++i) {
				std::uint32_t id = 0;
				if (infoAt(words[i], id)) {
					found += std::format("; +{:X} INFO {:08X} {}", i * 8, id, InfoEmotion(words[i]));
					continue;
				}
				if (words[i] > 0x10000 && words[i] < 0x7FFFFFFFFFFF) {
					std::array<std::uintptr_t, 8> inner{};
					if (SafeCopy(inner.data(), reinterpret_cast<const void*>(words[i]), sizeof(inner))) {
						for (std::size_t k = 0; k < inner.size(); ++k) {
							if (infoAt(inner[k], id)) {
								found += std::format("; +{:X}->+{:X} INFO {:08X} {}", i * 8, k * 8, id, InfoEmotion(inner[k]));
							}
						}
					}
				}
			}
			REX::INFO("Your choice '{}': {}", a_text.substr(0, 60), found.empty() ? std::string{ "no dialogue records in the topic manager" } : found.substr(2));
		}

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

		// On-screen messages (bShowMessages, a testing aid): each line's expression tag and each
		// shot the camera uses, as one message when a line and the cut it brings come together,
		// e.g. "Sarah Morgan: Surprised (#3) | Camera: CloseUp (emotion close-up)".
		using MsgClock = std::chrono::steady_clock;

		struct HeldShot
		{
			std::string         text;
			MsgClock::time_point at{};
		};

		struct HeldLine
		{
			std::string         text;
			bool                 closeUp = false;
			bool                 waiting = false;
			MsgClock::time_point at{};
		};

		// How long a shot waits for a line to pair with (the camera may cut a frame or two
		// before the line is noticed), and how long a line waits for its cut.
		constexpr auto kShotWait = 350ms;
		constexpr auto kLineWait = 500ms;
		constexpr auto kCutBeforeLine = 300ms;

		std::mutex           g_hudQueueLock;  // worker threads -> main thread
		std::deque<HeldShot> g_hudQueue;
		std::deque<HeldShot> g_heldShots;  // main thread only
		HeldLine             g_heldLine;   // main thread only

		void ShowHudMessage(const std::string& a_text)
		{
			// During a filmed conversation, in the top bar (0.7.6: the game's notifications go
			// by too fast); otherwise the game's own notification.
			if (Hud::ShowMessage(a_text)) {
				return;
			}
			if (auto* source = RE::ShowHUDMessageEvent::GetEventSource()) {
				RE::ShowHUDMessageEvent event{ RE::BSFixedString{ a_text.c_str() }, RE::BSFixedString{ "" }, false, false };
				source->Notify(event);
			}
		}

		// A shot message from the camera's worker threads, shown from the main thread.
		void QueueHudMessage(std::string a_text)
		{
			std::scoped_lock lock{ g_hudQueueLock };
			if (g_hudQueue.size() < 6) {
				g_hudQueue.push_back({ std::move(a_text), MsgClock::now() });
			}
		}

		// The line's own part: a close-up emotion the camera didn't cut for says so.
		std::string LineText(const HeldLine& a_line)
		{
			if (!a_line.closeUp) {
				return a_line.text;
			}
			MsgClock::time_point cutAt{};
			const int            kind = Director::LastEmotionCut(cutAt);
			return kind != 0 && cutAt >= a_line.at - kCutBeforeLine ? a_line.text : a_line.text + " - no close-up";
		}

		// Main thread, every tick.
		void FlushHudMessages(MsgClock::time_point a_now)
		{
			{
				std::scoped_lock lock{ g_hudQueueLock };
				while (!g_hudQueue.empty()) {
					g_heldShots.push_back(std::move(g_hudQueue.front()));
					g_hudQueue.pop_front();
				}
			}
			auto& line = g_heldLine;
			if (line.waiting) {
				// Shots from just before the line can be its cut; take the latest of them, once
				// one arrives after the line starts or the wait is over.
				const bool cutSince = std::ranges::any_of(g_heldShots, [&](const HeldShot& a_shot) { return a_shot.at >= line.at; });
				if (cutSince || a_now >= line.at + kLineWait) {
					auto paired = g_heldShots.end();
					for (auto it = g_heldShots.begin(); it != g_heldShots.end(); ++it) {
						if (it->at >= line.at - kCutBeforeLine) {
							paired = it;
						}
					}
					while (!g_heldShots.empty() && g_heldShots.begin() != paired) {
						ShowHudMessage(g_heldShots.front().text);
						g_heldShots.pop_front();
					}
					if (paired != g_heldShots.end()) {
						ShowHudMessage(std::format("{} | {}", LineText(line), g_heldShots.front().text));
						g_heldShots.pop_front();
					} else {
						ShowHudMessage(std::format("{} | same shot", LineText(line)));
					}
					line.waiting = false;
				}
			}
			// Shots with no line to pair with.
			while (!g_heldShots.empty() && a_now >= g_heldShots.front().at + kShotWait &&
				   !(line.waiting && g_heldShots.front().at >= line.at - kCutBeforeLine)) {
				ShowHudMessage(g_heldShots.front().text);
				g_heldShots.pop_front();
			}
		}

		void LogLineRecord(RE::Actor* a_speaker)
		{
			if (!a_speaker || !a_speaker->currentProcess || !a_speaker->currentProcess->high) {
				return;
			}
			const char* raw = a_speaker->currentProcess->high->strVoiceSubtitle.c_str();
			const auto  emotion = LineEmotion(a_speaker);
			const int   mode = Director::EmotionCloseUps();
			const int   tier = mode != 0 ? CloseUpTier(emotion) : 0;
			const bool  closeUp = tier > 0 && (mode != 3 || tier == 3);
			REX::INFO("Line emotion: {} for '{}'{}", emotion.empty() ? "unknown" : emotion, std::string{ raw ? raw : "" }.substr(0, 60), closeUp ? " (close-up emotion)" : "");
			// Your face answers the line (0.8): mirrored or a listener's response; untagged
			// lines give your face back to the game.
			if (Settings::Get().playerFace && g_state.active && !g_state.handedBack) {
				const char* who = a_speaker->GetDisplayFullName();
				auto expression = Face::ListeningTo(emotion);
				auto why = std::format("listening to {}: {}", who ? who : "the speaker", emotion.empty() ? "no expression" : emotion);
				if (expression.empty()) {
					// A neutral moment: now and then your background shows (0.7.11).
					std::string background;
					if (expression = Face::MaybeSignature(); !expression.empty()) {
						(void)Face::Signature(&background);
						why += std::format("; your {} side", background);
					}
				}
				Face::Listen(expression, why);
			}
			g_playerFaceShown.store(Settings::Get().playerFace && !Face::Current().empty(), std::memory_order_release);
			g_faceLineAt.store(std::chrono::steady_clock::now().time_since_epoch().count(), std::memory_order_release);
			if (Settings::Get().showMessages) {
				static std::unordered_map<std::string, int> seen;
				const auto  key = emotion.empty() ? std::string{ "unknown" } : emotion;
				const int   count = ++seen[key];
				const char* name = a_speaker->GetDisplayFullName();
				if (g_heldLine.waiting) {  // a line cut short before its message went out
					ShowHudMessage(LineText(g_heldLine));
				}
				// Your face's answer to the line, when it has one (0.7.11).
				const auto& you = Face::Current();
				const auto  yours = Settings::Get().playerFace && !you.empty() ? std::format(" | You: {}", you) : std::string{};
				g_heldLine = { std::format("{}: {} (#{}){}", name ? name : "Speaker", key, count, yours), closeUp, true, MsgClock::now() };
			}
		}

		// NPC-to-NPC conversations (planned for 0.8, groundwork): who near you is talking to
		// whom, for the log (and on screen with bShowMessages). Nothing is filmed yet. Each
		// line an NPC starts within 20 m is noted with its words, expression, who the speaker
		// faces (the person they're addressing, if anyone within 5 m) and the group around
		// them: people within 5 m of one another who've spoken in the last 20 s, plus whoever
		// they addressed. Lines in your own conversation aren't counted.
		struct Talker
		{
			std::string                           name;
			RE::NiPoint3                          feet{};
			RE::NiPoint3                          head{};
			float                                 heading = 0.0f;
			bool                                  talking = false;
			bool                                  pending = false;  // a line started; waiting for its subtitle
			std::chrono::steady_clock::time_point lineStart{};
			std::chrono::steady_clock::time_point lastLine{};
			std::chrono::steady_clock::time_point lastHeard{};  // last seen talking (0.8.1)
			std::chrono::steady_clock::time_point prevLine{};   // the line before the last (0.8.6)
			std::chrono::steady_clock::time_point seen{};
			std::uint32_t                         addressed = 0;   // formID; 0x14 = you
			std::uint32_t                         lines = 0;
			std::uintptr_t                        lastInfo = 0;    // the dialogue record last seen on them
			std::string                           lastSubtitle;
			bool                                  primed = false;  // seen once (what they said before isn't a new line)
			const void*                           scene = nullptr;  // the scene they play a part in, if any
			const void*                           lineScene = nullptr;  // the scene their last line was part of (0.8.10)
			std::uintptr_t                        lineSaid = 0;  // their last reported line, by its words and record (0.8.10)
		};

		std::unordered_map<std::uint32_t, Talker> g_talkers;

		constexpr float kOverhearRange = 20.0f;  // from you
		constexpr float kGroupReach = 5.0f;      // between people in one conversation
		constexpr auto  kGroupMemory = 20s;      // how long a speaker stays part of a group

		float FlatDistance(const RE::NiPoint3& a_a, const RE::NiPoint3& a_b)
		{
			const float dx = a_a.x - a_b.x, dy = a_a.y - a_b.y;
			return std::sqrt(dx * dx + dy * dy);
		}

		// Of the people within kGroupReach, the one the speaker is facing most directly
		// (within 60 degrees of their heading); 0 if none.
		std::uint32_t AddressedBy(std::uint32_t a_speaker, const Talker& a_t, const RE::NiPoint3& a_playerFeet, const RE::NiPoint3& a_playerHead)
		{
			// In a scene, they're answering whoever in the same scene spoke last (0.7.11 test:
			// Andreja answering Matteo faced away from him, so came out "to no one").
			if (a_t.scene) {
				std::uint32_t partner = 0;
				auto          latest = std::chrono::steady_clock::time_point{};
				for (const auto& [id, other] : g_talkers) {
					if (id != a_speaker && other.scene == a_t.scene && other.lines > 0 && other.lastLine > latest && FlatDistance(other.feet, a_t.feet) < 15.0f) {
						latest = other.lastLine;
						partner = id;
					}
				}
				if (partner != 0) {
					return partner;
				}
				// 0.8.7 test: a scene's opening line (Nyssa to Sarah) came out "to you", Nyssa
				// facing you, and filming waited 6 s for Sarah's answer. Nobody in the scene has
				// spoken yet: the nearest other person in the same scene. (Not when you're part of
				// the scene yourself: then a line facing you is yours.)
				auto*       player = RE::PlayerCharacter::GetSingleton();
				const bool  youInScene = player && CurrentSceneUnsafe(player) == a_t.scene;
				float       nearest = 15.0f;
				for (const auto& [id, other] : g_talkers) {
					if (!youInScene && id != a_speaker && other.scene == a_t.scene) {
						if (const float d = FlatDistance(other.feet, a_t.feet); d < nearest) {
							nearest = d;
							partner = id;
						}
					}
				}
				if (partner != 0) {
					return partner;
				}
			}
			const float   fx = std::sin(a_t.heading), fy = std::cos(a_t.heading);
			std::uint32_t best = 0;
			float         bestCos = 0.5f;
			auto          consider = [&](std::uint32_t a_id, const RE::NiPoint3& a_feet, const RE::NiPoint3& a_head) {
                const float d = FlatDistance(a_t.feet, a_feet);
                if (d < 0.2f || d > kGroupReach) {
                    return;
                }
                const float dx = a_head.x - a_t.head.x, dy = a_head.y - a_t.head.y;
                const float len = std::sqrt(dx * dx + dy * dy);
                const float c = len > 0.01f ? (dx * fx + dy * fy) / len : 0.0f;
                if (c > bestCos) {
                    bestCos = c;
                    best = a_id;
                }
			};
			for (const auto& [id, other] : g_talkers) {
				if (id != a_speaker) {
					consider(id, other.feet, other.head);
				}
			}
			consider(0x14, a_playerFeet, a_playerHead);
			return best;
		}

		std::string TalkerName(std::uint32_t a_id)
		{
			if (a_id == 0x14) {
				return "you";
			}
			const auto it = g_talkers.find(a_id);
			return it != g_talkers.end() && !it->second.name.empty() ? it->second.name : std::format("{:08X}", a_id);
		}

		// The speaker's group: people within kGroupReach of a member who've spoken lately,
		// and whoever a member addressed.
		std::vector<std::uint32_t> GroupOf(std::uint32_t a_speaker, std::chrono::steady_clock::time_point a_now)
		{
			std::vector<std::uint32_t> group{ a_speaker };
			for (std::size_t i = 0; i < group.size(); ++i) {
				const auto it = g_talkers.find(group[i]);
				if (it == g_talkers.end()) {
					continue;
				}
				const auto& member = it->second;
				if (member.addressed != 0 && member.addressed != 0x14 && std::ranges::find(group, member.addressed) == group.end()) {
					group.push_back(member.addressed);
				}
				for (const auto& [id, other] : g_talkers) {
					if (std::ranges::find(group, id) != group.end() || other.lines == 0 || a_now - other.lastLine > kGroupMemory) {
						continue;
					}
					if (FlatDistance(member.feet, other.feet) <= kGroupReach) {
						group.push_back(id);
					}
				}
			}
			return group;
		}

		void ReportOverheardLine(RE::Actor* a_actor, Talker& a_t, std::chrono::steady_clock::time_point a_now, const RE::NiPoint3& a_playerFeet)
		{
			const auto& s = Settings::Get();
			const char* raw = a_actor->currentProcess && a_actor->currentProcess->high ? a_actor->currentProcess->high->strVoiceSubtitle.c_str() : nullptr;
			const std::string text{ raw ? raw : "" };
			const auto        info = LineInfo(a_actor);
			std::uint32_t     infoType = 0, infoId = 0;
			const char*       infoEditorId = nullptr;
			if (info) {
				ReadFormInfo(info, infoType, infoId, infoEditorId);
			}
			const auto  emotion = info ? InfoEmotion(info) : std::string{ "unknown" };
			const auto  group = GroupOf(a_actor->formID, a_now);
			bool        inScene = false;
			const auto  scene = SceneLabel(a_actor, inScene);
			const bool  randomScene = a_actor->boolFlags.any(RE::Actor::BOOL_FLAGS::kInRandomScene);
			const float distance = FlatDistance(a_t.feet, a_playerFeet);
			std::string members;
			for (const auto id : group) {
				members += (members.empty() ? "" : ", ") + TalkerName(id);
			}
			const std::string to = a_t.addressed ? TalkerName(a_t.addressed) : std::string{ "no one in particular" };
			if (s.debugLog) {
				REX::INFO("Overheard: '{}' ({:08X}) to {}, {:.1f} m from you: {} '{}' (INFO {:08X}); group of {}: {}; {}{}", a_t.name, a_actor->formID, to, distance, emotion,
					text.substr(0, 80), infoId, group.size(), members, scene, randomScene ? ", random scene" : "");
			}
			if (s.showMessages && a_t.addressed != 0 && a_t.addressed != 0x14) {  // NPC to NPC only
				// 0.7.7 test: Andreja's answers to Matteo didn't seem to appear; note each one sent.
				if (s.debugLog) {
					REX::INFO("Overheard: message shown for '{}' to {}", a_t.name, to);
				}
				ShowHudMessage(std::format("Overheard: {} -> {}: {} (group of {}, {:.0f} m)", a_t.name, to, emotion, group.size(), distance));
			}
		}

		// ---- Nearby actors ----------------------------------------------------------------------
		// The game's list of actors in high process. CommonLibSF puts it at ProcessLists +58,
		// where 1.16.244 has nothing (0.7.6 test: always empty, so neither bystanders nor
		// overheard lines ever saw anyone). Starfield.exe 1.16.244 itself reads it 8 bytes
		// further on: count at +60, the handles' pointer at +68 (25 loops over it, e.g. at
		// +BBF454 measuring each actor's distance; the other three lists follow at +70, +80
		// and +90). If it ever comes back empty, the actors in your cell are used instead.
		constexpr std::size_t kHighActors = 0x60;

		__declspec(noinline) RE::Actor* ResolveActorHandle(std::uint32_t a_handle)
		{
			RE::BSPointerHandle<RE::Actor> handle;
			static_assert(sizeof(handle) == sizeof(a_handle));
			std::memcpy(&handle, &a_handle, sizeof(a_handle));
			return handle.get().get();  // the list keeps it alive
		}

		bool SafeResolveActor(std::uint32_t a_handle, RE::Actor*& a_out)
		{
			__try {
				a_out = ResolveActorHandle(a_handle);
				return true;
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				a_out = nullptr;
				return false;
			}
		}

		// The actors behind an array of handles at a_base + a_offset (at most a_max of them).
		int ReadActorArray(std::uintptr_t a_base, std::size_t a_offset, std::size_t a_max, std::vector<RE::Actor*>* a_out)
		{
			std::uint32_t  header[2]{};
			std::uintptr_t data = 0;
			if (!SafeCopy(header, reinterpret_cast<const void*>(a_base + a_offset), sizeof(header)) ||
				!SafeCopy(&data, reinterpret_cast<const void*>(a_base + a_offset + 8), sizeof(data))) {
				return -1;
			}
			const std::uint32_t size = header[0], capacity = header[1];
			if (size == 0 || size > 4000 || capacity < size || capacity > 8000 || data < 0x10000 || data > 0x7FFFFFFFFFFF) {
				return -1;
			}
			std::vector<std::uint32_t> handles((std::min<std::size_t>)(size, a_max));
			if (!SafeCopy(handles.data(), reinterpret_cast<const void*>(data), handles.size() * sizeof(std::uint32_t))) {
				return -1;
			}
			int actors = 0;
			for (const auto h : handles) {
				RE::Actor* actor = nullptr;
				if (h == 0 || !SafeResolveActor(h, actor) || !actor) {
					continue;
				}
				std::uint32_t type = 0, id = 0;
				const char*   editorId = nullptr;
				if (ReadFormInfo(reinterpret_cast<std::uintptr_t>(actor), type, id, editorId) && type == std::to_underlying(RE::FormType::kACHR)) {
					++actors;
					if (a_out) {
						a_out->push_back(actor);
					}
				}
			}
			return actors;
		}

		// Who's around: the game's actor list, or else the actors in your cell.
		// Who's around: the game's high-process actor list (confirmed in 1.16.244). 0.7.9 also
		// read the other three lists and fell back to your cell's references: that crashed
		// while a save loaded (the cell's layout isn't confirmed for this version, and the
		// lists change under it). Now only the high list, never while the game is loading.
		// a_sizes: how many entries each of the four lists holds (sizes only, nothing read).
		bool GameLoading()
		{
			auto* ui = RE::UI::GetSingleton();
			static const RE::BSFixedString loading{ "LoadingMenu" };
			return !ui || ui->IsMenuOpen(loading);
		}

		std::vector<RE::NiPointer<RE::Actor>> NearbyActors(RE::PlayerCharacter* a_player, const char*& a_source, std::array<int, 4>* a_sizes = nullptr)
		{
			(void)a_player;
			std::vector<RE::NiPointer<RE::Actor>> out;
			a_source = "nothing";
			constexpr std::uint64_t kProcessListsSingleton = 937584;
			constexpr std::uint64_t kGetSmartPointer = 35638;  // turns a handle into the actor
			if (GameLoading() || !Hud::AddressKnown(kProcessListsSingleton) || !Hud::AddressKnown(kGetSmartPointer)) {
				a_source = "nothing (loading)";
				return out;
			}
			auto* lists = RE::ProcessLists::GetSingleton();
			if (!lists) {
				return out;
			}
			const auto base = reinterpret_cast<std::uintptr_t>(lists);
			if (a_sizes) {
				for (std::size_t i = 0; i < 4; ++i) {
					std::uint32_t size = 0;
					SafeCopy(&size, reinterpret_cast<const void*>(base + kHighActors + i * 0x10), sizeof(size));
					(*a_sizes)[i] = static_cast<int>(size);
				}
			}
			std::vector<RE::Actor*> actors;
			if (ReadActorArray(base, kHighActors, 2000, &actors) > 0) {
				for (auto* actor : actors) {
					out.emplace_back(actor);
				}
				a_source = "the game's actor list";
			}
			return out;
		}

		// An actor's name and whether it's talking, guarded: 0.7.8 crashed in the game's
		// display-name function on an actor from the lower-process lists, not fully set up yet.
		bool SafeActorBasics(RE::Actor* a_actor, const char*& a_name, bool& a_talking)
		{
			__try {
				a_name = a_actor->GetDisplayFullName();
				a_talking = a_actor->IsTalking();
				return true;
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				a_name = nullptr;
				a_talking = false;
				return false;
			}
		}

		void UpdateOverheard(std::chrono::steady_clock::time_point a_now)
		{
			const auto&             s = Settings::Get();
			auto*                   player = RE::PlayerCharacter::GetSingleton();
			if (!(s.debugLog || s.showMessages || s.filmConversations) || !player) {
				return;
			}
			// 0.7.5 test: no line was ever noticed. Say why, now and then.
			static auto nextSummary = a_now + 10s;
			const bool  summary = s.debugLog && a_now >= nextSummary;
			if (summary) {
				nextSummary = a_now + 10s;
			}
			const char*        source = nullptr;
			std::array<int, 4> counts{ -1, -1, -1, -1 };
			const auto         nearby = NearbyActors(player, source, &counts);
			std::string        around;  // who's within 20 m (for the summary)
			int         listed = 0, inRange = 0, talkingNow = 0, withLine = 0;
			std::string talkers;
			const auto playerFeet = player->GetPosition();
			const auto playerHeadAt = Camera::HeadPosition(player);
			const auto playerHead = playerHeadAt ? *playerHeadAt : RE::NiPoint3{ playerFeet.x, playerFeet.y, playerFeet.z + 1.6f };
			for (const auto& actor : nearby) {
				++listed;
				if (!actor || actor.get() == player || actor->IsDead()) {
					continue;
				}
				const auto feet = actor->GetPosition();
				if (FlatDistance(feet, playerFeet) > kOverhearRange || std::abs(feet.z - playerFeet.z) > 6.0f) {
					continue;
				}
				// Only actors with their full (high) process are asked anything beyond where they
				// stand: the others may not be fully set up (0.7.8 crash). They're still counted.
				const bool highProcess = actor->currentProcess && actor->currentProcess->high;
				const char* name = nullptr;
				bool        talkingNowFlag = false;
				if (!highProcess || !SafeActorBasics(actor.get(), name, talkingNowFlag)) {
					++inRange;
					if (summary && inRange <= 15) {
						around += std::format("{}{:08X} (not high process)", around.empty() ? "" : ", ", actor->formID);
					}
					continue;
				}
				++inRange;
				auto& t = g_talkers[actor->formID];
				if (t.name.empty()) {
					t.name = name && *name ? name : std::format("{:08X}", actor->formID);
				}
				if (summary && inRange <= 15) {
					around += std::format("{}{}", around.empty() ? "" : ", ", t.name);
				}
				const auto head = Camera::HeadPosition(actor.get());
				t.feet = feet;
				t.head = head ? *head : RE::NiPoint3{ feet.x, feet.y, feet.z + 1.6f };
				t.heading = actor->GetAngleZ();
				t.seen = a_now;
				// Everyone's scene, not only a speaker's (0.8.8): the first line of a scene can
				// then be put to the partner who hasn't spoken yet.
				t.scene = CurrentSceneUnsafe(actor.get());
				const bool talking = talkingNowFlag;
				const bool ours = g_state.active && actor->formID == g_state.speakerId;
				// A new line: they start talking, or a new dialogue record or subtitle turns up
				// on them (in case "talking" isn't set for lines not spoken to you).
				const auto  info = LineInfo(actor.get());
				const char* sub = actor->currentProcess && actor->currentProcess->high ? actor->currentProcess->high->strVoiceSubtitle.c_str() : nullptr;
				const std::string subtitle{ sub ? sub : "" };
				const bool        newInfo = t.primed && info != 0 && info != t.lastInfo;
				const bool        newSubtitle = t.primed && !subtitle.empty() && subtitle != t.lastSubtitle;
				if (talking) {
					t.lastHeard = a_now;
					++talkingNow;
					talkers += (talkers.empty() ? "" : ", ") + t.name + (ours ? " (yours)" : "");
				}
				if (info) {
					++withLine;
				}
				if (!ours && !t.pending && ((talking && !t.talking) || newInfo || newSubtitle)) {
					t.pending = true;
					t.lineStart = a_now;
					t.prevLine = t.lastLine;
					t.lastLine = a_now;
					++t.lines;
					if (s.debugLog) {
						REX::INFO("Overheard: '{}' starts a line ({}{}{})", t.name, talking && !t.talking ? "talking" : "", newInfo ? " new record" : "", newSubtitle ? " new subtitle" : "");
					}
				}
				t.talking = talking;
				t.lastInfo = info ? info : t.lastInfo;
				t.lastSubtitle = subtitle.empty() ? t.lastSubtitle : subtitle;
				t.primed = true;
				// Report once the subtitle is there (it often isn't when the line starts), or
				// after 0.6 s without one.
				if (t.pending) {
					const char* raw = actor->currentProcess && actor->currentProcess->high ? actor->currentProcess->high->strVoiceSubtitle.c_str() : nullptr;
					if ((raw && *raw) || a_now - t.lineStart > 600ms) {
						t.pending = false;
						t.scene = CurrentSceneUnsafe(actor.get());
						t.lineScene = t.scene;
						t.lineSaid = raw && *raw ? static_cast<std::uintptr_t>(std::hash<std::string_view>{}(std::string_view{ raw })) ^ t.lastInfo : 0;
						t.addressed = AddressedBy(actor->formID, t, playerFeet, playerHead);
						ReportOverheardLine(actor.get(), t, a_now, playerFeet);
					}
				}
			}
			std::erase_if(g_talkers, [&](const auto& a_entry) { return a_now - a_entry.second.seen > 30s; });
			if (summary) {
				REX::INFO("Overheard: {} actors in {} (the game's lists hold: high {}, low {}, middle-high {}, middle-low {}), {} within 20 m, {} with a dialogue record, {} talking now{}{}", listed, source,
					counts[0], counts[1], counts[2], counts[3], inRange, withLine, talkingNow, talkers.empty() ? "" : ": ", talkers);
				if (!around.empty()) {
					REX::INFO("Overheard: within 20 m: {}{}", around, inRange > 15 ? ", ..." : "");
				}
			}
		}

		// ---- Filming NPC-to-NPC conversations (0.8, first step) ------------------------------
		// When people near you talk to each other (the overheard lines), a prompt offers to
		// film them; the listen key (iListenKey) starts and stops it. The camera then films
		// whoever is speaking against whoever they're addressing, through the gameplay
		// camera's own update (no conversation menu), one shot per line, picked from a few of
		// the shot library's singles and the two-shot. Moving, the listen key, a conversation
		// of your own or 10 s of silence hand the camera back.
		struct Filming
		{
			std::mutex                            lock;
			bool                                  active{ false };
			std::uint32_t                         speaker{ 0 };
			std::uint32_t                         listener{ 0 };
			Shots::ShotType                       shot{ Shots::ShotType::kOverPlayerShoulder };
			std::uint32_t                         line{ 0 };   // counts lines, to pick a new shot per line
			std::optional<RE::NiPoint3>           smoothSpeaker, smoothListener;
			bool                                  applied{ false };
			std::optional<Camera::Pose>           pose;        // this line's camera, set up once (0.8.3)
			std::uint32_t                         poseLine{ 0 };
			RE::NiPoint3                          anchor{};    // the person in shot (two-shot: between them), eased
			RE::NiPoint3                          lookOffset{};  // the aim, relative to the anchor
			std::vector<std::uint32_t>            group;     // the scene's people lately, for group shots (0.8.10)
			bool                                  groupShot{ false };  // this line's shot is the group shot
			std::uint32_t                         lastGroupLine{ 0 };
		} g_film;

		struct FilmMain  // main thread only
		{
			bool                                  wasKeyDown{ false };
			std::chrono::steady_clock::time_point buttonSince{};  // the controller button, held since
			bool                                  buttonFired{ false };
			RE::NiPoint3                          playerAt{};
			std::chrono::steady_clock::time_point lastLine{};
			std::uint32_t                         lastSpeaker{ 0 };
			RE::NiPoint3                          stillAt{};       // where you've stood still since stillSince
			std::chrono::steady_clock::time_point stillSince{};
			std::chrono::steady_clock::time_point autoBlockedUntil{};  // you stopped it: no auto start before
			bool                                  autoStarted{ false };
			std::chrono::steady_clock::time_point startedAt{};
			const void*                           scene{ nullptr };          // the scene being filmed
			const void*                           declinedScene{ nullptr };  // you ended it: not again on its own for 2 minutes (0.8.6)
			std::chrono::steady_clock::time_point declinedAt{};
			std::chrono::steady_clock::time_point stoppedAt{};   // when filming last stopped (0.8.15)
			std::vector<std::uint32_t>            people;    // who has spoken or been spoken to in what's filmed (0.8.10)
			std::vector<std::uintptr_t>           infos;     // the lines filmed (by words and record), to notice repeats
			std::uintptr_t                        lastSaid{ 0 };
			int                                   repeats{ 0 };  // lines in a row heard before
		} g_filmMain;

		std::atomic<std::uint64_t> g_padUsedAt{ 0 };  // GetTickCount64 of the last controller input

		// The controller's listen button (0.8.1): read straight from XInput (Xbox controllers,
		// and most others through Steam Input), loaded when first needed. Held for fListenHold
		// seconds, so a tap still does what the game uses the button for.
		bool ControllerButtonHeld(int a_mask)
		{
			struct Pad
			{
				std::uint32_t packet;
				std::uint16_t buttons;
				std::uint8_t  leftTrigger, rightTrigger;
				std::int16_t  lx, ly, rx, ry;
			};
			using GetState_t = DWORD(WINAPI*)(DWORD, Pad*);
			static GetState_t getState = [] {
				for (const auto* dll : { L"xinput1_4.dll", L"xinput1_3.dll", L"xinput9_1_0.dll" }) {
					if (const HMODULE module = ::LoadLibraryW(dll)) {
						if (const auto fn = reinterpret_cast<GetState_t>(::GetProcAddress(module, "XInputGetState"))) {
							return fn;
						}
					}
				}
				REX::WARN("Filming people talking: no XInput; the controller's listen button is off");
				return static_cast<GetState_t>(nullptr);
			}();
			if (!getState || a_mask == 0) {
				return false;
			}
			bool held = false;
			for (DWORD pad = 0; pad < 4; ++pad) {
				Pad state{};
				if (getState(pad, &state) != ERROR_SUCCESS) {
					continue;
				}
				// Any real input (not stick noise) marks the controller as in use, for the prompt.
				if (state.buttons != 0 || state.leftTrigger > 30 || state.rightTrigger > 30 || std::abs(state.lx) > 9000 || std::abs(state.ly) > 9000 ||
					std::abs(state.rx) > 9000 || std::abs(state.ry) > 9000) {
					g_padUsedAt.store(::GetTickCount64(), std::memory_order_relaxed);
				}
				if ((state.buttons & a_mask) == a_mask) {
					held = true;
				}
			}
			return held;
		}

		// Whether the actor is in combat. Actor's virtual IsInCombat is slot 0x16B (+0xB58) in
		// 1.16.244, not CommonLibSF's 0x16C: the Papyrus natives Actor.IsInCombat (RVA
		// 0x1F1F690) and Actor.GetCombatState (0x1FC6DE0) both call [vtable+0xB58] (0.8.2).
		bool InCombat(RE::Actor* a_actor)
		{
			if (!a_actor) {
				return false;
			}
			using IsInCombat_t = bool (*)(RE::Actor*);
			const auto* vtable = *reinterpret_cast<const std::uintptr_t* const*>(a_actor);
			return reinterpret_cast<IsInCombat_t>(vtable[0xB58 / sizeof(std::uintptr_t)])(a_actor);
		}

		// The controller was used more recently than the keyboard and mouse.
		bool UsingController()
		{
			LASTINPUTINFO info{ sizeof(LASTINPUTINFO), 0 };
			const auto    pad = g_padUsedAt.load(std::memory_order_relaxed);
			if (pad == 0 || !::GetLastInputInfo(&info)) {
				return false;
			}
			// GetLastInputInfo's tick is 32-bit: compare within the last 49 days' wrap.
			return static_cast<std::uint32_t>(pad) - info.dwTime < 0x80000000u && static_cast<std::uint32_t>(pad) != info.dwTime;
		}

		bool GameHasFocus()
		{
			const HWND window = ::GetForegroundWindow();
			DWORD      pid = 0;
			::GetWindowThreadProcessId(window, &pid);
			return window && pid == ::GetCurrentProcessId();
		}

		// ---- The preset key (0.8.3) ------------------------------------------------------
		void UpdatePresetKey()
		{
			const auto&  s = Settings::Get();
			static bool  wasDown = false;
			bool         down = false;
			if (s.presetKey > 0 && GameHasFocus() && (::GetAsyncKeyState(s.presetKey) & 0x8000) != 0) {
				// Not while typing in the console.
				static const RE::BSFixedString console{ "Console" };
				auto*                          ui = RE::UI::GetSingleton();
				down = !(ui && ui->IsMenuOpen(console));
			}
			if (down && !wasDown) {
				constexpr std::array<const char*, 7> kOrder{ "Standard", "Close", "Room", "Frontier", "Starborn", "Constellation", "Nebula" };
				std::string current{ s.preset };
				std::ranges::transform(current, current.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
				std::size_t next = 0;
				for (std::size_t i = 0; i < kOrder.size(); ++i) {
					std::string name{ kOrder[i] };
					std::ranges::transform(name, name.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
					if (name == current) {
						next = (i + 1) % kOrder.size();
						break;
					}
				}
				Settings::SetPresetOverride(kOrder[next]);
				Director::SwitchPreset();
				g_remeasureRoom.store(true);
				REX::INFO("Preset key: now {} (until the game is restarted; sPreset in the INI sets the one it starts with)", kOrder[next]);
			}
			wasDown = down;
			if (s.showMessages) {
				std::string label = std::format("PRESET: {}", Settings::Get().preset);
				std::ranges::transform(label, label.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
				Hud::SetPresetLabel(label);
			}
		}


		const char* KeyName(int a_key)
		{
			static std::string name;
			const UINT         scan = ::MapVirtualKeyA(static_cast<UINT>(a_key), MAPVK_VK_TO_VSC);
			char               buffer[64]{};
			name = ::GetKeyNameTextA(static_cast<LONG>(scan << 16), buffer, sizeof(buffer)) > 0 ? buffer : std::format("key {}", a_key);
			return name.c_str();
		}

		const char* ButtonName(int a_mask)
		{
			switch (a_mask) {
			case 0x0001: return "D-pad up";
			case 0x0002: return "D-pad down";
			case 0x0004: return "D-pad left";
			case 0x0008: return "D-pad right";
			case 0x0010: return "Menu";
			case 0x0020: return "View";
			case 0x0040: return "L3";
			case 0x0080: return "R3";
			case 0x0100: return "LB";
			case 0x0200: return "RB";
			case 0x1000: return "A";
			case 0x2000: return "B";
			case 0x4000: return "X";
			case 0x8000: return "Y";
			default: return "the listen button";
			}
		}

		void StopFilming(const char* a_why)
		{
			{
				std::scoped_lock lock{ g_film.lock };
				if (!g_film.active) {
					return;
				}
				g_film.active = false;
				g_film.smoothSpeaker.reset();
				g_film.smoothListener.reset();
				g_film.pose.reset();
			}
			g_lensActive.store(false, std::memory_order_relaxed);
			Camera::RestoreRender();
			g_filmMain.stoppedAt = std::chrono::steady_clock::now();
			REX::INFO("Filming people talking: stopped ({})", a_why);
		}

		bool FilmingActive()
		{
			std::scoped_lock lock{ g_film.lock };
			return g_film.active;
		}

		// Main thread, every tick (after UpdateOverheard).
		void UpdateFilming(std::chrono::steady_clock::time_point a_now)
		{
			const auto& s = Settings::Get();
			auto*       player = RE::PlayerCharacter::GetSingleton();
			std::string prompt;  // the on-screen prompt this tick ("" = none)
			struct PromptOut
			{
				std::string& text;
				~PromptOut()
				{
					Hud::SetPrompt(text);
					const bool filming = FilmingActive();
					Hud::SetHideRollover(filming);
					Hud::SetFilmingOthers(filming && !g_state.active);
					// 0.8.4 test: filming from first person showed your character in a T-pose (the
					// game doesn't animate your full body in first person); 0.8.6: with your body
					// hidden, your first-person armour still showed, T-posed. The first-person body
					// isn't a node that can be found and hidden (0.8.7: "Player1stPersonBody" is a
					// label the game passes around, not a name in the scene), so the game goes to
					// third person while it films (both camera hooks film), you're hidden, and
					// first person comes back when it stops (unless your own conversation took
					// over: it puts visibility and the view back itself).
					static bool hid = false;
					auto*       camera = RE::PlayerCamera::GetSingleton();
					const bool  firstPerson = camera && CurrentCameraStateIndex(camera) == 0;
					if (filming && !g_state.active) {
						if (firstPerson && !g_filmThirdPerson && camera) {
							camera->ForceThirdPerson();
							g_filmThirdPerson = true;
							REX::INFO("Filming people talking: from first person; the game's view is third person while it films, you hidden");
						}
						if (g_filmThirdPerson) {
							Visibility::HidePlayer();
							hid = true;
						}
					} else {
						if (hid) {
							if (!g_state.active) {
								Visibility::Restore(true);
							}
							hid = false;
						}
						if (g_filmThirdPerson) {
							g_filmThirdPerson = false;
							if (!g_state.active) {
								g_restoreFirstPersonPending = true;  // back to first person (StepFirstPersonRestore)
								g_restoreFirstPersonDeadline = std::chrono::steady_clock::now() + 1500ms;
								g_restoreFirstPersonNext = {};
							}
						}
					}
				}
			} promptOut{ prompt };
			if (!s.filmConversations || !player || g_state.active) {
				StopFilming(g_state.active ? "your own conversation" : "off");
				return;
			}
			// Never in combat: no prompt, no starting, and filming stops.
			if (InCombat(player)) {
				StopFilming("combat");
				g_filmMain.stillSince = {};
				return;
			}
			// The conversations going on near you: NPC-to-NPC lines within reach.
			// 0.8.10 test: two scenes side by side at TerraBrew were filmed as one, cutting
			// between them. One at a time now: while filming a scene, only its lines; otherwise
			// the nearest conversation; when the one filmed is over, on to the next if there is one.
			struct Candidate
			{
				std::uint32_t                         id;
				std::uint32_t                         addressed;
				std::chrono::steady_clock::time_point lastLine;
				const void*                           scene;
				float                                 distance;
				std::uintptr_t                        info;
			};
			std::vector<Candidate> candidates;
			const auto             playerFeet = player->GetPosition();
			for (const auto& [id, t] : g_talkers) {
				if (t.lines == 0 || t.addressed == 0 || t.addressed == 0x14 || FlatDistance(t.feet, playerFeet) > s.listenRange) {
					continue;
				}
				// Still part of it a while after they last spoke, counted from the end of the line
				// (0.8.0 test: filming stopped during long lines and the pauses in Keeper Aquilus's
				// group). 0.8.4: 10 s while they're still playing their part in a scene (the
				// game's own pauses), 3 s once they aren't (0.8.3 test: the camera lingered long
				// after the conversation was over).
				// 0.8.4 test: still too long; now fFilmEndSecondsNpc (0.5 s) once they're not in
				// a scene, fFilmEndSecondsSceneNpc (4 s) while they are.
				const float quiet = std::chrono::duration<float>(a_now - (std::max)(t.lastLine, t.lastHeard)).count();
				if (quiet > s.filmEndSecondsScene) {
					continue;
				}
				if (quiet > s.filmEndSeconds) {
					auto* actor = RE::TESForm::LookupByID<RE::Actor>(id);
					if (!actor || !CurrentSceneUnsafe(actor)) {
						continue;
					}
				}
				candidates.push_back({ id, t.addressed, t.lastLine, t.lineScene, FlatDistance(t.feet, playerFeet), t.lineSaid });
			}
			bool filming;
			{
				std::scoped_lock lock{ g_film.lock };
				filming = g_film.active;
			}
			// The latest line among those that pass a_keep.
			const auto latestOf = [&](auto a_keep) {
				const Candidate* best = nullptr;
				for (const auto& c : candidates) {
					if (a_keep(c) && (!best || c.lastLine > best->lastLine)) {
						best = &c;
					}
				}
				return best;
			};
			// The nearest conversation (its latest line), among those that pass a_keep.
			const auto nearestOf = [&](auto a_keep) {
				const Candidate* closest = nullptr;  // (not "near": a Windows macro)
				for (const auto& c : candidates) {
					if (a_keep(c) && (!closest || c.distance < closest->distance)) {
						closest = &c;
					}
				}
				if (!closest || !closest->scene) {
					return closest;
				}
				const void* scene = closest->scene;
				return latestOf([scene](const Candidate& a_c) { return a_c.scene == scene; });
			};
			const auto declinedNow = [&](const void* a_scene) { return a_scene && a_scene == g_filmMain.declinedScene && a_now - g_filmMain.declinedAt < 120s; };
			const auto filmedPerson = [&](std::uint32_t a_id) { return std::ranges::find(g_filmMain.people, a_id) != g_filmMain.people.end(); };
			const Candidate* pick = nullptr;
			if (filming && g_filmMain.scene) {
				const void* filmed = g_filmMain.scene;
				pick = latestOf([filmed](const Candidate& a_c) { return a_c.scene == filmed; });
				// The same people going on into the game's next scene (0.8.10, as CCC follows a
				// scene into the next): theirs is followed, without leaving the camera.
				const auto* onward = latestOf([&](const Candidate& a_c) {
					return a_c.scene && a_c.scene != filmed && !declinedNow(a_c.scene) && filmedPerson(a_c.id);
				});
				if (onward && (!pick || onward->lastLine > pick->lastLine)) {
					REX::INFO("Filming people talking: {} goes on into the next scene; following it", TalkerName(onward->id));
					g_filmMain.scene = onward->scene;
					pick = onward;
				}
			} else if (filming) {
				pick = latestOf([](const Candidate&) { return true; });  // (the listen key, not a scene: as before)
			} else {
				// The nearest conversation scene you haven't walked away from; else the nearest
				// anything (for the listen key). 0.8.10 test: a customer's "Excuse me." (no scene)
				// was nearer than Helena's scene starting, which waited 1.5 s for the bark to end.
				pick = nearestOf([&](const Candidate& a_c) { return a_c.scene && !declinedNow(a_c.scene); });
				if (!pick) {
					pick = nearestOf([](const Candidate&) { return true; });
				}
			}
			std::uint32_t speaker = pick ? pick->id : 0;
			std::uint32_t listener = pick ? pick->addressed : 0;
			auto          latest = pick ? pick->lastLine : std::chrono::steady_clock::time_point{};
			// A menu over the game (pause, inventory, map, scanner...): no prompt, and the time
			// standing still starts again when it closes.
			bool inMenu = false;
			if (auto* ui = RE::UI::GetSingleton()) {
				static const std::array<RE::BSFixedString, 14> kMenus{ RE::BSFixedString{ "PauseMenu" }, RE::BSFixedString{ "DataMenu" },
					RE::BSFixedString{ "InventoryMenu" }, RE::BSFixedString{ "ContainerMenu" }, RE::BSFixedString{ "BarterMenu" },
					RE::BSFixedString{ "GalaxyStarMapMenu" }, RE::BSFixedString{ "PhotoModeMenu" }, RE::BSFixedString{ "Console" },
					RE::BSFixedString{ "LoadingMenu" }, RE::BSFixedString{ "WorkshopMenu" }, RE::BSFixedString{ "SkillsMenu" },
					RE::BSFixedString{ "StatusMenu" }, RE::BSFixedString{ "ScopeMenu" }, RE::BSFixedString{ "MonocleMenu" } };
				for (const auto& menu : kMenus) {
					if (ui->IsMenuOpen(menu)) {
						inMenu = true;
						break;
					}
				}
			}
			// The listen key, or the controller's listen button held for fListenHold seconds.
			const bool focus = GameHasFocus();
			const bool keyDown = s.listenKey > 0 && focus && (::GetAsyncKeyState(s.listenKey) & 0x8000) != 0;
			bool       pressed = keyDown && !g_filmMain.wasKeyDown;
			g_filmMain.wasKeyDown = keyDown;
			const bool buttonDown = s.listenButton > 0 && focus && ControllerButtonHeld(s.listenButton);
			if (!buttonDown) {
				g_filmMain.buttonSince = {};
				g_filmMain.buttonFired = false;
			} else if (g_filmMain.buttonSince == std::chrono::steady_clock::time_point{}) {
				g_filmMain.buttonSince = a_now;
			} else if (!g_filmMain.buttonFired && std::chrono::duration<float>(a_now - g_filmMain.buttonSince).count() >= s.listenHold) {
				g_filmMain.buttonFired = true;
				pressed = true;
			}
			if (filming) {
				const auto feet = player->GetPosition();
				// 0.8.6: it can start while you're walking; for its first second where you stand
				// is still settling (you stop walking), then moving ends it.
				if (a_now - g_filmMain.startedAt < 1s) {
					g_filmMain.playerAt = feet;
				}
				const float moved = FlatDistance(feet, g_filmMain.playerAt);
				if (pressed) {
					StopFilming("listen key");
					g_filmMain.autoBlockedUntil = a_now + 60s;  // you stopped it: don't start it again on its own for a while
					g_filmMain.declinedScene = g_filmMain.scene;
					g_filmMain.declinedAt = a_now;
				} else if (moved > 0.5f) {
					StopFilming("you moved");
					g_filmMain.declinedScene = g_filmMain.scene;  // not this scene again on its own
					g_filmMain.declinedAt = a_now;
				} else if (speaker == 0) {
					// Over: on to the next conversation scene nearby, if there's one (and it starts
					// on its own), without leaving the filming camera.
					const void* filmed = g_filmMain.scene;
					const auto* next = s.autoListen && filmed ? nearestOf([&](const Candidate& a_c) {
						return a_c.scene && a_c.scene != filmed && !(a_c.scene == g_filmMain.declinedScene && a_now - g_filmMain.declinedAt < 120s);
					}) :
					                                            nullptr;
					if (next) {
						std::scoped_lock lock{ g_film.lock };
						g_filmMain.scene = next->scene;
						g_filmMain.lastLine = next->lastLine;
						g_filmMain.lastSpeaker = next->id;
						g_film.speaker = next->id;
						g_film.listener = next->addressed;
						++g_film.line;
						g_filmMain.people = { next->id };
						if (next->addressed != 0 && next->addressed != 0x14) {
							g_filmMain.people.push_back(next->addressed);
						}
						g_filmMain.infos.clear();
						if (next->info != 0) {
							g_filmMain.infos.push_back(next->info);
						}
						g_filmMain.lastSaid = next->info;
						g_filmMain.repeats = 0;
						g_film.group.clear();
						REX::INFO("Filming people talking: that one's over; on to {} and {}, nearby", TalkerName(next->id), TalkerName(next->addressed));
					} else {
						StopFilming("they've stopped talking");
					}
				} else {
					bool repeating = false;
					{
						std::scoped_lock lock{ g_film.lock };
						if (latest != g_filmMain.lastLine || speaker != g_filmMain.lastSpeaker) {
							g_filmMain.lastLine = latest;
							g_filmMain.lastSpeaker = speaker;
							g_film.speaker = speaker;
							g_film.listener = listener;
							++g_film.line;
							for (const auto id : { speaker, listener }) {
								if (id != 0 && id != 0x14 && std::ranges::find(g_filmMain.people, id) == g_filmMain.people.end()) {
									g_filmMain.people.push_back(id);
								}
							}
						}
						// A scene only repeating itself (0.8.10, as CCC hands back): two lines in a row
						// already filmed. A line is known by its words once it's reported (one record
						// can hold several lines).
						if (pick->info != 0 && pick->info != g_filmMain.lastSaid) {
							g_filmMain.lastSaid = pick->info;
							if (std::ranges::find(g_filmMain.infos, pick->info) != g_filmMain.infos.end()) {
								repeating = ++g_filmMain.repeats >= 2;
							} else {
								g_filmMain.infos.push_back(pick->info);
								g_filmMain.repeats = 0;
							}
						}
						// Who's in it lately, for the group shot: the scene's people who've spoken
						// in the last 30 s, and whoever they spoke to.
						g_film.group.clear();
						for (const auto& [id, t] : g_talkers) {
							if (t.lineScene && t.lineScene == g_filmMain.scene && t.lines > 0 && a_now - t.lastLine < 30s) {
								for (const auto who : { id, t.addressed }) {
									if (who != 0 && who != 0x14 && std::ranges::find(g_film.group, who) == g_film.group.end()) {
										g_film.group.push_back(who);
									}
								}
							}
						}
					}
					if (repeating) {
						StopFilming("the scene is only repeating itself");
						g_filmMain.declinedScene = g_filmMain.scene;
						g_filmMain.declinedAt = a_now;
					}
				}
				return;
			}
			if (speaker == 0 || inMenu) {
				return;
			}
			// Starting on its own (bAutoListen): sitting for 1.5 s, or standing still for
			// fAutoListenAfter seconds, near a conversation; not for a minute after you stopped
			// one with the listen key.
			// 0.8.6: barks and one-liners ("Need something?", "Hey! You wanna dance?") aren't
			// offered or started on their own: in the 0.8.5 log every real conversation between
			// NPCs was one of the game's scenes (FC_AC_RandConv..., ...CityLife_Encounter...),
			// and every one-liner wasn't. (The listen key still films any.)
			const void* scene = pick->scene;
			const bool  conversation = scene != nullptr;
			// 0.8.6: a scene starts it on its own even while you're moving (scenes are few, and
			// moving ends it), but not one you've already walked away from or stopped.
			const bool declined = scene == g_filmMain.declinedScene && a_now - g_filmMain.declinedAt < 120s;
			// Only on a line that started after filming last stopped (0.8.14 test: a talking
			// flag flickering after Amoli and Gerhard's scene ended started it again for a
			// second, a flip to third person and back).
			const bool freshLine = pick->lastLine > g_filmMain.stoppedAt;
			const bool autoStart = s.autoListen && conversation && !declined && freshLine && a_now >= g_filmMain.autoBlockedUntil;
			if (pressed || autoStart) {
				std::scoped_lock lock{ g_film.lock };
				g_film.active = true;
				g_film.speaker = speaker;
				g_film.listener = listener;
				g_film.line = 1;
				g_film.applied = false;
				g_filmMain.playerAt = player->GetPosition();
				g_filmMain.startedAt = a_now;
				g_filmMain.scene = scene;
				g_filmMain.lastLine = latest;
				g_filmMain.lastSpeaker = speaker;
				g_filmMain.autoStarted = !pressed;
				g_filmMain.people.clear();
				g_filmMain.infos.clear();
				g_filmMain.repeats = 0;
				for (const auto id : { speaker, listener }) {
					if (id != 0 && id != 0x14) {
						g_filmMain.people.push_back(id);
					}
				}
				if (pick->info != 0) {
					g_filmMain.infos.push_back(pick->info);
				}
				g_filmMain.lastSaid = pick->info;
				g_film.group.clear();
				g_film.groupShot = false;
				g_lensWanted.store(g_filmLens.load(std::memory_order_relaxed), std::memory_order_relaxed);
				g_lensActive.store(true, std::memory_order_relaxed);
				REX::INFO("Filming people talking: {} and {}{}", TalkerName(speaker), TalkerName(listener),
					pressed ? "" : " (on its own: a conversation scene nearby)");
				return;
			}
			// The prompt (bListenPrompt), while there's a conversation to listen in on; not with
			// bAutoListen on (0.8.10: it starts on its own anyway).
			if (s.listenPrompt && !s.autoListen && conversation && (s.listenKey > 0 || s.listenButton > 0)) {
				const bool pad = s.listenButton > 0 && (s.listenKey <= 0 || UsingController());
				// A face button shows as its icon ("{B}", 0.8.4); other buttons and keys by name.
				std::string key = pad ? ButtonName(s.listenButton) : KeyName(s.listenKey);
				if (pad && key.size() == 1) {
					key = "{" + key + "}";
				}
				prompt = std::format("Listen in on nearby conversation. {} {}", pad ? "Hold" : "Press", key);
			}
		}

		// Camera thread (the gameplay camera's update): the shot of the current speaker.
		// 0.8.3: each line's shot is chosen once, from shots the obstacle check finds clear
		// (walls, things, people standing still), and its camera position stays put for the
		// line; the aim follows the person in shot through a dead band, slowly, so the picture
		// doesn't jitter with their head (0.8.2 test: horizontal jitter; wide shots blocked by
		// things and people at TerraBrew).
		void FilmFrame()
		{
			std::uint32_t              speakerId = 0, listenerId = 0, line = 0;
			std::vector<std::uint32_t> groupIds;
			{
				std::scoped_lock lock{ g_film.lock };
				if (!g_film.active) {
					return;
				}
				speakerId = g_film.speaker;
				listenerId = g_film.listener;
				line = g_film.line;
				groupIds = g_film.group;
			}
			auto* speaker = RE::TESForm::LookupByID<RE::Actor>(speakerId);
			auto* listener = RE::TESForm::LookupByID<RE::Actor>(listenerId);
			auto* camera = RE::PlayerCamera::GetSingleton();
			if (!speaker || !listener || !camera || !camera->cameraRoot) {
				return;
			}
			const auto speakerHead = Camera::HeadPosition(speaker);
			const auto listenerHead = Camera::HeadPosition(listener);
			if (!speakerHead || !listenerHead) {
				return;
			}
			const RE::NiPoint3 sh = *speakerHead, lh = *listenerHead;
			// Three or more people in the scene lately (0.8.10): their heads, for the group shot.
			std::vector<RE::NiPoint3> groupHeads;
			if (groupIds.size() >= 3) {
				for (const auto id : groupIds) {
					if (auto* who = RE::TESForm::LookupByID<RE::Actor>(id)) {
						if (const auto head = Camera::HeadPosition(who)) {
							groupHeads.push_back(*head);
						}
					}
				}
			}
			RE::NiPoint3 groupCenter = sh;
			if (!groupHeads.empty()) {
				groupCenter = {};
				const float n = static_cast<float>(groupHeads.size());
				for (const auto& h : groupHeads) {
					groupCenter.x += h.x / n;
					groupCenter.y += h.y / n;
					groupCenter.z += h.z / n;
				}
			}

			std::scoped_lock lock{ g_film.lock };
			const auto subjectOf = [&](Shots::ShotType a_type) {
				if (g_film.groupShot && groupHeads.size() >= 3) {
					return groupCenter;
				}
				return a_type == Shots::ShotType::kTwoShot ? RE::NiPoint3{ (sh.x + lh.x) * 0.5f, (sh.y + lh.y) * 0.5f, (sh.z + lh.z) * 0.5f } : sh;
			};
			const bool newLine = g_film.poseLine != line || !g_film.pose;
			// The speaker walked off from where the shot was set up: set it up again.
			const bool drifted = g_film.pose && Distance(subjectOf(g_film.shot), g_film.anchor) > 1.5f;
			if (newLine || drifted) {
				g_film.groupShot = false;  // (set again below if this one is a group shot)
				static std::mt19937 rng{ std::random_device{}() };
				// 0.8.15: closer for two people (close-ups, three-quarters and shoulders most, a
				// medium now and then); the two-shot only with three or more, who also get the
				// group shot.
				const bool groupScene = groupHeads.size() >= 3;
				constexpr std::array kPair{ Shots::ShotType::kCloseUp, Shots::ShotType::kCloseUp, Shots::ShotType::kThreeQuarterNpc,
					Shots::ShotType::kThreeQuarterNpc, Shots::ShotType::kOverPlayerShoulder, Shots::ShotType::kOverPlayerShoulder, Shots::ShotType::kMediumNpc };
				constexpr std::array kGroup{ Shots::ShotType::kOverPlayerShoulder, Shots::ShotType::kOverPlayerShoulder, Shots::ShotType::kThreeQuarterNpc,
					Shots::ShotType::kMediumNpc, Shots::ShotType::kCloseUp, Shots::ShotType::kTwoShot };
				std::vector<Shots::ShotType> order;
				auto first = groupScene ? kGroup[std::uniform_int_distribution<std::size_t>{ 0, kGroup.size() - 1 }(rng)] :
				                          kPair[std::uniform_int_distribution<std::size_t>{ 0, kPair.size() - 1 }(rng)];
				if (newLine && first == g_film.shot && first != Shots::ShotType::kOverPlayerShoulder) {
					first = Shots::ShotType::kOverPlayerShoulder;
				}
				if (drifted && !newLine) {
					first = g_film.shot;
				}
				order.push_back(first);
				std::vector<Shots::ShotType> rest{ Shots::ShotType::kOverPlayerShoulder, Shots::ShotType::kThreeQuarterNpc, Shots::ShotType::kMediumNpc,
					Shots::ShotType::kCloseUp };
				if (groupScene) {
					rest.push_back(Shots::ShotType::kTwoShot);
				}
				std::ranges::shuffle(rest, rng);
				for (const auto type : rest) {
					if (std::ranges::find(order, type) == order.end()) {
						order.push_back(type);
					}
				}
				Shots::Input in;
				in.npcHead = sh;
				in.playerHead = lh;
				in.lensDegrees = Director::LensDegrees();
				in.maxDistance = 7.0f;
				in.enforceLine = true;
				std::optional<Shots::Result> chosen, fallback;
				Shots::ShotType              chosenType = first, fallbackType = first;
				float                        fallbackRoom = -1.0f;  // how far from the speaker the camera can stand clear
				std::string                  skipped;
				for (const auto type : order) {
					in.lensDegrees = FilmLens(type, false);
					auto solved = Shots::Solve(type, in);
					if (!solved.ok) {
						continue;
					}
					const bool overShoulder = type == Shots::ShotType::kOverPlayerShoulder;
					const auto toSpeaker = TestShot(sh, solved.pose.position, lh, overShoulder);
					bool       clear = toSpeaker.usable && toSpeaker.blockedAt < 0.0f;
					if (clear && type == Shots::ShotType::kTwoShot) {
						const auto toListener = TestShot(lh, solved.pose.position, sh, false);
						clear = toListener.usable && toListener.blockedAt < 0.0f;
					}
					if (clear) {
						chosen = solved;
						chosenType = type;
						break;
					}
					// 0.8.6 test (Neon Security's headquarters): with nothing clear the first shot
					// was used as it was, the camera beyond the room's walls. Now the one with the
					// most room, pulled in short of the wall.
					const float room = toSpeaker.blockedAt >= 0.0f ? toSpeaker.blockedAt : toSpeaker.length;
					if (room > fallbackRoom) {
						fallbackRoom = room;
						fallback = solved;
						fallbackType = type;
					}
					skipped += std::format("{}{}", skipped.empty() ? "" : ", ", Shots::Key(type));
				}
				// A group shot now and then (0.8.10): with three or more people in the scene, about
				// one new line in four (never two in a row), from outside the group across from the
				// speaker, wide enough for everyone; only when clear.
				bool group = false;
				if (newLine && groupHeads.size() >= 3 && line > g_film.lastGroupLine + 1 && std::uniform_int_distribution<int>{ 0, 3 }(rng) == 0) {
					float radius = 0.0f;
					for (const auto& h : groupHeads) {
						radius = (std::max)(radius, std::hypot(h.x - groupCenter.x, h.y - groupCenter.y));
					}
					float dx = groupCenter.x - sh.x, dy = groupCenter.y - sh.y;
					float len = std::hypot(dx, dy);
					if (len < 0.2f) {  // the speaker in the middle: across from whoever they're addressing
						dx = sh.x - lh.x;
						dy = sh.y - lh.y;
						len = std::hypot(dx, dy);
					}
					const float half = FilmLens(Shots::ShotType::kTwoShot, true) * 0.5f * 0.017453292f;
					const float back = (radius + 0.5f) / std::tan(half) + radius;
					if (len > 0.01f && back <= 9.0f) {
						Camera::Pose groupPose{};
						groupPose.position = { groupCenter.x + dx / len * back, groupCenter.y + dy / len * back, groupCenter.z + 0.35f };
						groupPose.lookAt = { groupCenter.x, groupCenter.y, groupCenter.z - 0.15f };
						const auto toSpeaker = TestShot(sh, groupPose.position, lh, false);
						if (toSpeaker.usable && toSpeaker.blockedAt < 0.0f) {
							chosen = Shots::Result{};
							chosen->ok = true;
							chosen->pose = groupPose;
							chosenType = Shots::ShotType::kMaster;
							group = true;
							g_film.lastGroupLine = line;
						} else {
							skipped += std::format("{}group", skipped.empty() ? "" : ", ");
						}
					}
				}
				if (!chosen && !fallback) {
					return;
				}
				if (!chosen) {
					chosenType = fallbackType;
					if (fallbackRoom >= 0.0f) {
						Camera::PullIn(fallback->pose, subjectOf(fallbackType), (std::max)(0.6f, fallbackRoom - 0.3f));
					}
				}
				const auto& use = chosen ? *chosen : *fallback;
				g_film.groupShot = group;
				g_film.shot = chosenType;
				g_filmLens.store(FilmLens(chosenType, group), std::memory_order_relaxed);
				g_film.pose = use.pose;
				g_film.anchor = subjectOf(chosenType);
				g_film.lookOffset = { use.pose.lookAt.x - g_film.anchor.x, use.pose.lookAt.y - g_film.anchor.y, use.pose.lookAt.z - g_film.anchor.z };
				g_film.poseLine = line;
				g_film.applied = false;
				if (Settings::Get().debugLog && (!skipped.empty() || !chosen)) {
					REX::INFO("Filming people talking: {} blocked from there{}", skipped,
						chosen ? std::string{} : std::format("; nothing clear: {} pulled in to {:.1f} m", Shots::Key(fallbackType), (std::max)(0.6f, fallbackRoom - 0.3f)));
				}
			} else {
				// The aim follows the person in shot through a dead band (4 cm), easing slowly.
				const auto  subject = subjectOf(g_film.shot);
				const float off = Distance(subject, g_film.anchor);
				if (off > 0.04f) {
					const float k = 0.06f * (off - 0.04f) / off;
					g_film.anchor.x += (subject.x - g_film.anchor.x) * k;
					g_film.anchor.y += (subject.y - g_film.anchor.y) * k;
					g_film.anchor.z += (subject.z - g_film.anchor.z) * k;
				}
			}
			g_lensWanted.store(g_filmLens.load(std::memory_order_relaxed), std::memory_order_relaxed);
			Camera::Pose pose = *g_film.pose;
			pose.lookAt = { g_film.anchor.x + g_film.lookOffset.x, g_film.anchor.y + g_film.lookOffset.y, g_film.anchor.z + g_film.lookOffset.z };
			if (Camera::ApplyRender(camera->cameraRoot.get(), pose, 0.0f) && !g_film.applied) {
				g_film.applied = true;
				REX::INFO("Filming people talking: {} on {}", g_film.groupShot ? std::format("group shot ({} people)", groupHeads.size()) : std::string{ Shots::Key(g_film.shot) },
					speaker->GetDisplayFullName() ? speaker->GetDisplayFullName() : "the speaker");
			}
		}

		void UpdateBystanders(std::chrono::steady_clock::time_point a_now)
		{
			struct Track
			{
				RE::NiPoint3                          at{};
				std::chrono::steady_clock::time_point since{};
				std::chrono::steady_clock::time_point seen{};
			};
			static std::unordered_map<std::uint32_t, Track> tracks;
			std::vector<Bystander>                          still;
			const auto&                                     s = Settings::Get();
			auto*                                           player = RE::PlayerCharacter::GetSingleton();
			// While filming people talking (0.8.3) too, leaving out the two of them.
			std::uint32_t              filmSpeaker = 0, filmListener = 0;
			std::vector<std::uint32_t> filmGroup;  // (0.8.10: the scene's people aren't in the way of its group shot)
			bool                       filming = false;
			{
				std::scoped_lock lock{ g_film.lock };
				filming = g_film.active;
				filmSpeaker = g_film.speaker;
				filmListener = g_film.listener;
				filmGroup = g_film.group;
			}
			if (s.avoidBystanders && ((g_state.active && !g_state.handedBack) || filming) && player) {
				const char* source = nullptr;
				const auto  nearby = NearbyActors(player, source);
				{
					const auto center = player->GetPosition();
					for (const auto& actor : nearby) {
						if (!actor || actor.get() == player || (g_state.active && actor->formID == g_state.speakerId) ||
							(filming && (actor->formID == filmSpeaker || actor->formID == filmListener || std::ranges::find(filmGroup, actor->formID) != filmGroup.end()))) {
							continue;
						}
						const auto  feet = actor->GetPosition();
						const float dx = feet.x - center.x, dy = feet.y - center.y;
						if (dx * dx + dy * dy > 15.0f * 15.0f) {
							continue;
						}
						auto& t = tracks[actor->formID];
						const float mx = feet.x - t.at.x, my = feet.y - t.at.y, mz = feet.z - t.at.z;
						if (t.seen == std::chrono::steady_clock::time_point{} || mx * mx + my * my + mz * mz > 0.25f * 0.25f) {
							t.at = feet;
							t.since = a_now;
						}
						t.seen = a_now;
						if (std::chrono::duration<float>(a_now - t.since).count() >= s.bystanderStill) {
							const auto head = Camera::HeadPosition(actor.get());
							still.push_back({ feet, head ? *head : RE::NiPoint3{ feet.x, feet.y, feet.z + 1.6f } });
						}
					}
				}
			}
			std::erase_if(tracks, [&](const auto& a_entry) { return a_now - a_entry.second.seen > std::chrono::seconds(5); });
			std::scoped_lock lock{ g_bystanderLock };
			if (still.size() != g_bystanders.size() && s.debugLog && g_state.active) {
				REX::INFO("Bystanders: {} standing still near the conversation", still.size());
			}
			g_bystanders = std::move(still);
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

				// Who's standing still near the conversation (bystanders the camera avoids).
				{
					static auto nextScan = now;
					if (now >= nextScan) {
						nextScan = now + 250ms;
						UpdateBystanders(now);
					}
				}
				// NPCs talking to each other near you (groundwork for filming them, 0.8).
				{
					static auto nextScan = now;
					if (now >= nextScan) {
						nextScan = now + 100ms;
						UpdateOverheard(now);
					}
					UpdateFilming(now);  // every tick, for the listen key
				}
				// The preset key (0.8.3): the next preset, on the fly; and the preset on screen
				// with the testing messages.
				UpdatePresetKey();

				// Letterbox, subtitles and dialogue options (0.6.3).
				{
					static auto last = now;
					const float dt = std::clamp(std::chrono::duration<float>(now - last).count(), 0.0f, 0.1f);
					last = now;
					Hud::SetFirstLine(g_state.active && g_state.npcLinesStarted <= 1);
					Hud::Update(g_state.active && !g_state.handedBack && !g_state.smallSpeaker, g_state.speakerWasTalking, dt);
				}

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
					auto*       speaker = g_state.active && (Settings::Get().debugLog || Settings::Get().showMessages || Settings::Get().playerFace) ? Speaker() : nullptr;
					const bool  talking = speaker && speaker->IsTalking();
					// A new part of their line while they keep talking (0.8.3): its record too.
					// Not within half a second of the last, where the counter merely caught up with
					// the line's start.
					static int  lastSerial = 0;
					static auto lastLogged = std::chrono::steady_clock::time_point{};
					const int   serial = g_lineSerial.load(std::memory_order_acquire);
					const auto  tick = std::chrono::steady_clock::now();
					if (talking && (!wasTalking || (serial != lastSerial && tick - lastLogged > 500ms))) {
						LogLineRecord(static_cast<RE::Actor*>(speaker));
						lastLogged = tick;
					}
					lastSerial = serial;
					FlushHudMessages(now);
					Face::Update(g_state.active && !g_state.handedBack);
					if (g_reactionStarted.exchange(false, std::memory_order_acq_rel) && g_state.active && !g_state.handedBack) {
						Face::OnReaction();
					}
					// The conversation lens while this mod films (0.8).
					// (Filming people talking near you uses the lens too, at least the game's 30.)
					const bool filmingOthers = FilmingActive();
					const float lens = Director::WantedLens();
					g_lensWanted.store(filmingOthers ? g_filmLens.load(std::memory_order_relaxed) : lens, std::memory_order_relaxed);  // (0.8.15: filming kept the last conversation's lens)
					g_lensActive.store(filmingOthers || (g_state.active && !g_state.handedBack && !g_state.smallSpeaker && lens >= 10.0f), std::memory_order_relaxed);
					if (!g_state.active) {
						g_lensCalls.store(0, std::memory_order_relaxed);
					}
					wasTalking = talking;
				}
				// Focus on the person in shot (0.5.0). The conversation blur reads its settings live
				// from the game's Dialogue_Close/Medium/Long_DOF records; set their focus distance
				// to the distance from the camera to the person in shot, every shot, and put them
				// back afterwards (and in quest scenes, where the game's own camera is used).
				{
					const auto&  st = Settings::Get();
					const bool   live = g_state.active && !g_state.handedBack && !g_state.smallSpeaker;
					static float lastDistance = -1.0f;
					if (live && st.focusOnSubject && ReadBlurRecords()) {
						g_blurReady.store(true, std::memory_order_release);
						const float d = g_shotDistance.load(std::memory_order_relaxed);
						WriteBlurRecords(d, st.deepFocusLongShots, st.deepFocusBeyond);
						if (st.debugLog && std::abs(d - lastDistance) > 0.25f) {
							lastDistance = d;
							static int logged = 0;
							if (logged++ < 200) {
								REX::INFO("Focus: {:.2f} m (the person in shot){}", d, d > kNoBlurBeyond ? ", no blur" : st.deepFocusLongShots && d > st.deepFocusBeyond ? ", deep sharp zone" : "");
							}
						}
					} else if (g_blurRecordsChanged) {
						PutBackBlurRecords();
						lastDistance = -1.0f;
						if (st.debugLog) {
							REX::INFO("Focus: the game's conversation blur settings put back");
						}
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

				if (camera && !g_state.active && !g_restoreFirstPersonPending && !g_filmThirdPerson) {
					// Remember which view the player uses outside conversations (not the third
					// person filming puts you in, 0.8.8).
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
	SFSE::Init(a_sfse, { .trampoline = true, .trampolineSize = 160 });
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
	{
		const bool lens = CCC::LensHookMain::Hook.Enable();
		const bool lens2 = CCC::LensHook2::Hook.Enable();
		const bool lens3 = CCC::LensHook3::Hook.Enable();
		if (lens) {
			REX::INFO("Conversation lens hook installed ({} of 2 other callers too)", (lens2 ? 1 : 0) + (lens3 ? 1 : 0));
		} else {
			REX::WARN("Could not install the conversation lens hook; conversations keep the game's 30 degree lens");
		}
	}

	CCC::g_mainThreadId = ::GetCurrentThreadId();
	if (const auto* tasks = SFSE::GetTaskInterface()) {
		tasks->AddPermanentTask([]() { CCC::Watchdog(); });
	} else {
		REX::WARN("SFSE task interface unavailable; camera watchdog disabled");
	}

	CCC::g_hooksSafe = true;
	const auto version = SFSE::GetPluginVersion();
	REX::INFO("Cinematic Conversation Camera SF {}.{}.{} loaded (enabled={}, shotMode={}, cameraTarget={}, revealPlayer={}, poseMode={})", version.major(),
		version.minor(), version.patch(), settings.enabled, static_cast<int>(settings.shotMode), settings.cameraTarget, settings.revealPlayer, settings.poseMode);
	return true;
}
