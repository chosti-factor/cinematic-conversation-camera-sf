#include "Settings.h"
#include <algorithm>

namespace CCC
{
	namespace
	{
		constexpr auto kIniPath = L".\\Data\\SFSE\\Plugins\\CinematicConversationCameraSF.ini";
		constexpr auto kUserIniPath = L".\\Data\\SFSE\\Plugins\\CinematicConversationCameraSF_user.ini";

		float ReadFloat(const wchar_t* a_path, const wchar_t* a_section, const wchar_t* a_key, float a_default)
		{
			wchar_t buffer[64]{};
			const auto fallback = std::to_wstring(a_default);
			::GetPrivateProfileStringW(a_section, a_key, fallback.c_str(), buffer, static_cast<DWORD>(std::size(buffer)), a_path);
			try {
				return std::stof(buffer);
			} catch (...) {
				return a_default;
			}
		}

		std::int32_t ReadInt(const wchar_t* a_path, const wchar_t* a_section, const wchar_t* a_key, std::int32_t a_default)
		{
			return static_cast<std::int32_t>(::GetPrivateProfileIntW(a_section, a_key, a_default, a_path));
		}

		std::vector<std::pair<std::string, std::string>> ReadSection(const wchar_t* a_path, const wchar_t* a_section)
		{
			std::vector<std::pair<std::string, std::string>> out;
			std::vector<wchar_t>                             buffer(16384);
			const auto n = ::GetPrivateProfileSectionW(a_section, buffer.data(), static_cast<DWORD>(buffer.size()), a_path);
			for (std::size_t i = 0; i < n;) {
				std::wstring_view line{ buffer.data() + i };
				i += line.size() + 1;
				if (line.empty() || line.front() == L';' || line.front() == L'#') {
					continue;
				}
				const auto eq = line.find(L'=');
				if (eq == std::wstring_view::npos) {
					continue;
				}
				auto narrow = [](std::wstring_view a_w, bool a_keepSpaces) {
					std::string text;
					for (const auto c : a_w) {
						if (a_keepSpaces || (c != L' ' && c != L'\t')) {
							text.push_back(static_cast<char>(c));
						}
					}
					while (!text.empty() && (text.back() == ' ' || text.back() == '\t')) {
						text.pop_back();
					}
					while (!text.empty() && (text.front() == ' ' || text.front() == '\t')) {
						text.erase(text.begin());
					}
					return text;
				};
				auto key = narrow(line.substr(0, eq), false);
				auto value = narrow(line.substr(eq + 1), true);
				if (const auto semi = value.find(';'); semi != std::string::npos) {
					value.resize(semi);
					while (!value.empty() && value.back() == ' ') {
						value.pop_back();
					}
				}
				if (!key.empty() && !value.empty()) {
					out.emplace_back(std::move(key), std::move(value));
				}
			}
			return out;
		}

		std::string ReadString(const wchar_t* a_path, const wchar_t* a_section, const wchar_t* a_key, const std::string& a_default)
		{
			wchar_t buffer[2048]{};  // 128 cut long lists such as sCloseUpEmotions short (0.4.15)
			::GetPrivateProfileStringW(a_section, a_key, L"", buffer, static_cast<DWORD>(std::size(buffer)), a_path);
			std::string out;
			for (const wchar_t* c = buffer; *c; ++c) {
				if (*c != L' ') {
					out.push_back(static_cast<char>(*c));
				}
			}
			return out.empty() ? a_default : out;
		}

		// The cutting styles, calmest first.
		struct CuttingStyle
		{
			const char* name;
			const char* oldName;  // the 0.4.2 names still work
			int         freshAngleChance;
			int         minLines;
			int         maxLines;
			float       shortestHold;
		};
		constexpr std::array kCuttingStyles{
			CuttingStyle{ "Cinematic", "Cinematic", 0, 4, 8, 3.0f },        // each person keeps their angle; film editing
			CuttingStyle{ "Odyssey", "Drama", 25, 3, 6, 2.4f },             // CCC's own rhythm
			CuttingStyle{ "SpaceWestern", "Television", 50, 2, 5, 2.0f },
			CuttingStyle{ "Dogfight", "Thriller", 75, 2, 4, 1.6f },
			CuttingStyle{ "SpaceOpera", "SoapOpera", 100, 1, 3, 1.2f },     // a new angle on every turn
		};

		void ApplyCuttingStyle(Settings& a_s)
		{
			std::string wanted;
			for (const char c : a_s.cuttingStyle) {
				if (c != ' ' && c != '-' && c != '_') {
					wanted.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
				}
			}
			auto lower = [](const char* a_text) {
				std::string out;
				for (const char* c = a_text; *c; ++c) {
					out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(*c))));
				}
				return out;
			};
			for (const auto& style : kCuttingStyles) {
				if (lower(style.name) == wanted || lower(style.oldName) == wanted) {
					a_s.freshAngleChance = style.freshAngleChance;
					a_s.minLines = style.minLines;
					a_s.maxLines = style.maxLines;
					a_s.shortestHold = style.shortestHold;
					return;
				}
			}
		}

		void LoadFrom(Settings& a_s, const wchar_t* a_path)
		{
			a_s.enabled = ReadInt(a_path, L"General", L"bEnabled", a_s.enabled ? 1 : 0) != 0;
			a_s.debugLog = ReadInt(a_path, L"General", L"bDebugLog", a_s.debugLog ? 1 : 0) != 0;
			// [Debug] since 0.4.16; [General] still read for INIs from 0.4.14-0.4.15.
			// bShowMessages (0.7.5) replaces bShowEmotions and bShowShots; an older INI's keys
			// still count when it has no bShowMessages (either one on = messages on).
			{
				std::int32_t legacy = -1;
				for (const auto* key : { L"bShowEmotions", L"bShowShots" }) {
					for (const auto* section : { L"General", L"Debug" }) {
						const auto v = ReadInt(a_path, section, key, -1);
						if (v >= 0) {
							legacy = std::max(legacy, v != 0 ? 1 : 0);
						}
					}
				}
				const std::int32_t fallback = legacy >= 0 ? legacy : (a_s.showMessages ? 1 : 0);
				a_s.showMessages = ReadInt(a_path, L"Debug", L"bShowMessages", fallback) != 0;
			}
			a_s.messageSeconds = std::clamp(ReadFloat(a_path, L"Debug", L"fMessageSeconds", a_s.messageSeconds), 1.0f, 30.0f);
			a_s.filmConversations = ReadInt(a_path, L"Overhear", L"bFilmConversations", a_s.filmConversations ? 1 : 0) != 0;
			a_s.listenKey = std::clamp(ReadInt(a_path, L"Overhear", L"iListenKey", a_s.listenKey), 0, 254);
			a_s.listenRange = std::clamp(ReadFloat(a_path, L"Overhear", L"fListenRange", a_s.listenRange), 3.0f, 25.0f);
			a_s.listenButton = std::clamp(ReadInt(a_path, L"Overhear", L"iListenButton", a_s.listenButton), 0, 0xFFFF);
			a_s.listenHold = std::clamp(ReadFloat(a_path, L"Overhear", L"fListenHold", a_s.listenHold), 0.2f, 2.0f);
			a_s.listenPrompt = ReadInt(a_path, L"Overhear", L"bListenPrompt", a_s.listenPrompt ? 1 : 0) != 0;
			a_s.autoListen = ReadInt(a_path, L"Overhear", L"bAutoListen", a_s.autoListen ? 1 : 0) != 0;
			a_s.autoListenAfter = std::clamp(ReadFloat(a_path, L"Overhear", L"fAutoListenAfter", a_s.autoListenAfter), 1.0f, 30.0f);
			a_s.roomShotMinRoom = std::clamp(ReadInt(a_path, L"Director", L"iRoomShotMinRoom", a_s.roomShotMinRoom), 30, 100);
			a_s.roomShotMaxIndoors = std::clamp(ReadFloat(a_path, L"Director", L"fRoomShotMaxIndoors", a_s.roomShotMaxIndoors), -1.0f, 25.0f);
			a_s.roomShotMaxOutdoors = std::clamp(ReadFloat(a_path, L"Director", L"fRoomShotMaxOutdoors", a_s.roomShotMaxOutdoors), -1.0f, 25.0f);
			a_s.conversationLens = ReadFloat(a_path, L"Camera", L"fConversationLens", a_s.conversationLens);
			a_s.conversationLens = a_s.conversationLens < 0.0f ? -1.0f : a_s.conversationLens < 10.0f ? 0.0f : std::clamp(a_s.conversationLens, 20.0f, 70.0f);
			a_s.playerFace = ReadInt(a_path, L"Face", L"bPlayerFace", a_s.playerFace ? 1 : 0) != 0;
			a_s.questionFaceChance = std::clamp(ReadInt(a_path, L"Face", L"iQuestionChance", a_s.questionFaceChance), 0, 100);
			a_s.signatureChance = std::clamp(ReadInt(a_path, L"Face", L"iSignatureChance", a_s.signatureChance), 0, 100);
			a_s.playerCloseUpChance = std::clamp(ReadInt(a_path, L"Face", L"iPlayerCloseUpChance", a_s.playerCloseUpChance), 0, 100);
			a_s.closeUpDistance = std::clamp(ReadInt(a_path, L"Director", L"iCloseUpDistance", a_s.closeUpDistance), 80, 150);
			a_s.dumpMenus = ReadInt(a_path, L"Debug", L"bDumpMenus", a_s.dumpMenus ? 1 : 0) != 0;
			a_s.letterbox = ReadInt(a_path, L"Interface", L"bLetterbox", a_s.letterbox ? 1 : 0) != 0;
			a_s.letterboxHeight = std::clamp(ReadFloat(a_path, L"Interface", L"fLetterboxHeight", a_s.letterboxHeight), 0.02f, 0.3f);
			a_s.letterboxFade = std::clamp(ReadFloat(a_path, L"Interface", L"fLetterboxFade", a_s.letterboxFade), 0.0f, 3.0f);
			a_s.fadeOptions = ReadInt(a_path, L"Interface", L"bFadeOptions", a_s.fadeOptions ? 1 : 0) != 0;
			a_s.subtitlesInBars = ReadInt(a_path, L"Interface", L"bSubtitlesInBars", a_s.subtitlesInBars ? 1 : 0) != 0;
			a_s.hideOptionsWhileSpeaking = ReadInt(a_path, L"Interface", L"bHideOptionsWhileSpeaking", a_s.hideOptionsWhileSpeaking ? 1 : 0) != 0;
			a_s.optionsFade = std::clamp(ReadFloat(a_path, L"Interface", L"fOptionsFade", a_s.optionsFade), 0.0f, 1.5f);

			a_s.director = ReadInt(a_path, L"Director", L"bEnabled", a_s.director ? 1 : 0) != 0;
			a_s.preset = ReadString(a_path, L"Director", L"sPreset", a_s.preset);
			// Only a file that names a style applies it, so a style in the main INI doesn't undo
			// fine-tuning there when the _user.ini is read after it.
			if (const auto style = ReadString(a_path, L"Director", L"sCuttingStyle", ""); !style.empty()) {
				a_s.cuttingStyle = style;
				ApplyCuttingStyle(a_s);
			}
			a_s.minLines = std::clamp(ReadInt(a_path, L"Director", L"iMinLines", a_s.minLines), 1, 20);
			a_s.maxLines = std::clamp(ReadInt(a_path, L"Director", L"iMaxLines", a_s.maxLines), 1, 20);
			a_s.shortLineSeconds = std::clamp(ReadFloat(a_path, L"Director", L"fShortLineSeconds", a_s.shortLineSeconds), 0.0f, 10.0f);
			a_s.shortestHold = std::clamp(ReadFloat(a_path, L"Director", L"fShortestHold", a_s.shortestHold), 0.3f, 9.0f);
			// 0 = the preset's own cap; anything else is the player's limit, 1-15 m.
			const auto cap = [](float a_value) { return a_value <= 0.0f ? 0.0f : std::clamp(a_value, 1.0f, 15.0f); };
			a_s.maxCameraDistance = cap(ReadFloat(a_path, L"Director", L"fMaxCameraDistance", a_s.maxCameraDistance));
			a_s.maxCameraDistanceExterior = cap(ReadFloat(a_path, L"Director", L"fMaxCameraDistanceExterior", a_s.maxCameraDistanceExterior));
			a_s.obstacleCheck = ReadInt(a_path, L"Director", L"bObstacleCheck", a_s.obstacleCheck ? 1 : 0) != 0;
			a_s.obstacleRadius = std::clamp(ReadFloat(a_path, L"Director", L"fObstacleRadius", a_s.obstacleRadius), 0.01f, 0.3f);
			a_s.true180 = ReadInt(a_path, L"Director", L"bTrue180", a_s.true180 ? 1 : 0) != 0;
			a_s.enforceLine = ReadInt(a_path, L"Director", L"bEnforceLine", a_s.enforceLine ? 1 : 0) != 0;
			a_s.followFace = ReadInt(a_path, L"Director", L"bFollowFace", a_s.followFace ? 1 : 0) != 0;
			a_s.reactionShots = ReadInt(a_path, L"Director", L"bReactionShots", a_s.reactionShots ? 1 : 0) != 0;
			a_s.reactionEvery = std::clamp(ReadInt(a_path, L"Director", L"iReactionEvery", a_s.reactionEvery), 1, 10);
			a_s.reactionChance = std::clamp(ReadInt(a_path, L"Director", L"iReactionChance", a_s.reactionChance), 0, 100);
			a_s.reactionSeconds = std::clamp(ReadFloat(a_path, L"Director", L"fReactionSeconds", a_s.reactionSeconds), 0.0f, 30.0f);
			a_s.reactionMinSeconds = std::clamp(ReadFloat(a_path, L"Director", L"fReactionMinSeconds", a_s.reactionMinSeconds), 0.0f, 10.0f);
			a_s.avoidBystanders = ReadInt(a_path, L"Director", L"bAvoidBystanders", a_s.avoidBystanders ? 1 : 0) != 0;
			a_s.bystanderStill = std::clamp(ReadFloat(a_path, L"Director", L"fBystanderStill", a_s.bystanderStill), 0.0f, 10.0f);
			a_s.persuasionBeat = ReadInt(a_path, L"Director", L"bPersuasionBeat", a_s.persuasionBeat ? 1 : 0) != 0;
			a_s.beatTags = ReadString(a_path, L"Director", L"sBeatTags", a_s.beatTags);
			a_s.faceDeadband = std::clamp(ReadFloat(a_path, L"Camera", L"fFaceDeadband", a_s.faceDeadband), 0.0f, 0.2f);
			a_s.cameraSway = std::clamp(ReadInt(a_path, L"Camera", L"iCameraSway", a_s.cameraSway), 0, 100);
			a_s.emotionCloseUps = ReadInt(a_path, L"Director", L"bEmotionCloseUps", a_s.emotionCloseUps ? 1 : 0) != 0;
			a_s.closeUpEmotions = ReadString(a_path, L"Director", L"sCloseUpEmotions", a_s.closeUpEmotions);
			a_s.extremeCloseUpChance = std::clamp(ReadInt(a_path, L"Director", L"iExtremeCloseUpChance", a_s.extremeCloseUpChance), 0, 100);
			a_s.closeUpChance = std::clamp(ReadInt(a_path, L"Director", L"iCloseUpChance", a_s.closeUpChance), 0, 100);
			a_s.plainShotsSeated = ReadInt(a_path, L"Director", L"bPlainShotsSeated", a_s.plainShotsSeated ? 1 : 0) != 0;
			a_s.plainShotsMerchants = ReadInt(a_path, L"Director", L"bPlainShotsMerchants", a_s.plainShotsMerchants ? 1 : 0) != 0;
			a_s.cameraMoves = ReadInt(a_path, L"Director", L"bCameraMoves", a_s.cameraMoves ? 1 : 0) != 0;
			for (auto& [key, value] : ReadSection(a_path, L"Shots")) {
				std::erase_if(a_s.shotOverrides, [&](const auto& a_p) { return a_p.first == key; });
				a_s.shotOverrides.emplace_back(std::move(key), std::move(value));
			}

			const auto mode = ReadInt(a_path, L"Camera", L"iShotMode", static_cast<std::int32_t>(a_s.shotMode));
			a_s.shotMode = static_cast<ShotMode>(std::clamp(mode, 0, 2));
			a_s.followSmoothing = std::clamp(ReadFloat(a_path, L"Camera", L"fFollowSmoothing", a_s.followSmoothing), 0.0f, 3.0f);
			a_s.busyFollowSmoothing = std::clamp(ReadFloat(a_path, L"Camera", L"fBusyFollowSmoothing", a_s.busyFollowSmoothing), 0.0f, 5.0f);
			a_s.busyHeadSpeed = std::clamp(ReadFloat(a_path, L"Camera", L"fBusyHeadSpeed", a_s.busyHeadSpeed), 0.02f, 5.0f);
			a_s.exitHoldSeconds = std::clamp(ReadFloat(a_path, L"Camera", L"fExitHoldSeconds", a_s.exitHoldSeconds), 0.0f, 5.0f);
			a_s.minSpeakerHeight = std::clamp(ReadFloat(a_path, L"Camera", L"fMinSpeakerHeight", a_s.minSpeakerHeight), 0.0f, 3.0f);
			a_s.gameCameraInScenes = ReadInt(a_path, L"Camera", L"bGameCameraInScenes", a_s.gameCameraInScenes ? 1 : 0) != 0;
			a_s.focusOnSubject = ReadInt(a_path, L"Camera", L"bFocusOnSubject", a_s.focusOnSubject ? 1 : 0) != 0;
			// sFocusStyle (0.5.1); bDeepFocusLongShots=1 from 0.5.0 INIs means Wide.
			if (ReadInt(a_path, L"Camera", L"bDeepFocusLongShots", -1) == 1) {
				a_s.focusStyle = "Wide";
			}
			a_s.focusStyle = ReadString(a_path, L"Camera", L"sFocusStyle", a_s.focusStyle);
			{
				std::string style = a_s.focusStyle;
				std::ranges::transform(style, style.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
				a_s.deepFocusLongShots = style == "wide";
			}
			a_s.deepFocusBeyond = std::clamp(ReadFloat(a_path, L"Camera", L"fDeepFocusBeyond", a_s.deepFocusBeyond), 0.0f, 20.0f);
			a_s.deepFocusBeyond = std::clamp(ReadFloat(a_path, L"Camera", L"fWideFocusBeyond", a_s.deepFocusBeyond), 0.0f, 20.0f);

			a_s.distanceBehind = ReadFloat(a_path, L"PlainCamera", L"fDistanceBehind", a_s.distanceBehind);
			a_s.sideOffset = ReadFloat(a_path, L"PlainCamera", L"fSideOffset", a_s.sideOffset);
			a_s.heightOffset = ReadFloat(a_path, L"PlainCamera", L"fHeightOffset", a_s.heightOffset);
			a_s.lookHeightOffset = ReadFloat(a_path, L"PlainCamera", L"fLookHeightOffset", a_s.lookHeightOffset);
			a_s.seatedForward = ReadFloat(a_path, L"PlainCamera", L"fSeatedForward", a_s.seatedForward);
			a_s.seatedSide = ReadFloat(a_path, L"PlainCamera", L"fSeatedSide", a_s.seatedSide);
			a_s.seatedHeight = ReadFloat(a_path, L"PlainCamera", L"fSeatedHeight", a_s.seatedHeight);
			a_s.vendorSide = ReadFloat(a_path, L"PlainCamera", L"fVendorSide", a_s.vendorSide);
			a_s.vendorHeight = ReadFloat(a_path, L"PlainCamera", L"fVendorHeight", a_s.vendorHeight);

			a_s.revealPlayer = ReadInt(a_path, L"Body", L"bRevealPlayer", a_s.revealPlayer ? 1 : 0) != 0;
			a_s.fixFirstPersonBody = ReadInt(a_path, L"Body", L"bFixFirstPersonBody", a_s.fixFirstPersonBody ? 1 : 0) != 0;
			a_s.bodyRebuildMethod = std::clamp(ReadInt(a_path, L"Body", L"iBodyRebuildMethod", a_s.bodyRebuildMethod), 0, 1);
			a_s.bodyUpdateFlags = std::clamp(ReadInt(a_path, L"Body", L"iBodyUpdateFlags", a_s.bodyUpdateFlags), 1, 0xFFFF);

			a_s.startMode = std::clamp(ReadInt(a_path, L"Transitions", L"iStartMode", a_s.startMode), 0, 2);
			a_s.startModeThird = std::clamp(ReadInt(a_path, L"Transitions", L"iStartModeThirdPerson", a_s.startModeThird), 0, 2);
			a_s.exitMode = std::clamp(ReadInt(a_path, L"Transitions", L"iExitMode", a_s.exitMode), 0, 2);

			a_s.stateHooks = ReadInt(a_path, L"Debug", L"bStateHooks", a_s.stateHooks ? 1 : 0) != 0;
			a_s.cameraTarget = std::clamp(ReadInt(a_path, L"Debug", L"iCameraTarget", a_s.cameraTarget), 0, 1);

			for (auto& [key, value] : ReadSection(a_path, L"GameSettings")) {
				std::erase_if(a_s.gameSettings, [&](const auto& a_p) { return a_p.first == key; });
				a_s.gameSettings.emplace_back(std::move(key), std::move(value));
			}
		}
	}

	Settings& Settings::Get()
	{
		static Settings instance;
		return instance;
	}

	void Settings::Load()
	{
		Settings fresh{};
		LoadFrom(fresh, kIniPath);
		LoadFrom(fresh, kUserIniPath);  // user overrides survive updates
		*this = fresh;
	}
}
