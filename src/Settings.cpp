#include "Settings.h"
#include <algorithm>

namespace CCC
{
	namespace
	{
		constexpr auto kIniPath = L".\\Data\\SFSE\\Plugins\\CinematicConversationCameraSF.ini";
		constexpr auto kUserIniPath = L".\\Data\\SFSE\\Plugins\\CinematicConversationCameraSF_user.ini";

		// A file's last write time (0 if it isn't there), to re-read the INI only after an edit.
		std::uint64_t WriteTime(const wchar_t* a_path)
		{
			WIN32_FILE_ATTRIBUTE_DATA data{};
			if (!::GetFileAttributesExW(a_path, GetFileExInfoStandard, &data)) {
				return 0;
			}
			return (static_cast<std::uint64_t>(data.ftLastWriteTime.dwHighDateTime) << 32) | data.ftLastWriteTime.dwLowDateTime;
		}
		std::uint64_t g_mainTime = ~0ull, g_userTime = ~0ull;  // as last read

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

		void LoadFrom(Settings& a_s, const wchar_t* a_path)
		{
			a_s.enabled = ReadInt(a_path, L"General", L"bEnabled", a_s.enabled ? 1 : 0) != 0;
			a_s.debugLog = ReadInt(a_path, L"General", L"bDebugLog", a_s.debugLog ? 1 : 0) != 0;
			// [General] since 1.0 (the settings page's General page, the user); [Debug] before.
			a_s.showMessages = ReadInt(a_path, L"General", L"bShowMessages", a_s.showMessages ? 1 : 0) != 0;
			a_s.presetKey = std::clamp(ReadInt(a_path, L"Debug", L"iPresetKey", a_s.presetKey), 0, 0xFE);
			a_s.messageSeconds = std::clamp(ReadFloat(a_path, L"Debug", L"fMessageSeconds", a_s.messageSeconds), 1.0f, 30.0f);
			a_s.filmConversations = ReadInt(a_path, L"SceneFilming", L"bFilmConversationsNpc", a_s.filmConversations ? 1 : 0) != 0;
			a_s.listenKey = std::clamp(ReadInt(a_path, L"SceneFilming", L"iListenKey", a_s.listenKey), 0, 254);
			a_s.listenRange = std::clamp(ReadFloat(a_path, L"SceneFilming", L"fListenRange", a_s.listenRange), 3.0f, 25.0f);
			a_s.listenButton = std::clamp(ReadInt(a_path, L"SceneFilming", L"iListenButton", a_s.listenButton), 0, 0xFFFF);
			a_s.listenHold = std::clamp(ReadFloat(a_path, L"Advanced", L"fListenHold", a_s.listenHold), 0.2f, 2.0f);
			a_s.listenPrompt = ReadInt(a_path, L"SceneFilming", L"bListenPrompt", a_s.listenPrompt ? 1 : 0) != 0;
			a_s.autoListen = ReadInt(a_path, L"SceneFilming", L"bAutoListen", a_s.autoListen ? 1 : 0) != 0;
			a_s.autoListenSprinting = ReadInt(a_path, L"SceneFilming", L"bAutoListenSprinting", a_s.autoListenSprinting ? 1 : 0) != 0;
			a_s.filmEndSeconds = std::clamp(ReadFloat(a_path, L"Advanced", L"fFilmEndSecondsNpc", a_s.filmEndSeconds), 0.1f, 10.0f);
			a_s.filmEndSecondsScene = std::clamp(ReadFloat(a_path, L"Advanced", L"fFilmEndSecondsSceneNpc", a_s.filmEndSecondsScene), a_s.filmEndSeconds, 15.0f);
			a_s.filmStopSeconds = std::clamp(ReadFloat(a_path, L"Advanced", L"fFilmStopSeconds", a_s.filmStopSeconds), 0.1f, 2.0f);
			a_s.filmStopDistance = std::clamp(ReadFloat(a_path, L"Advanced", L"fFilmStopDistance", a_s.filmStopDistance), 0.1f, 2.0f);
			a_s.filmWalkOnDistance = std::clamp(ReadFloat(a_path, L"Advanced", L"fFilmWalkOnDistance", a_s.filmWalkOnDistance), 0.5f, 10.0f);
			a_s.roomShotMinRoom = std::clamp(ReadInt(a_path, L"Advanced", L"iRoomShotMinRoom", a_s.roomShotMinRoom), 30, 100);
			a_s.roomShotMaxIndoors = std::clamp(ReadFloat(a_path, L"Advanced", L"fRoomShotMaxIndoors", a_s.roomShotMaxIndoors), -1.0f, 25.0f);
			a_s.roomShotMaxOutdoors = std::clamp(ReadFloat(a_path, L"Advanced", L"fRoomShotMaxOutdoors", a_s.roomShotMaxOutdoors), -1.0f, 25.0f);
			a_s.conversationLens = ReadFloat(a_path, L"Advanced", L"fConversationLens", a_s.conversationLens);
			// -1 (or anything below 10) = the preset's own lenses; 30 = the game's own.
			a_s.conversationLens = a_s.conversationLens < 10.0f ? -1.0f : std::clamp(a_s.conversationLens, 20.0f, 70.0f);
			a_s.playerFace = ReadInt(a_path, L"Director", L"bFacePlayer", a_s.playerFace ? 1 : 0) != 0;
			a_s.questionFaceChance = std::clamp(ReadInt(a_path, L"Advanced", L"iQuestionChancePlayer", a_s.questionFaceChance), 0, 100);
			a_s.signatureChance = std::clamp(ReadInt(a_path, L"Advanced", L"iSignatureChancePlayer", a_s.signatureChance), 0, 100);
			a_s.closeUpChanceMildPlayer = std::clamp(ReadInt(a_path, L"Advanced", L"iCloseUpChanceMildPlayer", a_s.closeUpChanceMildPlayer), 0, 100);
			a_s.closeUpChanceMediumPlayer = std::clamp(ReadInt(a_path, L"Advanced", L"iCloseUpChanceMediumPlayer", a_s.closeUpChanceMediumPlayer), 0, 100);
			a_s.closeUpChanceStrongPlayer = std::clamp(ReadInt(a_path, L"Advanced", L"iCloseUpChanceStrongPlayer", a_s.closeUpChanceStrongPlayer), 0, 100);
			a_s.letterbox = ReadInt(a_path, L"Interface", L"bLetterbox", a_s.letterbox ? 1 : 0) != 0;
			a_s.letterboxHeight = std::clamp(ReadFloat(a_path, L"Interface", L"fLetterboxHeight", a_s.letterboxHeight), 0.02f, 0.3f);
			a_s.letterboxFade = std::clamp(ReadFloat(a_path, L"Advanced", L"fLetterboxFade", a_s.letterboxFade), 0.0f, 3.0f);
			a_s.subtitlesInBars = ReadInt(a_path, L"Interface", L"bSubtitlesInBars", a_s.subtitlesInBars ? 1 : 0) != 0;
			a_s.hideOptionsWhileSpeaking = ReadInt(a_path, L"Interface", L"bHideOptionsWhileSpeaking", a_s.hideOptionsWhileSpeaking ? 1 : 0) != 0;
			a_s.optionsFade = std::clamp(ReadFloat(a_path, L"Advanced", L"fOptionsFade", a_s.optionsFade), 0.0f, 1.5f);

			a_s.director = ReadInt(a_path, L"Debug", L"bDirector", a_s.director ? 1 : 0) != 0;
			a_s.preset = ReadString(a_path, L"Director", L"sPreset", a_s.preset);
			// 0.9.8: the named cutting styles (sCuttingStyle) are gone; the cutting pace nudges the
			// preset's own rhythm instead of replacing it. The fine-tuning keys override it (0 =
			// the preset's own).
			// 0.9.26: fCuttingPace, in steps of 0.5 (iCuttingPace, whole steps, no longer read: 0.9.30).
			a_s.cuttingPace = ReadFloat(a_path, L"Director", L"fCuttingPace", a_s.cuttingPace);
			a_s.cuttingPace = std::clamp(std::round(a_s.cuttingPace * 2.0f) / 2.0f, -2.0f, 2.0f);
			for (std::size_t i = 0; i < Settings::kPresetNames.size(); ++i) {
				std::wstring key = L"fCuttingPace";
				for (const char* c = Settings::kPresetNames[i]; *c; ++c) {
					key.push_back(static_cast<wchar_t>(*c));
				}
				const float own = ReadFloat(a_path, L"Director", key.c_str(), a_s.cuttingPaces[i]);
				a_s.cuttingPaces[i] = std::isnan(own) ? own : std::clamp(std::round(own * 2.0f) / 2.0f, -2.0f, 2.0f);
			}
			a_s.minLines = std::clamp(ReadInt(a_path, L"Advanced", L"iMinLines", a_s.minLines), 0, 20);
			a_s.maxLines = std::clamp(ReadInt(a_path, L"Advanced", L"iMaxLines", a_s.maxLines), 0, 20);
			a_s.shortLineSeconds = std::clamp(ReadFloat(a_path, L"Advanced", L"fShortLineSeconds", a_s.shortLineSeconds), 0.0f, 10.0f);
			a_s.shortestHold = ReadFloat(a_path, L"Advanced", L"fShortestHold", a_s.shortestHold);
			a_s.shortestHold = a_s.shortestHold <= 0.0f ? 0.0f : std::clamp(a_s.shortestHold, 0.3f, 9.0f);
			// 0 = the preset's own cap; anything else is the player's limit, 1-15 m.
			const auto cap = [](float a_value) { return a_value <= 0.0f ? 0.0f : std::clamp(a_value, 1.0f, 15.0f); };
			a_s.maxCameraDistance = cap(ReadFloat(a_path, L"Advanced", L"fMaxCameraDistance", a_s.maxCameraDistance));
			a_s.maxCameraDistanceExterior = cap(ReadFloat(a_path, L"Advanced", L"fMaxCameraDistanceExterior", a_s.maxCameraDistanceExterior));
			a_s.obstacleCheck = ReadInt(a_path, L"Debug", L"bObstacleCheck", a_s.obstacleCheck ? 1 : 0) != 0;
			a_s.obstacleRadius = std::clamp(ReadFloat(a_path, L"Advanced", L"fObstacleRadius", a_s.obstacleRadius), 0.01f, 0.3f);
			a_s.true180 = ReadInt(a_path, L"Director", L"bTrue180", a_s.true180 ? 1 : 0) != 0;
			a_s.enforceLine = ReadInt(a_path, L"Advanced", L"bEnforceLine", a_s.enforceLine ? 1 : 0) != 0;
			a_s.followFace = ReadInt(a_path, L"Debug", L"bFollowFace", a_s.followFace ? 1 : 0) != 0;
			a_s.choiceCloseUps = std::clamp(ReadInt(a_path, L"Director", L"bChoiceCloseUpsPlayer", a_s.choiceCloseUps), -1, 1);
			a_s.choiceCloseUpSeconds = std::clamp(ReadFloat(a_path, L"Advanced", L"fChoiceCloseUpSecondsPlayer", a_s.choiceCloseUpSeconds), 0.5f, 6.0f);
			a_s.closeUpLimit = ReadFloat(a_path, L"Advanced", L"fCloseUpLimitSecondsNpc", a_s.closeUpLimit);
			a_s.closeUpLimit = a_s.closeUpLimit < 0.0f ? -1.0f : a_s.closeUpLimit == 0.0f ? 0.0f : std::clamp(a_s.closeUpLimit, 4.0f, 60.0f);
			a_s.listeningCloseUps = std::clamp(ReadInt(a_path, L"Advanced", L"bListeningCloseUpsPlayer", a_s.listeningCloseUps), -1, 1);
			a_s.reactionShots = std::clamp(ReadInt(a_path, L"Director", L"bReactionShotsPlayer", a_s.reactionShots), -1, 1);
			a_s.reactionChance = std::clamp(ReadInt(a_path, L"Advanced", L"iReactionChancePlayer", a_s.reactionChance), 0, 100);
			a_s.reactionAfterSeconds = ReadFloat(a_path, L"Advanced", L"fReactionAfterSecondsPlayer", a_s.reactionAfterSeconds);
			a_s.reactionAfterSeconds = a_s.reactionAfterSeconds < 0.0f ? -1.0f : std::clamp(a_s.reactionAfterSeconds, 2.0f, 120.0f);
			a_s.reactionLineMin = std::clamp(ReadFloat(a_path, L"Advanced", L"fReactionLineMinSecondsPlayer", a_s.reactionLineMin), 0.5f, 20.0f);
			a_s.reactionLineMax = std::clamp(ReadFloat(a_path, L"Advanced", L"fReactionLineMaxSecondsPlayer", a_s.reactionLineMax), a_s.reactionLineMin, 30.0f);
			a_s.reactionLongLineHold = std::clamp(ReadFloat(a_path, L"Advanced", L"fReactionLongLineSecondsPlayer", a_s.reactionLongLineHold), 0.0f, 30.0f);
			a_s.avoidBystanders = ReadInt(a_path, L"Debug", L"bAvoidBystandersNpc", a_s.avoidBystanders ? 1 : 0) != 0;
			a_s.bystanderStill = std::clamp(ReadFloat(a_path, L"Advanced", L"fBystanderStillNpc", a_s.bystanderStill), 0.0f, 10.0f);
			a_s.persuasionBeat = ReadInt(a_path, L"Director", L"bPersuasionBeatNpc", a_s.persuasionBeat ? 1 : 0) != 0;
			a_s.beatTags = ReadString(a_path, L"Advanced", L"sBeatTags", a_s.beatTags);
			a_s.faceDeadband = std::clamp(ReadFloat(a_path, L"Advanced", L"fFaceDeadband", a_s.faceDeadband), 0.0f, 0.2f);
			a_s.seatedSwayCap = std::clamp(ReadFloat(a_path, L"Advanced", L"fSeatedSwayCap", a_s.seatedSwayCap), 0.0f, 0.5f);
			a_s.cameraSway = std::clamp(ReadInt(a_path, L"Camera", L"iCameraSway", a_s.cameraSway), -1, 100);
			a_s.emotionCloseUps = std::clamp(ReadInt(a_path, L"Director", L"bEmotionCloseUpsNpc", a_s.emotionCloseUps), -1, 1);
			a_s.closeUpEmotionsMild = ReadString(a_path, L"Advanced", L"sCloseUpEmotionsMildNpc", a_s.closeUpEmotionsMild);
			a_s.closeUpEmotionsMedium = ReadString(a_path, L"Advanced", L"sCloseUpEmotionsMediumNpc", a_s.closeUpEmotionsMedium);
			a_s.closeUpEmotionsStrong = ReadString(a_path, L"Advanced", L"sCloseUpEmotionsStrongNpc", a_s.closeUpEmotionsStrong);
			a_s.extremeCloseUpChance = std::clamp(ReadInt(a_path, L"Advanced", L"iExtremeCloseUpChanceNpc", a_s.extremeCloseUpChance), -1, 100);
			a_s.closeUpChanceMild = std::clamp(ReadInt(a_path, L"Advanced", L"iCloseUpChanceMildNpc", a_s.closeUpChanceMild), 0, 100);
			a_s.closeUpChanceMedium = std::clamp(ReadInt(a_path, L"Advanced", L"iCloseUpChanceMediumNpc", a_s.closeUpChanceMedium), 0, 100);
			a_s.closeUpChanceStrong = std::clamp(ReadInt(a_path, L"Advanced", L"iCloseUpChanceStrongNpc", a_s.closeUpChanceStrong), 0, 100);
			// -1 was "the preset's own" in 0.9.19-0.9.20 only; a _user.ini that still has it keeps 40.
			if (const int extremePlayer = ReadInt(a_path, L"Advanced", L"iExtremeCloseUpChancePlayer", a_s.extremeCloseUpChancePlayer); extremePlayer >= 0) {
				a_s.extremeCloseUpChancePlayer = (std::min)(extremePlayer, 100);
			}
			a_s.plainShotsSeated = ReadInt(a_path, L"Advanced", L"bPlainShotsSeatedNpc", a_s.plainShotsSeated ? 1 : 0) != 0;
			a_s.plainShotsMerchants = ReadInt(a_path, L"Advanced", L"bPlainShotsMerchantsNpc", a_s.plainShotsMerchants ? 1 : 0) != 0;
			a_s.cameraMoves = ReadInt(a_path, L"Director", L"bCameraMoves", a_s.cameraMoves ? 1 : 0) != 0;
			for (auto& [key, value] : ReadSection(a_path, L"Shots")) {
				std::erase_if(a_s.shotOverrides, [&](const auto& a_p) { return a_p.first == key; });
				a_s.shotOverrides.emplace_back(std::move(key), std::move(value));
			}

			const auto mode = ReadInt(a_path, L"Advanced", L"iShotMode", static_cast<std::int32_t>(a_s.shotMode));
			a_s.shotMode = static_cast<ShotMode>(std::clamp(mode, 0, 2));
			a_s.followSmoothing = std::clamp(ReadFloat(a_path, L"Advanced", L"fFollowSmoothing", a_s.followSmoothing), 0.0f, 3.0f);
			a_s.busyFollowSmoothing = std::clamp(ReadFloat(a_path, L"Advanced", L"fBusyFollowSmoothing", a_s.busyFollowSmoothing), 0.0f, 5.0f);
			a_s.busyHeadSpeed = std::clamp(ReadFloat(a_path, L"Advanced", L"fBusyHeadSpeed", a_s.busyHeadSpeed), 0.02f, 5.0f);
			a_s.minSpeakerHeight = std::clamp(ReadFloat(a_path, L"Advanced", L"fMinSpeakerHeightNpc", a_s.minSpeakerHeight), 0.0f, 3.0f);
			a_s.largeSpeakerHeight = std::clamp(ReadFloat(a_path, L"Advanced", L"fLargeSpeakerHeightNpc", a_s.largeSpeakerHeight), 0.0f, 3.0f);
			a_s.largeSpeakerAngle = std::clamp(ReadFloat(a_path, L"Advanced", L"fLargeSpeakerAnglePlayer", a_s.largeSpeakerAngle), 0.0f, 60.0f);
			a_s.largeSpeakerPlainAngle = std::clamp(ReadFloat(a_path, L"Advanced", L"fLargeSpeakerPlainAnglePlayer", a_s.largeSpeakerPlainAngle), 0.0f, 60.0f);
			a_s.gameCameraInScenes = ReadInt(a_path, L"Camera", L"bGameCameraInScenes", a_s.gameCameraInScenes ? 1 : 0) != 0;
			a_s.focusOnSubject = ReadInt(a_path, L"Debug", L"bFocusOnSubject", a_s.focusOnSubject ? 1 : 0) != 0;
			// sFocusStyle (0.5.1).
			a_s.focusStyle = ReadString(a_path, L"Camera", L"sFocusStyle", a_s.focusStyle);
			{
				std::string style = a_s.focusStyle;
				std::ranges::transform(style, style.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
				a_s.deepFocusLongShots = style == "wide";
			}
			a_s.deepFocusBeyond = std::clamp(ReadFloat(a_path, L"Advanced", L"fWideFocusBeyond", a_s.deepFocusBeyond), 0.0f, 20.0f);

			a_s.distanceBehind = ReadFloat(a_path, L"Advanced", L"fDistanceBehind", a_s.distanceBehind);
			a_s.sideOffset = ReadFloat(a_path, L"Advanced", L"fSideOffset", a_s.sideOffset);
			a_s.heightOffset = ReadFloat(a_path, L"Advanced", L"fHeightOffset", a_s.heightOffset);
			a_s.lookHeightOffset = ReadFloat(a_path, L"Advanced", L"fLookHeightOffset", a_s.lookHeightOffset);
			a_s.seatedForward = ReadFloat(a_path, L"Advanced", L"fSeatedForward", a_s.seatedForward);
			a_s.seatedSide = ReadFloat(a_path, L"Advanced", L"fSeatedSide", a_s.seatedSide);
			a_s.seatedHeight = ReadFloat(a_path, L"Advanced", L"fSeatedHeight", a_s.seatedHeight);
			a_s.overSeatedBehind = ReadFloat(a_path, L"Advanced", L"fOverSeatedBehind", a_s.overSeatedBehind);
			a_s.overSeatedSide = ReadFloat(a_path, L"Advanced", L"fOverSeatedSide", a_s.overSeatedSide);
			a_s.overSeatedHeight = ReadFloat(a_path, L"Advanced", L"fOverSeatedHeight", a_s.overSeatedHeight);
			a_s.vendorSide = ReadFloat(a_path, L"Advanced", L"fVendorSideNpc", a_s.vendorSide);
			a_s.vendorHeight = ReadFloat(a_path, L"Advanced", L"fVendorHeightNpc", a_s.vendorHeight);

			a_s.revealPlayer = ReadInt(a_path, L"Debug", L"bRevealPlayer", a_s.revealPlayer ? 1 : 0) != 0;
			a_s.fixFirstPersonBody = ReadInt(a_path, L"Debug", L"bFixFirstPersonBodyPlayer", a_s.fixFirstPersonBody ? 1 : 0) != 0;

			a_s.speakerChangeRoot = ReadInt(a_path, L"Advanced", L"bSpeakerChangeRoot", a_s.speakerChangeRoot ? 1 : 0) != 0;
			a_s.exitStyle = std::clamp(ReadInt(a_path, L"Advanced", L"iExitStyle", a_s.exitStyle), 1, 2);
			a_s.exitCutSeconds = std::clamp(ReadFloat(a_path, L"Advanced", L"fExitCutSeconds", a_s.exitCutSeconds), 0.0f, 1.0f);

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

	namespace
	{
		std::string g_presetOverride;
	}

	void Settings::SetPresetOverride(const std::string& a_preset)
	{
		g_presetOverride = a_preset;
		Get().preset = a_preset.empty() ? Get().preset : a_preset;
	}

	void Settings::Load()
	{
		g_mainTime = WriteTime(kIniPath);
		g_userTime = WriteTime(kUserIniPath);
		Settings fresh{};
		LoadFrom(fresh, kIniPath);
		LoadFrom(fresh, kUserIniPath);  // user overrides survive updates
		if (!g_presetOverride.empty()) {
			fresh.preset = g_presetOverride;
		}
		*this = fresh;
	}

	bool Settings::LoadIfChanged()
	{
		if (WriteTime(kIniPath) == g_mainTime && WriteTime(kUserIniPath) == g_userTime) {
			return false;
		}
		Load();
		return true;
	}
}
