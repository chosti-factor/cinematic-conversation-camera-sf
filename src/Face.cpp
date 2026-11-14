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

		// Traits that change how your face behaves (perks in Starfield.esm).
		constexpr RE::TESFormID kIntrovert = 0x227FD8;
		constexpr RE::TESFormID kExtrovert = 0x227FD7;
		constexpr RE::TESFormID kEmpath = 0x227FD6;
		constexpr RE::TESFormID kWanted = 0x227FDD;

		bool        g_signatureRead = false;
		std::string g_signature;
		std::string g_background;
		bool        g_introvert = false;
		bool        g_extrovert = false;
		bool        g_empath = false;
		bool        g_wanted = false;
		std::string g_traits;  // for the log
		std::string g_signatureSource;  // what the last MaybeSignature came from
		std::atomic<int> g_closeUpPercent{ 100 };  // trait scale for your emotion close-ups (any thread)

		std::mt19937& Rng()
		{
			static std::mt19937 rng{ std::random_device{}() };
			return rng;
		}

		bool Roll(int a_percent)
		{
			return std::uniform_int_distribution<int>{ 0, 99 }(Rng()) < a_percent;
		}

		// Introvert: half as often; Extrovert: half as often again (at most 100%).
		int TraitScaled(int a_percent)
		{
			const int scaled = g_introvert ? a_percent / 2 : g_extrovert ? a_percent * 3 / 2 : a_percent;
			return std::clamp(scaled, 0, 100);
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
		(void)Signature();  // traits read for this conversation
		const auto tag = Lower(a_npcTag);
		// Introvert: a subtler face; 40% of lines get no visible response.
		if (g_introvert && !tag.empty() && tag != "none" && Roll(40)) {
			return {};
		}
		// Empath: feels what the speaker feels, so mirrors the expression itself (anger
		// excepted: that hurts, so concern).
		if (g_empath) {
			static const std::unordered_map<std::string, std::string> mirrored{
				{ "worried", "Worried" }, { "nervous", "Nervous" }, { "afraid", "Afraid" }, { "apologetic", "Apologetic" },
				{ "somber", "Somber" }, { "depressed", "Depressed" }, { "pleading", "Pleading" }, { "inpain", "InPain" },
				{ "happy", "Happy" }, { "amused", "Amused" }, { "grateful", "Grateful" }, { "relieved", "Relieved" },
				{ "surprised", "Surprised" }, { "awed", "Awed" }, { "confused", "Confused" }, { "thinking", "Thinking" },
				{ "concerned", "Concerned" }, { "tired", "Tired" }, { "irritated", "Concerned" }, { "angry", "Concerned" },
				{ "veryangry", "Concerned" }, { "rage", "Afraid" },
			};
			if (const auto it = mirrored.find(tag); it != mirrored.end()) {
				return it->second;
			}
		}
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
		if (const auto it = angry.find(tag); it != angry.end()) {
			return it->second[std::uniform_int_distribution<std::size_t>{ 0, it->second.size() - 1 }(Rng())];
		}
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
			{ "surprised", "Surprised" }, { "shocked", "Surprised" }, { "disbelief", "Surprised" }, { "awed", "Awed" },
			{ "disgust", "Disgust" },
		};
		const auto it = responses.find(tag);
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
				bool failed = false;
				auto has = [&](RE::TESFormID a_id) {
					auto* form = RE::TESForm::LookupByID(a_id);
					bool  result = false;
					if (!failed && form && form->GetFormType() == RE::FormType::kPERK && !HasPerkUnsafe(fn, player, static_cast<RE::BGSPerk*>(form), result)) {
						failed = true;
						REX::WARN("Face: checking your background and traits failed; none used");
					}
					return !failed && result;
				};
				for (const auto& bg : kBackgrounds) {
					if (has(bg.id)) {
						g_background = bg.name;
						g_signature = bg.expression;
						break;
					}
				}
				g_introvert = has(kIntrovert);
				g_extrovert = has(kExtrovert);
				g_empath = has(kEmpath);
				g_wanted = has(kWanted);
			} else {
				g_introvert = g_extrovert = g_empath = g_wanted = false;
			}
			g_traits.clear();
			for (const auto& [on, name] : { std::pair{ g_introvert, "Introvert (subtler)" }, std::pair{ g_extrovert, "Extrovert (fuller)" },
					 std::pair{ g_empath, "Empath (mirrors)" }, std::pair{ g_wanted, "Wanted (wary)" } }) {
				if (on) {
					g_traits += (g_traits.empty() ? "" : ", ") + std::string{ name };
				}
			}
			g_closeUpPercent.store(TraitScaled(100), std::memory_order_relaxed);
			REX::INFO("Face: your background {}{}; traits that show on your face: {}", g_background.empty() ? "isn't one of the 20 known" : g_background,
				g_signature.empty() ? "" : std::format(", signature expression {}", g_signature), g_traits.empty() ? "none" : g_traits);
		}
		if (a_background) {
			*a_background = g_background;
		}
		return g_signature;
	}

	std::string MaybeSignature()
	{
		const auto signature = Signature();
		// Wanted: a wary look now and then, whatever your background.
		if (g_wanted && Roll(15)) {
			g_signatureSource = "Wanted";
			return "Suspicious";
		}
		if (signature.empty()) {
			return {};
		}
		g_signatureSource = g_background;
		return Roll(TraitScaled(Settings::Get().signatureChance)) ? signature : std::string{};
	}

	const std::string& SignatureSource()
	{
		return g_signatureSource;
	}

	int CloseUpChance(int a_percent)
	{
		return std::clamp(a_percent * g_closeUpPercent.load(std::memory_order_relaxed) / 100, 0, 100);
	}

	const std::string& Current()
	{
		return g_current;
	}

	void Update(bool a_filming)
	{
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
