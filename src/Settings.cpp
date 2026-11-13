#include "Settings.h"

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
			a_s.showEmotions = ReadInt(a_path, L"General", L"bShowEmotions", a_s.showEmotions ? 1 : 0) != 0;
			a_s.showEmotions = ReadInt(a_path, L"Debug", L"bShowEmotions", a_s.showEmotions ? 1 : 0) != 0;

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
			a_s.emotionCloseUps = ReadInt(a_path, L"Director", L"bEmotionCloseUps", a_s.emotionCloseUps ? 1 : 0) != 0;
			a_s.closeUpEmotions = ReadString(a_path, L"Director", L"sCloseUpEmotions", a_s.closeUpEmotions);
			a_s.extremeCloseUpChance = std::clamp(ReadInt(a_path, L"Director", L"iExtremeCloseUpChance", a_s.extremeCloseUpChance), 0, 100);
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
