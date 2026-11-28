#pragma once

#include <format>
#include <string>

// Translations of the text this mod shows (1.1.1, a player's request: the settings page in their
// own language). As SFSE Menu Framework does for its own menus: a JSON file of English text to
// translated text, Data\SFSE\Plugins\CinematicConversationCameraSFStrings.json, shipped with every
// value in English. A translation replaces that file; anything missing or empty stays English.
// tools/strings.py writes the file from the source (every N_(), T() and Fmt() text) and the build
// checks it's current.

// Marks text for translation where it can't be looked up yet (a table of constants): N_("...") is
// the text itself; T() looks it up when it's shown.
#define N_(text) text

namespace CCC::Strings
{
	// Reads the translations file (once; at the plugin's load). a_path empty: the usual place.
	void Load(const std::wstring& a_path = {});

	// The text in the player's language, or the English given if it has no translation.
	// The pointer stays valid for the rest of the game.
	[[nodiscard]] const char* T(const char* a_english);

	// For a literal shown through printf-style formatting (an ImGui slider's format): any % in
	// the translation doubled, so it's shown, not read as a format.
	[[nodiscard]] std::string Literal(const char* a_english);

	// std::format text: the translation's, or the English one if the translation's placeholders
	// don't fit the values (a broken translation never breaks the page).
	template <class... Args>
	[[nodiscard]] std::string Fmt(const char* a_english, const Args&... a_args)
	{
		const char* text = T(a_english);
		try {
			return std::vformat(text, std::make_format_args(a_args...));
		} catch (const std::format_error&) {
			try {
				return std::vformat(a_english, std::make_format_args(a_args...));
			} catch (const std::format_error&) {
				return a_english;
			}
		}
	}

	// Parses a translations file's text (exposed for tools and tests): false if it isn't a JSON
	// object of strings.
	bool Parse(std::string_view a_json);

	// How many translations are loaded (non-empty values that differ from their English).
	[[nodiscard]] std::size_t Translated();
}
