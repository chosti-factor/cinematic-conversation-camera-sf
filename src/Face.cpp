#include "Face.h"

#include "Hud.h"
#include "Settings.h"

#include <random>

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

		// Starfield 1.16.244: the actor's entry for a perk (rank at +8), or null; behind Papyrus'
		// Actor.HasPerk (both natives call it). Address Library ID 101224 (RVA 0x1955080).
		constexpr std::uint64_t kPerkRankEntry = 101224;
		using PerkRankEntry_t = const std::uint8_t* (*)(RE::Actor*, RE::BGSPerk*);

		bool HasPerkUnsafe(PerkRankEntry_t a_fn, RE::Actor* a_actor, RE::BGSPerk* a_perk, bool& a_has)
		{
			__try {
				const auto* entry = a_fn(a_actor, a_perk);
				a_has = entry && entry[8] != 0;
				return true;
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				return false;
			}
		}

		// Backgrounds (perks in Starfield.esm) and the expression each wears now and then.
		struct BackgroundFace
		{
			RE::TESFormID    id;
			std::string_view name;
			std::string_view expression;
		};
		constexpr std::array kBackgrounds{
			BackgroundFace{ 0x22EC76, "Beast Hunter", "Serious" },
			BackgroundFace{ 0x22EC81, "Bouncer", "Stern" },
			BackgroundFace{ 0x22EC80, "Bounty Hunter", "Suspicious" },
			BackgroundFace{ 0x22EC7F, "Chef", "Happy" },
			BackgroundFace{ 0x22EC7E, "Combat Medic", "Concerned" },
			BackgroundFace{ 0x22EC7D, "Cyber Runner", "Smirk" },
			BackgroundFace{ 0x22EC7C, "Cyberneticist", "Concentrating" },
			BackgroundFace{ 0x22EC7B, "Diplomat", "Friendly" },
			BackgroundFace{ 0x22EC79, "Explorer", "Awed" },
			BackgroundFace{ 0x22EC78, "Gangster", "Smug" },
			BackgroundFace{ 0x22EC77, "Homesteader", "Friendly" },
			BackgroundFace{ 0x22EC7A, "Industrialist", "Confident" },
			BackgroundFace{ 0x22EC75, "Long Hauler", "Tired" },
			BackgroundFace{ 0x22EC73, "Pilgrim", "Relieved" },
			BackgroundFace{ 0x22EC72, "Professor", "Thinking" },
			BackgroundFace{ 0x22EC74, "Ronin", "Stern" },
			BackgroundFace{ 0x22EC71, "Sculptor", "Thinking" },
			BackgroundFace{ 0x22EC70, "Soldier", "Serious" },
			BackgroundFace{ 0x22EC6F, "Space Scoundrel", "Smirk" },
			BackgroundFace{ 0x22EC6E, "Xenobiologist", "Concentrating" },
		};

		// Your choice and the NPC's answer arrive together (your character isn't voiced, so
		// the answer starts as you choose; 0.7.12 test: the choice's face, noticed a moment
		// later, replaced the answer's for the whole line). The choice's face shows first, for
		// a moment, then the answer's.
		using Clock = std::chrono::steady_clock;
		constexpr auto kChoiceHold = std::chrono::milliseconds{ 1500 };
		Clock::time_point g_choiceAt{};
		bool              g_listenPending = false;
		std::string       g_listenExpression;
		std::string       g_listenWhy;

		bool        g_signatureRead = false;
		std::string g_signature;
		std::string g_background;

		bool Roll(int a_percent)
		{
			static std::mt19937 rng{ std::random_device{}() };
			return std::uniform_int_distribution<int>{ 0, 99 }(rng) < a_percent;
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
		// listener's response (concern for worry, attention for a question). Angry lines get
		// a guarded look or anger back, picked at random each line. Untagged or neutral
		// lines: your own face.
		static const std::unordered_map<std::string, std::vector<std::string>> angry{
			{ "irritated", { "Serious", "Irritated" } },
			{ "angry", { "Stern", "Irritated", "VeryAngry" } },
			{ "veryangry", { "Stern", "Irritated", "VeryAngry" } },
			{ "rage", { "Afraid", "VeryAngry", "Rage" } },
		};
		if (const auto it = angry.find(Lower(a_npcTag)); it != angry.end()) {
			static std::mt19937 rng{ std::random_device{}() };
			return it->second[std::uniform_int_distribution<std::size_t>{ 0, it->second.size() - 1 }(rng)];
		}
		static const std::unordered_map<std::string, std::string> responses{
			{ "friendly", "Friendly" }, { "confident", "Confident" }, { "amused", "Amused" }, { "happy", "Happy" }, { "grateful", "Friendly" },
			{ "impressed", "Happy" }, { "relieved", "Relieved" }, { "smirk", "Smirk" }, { "sarcastic", "Smirk" },
			{ "flirting", "Smirk" }, { "smug", "Smirk" }, { "sinistersmile", "Suspicious" },
			{ "thinking", "Concentrating" }, { "question", "Thinking" }, { "concentrating", "Concentrating" },
			{ "puzzled", "Thinking" }, { "confused", "Confused" },
			{ "concerned", "Concerned" }, { "worried", "Concerned" }, { "nervous", "Concerned" }, { "afraid", "Worried" },
			{ "apologetic", "Concerned" }, { "pleading", "Concerned" }, { "inpain", "Concerned" }, { "dying", "Somber" },
			{ "somber", "Somber" }, { "depressed", "Somber" }, { "tired", "Concerned" },
			{ "serious", "Serious" }, { "stern", "Serious" }, { "defiant", "Serious" }, { "suspicious", "Suspicious" },
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
			// No tag: a question ("...?") now and then gets a questioning look (0.7.12; the
			// first use of the option's text, kept to questions).
			auto text = a_optionText;
			while (!text.empty() && (text.back() == ' ' || text.back() == '"' || text.back() == '\'')) {
				text.remove_suffix(1);
			}
			if (!text.empty() && text.back() == '?' && Roll(Settings::Get().questionFaceChance)) {
				return "Question";
			}
			return {};
		}
		const auto tag = Lower(a_optionText.substr(open + 1, close - open - 1));
		static const std::vector<std::pair<std::string_view, std::string_view>> byTag{
			{ "persua", "Confident" }, { "intimidat", "Stern" }, { "threat", "Stern" }, { "manipulat", "Smirk" },
			{ "deceiv", "Smirk" }, { "lie", "Smirk" }, { "flirt", "Flirting" }, { "charm", "Friendly" },
			{ "lecture", "Serious" }, { "attack", "VeryAngry" }, { "bribe", "Smirk" }, { "pay", "Neutral" },
			{ "commerce", "Confident" }, { "diplomacy", "Friendly" }, { "theology", "Serious" }, { "medicine", "Concerned" },
			{ "leadership", "Confident" }, { "stealth", "Suspicious" },
			// Science and technical checks: thinking it through, or focused on the task.
			{ "astrophysics", "Thinking" }, { "geology", "Thinking" }, { "botany", "Thinking" }, { "zoology", "Thinking" },
			{ "xenosociology", "Thinking" }, { "research", "Thinking" }, { "surveying", "Thinking" }, { "scanning", "Thinking" },
			{ "chemistry", "Concentrating" }, { "security", "Concentrating" }, { "engineering", "Concentrating" },
			{ "robotics", "Concentrating" }, { "piloting", "Concentrating" }, { "starship", "Concentrating" },
			{ "outpost", "Concentrating" }, { "weapon", "Concentrating" }, { "spacesuit", "Concentrating" },
		};
		for (const auto& [key, expression] : byTag) {
			if (tag.find(key) != std::string::npos) {
				return std::string{ expression };
			}
		}
		// Background options ("[Space Scoundrel] ..."): that background's expression.
		for (const auto& bg : kBackgrounds) {
			if (tag.find(Lower(bg.name)) != std::string::npos) {
				return std::string{ bg.expression };
			}
		}
		// Trait options ("[Wanted] ...").
		static const std::vector<std::pair<std::string_view, std::string_view>> byTrait{
			{ "introvert", "Concentrating" }, { "extrovert", "Happy" }, { "empath", "Concerned" }, { "wanted", "Suspicious" },
			{ "kid stuff", "Friendly" }, { "street rat", "Smirk" }, { "uc native", "Confident" }, { "united colonies", "Confident" },
			{ "freestar", "Confident" }, { "raised universal", "Thinking" }, { "raised enlightened", "Confident" },
			{ "serpent", "Serious" }, { "hero worship", "Puzzled" }, { "dream home", "Concerned" }, { "alien dna", "Confident" },
			{ "spaced", "Thinking" }, { "taskmaster", "Stern" }, { "terra firma", "Relieved" },
		};
		for (const auto& [key, expression] : byTrait) {
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

	std::string Signature(std::string* a_background)
	{
		if (!g_signatureRead) {
			g_signatureRead = true;
			g_signature.clear();
			g_background.clear();
			auto*                   player = RE::PlayerCharacter::GetSingleton();
			constexpr std::uint64_t kLookupByID = 47401;
			if (player && Hud::AddressKnown(kPerkRankEntry) && Hud::AddressKnown(kLookupByID)) {
				const auto fn = reinterpret_cast<PerkRankEntry_t>(REL::ID(kPerkRankEntry).address());
				for (const auto& bg : kBackgrounds) {
					auto* form = RE::TESForm::LookupByID(bg.id);
					if (!form || form->GetFormType() != RE::FormType::kPERK) {
						continue;
					}
					bool has = false;
					if (!HasPerkUnsafe(fn, player, static_cast<RE::BGSPerk*>(form), has)) {
						REX::WARN("Face: checking your background failed; no signature expression");
						break;
					}
					if (has) {
						g_background = bg.name;
						g_signature = bg.expression;
						break;
					}
				}
			}
			REX::INFO("Face: your background {}{}", g_background.empty() ? "isn't one of the 20 known" : g_background,
				g_signature.empty() ? "" : std::format(", signature expression {}", g_signature));
		}
		if (a_background) {
			*a_background = g_background;
		}
		return g_signature;
	}

	std::string MaybeSignature()
	{
		const auto signature = Signature();
		if (signature.empty()) {
			return {};
		}
		static std::mt19937 rng{ std::random_device{}() };
		return std::uniform_int_distribution<int>{ 0, 99 }(rng) < Settings::Get().signatureChance ? signature : std::string{};
	}

	const std::string& Current()
	{
		return g_current;
	}

	void Listen(const std::string& a_expression, std::string_view a_why)
	{
		if (Clock::now() - g_choiceAt < kChoiceHold) {
			g_listenPending = true;
			g_listenExpression = a_expression;
			g_listenWhy = a_why;
			return;
		}
		g_listenPending = false;
		Set(a_expression, a_why);
	}

	void Choose(const std::string& a_expression, std::string_view a_why)
	{
		// The answer's face, if it's already showing, comes back after the choice's.
		if (!g_listenPending) {
			g_listenPending = true;
			g_listenExpression = g_current;
			g_listenWhy = "back to listening";
		}
		g_choiceAt = Clock::now();
		Set(a_expression, a_why);
	}

	void Update(bool a_filming)
	{
		if (g_listenPending && Clock::now() - g_choiceAt >= kChoiceHold) {
			g_listenPending = false;
			if (a_filming) {
				Set(g_listenExpression, g_listenWhy);
			}
		}
		if (!a_filming) {
			g_listenPending = false;
		}
		if (!a_filming) {
			g_signatureRead = false;  // read again next conversation (a new game, a new background)
		}
		if (!a_filming && !g_current.empty()) {
			Set({}, "conversation over");
			if (!g_current.empty()) {
				g_current.clear();  // couldn't reset: don't keep trying
			}
		}
	}
}
