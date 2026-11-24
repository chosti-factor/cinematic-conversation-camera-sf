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
			kBool,    // 0/1
			kTri,     // -1 the preset's own, 0 off, 1 on
			kFloat,   // a range
			kOwnInt,  // -1 the preset's own, otherwise a range
			kChoice   // one of a list of words
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

		// The player-facing settings (0.9.60-0.9.63, the INI review: the main choices; the rest is
		// in the INI's [Advanced] and [Debug], and each preset's own numbers hidden until the preset
		// editor (0.9.70); docs/INI_SETTINGS.md). Every entry
		// has a tooltip (0.9.64, the user).
		const Entry kEntries[]{
			// ---- General (0.9.70, the user: the mod's switch and the detailed log on a page of their own) ----
			{ "General", "The mod", "General", "bEnabled", "Cinematic conversations", Kind::kBool, 0, 1, nullptr,
				"Off: the whole mod stands down and conversations use the game's own camera." },
			{ "General", "Bug reports", "General", "bDebugLog", "Detailed log", Kind::kBool, 0, 1, nullptr,
				"A detailed log of what the camera does, for bug reports: CinematicConversationCameraSF.log in Documents\\My Games\\Starfield\\SFSE\\Logs. Off normally (it costs a little)." },
			// ---- Camera ----
			{ "Camera", "Look", "Director", "sPreset", "Preset", Kind::kChoice, 0, 0, nullptr,
				"Standard, Close and Room are CCC's own. Frontier (a space western), Starborn (Kubrick and Interstellar), "
				"Constellation (a hard sci-fi documentary) and Nebula (a space opera) are CCC SF's.",
				kPresets },
			{ "Camera", "Look", "Director", "fCuttingPace", "Cutting pace", Kind::kFloat, -2.0f, 2.0f, "%+.1f",
				"How lively the cutting is, relative to the preset's own rhythm: -2 calmest, 0 the preset as designed, +2 liveliest. Each preset keeps its own: this changes the pace of the preset in use. Mostly felt in long speeches, as the camera cuts on each turn whatever the pace." },
			{ "Camera", "Look", "Camera", "sFocusStyle", "Focus style", Kind::kChoice, 0, 0, nullptr,
				"Wide: long shots keep a deep sharp zone. Cinematic: a shallow one everywhere.", kFocusStyles },
			{ "Camera", "Movement", "Director", "bCameraMoves", "Camera moves", Kind::kBool, 0, 1, nullptr,
				"Push-ins, drifts, cranes and the like, as each preset has them." },
			{ "Camera", "Movement", "Camera", "iCameraSway", "Camera sway", Kind::kOwnInt, 0, 100, "%d%%",
				"0 a tripod, 100 fully handheld. The preset's own: none in CCC's presets, Starborn 15, Nebula 40, Frontier 45, Constellation 50 with a slow handheld wander." },
			{ "Camera", "Staging", "Director", "bTrue180", "180-degree rule", Kind::kBool, 0, 1, nullptr,
				"Keep the camera on one side of the line between you. Off: each person is shot over their own side." },
			{ "Camera", "Staging", "Camera", "bGameCameraInScenes", "Vanilla camera in quest scenes", Kind::kBool, 0, 1, nullptr,
				"On: quest scenes you take part in (someone walks up and starts talking, several people taking turns) keep the game's vanilla camera, which directs them. Everyday scenes (merchants, people chatting) keep this mod's camera. Off: this mod films every conversation." },
			// ---- Close-ups and reactions ----
			{ "Close-ups and reactions", "Them", "Director", "bEmotionCloseUpsNpc", "Emotion close-ups", Kind::kTri, 0, 0, nullptr,
				"A close-up of the speaker on lines with an expression: always on strong ones, now and then on milder ones (how often is the preset's). The preset's own: none in Standard and Room, strong lines only in Close, every tier in CCC SF's presets." },
			{ "Close-ups and reactions", "Them", "Director", "bPersuasionBeatNpc", "Hold on their answer", Kind::kBool, 0, 1, nullptr,
				"A close-up of the NPC's answer to every persuasion round, the verdict included, and to options with a skill check ([Intimidate], [Manipulate]...)." },
			{ "Close-ups and reactions", "You", "Director", "bReactionShotsPlayer", "Reaction shots of you", Kind::kTri, 0, 0, nullptr,
				"Cutting to you listening during their longer stretches of talk, for one of their lines. The preset's own: off in CCC's presets, on in CCC SF's." },
			{ "Close-ups and reactions", "You", "Director", "bChoiceCloseUpsPlayer", "Close-up on skill choices", Kind::kTri, 0, 0, nullptr,
				"A close-up of you when you pick a [Persuade], [Intimidate] or similar option, for the line you say. The preset's own: on in CCC SF's presets, off in CCC's." },
			{ "Close-ups and reactions", "You", "Director", "bFacePlayer", "Your expressions", Kind::kBool, 0, 1, nullptr,
				"Your face answers the NPC's lines and your choices (the game keeps it neutral)." },
			// ---- Screen ----
			{ "Screen", "Letterbox", "Interface", "bLetterbox", "Letterbox bars", Kind::kBool, 0, 1, nullptr,
				"Black bars across the top and bottom of the screen while a conversation is filmed, like a film frame. They sit under the dialogue menu, so its subtitles and options stay on top." },
			{ "Screen", "Letterbox", "Interface", "fLetterboxHeight", "Bar height", Kind::kFloat, 0.02f, 0.3f, "%.2f",
				"Each bar's share of the screen's height. 0.12 is close to a 2.39:1 film frame on a 16:9 screen." },
			{ "Screen", "Letterbox", "Interface", "bSubtitlesInBars", "Subtitles in the bottom bar", Kind::kBool, 0, 1, nullptr,
				"On: centred in the bottom bar while the bars are in. Off: where the game puts them (vanilla positioning, kept for mods that place subtitles themselves), which can overlap the top edge of the bottom bar." },
			{ "Screen", "Dialogue options", "Interface", "bHideOptionsWhileSpeaking", "Hide while they speak", Kind::kBool, 0, 1, nullptr,
				"The game hides your options during most of their lines, but brings them back before some lines end. On: hidden for the whole of each line, back as it ends (except their greeting, where you pick an option while they speak). Off: as the game does." },
			{ "Screen", "Testing", "Debug", "bShowMessages", "Testing messages", Kind::kBool, 0, 1, nullptr,
				"A testing aid, off normally: each shot the camera uses and why, and each line's expression, in the top bar." },
			// ---- Filming people talking ----
			{ "Filming people talking", "People talking near you", "SceneFilming", "bFilmConversationsNpc", "Film them", Kind::kBool, 0, 1, nullptr,
				"Film people talking to each other near you (the game's conversation scenes, not one-liners): a new shot for each line, letterboxed. Started with the listen prompt or on its own (below); moving ends it." },
			{ "Filming people talking", "People talking near you", "SceneFilming", "fListenRange", "Range (m)", Kind::kFloat, 3, 25, "%.0f",
				"How near a conversation has to be, in metres, to be offered or filmed." },
			{ "Filming people talking", "How it starts", "SceneFilming", "bListenPrompt", "Listen prompt", Kind::kBool, 0, 1, nullptr,
				"A prompt to start filming with the listen key (N, or hold B). Not shown while it starts on its own (the key still works then). With both off, nothing starts it." },
			{ "Filming people talking", "How it starts", "SceneFilming", "bAutoListen", "Start on its own", Kind::kBool, 0, 1, nullptr,
				"As a conversation scene starts near you, even while you walk or jog past (it holds you still until you let go of moving). No prompt then; the listen key still starts and stops it." },
			{ "Filming people talking", "How it starts", "SceneFilming", "bAutoListenSprinting", "Start on its own while sprinting", Kind::kBool, 0, 1, nullptr,
				"Also start as you sprint past a conversation (off: only while you walk or jog). With it on, a sprint past is stopped in its tracks." },
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
			{ "fLetterboxHeight", "bLetterbox" },
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
			if (std::string_view{ e.key } == "sPreset") {
				Settings::SetPresetOverride("");  // 0.9.66: as on a change; a preset key choice kept the reset from showing
			}
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

		// An on/off switch, as on a phone (0.9.64, the user: instead of a checkbox): a rounded track,
		// green when on and grey when off, its knob at the end that's on; the label beside it.
		// Clicking the switch or the label flips it; a controller or the keyboard flips it as a
		// button. Colours go through GetColorU32, so a greyed-out ("(off)") one fades with the rest.
		bool Switch(const Entry& a_entry, bool* a_on, const char* a_label = nullptr)
		{
			const auto  pos = ImGuiMCP::GetCursorScreenPos();
			const float h = ImGuiMCP::GetFrameHeight();
			const float w = h * 1.75f;
			bool        flipped = ImGuiMCP::InvisibleButton("##switch", ImGuiMCP::ImVec2{ w, h });
			const bool  hovered = ImGuiMCP::IsItemHovered();
			Help(a_entry);
			auto*       list = ImGuiMCP::GetWindowDrawList();
			const auto  track = *a_on ? ImGuiMCP::ImVec4{ 0.20f, 0.78f, 0.35f, 1.0f } : ImGuiMCP::ImVec4{ 0.40f, 0.40f, 0.43f, 1.0f };
			const auto  trackLit = ImGuiMCP::ImVec4{ track.x + 0.06f, track.y + 0.06f, track.z + 0.06f, 1.0f };
			ImGuiMCP::ImDrawListManager::AddRectFilled(list, pos, ImGuiMCP::ImVec2{ pos.x + w, pos.y + h },
				ImGuiMCP::GetColorU32(hovered ? trackLit : track), h * 0.5f, 0);
			const float r = h * 0.5f - 2.0f;
			const float knobX = *a_on ? pos.x + w - h * 0.5f : pos.x + h * 0.5f;
			ImGuiMCP::ImDrawListManager::AddCircleFilled(list, ImGuiMCP::ImVec2{ knobX, pos.y + h * 0.5f }, r,
				ImGuiMCP::GetColorU32(ImGuiMCP::ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f }), 24);
			ImGuiMCP::SameLine();
			ImGuiMCP::AlignTextToFramePadding();
			ImGuiMCP::TextUnformatted(a_label ? a_label : a_entry.label);
			if (ImGuiMCP::IsItemClicked()) {
				flipped = true;
			}
			Help(a_entry);
			if (flipped) {
				*a_on = !*a_on;
			}
			return flipped;
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
					if (Switch(e, &on)) {
						Write(a_index, on ? "1" : "0");
					}
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
			case Kind::kFloat:
				{
					// The cutting pace in half steps (0.9.27), drawn as a whole-number slider of
					// steps so a controller moves it a step per press (a float slider rounded to 0.5
					// never moved with a controller, whose 0.1 nudges were rounded straight back).
					if (std::string_view{ e.key } == "fCuttingPace") {
						constexpr float kStepSize = 0.5f;
						int             steps = static_cast<int>(std::lround(std::clamp(AsFloat(v.text, 0.0f), e.min, e.max) / kStepSize));
						const auto      shown = std::format("{:+.1f}", steps * kStepSize);
						// It names the preset it belongs to (each has its own); "###" keeps the
						// widget the same while the name changes.
						const auto keyName = KeyOf(e);
						const auto label = keyName.size() > 12 ? std::format("{} ({})###{}", e.label, keyName.substr(12), e.key) : std::string{ e.label };
						ImGuiMCP::SliderInt(label.c_str(), &steps, static_cast<int>(std::lround(e.min / kStepSize)), static_cast<int>(std::lround(e.max / kStepSize)), shown.c_str());
						Help(e);
						if (ImGuiMCP::IsItemDeactivatedAfterEdit()) {
							Write(a_index, std::format("{:.2f}", steps * kStepSize));
						} else if (ImGuiMCP::IsItemActive()) {
							v.text = std::format("{:.2f}", steps * kStepSize);
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
				{
					// 0.9.66 (the user): "the preset's own" is a switch like the others, and the
					// slider stays on the page, greyed out while the switch is on.
					const int  low = static_cast<int>(e.min);
					const int  high = static_cast<int>(e.max);
					const int  middle = low + (high - low) / 2;
					const int  raw = AsInt(v.text, -1);
					bool       own = raw < low;
					const auto ownLabel = std::format("{}: the preset's own", e.label);
					if (Switch(e, &own, ownLabel.c_str())) {
						Write(a_index, own ? "-1" : std::to_string(middle));
					}
					int        value = own ? middle : std::clamp(raw, low, high);
					const auto outer = g_needsNote;
					if (own) {
						g_needsNote = std::format("Needs \"{}\" off.", ownLabel);
						ImGuiMCP::BeginDisabled(true);
					}
					ImGuiMCP::SliderInt(e.label, &value, low, high, own ? "the preset's own" : (e.format ? e.format : "%d"));
					Help(e);
					if (own) {
						ImGuiMCP::EndDisabled();
						g_needsNote = outer;
					} else if (ImGuiMCP::IsItemDeactivatedAfterEdit()) {
						Write(a_index, std::to_string(value));
					} else if (ImGuiMCP::IsItemActive()) {
						v.text = std::to_string(value);
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
			bool        headsUp = false;  // "Start on its own" is on: its note at the end of the page (0.9.68)
			for (std::size_t i = 0; i < kCount; ++i) {
				const auto& e = kEntries[i];
				if (std::string_view{ e.page } != a_page) {
					continue;
				}
				if (!group || std::string_view{ group } != e.group) {
					// Each group under its own greyed line with its name (0.9.64: a gap above, so
					// the groups read apart).
					if (group) {
						ImGuiMCP::Spacing();
						ImGuiMCP::Spacing();
					}
					group = e.group;
					ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, ImGuiMCP::GetStyle()->Colors[ImGuiMCP::ImGuiCol_TextDisabled]);
					ImGuiMCP::SeparatorText(group);
					ImGuiMCP::PopStyleColor(1);
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
				// 0.9.64 (the user: hard to tell apart): a wider step, and a greyed corner line from
				// the setting above into this one, as in a tree. 0.9.68 (the user: at least four
				// times more): 3.4 times the font size, 136 px at the framework's 40 px (it was 34),
				// so it keeps its proportion at any UI scale.
				const float kStep = ImGuiMCP::GetFontSize() * 3.4f;
				if (depth > 0) {
					ImGuiMCP::Indent(kStep * static_cast<float>(depth));
					const auto  at = ImGuiMCP::GetCursorScreenPos();
					const float h = ImGuiMCP::GetFrameHeight();
					const float x = at.x - kStep * 0.6f;
					const auto  grey = ImGuiMCP::GetColorU32(ImGuiMCP::ImGuiCol_TextDisabled);
					auto*       list = ImGuiMCP::GetWindowDrawList();
					ImGuiMCP::ImDrawListManager::AddLine(list, ImGuiMCP::ImVec2{ x, at.y - h * 0.35f }, ImGuiMCP::ImVec2{ x, at.y + h * 0.5f }, grey, 1.5f);
					ImGuiMCP::ImDrawListManager::AddLine(list, ImGuiMCP::ImVec2{ x, at.y + h * 0.5f }, ImGuiMCP::ImVec2{ at.x - 6.0f, at.y + h * 0.5f }, grey, 1.5f);
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
				// spelled out (0.9.33, the user's request); not while filming is off. 0.9.68 (the
				// user): at the end of the page, not between the setting and the next one.
				if (std::string_view{ e.key } == "bAutoListen" && !off && AsFloat(g_values[i].text, 1.0f) != 0.0f) {
					headsUp = true;
				}
				if (depth > 0) {
					ImGuiMCP::Unindent(kStep * static_cast<float>(depth));
				}
			}
			if (headsUp) {
				ImGuiMCP::Spacing();
				ImGuiMCP::PushTextWrapPos(0.0f);
				ImGuiMCP::TextColored(ImGuiMCP::ImVec4{ 1.0f, 0.78f, 0.35f, 1.0f }, "%s",
					"Heads up: with \"Start on its own\" on, when a conversation scene starts near you, the camera is taken over to film it, and you're stopped where you are (looking around too) until you let go of the movement controls. Moving again ends the filming and gives you your controls back.");
				ImGuiMCP::PopTextWrapPos();
			}
			ImGuiMCP::Spacing();
			ImGuiMCP::Separator();
			ImGuiMCP::TextDisabled("%s", "Changes go to CinematicConversationCameraSF_user.ini (with Mod Organizer, in its overwrite folder). More settings, each explained, are in CinematicConversationCameraSF.ini (Data\\SFSE\\Plugins). The preset key (U) switches presets on the fly during a conversation.");
			if (!g_status.empty()) {
				ImGuiMCP::TextDisabled("%s", g_status.c_str());
			}
		}

		void __stdcall DrawGeneral() { DrawPage("General"); }
		void __stdcall DrawCamera() { DrawPage("Camera"); }
		void __stdcall DrawCloseUps() { DrawPage("Close-ups and reactions"); }
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
		SFSEMenuFramework::AddSectionItem("General", &DrawGeneral);
		SFSEMenuFramework::AddSectionItem("Camera", &DrawCamera);
		SFSEMenuFramework::AddSectionItem("Close-ups and reactions", &DrawCloseUps);
		SFSEMenuFramework::AddSectionItem("Screen", &DrawScreen);
		SFSEMenuFramework::AddSectionItem("Filming people talking", &DrawFilming);
		REX::INFO("Settings page: added to SFSE Menu Framework ({} settings)", kCount);
	}
}
