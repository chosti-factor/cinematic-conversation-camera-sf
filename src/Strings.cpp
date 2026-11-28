// Cinematic Conversation Camera SF: translations of the text it shows (1.1.1; see Strings.h).
#include "Strings.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>
#include <unordered_map>

namespace CCC::Strings
{
	namespace
	{
		constexpr auto kPath = L".\\Data\\SFSE\\Plugins\\CinematicConversationCameraSFStrings.json";

		// English text -> translation. Filled once, before anything is drawn, and never changed
		// after, so the pointers T() hands out stay valid.
		std::unordered_map<std::string, std::string> g_table;
		std::size_t                                  g_translated = 0;

		void AppendUtf8(std::string& a_out, std::uint32_t a_cp)
		{
			if (a_cp < 0x80) {
				a_out.push_back(static_cast<char>(a_cp));
			} else if (a_cp < 0x800) {
				a_out.push_back(static_cast<char>(0xC0 | (a_cp >> 6)));
				a_out.push_back(static_cast<char>(0x80 | (a_cp & 0x3F)));
			} else if (a_cp < 0x10000) {
				a_out.push_back(static_cast<char>(0xE0 | (a_cp >> 12)));
				a_out.push_back(static_cast<char>(0x80 | ((a_cp >> 6) & 0x3F)));
				a_out.push_back(static_cast<char>(0x80 | (a_cp & 0x3F)));
			} else {
				a_out.push_back(static_cast<char>(0xF0 | (a_cp >> 18)));
				a_out.push_back(static_cast<char>(0x80 | ((a_cp >> 12) & 0x3F)));
				a_out.push_back(static_cast<char>(0x80 | ((a_cp >> 6) & 0x3F)));
				a_out.push_back(static_cast<char>(0x80 | (a_cp & 0x3F)));
			}
		}

		// A small JSON reader: one object whose values are strings, which is all the file holds.
		class Reader
		{
		public:
			explicit Reader(std::string_view a_text) :
				_text(a_text)
			{
				if (_text.starts_with("\xEF\xBB\xBF")) {  // a UTF-8 byte order mark (Notepad's)
					_at = 3;
				}
			}

			bool Object(std::unordered_map<std::string, std::string>& a_out)
			{
				Skip();
				if (!Take('{')) {
					return false;
				}
				Skip();
				if (Take('}')) {
					return true;
				}
				for (;;) {
					std::string key, value;
					Skip();
					if (!String(key)) {
						return false;
					}
					Skip();
					if (!Take(':')) {
						return false;
					}
					Skip();
					if (!String(value)) {
						return false;
					}
					a_out[std::move(key)] = std::move(value);
					Skip();
					if (Take(',')) {
						continue;
					}
					if (Take('}')) {
						Skip();
						return _at == _text.size();
					}
					return false;
				}
			}

		private:
			void Skip()
			{
				while (_at < _text.size() && (_text[_at] == ' ' || _text[_at] == '\t' || _text[_at] == '\r' || _text[_at] == '\n')) {
					++_at;
				}
			}

			bool Take(char a_c)
			{
				if (_at < _text.size() && _text[_at] == a_c) {
					++_at;
					return true;
				}
				return false;
			}

			bool Hex4(std::uint32_t& a_out)
			{
				if (_at + 4 > _text.size()) {
					return false;
				}
				a_out = 0;
				for (int i = 0; i < 4; ++i) {
					const char c = _text[_at++];
					a_out <<= 4;
					if (c >= '0' && c <= '9') {
						a_out |= static_cast<std::uint32_t>(c - '0');
					} else if (c >= 'a' && c <= 'f') {
						a_out |= static_cast<std::uint32_t>(c - 'a' + 10);
					} else if (c >= 'A' && c <= 'F') {
						a_out |= static_cast<std::uint32_t>(c - 'A' + 10);
					} else {
						return false;
					}
				}
				return true;
			}

			bool String(std::string& a_out)
			{
				if (!Take('"')) {
					return false;
				}
				while (_at < _text.size()) {
					const char c = _text[_at++];
					if (c == '"') {
						return true;
					}
					if (c != '\\') {
						a_out.push_back(c);  // UTF-8 passes through as it is
						continue;
					}
					if (_at >= _text.size()) {
						return false;
					}
					switch (const char e = _text[_at++]) {
					case '"':
					case '\\':
					case '/':
						a_out.push_back(e);
						break;
					case 'b':
						a_out.push_back('\b');
						break;
					case 'f':
						a_out.push_back('\f');
						break;
					case 'n':
						a_out.push_back('\n');
						break;
					case 'r':
						a_out.push_back('\r');
						break;
					case 't':
						a_out.push_back('\t');
						break;
					case 'u':
						{
							std::uint32_t cp = 0;
							if (!Hex4(cp)) {
								return false;
							}
							if (cp >= 0xD800 && cp < 0xDC00 && _at + 6 <= _text.size() && _text[_at] == '\\' && _text[_at + 1] == 'u') {
								_at += 2;
								std::uint32_t low = 0;
								if (!Hex4(low) || low < 0xDC00 || low >= 0xE000) {
									return false;
								}
								cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
							}
							AppendUtf8(a_out, cp);
							break;
						}
					default:
						return false;
					}
				}
				return false;
			}

			std::string_view _text;
			std::size_t      _at{ 0 };
		};
	}

	bool Parse(std::string_view a_json)
	{
		std::unordered_map<std::string, std::string> table;
		if (!Reader{ a_json }.Object(table)) {
			return false;
		}
		g_table = std::move(table);
		g_translated = 0;
		for (const auto& [english, text] : g_table) {
			if (!english.starts_with('_') && !text.empty() && text != english) {  // "_about" is a note
				++g_translated;
			}
		}
		return true;
	}

	void Load(const std::wstring& a_path)
	{
		const std::filesystem::path path{ a_path.empty() ? std::wstring{ kPath } : a_path };
		std::ifstream               file{ path, std::ios::binary };
		if (!file) {
#ifndef CCC_STRINGS_TEST
			REX::INFO("Translations: no {} (English)", path.filename().string());
#endif
			return;
		}
		const std::string text{ std::istreambuf_iterator<char>{ file }, std::istreambuf_iterator<char>{} };
		const bool        ok = Parse(text);
#ifndef CCC_STRINGS_TEST
		if (ok) {
			REX::INFO("Translations: {} of {} texts translated ({})", g_translated, g_table.size(), path.filename().string());
		} else {
			REX::WARN("Translations: {} isn't a JSON object of texts (a missing quote or comma?); English instead", path.filename().string());
		}
#else
		(void)ok;
#endif
	}

	const char* T(const char* a_english)
	{
		if (!a_english || g_table.empty()) {
			return a_english;
		}
		const auto it = g_table.find(a_english);
		return it != g_table.end() && !it->second.empty() ? it->second.c_str() : a_english;
	}

	std::string Literal(const char* a_english)
	{
		std::string out;
		for (const char* c = T(a_english); c && *c; ++c) {
			out.push_back(*c);
			if (*c == '%') {
				out.push_back('%');
			}
		}
		return out;
	}

	std::size_t Translated()
	{
		return g_translated;
	}
}
