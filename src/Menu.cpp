// Cinematic Conversation Camera SF: the in-game settings page (SFSE Menu Framework).
#include "Menu.h"

#include "Settings.h"

#include <ranges>
#include <span>

#include <SFSEMCP/SFSEMenuFramework.hpp>

namespace CCC::Menu
{
	namespace
	{
		constexpr auto kIniPath = L".\\Data\\SFSE\\Plugins\\CinematicConversationCameraSF.ini";
		constexpr auto kUserIniPath = L".\\Data\\SFSE\\Plugins\\CinematicConversationCameraSF_user.ini";

		enum class Kind
		{
			kBool,      // 0/1
			kTri,       // -1 the preset's own, 0 off, 1 on
			kInt,       // a range
			kFloat,     // a range
			kOwnInt,    // -1 the preset's own, otherwise a range
			kOwnFloat,  // -1 (or below the range) the preset's own, otherwise a range
			kZeroFloat, // 0 the preset's own, otherwise a range
			kChoice     // one of a list of words
		};

		struct Entry
		{
			const char*  page;
			const char*  group;     // a heading within the page
			const char*  section;   // INI section
			const char*  key;       // INI key
			const char*  label;
			Kind         kind;
			float        min{ 0.0f };
			float        max{ 1.0f };
			const char*  format{ nullptr };
			const char*  help{ nullptr };
			std::span<const char* const> choices{};
		};

		constexpr const char* kPresets[]{ "Standard", "Close", "Room", "Frontier", "Starborn", "Constellation", "Nebula", "NebulaTest" };
		constexpr const char* kFocusStyles[]{ "Wide", "Cinematic" };

		// The player-facing settings. Test switches, transitions and the plain camera's offsets
		// stay INI-only.
		const Entry kEntries[]{
			// ---- Camera ----
			{ "Camera", "The mod", "General", "bEnabled", "Cinematic conversations", Kind::kBool, 0, 1, nullptr,
				"Off: the whole mod stands down and conversations use the game's own camera." },
			{ "Camera", "Look", "Director", "sPreset", "Preset", Kind::kChoice, 0, 0, nullptr,
				"Standard, Close and Room are CCC's own. Frontier (a space western), Starborn (Kubrick and Interstellar), "
				"Constellation (a hard sci-fi documentary) and Nebula (a space opera) are CCC SF's. NebulaTest is for testing only.",
				kPresets },
			{ "Camera", "Look", "Director", "iCuttingPace", "Cutting pace", Kind::kInt, -2, 2, "%+d",
				"How lively the cutting is, relative to the preset's own rhythm: -2 calmest, 0 the preset as designed, +2 liveliest." },
			{ "Camera", "Look", "Camera", "iCameraSway", "Camera sway", Kind::kOwnInt, 0, 100, "%d%%",
				"0 a tripod, 100 fully handheld." },
			{ "Camera", "Look", "Camera", "fConversationLens", "Lens (degrees)", Kind::kOwnFloat, 20, 70, "%.0f",
				"One lens for every angle instead of each angle's own. 30 is the game's own." },
			{ "Camera", "Look", "Director", "bCameraMoves", "Camera moves", Kind::kBool, 0, 1, nullptr,
				"Push-ins, drifts, cranes and the like, as each preset has them." },
			{ "Camera", "Look", "Director", "bFollowFace", "Close-ups follow the face", Kind::kBool, 0, 1, nullptr, nullptr },
			{ "Camera", "Distance", "Director", "fMaxCameraDistance", "Furthest indoors (m)", Kind::kZeroFloat, 1, 15, "%.1f",
				"How far the camera may stand from the person it frames, indoors." },
			{ "Camera", "Distance", "Director", "fMaxCameraDistanceExterior", "Furthest outdoors (m)", Kind::kZeroFloat, 1, 15, "%.1f",
				"How far the camera may stand from the person it frames, outdoors." },
			{ "Camera", "Staging", "Director", "bTrue180", "180-degree rule", Kind::kBool, 0, 1, nullptr,
				"Keep the camera on one side of the line between you. Off: each person is shot over their own side." },
			{ "Camera", "Staging", "Director", "bEnforceLine", "Never across the eyeline", Kind::kBool, 0, 1, nullptr, nullptr },
			{ "Camera", "Staging", "Director", "bObstacleCheck", "Avoid walls and objects", Kind::kBool, 0, 1, nullptr, nullptr },
			{ "Camera", "Staging", "Director", "bAvoidBystandersNpc", "Avoid people in the way", Kind::kBool, 0, 1, nullptr, nullptr },
			{ "Camera", "Staging", "Director", "bPlainShotsSeatedNpc", "Plain shot for seated people", Kind::kBool, 0, 1, nullptr,
				"An over-the-shoulder shot instead of the shot library, whose angles often put the chair in the way." },
			{ "Camera", "Staging", "Director", "bPlainShotsMerchantsNpc", "Plain shot for merchants", Kind::kBool, 0, 1, nullptr,
				"An over-the-shoulder shot instead of the shot library, whose angles often put the counter in the way." },
			{ "Camera", "Staging", "Camera", "bGameCameraInScenes", "Game's camera in quest scenes", Kind::kBool, 0, 1, nullptr, nullptr },
			{ "Camera", "Staging", "Camera", "fLargeSpeakerHeightNpc", "Large speaker from (m)", Kind::kFloat, 0.0f, 2.5f, "%.2f",
				"Head this high above their feet (Vasco 1.75, people about 1.6): shots of you keep wider so their body doesn't hide you. 0 = off." },
			{ "Camera", "Staging", "Camera", "fLargeSpeakerAnglePlayer", "Large speaker: shots of you from (degrees)", Kind::kFloat, 0.0f, 60.0f, "%.0f",
				"How far off the line between you the shots of you stay with a large speaker." },
			{ "Camera", "Staging", "Camera", "fLargeSpeakerPlainAnglePlayer", "Large speaker: plain shot of you (degrees)", Kind::kFloat, 0.0f, 60.0f, "%.0f",
				"The plain over-the-shoulder shot of you, swung round this far with a large speaker (standing close, 38 still showed part of Vasco)." },
			{ "Camera", "Focus", "Camera", "bFocusOnSubject", "Focus on the person in shot", Kind::kBool, 0, 1, nullptr, nullptr },
			{ "Camera", "Focus", "Camera", "sFocusStyle", "Focus style", Kind::kChoice, 0, 0, nullptr,
				"Wide: long shots keep a deep sharp zone. Cinematic: a shallow one everywhere.", kFocusStyles },

			// ---- Close-ups and reactions ----
			{ "Close-ups and reactions", "Their close-ups", "Director", "bEmotionCloseUpsNpc", "Emotion close-ups", Kind::kTri, 0, 0, nullptr,
				"A close-up of the speaker on lines with a strong expression." },
			{ "Close-ups and reactions", "Their close-ups", "Director", "iCloseUpChanceMildNpc", "Mild lines", Kind::kInt, 0, 100, "%d%%", nullptr },
			{ "Close-ups and reactions", "Their close-ups", "Director", "iCloseUpChanceMediumNpc", "Medium lines", Kind::kInt, 0, 100, "%d%%", nullptr },
			{ "Close-ups and reactions", "Their close-ups", "Director", "iCloseUpChanceStrongNpc", "Strong lines", Kind::kInt, 0, 100, "%d%%", nullptr },
			{ "Close-ups and reactions", "Their close-ups", "Director", "iExtremeCloseUpChanceNpc", "Extreme close-ups", Kind::kOwnInt, 0, 100, "%d%%",
				"Of those close-ups, how many are extreme. The preset's own: Frontier 20%, the others 40%." },
			{ "Close-ups and reactions", "Their close-ups", "Director", "fCloseUpLimitSecondsNpc", "Longest close-up (s)", Kind::kOwnFloat, 4.0f, 60.0f, "%.0f",
				"One close-up of them at most this long, then a wider shot of them. The preset's own: Frontier 12, Starborn 18, Constellation 14, Nebula 12, CCC's none." },
			{ "Close-ups and reactions", "Persuasion", "Director", "bPersuasionBeatNpc", "Hold on their answer", Kind::kBool, 0, 1, nullptr,
				"A close-up of the NPC's answer to every persuasion round." },
			{ "Close-ups and reactions", "You", "Director", "bReactionShotsPlayer", "Reaction shots of you", Kind::kTri, 0, 0, nullptr,
				"Cutting to you listening during their longer lines." },
			{ "Close-ups and reactions", "You", "Director", "iReactionChancePlayer", "Reaction chance", Kind::kInt, 0, 100, "%d%%", nullptr },
			{ "Close-ups and reactions", "You", "Director", "bListeningCloseUpsPlayer", "Listening reactions in close-up", Kind::kTri, 0, 0, nullptr,
				"Reactions of you listening may be close-ups. The preset's own: yes, except Frontier (mediums and three-quarters)." },
			{ "Close-ups and reactions", "You", "Director", "fChoiceCloseUpSecondsPlayer", "Skill-choice close-up (s)", Kind::kFloat, 0.5f, 6.0f, "%.1f",
				"How long the close-up of you lasts as you pick a tagged option, over the start of their answer." },
			{ "Close-ups and reactions", "You", "Director", "bChoiceCloseUpsPlayer", "Close-up on skill choices", Kind::kTri, 0, 0, nullptr,
				"A close-up of you when you pick a [Persuade], [Intimidate] or similar option." },
			{ "Close-ups and reactions", "You", "Director", "iExtremeCloseUpChancePlayer", "Extreme close-ups of you", Kind::kInt, 0, 100, "%d%%", nullptr },

			// ---- You ----
			{ "You", "Your character", "Body", "bRevealPlayer", "Show your character", Kind::kBool, 0, 1, nullptr,
				"Starfield hides you in conversations." },
			{ "You", "Your character", "Body", "bFixFirstPersonBodyPlayer", "Full body from first person", Kind::kBool, 0, 1, nullptr,
				"Build your full body when a conversation starts in first person (otherwise the first-person body shows)." },
			{ "You", "Your face", "Face", "bFacePlayer", "Your expressions", Kind::kBool, 0, 1, nullptr,
				"Your face answers the NPC's lines and your choices." },
			{ "You", "Your face", "Face", "iSignatureChancePlayer", "Your background's expression", Kind::kInt, 0, 100, "%d%%",
				"At moments your face would be neutral, how often it shows your background's expression instead." },
			{ "You", "Your face", "Face", "iQuestionChancePlayer", "Asking a question", Kind::kInt, 0, 100, "%d%%", nullptr },

			// ---- Screen ----
			{ "Screen", "Letterbox", "Interface", "bLetterbox", "Letterbox bars", Kind::kBool, 0, 1, nullptr, nullptr },
			{ "Screen", "Letterbox", "Interface", "fLetterboxHeight", "Bar height", Kind::kFloat, 0.02f, 0.3f, "%.2f", "A share of the screen's height, each bar." },
			{ "Screen", "Letterbox", "Interface", "fLetterboxFade", "Fade (s)", Kind::kFloat, 0.0f, 3.0f, "%.1f", nullptr },
			{ "Screen", "Letterbox", "Interface", "bSubtitlesInBars", "Subtitles in the bottom bar", Kind::kBool, 0, 1, nullptr, nullptr },
			{ "Screen", "Dialogue options", "Interface", "bHideOptionsWhileSpeaking", "Hide while they speak", Kind::kBool, 0, 1, nullptr, nullptr },
			{ "Screen", "Dialogue options", "Interface", "fOptionsFade", "Fade (s)", Kind::kFloat, 0.0f, 1.5f, "%.1f", "0 = no fade." },
			{ "Screen", "Testing", "Debug", "bShowMessages", "Testing messages", Kind::kBool, 0, 1, nullptr,
				"The shot and the line's expression in the top bar." },

			// ---- Filming people talking ----
			{ "Filming people talking", "People talking near you", "Overhear", "bFilmConversationsNpc", "Film them", Kind::kBool, 0, 1, nullptr, nullptr },
			{ "Filming people talking", "People talking near you", "Overhear", "bAutoListen", "Start on its own", Kind::kBool, 0, 1, nullptr,
				"When you stop near a conversation; otherwise with the listen key." },
			{ "Filming people talking", "People talking near you", "Overhear", "bListenPrompt", "Listen prompt", Kind::kBool, 0, 1, nullptr, nullptr },
			{ "Filming people talking", "People talking near you", "Overhear", "fListenRange", "Range (m)", Kind::kFloat, 3, 25, "%.0f", nullptr },
			{ "Filming people talking", "Ending it by moving", "Overhear", "fFilmStopDistance", "Distance that ends it (m)", Kind::kFloat, 0.1f, 2.0f, "%.2f",
				"Once you've stood still, moving this far from that spot ends the filming." },
			{ "Filming people talking", "Ending it by moving", "Overhear", "fFilmStopSeconds", "Standing still first (s)", Kind::kFloat, 0.1f, 2.0f, "%.2f",
				"How long you stand still before the distance above applies." },
			{ "Filming people talking", "Ending it by moving", "Overhear", "fFilmWalkOnDistance", "Walking on before you stop (m)", Kind::kFloat, 0.5f, 10.0f, "%.1f",
				"If filming starts as you walk up, walking on this far from where it started ends it (time to let go of the stick)." },
		};

		// ---- Reading and writing the INI ----------------------------------------------------

		std::wstring Wide(const char* a_text)
		{
			std::wstring out;
			for (const char* c = a_text; *c; ++c) {
				out.push_back(static_cast<wchar_t>(static_cast<unsigned char>(*c)));
			}
			return out;
		}

		std::string Narrow(const wchar_t* a_text)
		{
			std::string out;
			for (const wchar_t* c = a_text; *c; ++c) {
				out.push_back(*c < 0x80 ? static_cast<char>(*c) : '?');
			}
			return out;
		}

		std::optional<std::string> ReadRaw(const wchar_t* a_path, const Entry& a_entry)
		{
			wchar_t buffer[512]{};
			const auto section = Wide(a_entry.section);
			const auto key = Wide(a_entry.key);
			::GetPrivateProfileStringW(section.c_str(), key.c_str(), L"\x01", buffer, 511, a_path);
			if (buffer[0] == L'\x01') {
				return std::nullopt;
			}
			std::string value = Narrow(buffer);
			if (const auto comment = value.find(';'); comment != std::string::npos) {
				value.erase(comment);
			}
			while (!value.empty() && (value.back() == ' ' || value.back() == '\t')) {
				value.pop_back();
			}
			return value;
		}

		struct Value
		{
			std::string text;      // as in the INI
			bool        mine{ false };  // set in the _user.ini
		};

		constexpr std::size_t kCount = std::extent_v<decltype(kEntries)>;
		std::array<Value, kCount>              g_values;
		std::chrono::steady_clock::time_point  g_readAt{};
		std::string                            g_status;

		void ReadAll()
		{
			for (std::size_t i = 0; i < kCount; ++i) {
				const auto& e = kEntries[i];
				if (auto mine = ReadRaw(kUserIniPath, e)) {
					g_values[i] = { *mine, true };
				} else {
					g_values[i] = { ReadRaw(kIniPath, e).value_or(""), false };
				}
			}
			g_readAt = std::chrono::steady_clock::now();
		}

		void Write(std::size_t a_index, const std::string& a_text)
		{
			const auto& e = kEntries[a_index];
			const auto  section = Wide(e.section);
			const auto  key = Wide(e.key);
			const auto  text = Wide(a_text.c_str());
			if (::WritePrivateProfileStringW(section.c_str(), key.c_str(), text.c_str(), kUserIniPath)) {
				g_values[a_index] = { a_text, true };
				g_status = std::format("Saved {}={} to your _user.ini.", e.key, a_text);
				if (std::string_view{ e.key } == "sPreset") {
					Settings::SetPresetOverride("");  // the preset key's choice gives way to this one
				}
			} else {
				g_status = std::format("Couldn't write {} to CinematicConversationCameraSF_user.ini (Data\\SFSE\\Plugins).", e.key);
			}
		}

		void Reset(std::size_t a_index)
		{
			const auto& e = kEntries[a_index];
			const auto  section = Wide(e.section);
			const auto  key = Wide(e.key);
			::WritePrivateProfileStringW(section.c_str(), key.c_str(), nullptr, kUserIniPath);  // removes the line
			g_values[a_index] = { ReadRaw(kIniPath, e).value_or(""), false };
			g_status = std::format("{} back to the INI's value.", e.key);
		}

		int AsInt(const std::string& a_text, int a_default)
		{
			try {
				return a_text.empty() ? a_default : std::stoi(a_text);
			} catch (...) {
				return a_default;
			}
		}

		float AsFloat(const std::string& a_text, float a_default)
		{
			try {
				return a_text.empty() ? a_default : std::stof(a_text);
			} catch (...) {
				return a_default;
			}
		}

		// ---- Drawing ------------------------------------------------------------------------

		void Help(const Entry& a_entry)
		{
			if (a_entry.help) {
				ImGuiMCP::SetItemTooltip("%s", a_entry.help);
			}
		}

		// The widget for one setting; a change is written straight away (sliders on release).
		void DrawEntry(std::size_t a_index)
		{
			const auto& e = kEntries[a_index];
			auto&       v = g_values[a_index];
			ImGuiMCP::PushID(e.key);
			switch (e.kind) {
			case Kind::kBool:
				{
					bool on = AsInt(v.text, 0) != 0;
					if (ImGuiMCP::Checkbox(e.label, &on)) {
						Write(a_index, on ? "1" : "0");
					}
					Help(e);
					break;
				}
			case Kind::kTri:
				{
					constexpr const char* kTri[]{ "Preset's own", "Off", "On" };
					int pick = std::clamp(AsInt(v.text, -1), -1, 1) + 1;
					if (ImGuiMCP::Combo(e.label, &pick, kTri, 3)) {
						Write(a_index, std::to_string(pick - 1));
					}
					Help(e);
					break;
				}
			case Kind::kInt:
				{
					int value = std::clamp(AsInt(v.text, static_cast<int>(e.min)), static_cast<int>(e.min), static_cast<int>(e.max));
					ImGuiMCP::SliderInt(e.label, &value, static_cast<int>(e.min), static_cast<int>(e.max), e.format ? e.format : "%d");
					Help(e);
					if (ImGuiMCP::IsItemDeactivatedAfterEdit()) {
						Write(a_index, std::to_string(value));
					} else if (ImGuiMCP::IsItemActive()) {
						v.text = std::to_string(value);  // follows the drag; written on release
					}
					break;
				}
			case Kind::kFloat:
				{
					float value = std::clamp(AsFloat(v.text, e.min), e.min, e.max);
					ImGuiMCP::SliderFloat(e.label, &value, e.min, e.max, e.format ? e.format : "%.2f");
					Help(e);
					if (ImGuiMCP::IsItemDeactivatedAfterEdit()) {
						Write(a_index, std::format("{:.2f}", value));
					} else if (ImGuiMCP::IsItemActive()) {
						v.text = std::format("{:.2f}", value);
					}
					break;
				}
			case Kind::kOwnInt:
			case Kind::kOwnFloat:
			case Kind::kZeroFloat:
				{
					const bool isInt = e.kind == Kind::kOwnInt;
					const float raw = AsFloat(v.text, e.kind == Kind::kZeroFloat ? 0.0f : -1.0f);
					bool own = e.kind == Kind::kZeroFloat ? raw <= 0.0f : raw < e.min;
					const auto ownLabel = std::format("{}: the preset's own", e.label);
					if (ImGuiMCP::Checkbox(ownLabel.c_str(), &own)) {
						if (own) {
							Write(a_index, e.kind == Kind::kZeroFloat ? "0" : "-1");
						} else {
							const float start = e.min + (e.max - e.min) * 0.5f;
							Write(a_index, isInt ? std::to_string(static_cast<int>(start)) : std::format("{:.1f}", start));
						}
					}
					Help(e);
					if (!own) {
						if (isInt) {
							int value = std::clamp(static_cast<int>(raw), static_cast<int>(e.min), static_cast<int>(e.max));
							ImGuiMCP::SliderInt(e.label, &value, static_cast<int>(e.min), static_cast<int>(e.max), e.format ? e.format : "%d");
							if (ImGuiMCP::IsItemDeactivatedAfterEdit()) {
								Write(a_index, std::to_string(value));
							} else if (ImGuiMCP::IsItemActive()) {
								v.text = std::to_string(value);
							}
						} else {
							float value = std::clamp(raw, e.min, e.max);
							ImGuiMCP::SliderFloat(e.label, &value, e.min, e.max, e.format ? e.format : "%.1f");
							if (ImGuiMCP::IsItemDeactivatedAfterEdit()) {
								Write(a_index, std::format("{:.1f}", value));
							} else if (ImGuiMCP::IsItemActive()) {
								v.text = std::format("{:.1f}", value);
							}
						}
					}
					break;
				}
			case Kind::kChoice:
				{
					int pick = 0;
					for (std::size_t c = 0; c < e.choices.size(); ++c) {
						std::string a{ e.choices[c] }, b{ v.text };
						std::ranges::transform(a, a.begin(), [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
						std::ranges::transform(b, b.begin(), [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
						if (a == b) {
							pick = static_cast<int>(c);
						}
					}
					if (ImGuiMCP::Combo(e.label, &pick, e.choices.data(), static_cast<int>(e.choices.size()))) {
						Write(a_index, e.choices[static_cast<std::size_t>(pick)]);
					}
					Help(e);
					break;
				}
			}
			if (v.mine) {
				ImGuiMCP::SameLine();
				if (ImGuiMCP::SmallButton("Reset")) {
					Reset(a_index);
				}
				ImGuiMCP::SetItemTooltip("%s", "Back to the INI's value (removes your change from the _user.ini).");
			}
			ImGuiMCP::PopID();
		}

		void DrawPage(const char* a_page)
		{
			// Re-read every couple of seconds, so edits made to the files by hand show up too.
			if (std::chrono::steady_clock::now() - g_readAt > std::chrono::seconds(2) && !ImGuiMCP::IsAnyItemActive()) {
				ReadAll();
			}
			const char* group = nullptr;
			for (std::size_t i = 0; i < kCount; ++i) {
				const auto& e = kEntries[i];
				if (std::string_view{ e.page } != a_page) {
					continue;
				}
				if (!group || std::string_view{ group } != e.group) {
					group = e.group;
					ImGuiMCP::SeparatorText(group);
				}
				DrawEntry(i);
			}
			ImGuiMCP::Spacing();
			ImGuiMCP::Separator();
			ImGuiMCP::TextDisabled("%s", "Changes go to CinematicConversationCameraSF_user.ini and apply from the next conversation.");
			if (!g_status.empty()) {
				ImGuiMCP::TextDisabled("%s", g_status.c_str());
			}
		}

		void __stdcall DrawCamera() { DrawPage("Camera"); }
		void __stdcall DrawCloseUps() { DrawPage("Close-ups and reactions"); }
		void __stdcall DrawYou() { DrawPage("You"); }
		void __stdcall DrawScreen() { DrawPage("Screen"); }
		void __stdcall DrawFilming() { DrawPage("Filming people talking"); }
	}

	bool IsOpen()
	{
		static const bool installed = SFSEMenuFramework::IsInstalled();
		return installed && SFSEMenuFramework::IsAnyBlockingWindowOpen();
	}

	void Register()
	{
		if (!SFSEMenuFramework::IsInstalled()) {
			REX::INFO("Settings page: SFSE Menu Framework isn't installed; the INI is the only way to change settings");
			return;
		}
		ReadAll();
		SFSEMenuFramework::SetSection("Cinematic Conversation Camera SF");
		SFSEMenuFramework::AddSectionItem("Camera", &DrawCamera);
		SFSEMenuFramework::AddSectionItem("Close-ups and reactions", &DrawCloseUps);
		SFSEMenuFramework::AddSectionItem("You", &DrawYou);
		SFSEMenuFramework::AddSectionItem("Screen", &DrawScreen);
		SFSEMenuFramework::AddSectionItem("Filming people talking", &DrawFilming);
		REX::INFO("Settings page: added to SFSE Menu Framework ({} settings)", kCount);
	}
}
