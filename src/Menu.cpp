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

		constexpr const char* kPresets[]{ "Standard", "Close", "Room", "Frontier", "Starborn", "Constellation", "Nebula" };
		constexpr const char* kFocusStyles[]{ "Wide", "Cinematic" };

		// The player-facing settings. Test switches, transitions and the plain camera's offsets
		// stay INI-only.
		const Entry kEntries[]{
			// ---- Camera ----
			{ "Camera", "The mod", "General", "bEnabled", "Cinematic conversations", Kind::kBool, 0, 1, nullptr,
				"Off: the whole mod stands down and conversations use the game's own camera." },
			{ "Camera", "Look", "Director", "sPreset", "Preset", Kind::kChoice, 0, 0, nullptr,
				"Standard, Close and Room are CCC's own. Frontier (a space western), Starborn (Kubrick and Interstellar), "
				"Constellation (a hard sci-fi documentary) and Nebula (a space opera) are CCC SF's.",
				kPresets },
			{ "Camera", "Look", "Director", "fCuttingPace", "Cutting pace", Kind::kFloat, -2.0f, 2.0f, "%+.1f",
				"How lively the cutting is, relative to the preset's own rhythm: -2 calmest, 0 the preset as designed, +2 liveliest. Each preset keeps its own: this changes the pace of the preset in use. Mostly felt in long speeches, as the camera cuts on each turn whatever the pace." },
			{ "Camera", "Look", "Director", "bCameraMoves", "Camera moves", Kind::kBool, 0, 1, nullptr,
				"Push-ins, drifts, cranes and the like, as each preset has them." },
			{ "Camera", "Look", "Director", "bFollowFace", "Close-ups follow the face", Kind::kBool, 0, 1, nullptr, nullptr },
			{ "Camera", "Staging", "Director", "bTrue180", "180-degree rule", Kind::kBool, 0, 1, nullptr,
				"Keep the camera on one side of the line between you. Off: each person is shot over their own side." },
			{ "Camera", "Staging", "Director", "bObstacleCheck", "Avoid walls and objects", Kind::kBool, 0, 1, nullptr, nullptr },
			{ "Camera", "Staging", "Director", "bAvoidBystandersNpc", "Avoid people in the way", Kind::kBool, 0, 1, nullptr, nullptr },
			{ "Camera", "Staging", "Camera", "bGameCameraInScenes", "Game's camera in quest scenes", Kind::kBool, 0, 1, nullptr, nullptr },
			{ "Camera", "Focus", "Camera", "bFocusOnSubject", "Focus on the person in shot", Kind::kBool, 0, 1, nullptr, nullptr },
			{ "Camera", "Focus", "Camera", "sFocusStyle", "Focus style", Kind::kChoice, 0, 0, nullptr,
				"Wide: long shots keep a deep sharp zone. Cinematic: a shallow one everywhere.", kFocusStyles },

			// ---- Close-ups and reactions ----
			{ "Close-ups and reactions", "Their close-ups", "Director", "bEmotionCloseUpsNpc", "Emotion close-ups", Kind::kTri, 0, 0, nullptr,
				"A close-up of the speaker on lines with an expression: always on strong ones, now and then on milder ones (how often: in the INI)." },
			{ "Close-ups and reactions", "Persuasion", "Director", "bPersuasionBeatNpc", "Hold on their answer", Kind::kBool, 0, 1, nullptr,
				"A close-up of the NPC's answer to every persuasion round." },
			{ "Close-ups and reactions", "You", "Director", "bReactionShotsPlayer", "Reaction shots of you", Kind::kTri, 0, 0, nullptr,
				"Cutting to you listening during their longer lines." },
			{ "Close-ups and reactions", "You", "Director", "bChoiceCloseUpsPlayer", "Close-up on skill choices", Kind::kTri, 0, 0, nullptr,
				"A close-up of you when you pick a [Persuade], [Intimidate] or similar option." },

			// ---- You ----
			{ "You", "Your character", "Body", "bRevealPlayer", "Show your character", Kind::kBool, 0, 1, nullptr,
				"Starfield hides you in conversations." },
			{ "You", "Your face", "Face", "bFacePlayer", "Your expressions", Kind::kBool, 0, 1, nullptr,
				"Your face answers the NPC's lines and your choices." },

			// ---- Screen ----
			{ "Screen", "Letterbox", "Interface", "bLetterbox", "Letterbox bars", Kind::kBool, 0, 1, nullptr, nullptr },
			{ "Screen", "Letterbox", "Interface", "fLetterboxHeight", "Bar height", Kind::kFloat, 0.02f, 0.3f, "%.2f", "A share of the screen's height, each bar." },
			{ "Screen", "Letterbox", "Interface", "fLetterboxFade", "Fade (s)", Kind::kFloat, 0.0f, 3.0f, "%.1f", nullptr },
			{ "Screen", "Letterbox", "Interface", "bSubtitlesInBars", "Subtitles in the bottom bar", Kind::kBool, 0, 1, nullptr,
				"On: centred in the bottom bar while the bars are in. Off: where the game puts them (vanilla positioning, kept for mods that place subtitles themselves), which can overlap the top edge of the bottom bar." },
			{ "Screen", "Dialogue options", "Interface", "bHideOptionsWhileSpeaking", "Hide while they speak", Kind::kBool, 0, 1, nullptr,
				"The game hides your options during most of their lines, but brings them back before some lines end. On: hidden for the whole of each line, back as it ends (except their greeting, where you pick an option while they speak). Off: as the game does." },
			{ "Screen", "Dialogue options", "Interface", "fOptionsFade", "Fade (s)", Kind::kFloat, 0.0f, 1.5f, "%.1f", "0 = no fade." },
			{ "Screen", "Testing", "Debug", "bShowMessages", "Testing messages", Kind::kBool, 0, 1, nullptr,
				"The shot and the line's expression in the top bar." },
			{ "Screen", "Testing", "General", "bDebugLog", "Detailed log", Kind::kBool, 0, 1, nullptr,
				"A detailed log of what the camera does, for bug reports: CinematicConversationCameraSF.log in Documents\\My Games\\Starfield\\SFSE\\Logs. Off normally (it costs a little)." },

			// ---- Filming people talking ----
			{ "Filming people talking", "People talking near you", "Overhear", "bFilmConversationsNpc", "Film them", Kind::kBool, 0, 1, nullptr, nullptr },
			{ "Filming people talking", "People talking near you", "Overhear", "bAutoListen", "Start on its own", Kind::kBool, 0, 1, nullptr,
				"As a conversation scene starts near you, even while you walk or jog past (it holds you still until you let go of moving). No prompt then; the listen key still starts and stops it." },
			{ "Filming people talking", "People talking near you", "Overhear", "bAutoListenSprinting", "Start on its own while sprinting", Kind::kBool, 0, 1, nullptr,
				"Also start as you sprint past a conversation (off: only while you walk or jog). With it on, a sprint past is stopped in its tracks." },
			{ "Filming people talking", "People talking near you", "Overhear", "bListenPrompt", "Listen prompt", Kind::kBool, 0, 1, nullptr,
				"A prompt to start filming with the listen key (N, or hold B). Not shown while it starts on its own (the key still works then). With both off, nothing starts it." },
			{ "Filming people talking", "People talking near you", "Overhear", "fListenRange", "Range (m)", Kind::kFloat, 3, 25, "%.0f", nullptr },
		};

		// Settings that only do something while another is on (0.9.44, the user's request): drawn
		// indented under it, greyed out with "(off)" while it's off. A three-way parent (the
		// preset's own / off / on) counts as on unless it's off; a number, above 0.
		struct Needs
		{
			const char* key;
			const char* parent;
		};
		constexpr Needs kNeeds[]{
			{ "sFocusStyle", "bFocusOnSubject" },
			{ "fLetterboxHeight", "bLetterbox" },
			{ "fLetterboxFade", "bLetterbox" },
			{ "bSubtitlesInBars", "bLetterbox" },
			{ "bAutoListen", "bFilmConversationsNpc" },
			{ "bAutoListenSprinting", "bAutoListen" },
			{ "bListenPrompt", "bFilmConversationsNpc" },
			{ "fListenRange", "bFilmConversationsNpc" },
		};

		const char* ParentOf(const char* a_key)
		{
			for (const auto& n : kNeeds) {
				if (std::string_view{ n.key } == a_key) {
					return n.parent;
				}
			}
			return nullptr;
		}

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

		// The INI key an entry reads and writes (0.9.27): the cutting pace is each preset's own,
		// fCuttingPace<Preset> for the preset in use (fCuttingPace itself is the fallback).
		std::string KeyOf(const Entry& a_entry)
		{
			if (std::string_view{ a_entry.key } != "fCuttingPace") {
				return a_entry.key;
			}
			std::string preset{ Settings::Get().preset };
			for (const char* name : Settings::kPresetNames) {
				std::string lower{ name };
				std::string mine{ preset };
				std::ranges::transform(lower, lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
				std::ranges::transform(mine, mine.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
				if (lower == mine) {
					return std::string{ "fCuttingPace" } + name;
				}
			}
			return a_entry.key;
		}

		std::optional<std::string> ReadKey(const wchar_t* a_path, const char* a_section, const std::string& a_key)
		{
			wchar_t buffer[512]{};
			const auto section = Wide(a_section);
			const auto key = Wide(a_key.c_str());
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

		std::optional<std::string> ReadRaw(const wchar_t* a_path, const Entry& a_entry)
		{
			return ReadKey(a_path, a_entry.section, KeyOf(a_entry));
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
				} else if (auto main = ReadRaw(kIniPath, e)) {
					g_values[i] = { *main, false };
				} else if (KeyOf(e) != e.key) {
					// No pace of this preset's own: the general fCuttingPace (yours, or the INI's).
					auto general = ReadKey(kUserIniPath, e.section, e.key);
					if (!general) {
						general = ReadKey(kIniPath, e.section, e.key);
					}
					g_values[i] = { general.value_or(""), false };
				} else {
					g_values[i] = { "", false };
				}
			}
			g_readAt = std::chrono::steady_clock::now();
		}

		void Write(std::size_t a_index, const std::string& a_text)
		{
			const auto& e = kEntries[a_index];
			const auto  name = KeyOf(e);
			const auto  section = Wide(e.section);
			const auto  key = Wide(name.c_str());
			const auto  text = Wide(a_text.c_str());
			if (::WritePrivateProfileStringW(section.c_str(), key.c_str(), text.c_str(), kUserIniPath)) {
				g_values[a_index] = { a_text, true };
				g_status = std::format("Saved {}={} to your _user.ini.", name, a_text);
				if (std::string_view{ e.key } == "sPreset") {
					Settings::SetPresetOverride("");  // the preset key's choice gives way to this one
				}
			} else {
				g_status = std::format("Couldn't write {} to CinematicConversationCameraSF_user.ini (Data\\SFSE\\Plugins).", name);
			}
		}

		void Reset(std::size_t a_index)
		{
			const auto& e = kEntries[a_index];
			const auto  name = KeyOf(e);
			const auto  section = Wide(e.section);
			const auto  key = Wide(name.c_str());
			::WritePrivateProfileStringW(section.c_str(), key.c_str(), nullptr, kUserIniPath);  // removes the line
			ReadAll();
			g_status = std::format("{} back to the INI's value.", name);
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

		std::string g_needsNote;  // while drawing a greyed-out setting: what it needs

		void Help(const Entry& a_entry)
		{
			if (!g_needsNote.empty()) {
				ImGuiMCP::SetItemTooltip("%s%s%s", g_needsNote.c_str(), a_entry.help ? "\n\n" : "", a_entry.help ? a_entry.help : "");
			} else if (a_entry.help) {
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
					// Sliders in set steps (0.9.27), drawn as whole-number sliders of steps so a
					// controller moves them a step per press: the cutting pace in halves (a float
					// slider rounded to 0.5 never moved with a controller, whose 0.1 nudges were
					// rounded straight back), and the finest ones in 0.05 (a controller crossed
					// them 0.01 at a time).
					const std::string_view key{ e.key };
					const float            step = key == "fCuttingPace" ? 0.5f : 0.0f;  // (the 0.05 steps' sliders left the page in 0.9.60)
					if (step > 0.0f) {
						const float fallback = key == "fCuttingPace" ? 0.0f : e.min;
						int         steps = static_cast<int>(std::lround(std::clamp(AsFloat(v.text, fallback), e.min, e.max) / step));
						const auto  shown = key == "fCuttingPace" ? std::format("{:+.1f}", steps * step) : std::format("{:.2f}", steps * step);
						// The cutting pace names the preset it belongs to (0.9.27: each has its own);
						// "###" keeps the widget the same while the name changes.
						const auto keyName = KeyOf(e);
						const auto label = key == "fCuttingPace" && keyName.size() > 12 ? std::format("{} ({})###{}", e.label, keyName.substr(12), e.key) : std::string{ e.label };
						ImGuiMCP::SliderInt(label.c_str(), &steps, static_cast<int>(std::lround(e.min / step)), static_cast<int>(std::lround(e.max / step)), shown.c_str());
						Help(e);
						if (ImGuiMCP::IsItemDeactivatedAfterEdit()) {
							Write(a_index, std::format("{:.2f}", steps * step));
						} else if (ImGuiMCP::IsItemActive()) {
							v.text = std::format("{:.2f}", steps * step);
						}
						break;
					}
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
			// Where changes go and when they apply, at the top of every page (0.9.27; it was only
			// at the bottom, easy to miss).
			ImGuiMCP::TextDisabled("%s", "Changes apply from the next conversation (made during one, from the one after).");
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
				// Indented under what it needs, greyed out while that's off (0.9.44).
				int         depth = 0;
				const char* off = nullptr;  // the label of the first setting up the chain that's off
				for (const char* parent = ParentOf(e.key); parent && depth < 4; parent = ParentOf(parent)) {
					++depth;
					for (std::size_t p = 0; p < kCount; ++p) {
						if (std::string_view{ kEntries[p].key } == parent) {
							if (!off && AsFloat(g_values[p].text, 1.0f) == 0.0f) {
								off = kEntries[p].label;
							}
							break;
						}
					}
				}
				constexpr float kStep = 18.0f;
				if (depth > 0) {
					ImGuiMCP::Indent(kStep * static_cast<float>(depth));
				}
				g_needsNote = off ? std::format("Needs \"{}\" on.", off) : std::string{};
				if (off) {
					ImGuiMCP::BeginDisabled(true);
				}
				DrawEntry(i);
				if (off) {
					ImGuiMCP::SameLine();
					ImGuiMCP::TextDisabled("%s", "(off)");
					ImGuiMCP::EndDisabled();
				}
				g_needsNote.clear();
				// While "Start on its own" is on, what it does to your camera and your controls,
				// spelled out under it (0.9.33, the user's request); not while filming is off.
				if (std::string_view{ e.key } == "bAutoListen" && !off && AsFloat(g_values[i].text, 1.0f) != 0.0f) {
					ImGuiMCP::PushTextWrapPos(0.0f);
					ImGuiMCP::TextColored(ImGuiMCP::ImVec4{ 1.0f, 0.78f, 0.35f, 1.0f }, "%s",
						"Heads up: when a conversation scene starts near you, the camera is taken over to film it, and you're stopped where you are (looking around too) until you let go of the movement controls. Moving again ends the filming and gives you your controls back.");
					ImGuiMCP::PopTextWrapPos();
				}
				if (depth > 0) {
					ImGuiMCP::Unindent(kStep * static_cast<float>(depth));
				}
			}
			ImGuiMCP::Spacing();
			ImGuiMCP::Separator();
			ImGuiMCP::TextDisabled("%s", "Changes go to CinematicConversationCameraSF_user.ini (with Mod Organizer, in its overwrite folder). More settings, each explained, are in CinematicConversationCameraSF.ini (Data\\SFSE\\Plugins). The preset key (U) switches presets at once, mid-conversation too.");
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
