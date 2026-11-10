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

		void LoadFrom(Settings& a_s, const wchar_t* a_path)
		{
			a_s.enabled = ReadInt(a_path, L"General", L"bEnabled", a_s.enabled ? 1 : 0) != 0;
			a_s.debugLog = ReadInt(a_path, L"General", L"bDebugLog", a_s.debugLog ? 1 : 0) != 0;

			const auto mode = ReadInt(a_path, L"Camera", L"iShotMode", static_cast<std::int32_t>(a_s.shotMode));
			a_s.shotMode = static_cast<ShotMode>(std::clamp(mode, 0, 2));

			a_s.distanceBehind = ReadFloat(a_path, L"Camera", L"fDistanceBehind", a_s.distanceBehind);
			a_s.sideOffset = ReadFloat(a_path, L"Camera", L"fSideOffset", a_s.sideOffset);
			a_s.heightOffset = ReadFloat(a_path, L"Camera", L"fHeightOffset", a_s.heightOffset);
			a_s.lookHeightOffset = ReadFloat(a_path, L"Camera", L"fLookHeightOffset", a_s.lookHeightOffset);
			a_s.blendSeconds = (std::max)(0.0f, ReadFloat(a_path, L"Camera", L"fBlendSeconds", a_s.blendSeconds));

			a_s.shotStyle = std::clamp(ReadInt(a_path, L"Camera", L"iShotStyle", a_s.shotStyle), 0, 1);
			a_s.seatedForward = ReadFloat(a_path, L"Camera", L"fSeatedForward", a_s.seatedForward);
			a_s.seatedSide = ReadFloat(a_path, L"Camera", L"fSeatedSide", a_s.seatedSide);
			a_s.seatedHeight = ReadFloat(a_path, L"Camera", L"fSeatedHeight", a_s.seatedHeight);
			a_s.exitHoldSeconds = std::clamp(ReadFloat(a_path, L"Camera", L"fExitHoldSeconds", a_s.exitHoldSeconds), 0.0f, 5.0f);
			a_s.followSmoothing = std::clamp(ReadFloat(a_path, L"Camera", L"fFollowSmoothing", a_s.followSmoothing), 0.0f, 3.0f);
			a_s.cameraTarget = std::clamp(ReadInt(a_path, L"Camera", L"iCameraTarget", a_s.cameraTarget), 0, 1);
			a_s.revealPlayer = ReadInt(a_path, L"Camera", L"bRevealPlayer", a_s.revealPlayer ? 1 : 0) != 0;
			a_s.fixFirstPersonBody = ReadInt(a_path, L"Camera", L"bFixFirstPersonBody", a_s.fixFirstPersonBody ? 1 : 0) != 0;

			a_s.poseMode = std::clamp(ReadInt(a_path, L"Debug", L"iPoseMode", a_s.poseMode), 0, 2);
		}
	}

	Settings& Settings::Get()
	{
		static Settings instance;
		return instance;
	}

	void Settings::Load()
	{
		*this = Settings{};
		LoadFrom(*this, kIniPath);
		LoadFrom(*this, kUserIniPath);  // user overrides survive updates
	}
}
