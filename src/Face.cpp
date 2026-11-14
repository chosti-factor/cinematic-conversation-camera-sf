#include "Face.h"

#include "Hud.h"
#include "Settings.h"

namespace CCC::Face
{
	namespace
	{
		// Starfield 1.16.244: the actor-level function behind Papyrus'
		// Actor.ChangeAnimFaceArchetype(keyword); none puts the default back. Address Library
		// ID 100510 (RVA 0x18E0220, as rrrafau's PointCameraAtPlayer uses).
		constexpr std::uint64_t kChangeAnimFaceArchetype = 100510;
		using ChangeAnimFaceArchetype_t = void (*)(RE::Actor*, RE::BGSKeyword*);

		std::string g_current;      // expression set now ("" = the game's own)
		bool        g_broken = false;

		bool CallUnsafe(ChangeAnimFaceArchetype_t a_fn, RE::Actor* a_actor, RE::BGSKeyword* a_keyword)
		{
			__try {
				a_fn(a_actor, a_keyword);
				return true;
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				return false;
			}
		}

		bool Apply(RE::BGSKeyword* a_keyword)
		{
			if (g_broken) {
				return false;
			}
			if (!Hud::AddressKnown(kChangeAnimFaceArchetype)) {
				g_broken = true;
				REX::WARN("Face: the Address Library has no entry {}; your character's expressions are off", kChangeAnimFaceArchetype);
				return false;
			}
			auto* player = RE::PlayerCharacter::GetSingleton();
			if (!player) {
				return false;
			}
			const auto fn = reinterpret_cast<ChangeAnimFaceArchetype_t>(REL::ID(kChangeAnimFaceArchetype).address());
			if (!CallUnsafe(fn, player, a_keyword)) {
				g_broken = true;
				REX::WARN("Face: setting your expression failed; your character's expressions are off for this session");
				return false;
			}
			return true;
		}

		std::string Lower(std::string_view a_text)
		{
			std::string out;
			for (const char c : a_text) {
				out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
			}
			return out;
		}
	}

	std::string ListeningTo(std::string_view a_npcTag)
	{
		// Subtle by default: mirrored where it's natural (a smile for a smile), otherwise a
		// listener's response (concern for worry, attention for a question); angry lines get
		// a guarded look rather than anger back. Untagged or neutral lines: your own face.
		static const std::unordered_map<std::string, std::string> responses{
			{ "friendly", "Friendly" }, { "amused", "Amused" }, { "happy", "Happy" }, { "grateful", "Friendly" },
			{ "impressed", "Happy" }, { "relieved", "Relieved" }, { "smirk", "Smirk" }, { "sarcastic", "Smirk" },
			{ "flirting", "Smirk" }, { "smug", "Smirk" }, { "sinistersmile", "Suspicious" },
			{ "thinking", "Concentrating" }, { "question", "Thinking" }, { "concentrating", "Concentrating" },
			{ "puzzled", "Thinking" }, { "confused", "Confused" },
			{ "concerned", "Concerned" }, { "worried", "Concerned" }, { "nervous", "Concerned" }, { "afraid", "Worried" },
			{ "apologetic", "Concerned" }, { "pleading", "Concerned" }, { "inpain", "Concerned" }, { "dying", "Somber" },
			{ "somber", "Somber" }, { "depressed", "Somber" }, { "tired", "Concerned" },
			{ "serious", "Serious" }, { "stern", "Serious" }, { "defiant", "Serious" }, { "suspicious", "Suspicious" },
			{ "irritated", "Serious" }, { "angry", "Stern" }, { "veryangry", "Stern" }, { "rage", "Afraid" },
			{ "surprised", "Surprised" }, { "shocked", "Surprised" }, { "disbelief", "Surprised" }, { "awed", "Awed" },
			{ "disgust", "Disgust" },
		};
		const auto it = responses.find(Lower(a_npcTag));
		return it != responses.end() ? it->second : std::string{};
	}

	std::string ForChoice(std::string_view a_optionText)
	{
		// The skill tag at the start of the option ("[Persuade] ...").
		const auto open = a_optionText.find('[');
		const auto close = a_optionText.find(']');
		if (open == std::string_view::npos || close == std::string_view::npos || close < open || open > 3) {
			return {};
		}
		const auto tag = Lower(a_optionText.substr(open + 1, close - open - 1));
		static const std::vector<std::pair<std::string_view, std::string_view>> byTag{
			{ "persua", "Confident" }, { "intimidat", "Stern" }, { "threat", "Stern" }, { "manipulat", "Smirk" },
			{ "deceiv", "Smirk" }, { "lie", "Smirk" }, { "flirt", "Flirting" }, { "charm", "Friendly" },
			{ "lecture", "Serious" }, { "attack", "VeryAngry" }, { "bribe", "Smirk" }, { "pay", "Neutral" },
			{ "commerce", "Confident" }, { "diplomacy", "Friendly" }, { "theology", "Serious" }, { "medicine", "Concerned" },
		};
		for (const auto& [key, expression] : byTag) {
			if (tag.find(key) != std::string::npos) {
				return std::string{ expression };
			}
		}
		return {};
	}

	void Set(const std::string& a_expression, std::string_view a_why)
	{
		if ((!Settings::Get().playerFace && !a_expression.empty()) || a_expression == g_current) {
			return;
		}
		RE::BGSKeyword* keyword = nullptr;
		if (!a_expression.empty()) {
			const std::string editorId = "AnimFaceArchetype" + a_expression;
			keyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>(editorId.c_str());
			if (!keyword) {
				static int warned = 0;
				if (warned++ < 10) {
					REX::WARN("Face: no expression '{}' in the game ({} not found)", a_expression, editorId);
				}
				return;
			}
		}
		if (Apply(keyword)) {
			REX::INFO("Face: you look {} ({})", a_expression.empty() ? "as the game has you" : a_expression, a_why);
			g_current = a_expression;
		}
	}

	void Update(bool a_filming)
	{
		if (!a_filming && !g_current.empty()) {
			Set({}, "conversation over");
			if (!g_current.empty()) {
				g_current.clear();  // couldn't reset: don't keep trying
			}
		}
	}
}
