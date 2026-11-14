#include "Hud.h"

#include "Settings.h"

namespace CCC::Hud
{
	namespace
	{
		using Value = RE::Scaleform::GFx::Value;
		using MovieRoot = RE::Scaleform::GFx::ASMovieRootBase;

		// ---- Address Library check -------------------------------------------------------
		// CommonLibSF's Scaleform value functions are looked up by Address Library ID on first
		// use, and a missing ID ends the game with an error box. Check them first.
		struct IddbPeek : REL::IDDB
		{
			static bool Has(std::uint64_t a_id)
			{
				const auto* db = static_cast<const IddbPeek*>(REL::IDDB::GetSingleton());
				if (!db) {
					return false;
				}
				const auto v5 = db->get_id2offset<std::uint32_t>();
				if (!v5.empty()) {
					return a_id < v5.size() && v5[a_id] != 0;
				}
				const auto v0 = db->get_id2offset<REL::IDDB::MAPPING>();
				const auto it = std::lower_bound(v0.begin(), v0.end(), a_id, [](const REL::IDDB::MAPPING& a_m, std::uint64_t a_v) { return a_m.id < a_v; });
				return it != v0.end() && it->id == a_id && it->offset != 0;
			}
		};

		enum class Ready
		{
			kUnchecked,
			kYes,
			kNo
		};
		Ready g_ready = Ready::kUnchecked;

		bool CheckReady()
		{
			if (g_ready != Ready::kUnchecked) {
				return g_ready == Ready::kYes;
			}
			// ObjectAddRef, ObjectRelease, HasMember, GetMember, SetMember, Invoke; the two
			// vtables (movie, AS3 movie root) to recognise the objects before using them.
			constexpr std::array<std::uint64_t, 8> ids{ 169745, 169746, 169747, 169748, 169749, 169750, 303817, 305584 };
			for (const auto id : ids) {
				if (!IddbPeek::Has(id)) {
					REX::WARN("HUD: Address Library has no entry {}; letterbox and subtitle features are off", id);
					g_ready = Ready::kNo;
					return false;
				}
			}
			g_ready = Ready::kYes;
			return true;
		}

		bool g_broken = false;  // a Scaleform call faulted: everything here stays off this session

		// ---- Fault-guarded Scaleform calls ------------------------------------------------
		// No objects with destructors in these, so they can use structured exception handling.
		bool SafeGetVariable(MovieRoot* a_root, Value* a_out, const char* a_path)
		{
			__try {
				return a_root->GetVariable(a_out, a_path);
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				g_broken = true;
				return false;
			}
		}

		bool SafeCreateObject(MovieRoot* a_root, Value* a_out, const char* a_class)
		{
			__try {
				a_root->CreateObject(a_out, a_class, nullptr, 0);
				return a_out->IsObject();
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				g_broken = true;
				return false;
			}
		}

		bool SafeGet(const Value* a_obj, const char* a_name, Value* a_out)
		{
			__try {
				return a_obj->IsObject() && a_obj->GetMember(a_name, a_out);
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				g_broken = true;
				return false;
			}
		}

		bool SafeSet(Value* a_obj, const char* a_name, const Value* a_value)
		{
			__try {
				return a_obj->IsObject() && a_obj->SetMember(a_name, *a_value);
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				g_broken = true;
				return false;
			}
		}

		bool SafeInvoke(Value* a_obj, const char* a_name, Value* a_result, const Value* a_args, std::size_t a_count)
		{
			__try {
				return a_obj->IsObject() && a_obj->Invoke(a_name, a_result, a_args, a_count);
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				g_broken = true;
				return false;
			}
		}

		bool VtableIs(const void* a_object, std::uint64_t a_id)
		{
			std::uintptr_t vtable = 0;
			__try {
				vtable = *static_cast<const std::uintptr_t*>(a_object);
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				return false;
			}
			return vtable == REL::ID(a_id).address();
		}

		double Number(const Value& a_v, double a_default = 0.0)
		{
			switch (a_v.GetType()) {
			case RE::Scaleform::GFx::Value::ValueType::kNumber:
				return a_v.GetNumber();
			case RE::Scaleform::GFx::Value::ValueType::kInt:
				return a_v.GetInt();
			case RE::Scaleform::GFx::Value::ValueType::kUInt:
				return a_v.GetUInt();
			default:
				return a_default;
			}
		}

		double Member(const Value& a_obj, const char* a_name, double a_default = 0.0)
		{
			Value v;
			return SafeGet(&a_obj, a_name, &v) ? Number(v, a_default) : a_default;
		}

		std::string StringMember(const Value& a_obj, const char* a_name)
		{
			Value v;
			if (!SafeGet(&a_obj, a_name, &v) || !v.IsString()) {
				return {};
			}
			const char* s = v.GetString();
			return s ? std::string{ s } : std::string{};
		}

		// ---- Finding a menu's movie and its root -------------------------------------------
		struct MovieRef
		{
			RE::Scaleform::Ptr<RE::Scaleform::GFx::Movie> movie;
			MovieRoot*                                    root{ nullptr };
		};

		std::optional<MovieRef> OpenMovie(const char* a_menu)
		{
			auto* ui = RE::UI::GetSingleton();
			if (!ui) {
				return std::nullopt;
			}
			const RE::BSFixedString name{ a_menu };
			if (!ui->IsMenuOpen(name)) {
				return std::nullopt;
			}
			MovieRef ref;
			ref.movie = ui->GetMenuMovie(name);
			auto* movie = ref.movie.get();
			if (!movie || !VtableIs(movie, 303817)) {
				static bool warned = false;
				if (!warned && movie) {
					warned = true;
					REX::WARN("HUD: {}'s movie isn't the expected type; letterbox and subtitle features are off", a_menu);
					g_broken = true;
				}
				return std::nullopt;
			}
			ref.root = movie->asMovieRoot.get();
			if (!ref.root || !VtableIs(ref.root, 305584)) {
				static bool warned = false;
				if (!warned) {
					warned = true;
					REX::WARN("HUD: {}'s movie root isn't the expected type; letterbox and subtitle features are off", a_menu);
					g_broken = true;
				}
				return std::nullopt;
			}
			return ref;
		}

		// The movie's top display object. AS3 movies name it differently from AS2 ones; the
		// first path that gives a display object is kept.
		bool RootObject(MovieRoot* a_root, Value& a_out, const char* a_menu)
		{
			static std::string chosen;
			if (!chosen.empty()) {
				return SafeGetVariable(a_root, &a_out, chosen.c_str()) && a_out.IsDisplayObject();
			}
			std::string tried;
			for (const char* path : { "root", "root1", "_root", "stage.getChildAt(0)" }) {
				Value v;
				if (SafeGetVariable(a_root, &v, path) && v.IsDisplayObject()) {
					chosen = path;
					a_out = v;
					REX::INFO("HUD: {}'s top object is '{}'", a_menu, path);
					return true;
				}
				tried += std::format(" {}={}", path, static_cast<int>(v.GetType()));
			}
			static bool warned = false;
			if (!warned) {
				warned = true;
				REX::WARN("HUD: couldn't find {}'s top object (tried{})", a_menu, tried);
			}
			return false;
		}

		// ---- Menu layout dump (diagnostic) -------------------------------------------------
		// Once per session per menu: the display list, so the subtitle and option clips can be
		// found by name.
		void DumpTree(const Value& a_obj, const std::string& a_path, int a_depth, int& a_count)
		{
			if (a_count >= 400 || g_broken) {
				return;
			}
			++a_count;
			const int   children = static_cast<int>(Member(a_obj, "numChildren", 0.0));
			std::string text = StringMember(a_obj, "text");
			if (text.size() > 60) {
				text.resize(60);
			}
			for (auto& c : text) {
				if (c == '\n' || c == '\r') {
					c = ' ';
				}
			}
			Value visible;
			const bool shown = !SafeGet(&a_obj, "visible", &visible) || !visible.IsBoolean() || visible.GetBoolean();
			REX::INFO("HUD dump {}{}: x {:.0f} y {:.0f} w {:.0f} h {:.0f} alpha {:.2f}{}{}{}", std::string(static_cast<std::size_t>(a_depth) * 2, ' '), a_path,
				Member(a_obj, "x"), Member(a_obj, "y"), Member(a_obj, "width"), Member(a_obj, "height"), Member(a_obj, "alpha", 1.0), shown ? "" : " hidden",
				children > 0 ? std::format(" ({} children)", children) : std::string{}, text.empty() ? std::string{} : std::format(" text '{}'", text));
			if (a_depth >= 6) {
				return;
			}
			for (int i = 0; i < children && a_count < 400; ++i) {
				Value index{ static_cast<std::int32_t>(i) };
				Value child;
				if (!SafeInvoke(const_cast<Value*>(&a_obj), "getChildAt", &child, &index, 1) || !child.IsDisplayObject()) {
					continue;
				}
				std::string name = StringMember(child, "name");
				DumpTree(child, a_path + "." + (name.empty() ? std::format("[{}]", i) : name), a_depth + 1, a_count);
			}
		}

		void Dump(const char* a_menu)
		{
			const auto ref = OpenMovie(a_menu);
			if (!ref) {
				return;
			}
			Value top;
			if (!RootObject(ref->root, top, a_menu)) {
				return;
			}
			Value stage;
			if (SafeGet(&top, "stage", &stage) && stage.IsObject()) {
				REX::INFO("HUD dump {}: stage {:.0f} x {:.0f}", a_menu, Member(stage, "stageWidth"), Member(stage, "stageHeight"));
			}
			int count = 0;
			DumpTree(top, a_menu, 0, count);
			REX::INFO("HUD dump {}: {} objects{}", a_menu, count, count >= 400 ? " (stopped at 400)" : "");
		}

		// Big movies (the HUD has hundreds of elements): visit everything, but log only the top
		// two levels and whatever sits under an element whose name suggests subtitles.
		bool Interesting(const std::string& a_name)
		{
			std::string n = a_name;
			std::ranges::transform(n, n.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
			for (const char* key : { "subtitle", "speaker", "caption", "dialog", "conversation", "talk" }) {
				if (n.find(key) != std::string::npos) {
					return true;
				}
			}
			return false;
		}

		void SearchTree(const Value& a_obj, const std::string& a_path, int a_depth, int a_under, int& a_visited, int& a_logged)
		{
			if (a_visited >= 5000 || a_logged >= 300 || g_broken) {
				return;
			}
			++a_visited;
			const std::string name = a_path.substr(a_path.rfind('.') + 1);
			const bool        match = Interesting(name);
			const int         under = match ? 4 : a_under;
			const int         children = static_cast<int>(Member(a_obj, "numChildren", 0.0));
			if (a_depth <= 1 || match || a_under > 0) {
				++a_logged;
				std::string text = StringMember(a_obj, "text");
				if (text.size() > 60) {
					text.resize(60);
				}
				for (auto& c : text) {
					if (c == '\n' || c == '\r') {
						c = ' ';
					}
				}
				Value visible;
				const bool shown = !SafeGet(&a_obj, "visible", &visible) || !visible.IsBoolean() || visible.GetBoolean();
				REX::INFO("HUD find {}{}: x {:.0f} y {:.0f} w {:.0f} h {:.0f} alpha {:.2f}{}{}{}", match ? "* " : "", a_path, Member(a_obj, "x"), Member(a_obj, "y"),
					Member(a_obj, "width"), Member(a_obj, "height"), Member(a_obj, "alpha", 1.0), shown ? "" : " hidden",
					children > 0 ? std::format(" ({} children)", children) : std::string{}, text.empty() ? std::string{} : std::format(" text '{}'", text));
			}
			if (a_depth >= 10) {
				return;
			}
			for (int i = 0; i < children; ++i) {
				Value index{ static_cast<std::int32_t>(i) };
				Value child;
				if (!SafeInvoke(const_cast<Value*>(&a_obj), "getChildAt", &child, &index, 1) || !child.IsDisplayObject()) {
					continue;
				}
				const std::string childName = StringMember(child, "name");
				SearchTree(child, a_path + "." + (childName.empty() ? std::format("[{}]", i) : childName), a_depth + 1, under > 0 ? under - 1 : 0, a_visited,
					a_logged);
			}
		}

		void Search(const char* a_menu)
		{
			const auto ref = OpenMovie(a_menu);
			if (!ref) {
				REX::INFO("HUD find {}: not open", a_menu);
				return;
			}
			Value top;
			if (!RootObject(ref->root, top, a_menu)) {
				return;
			}
			int visited = 0, logged = 0;
			SearchTree(top, a_menu, 0, 0, visited, logged);
			REX::INFO("HUD find {}: {} elements visited, {} logged", a_menu, visited, logged);
		}

		// The dialogue menu's lists, by path from its top object.
		bool Child(Value& a_parent, const char* a_name, Value& a_out)
		{
			Value name{ a_name };
			return SafeInvoke(&a_parent, "getChildByName", &a_out, &name, 1) && a_out.IsDisplayObject();
		}

		// ---- Dialogue options fade in --------------------------------------------------------
		// The game shows and hides the option list (DialogueList_mc; PersuasionList_mc during a
		// persuasion) itself. When it appears, ease it in instead of letting it pop.
		struct OptionsFade
		{
			bool  wasVisible[2]{ false, false };
			float shownFor[2]{ 99.0f, 99.0f };
			bool  listDumped{ false };
		} g_options;

		void UpdateOptions(Value& a_top, float a_delta, bool a_dump)
		{
			const auto& s = Settings::Get();
			Value       menu;
			if (!Child(a_top, "Menu_mc", menu)) {
				return;
			}
			const char* names[2]{ "DialogueList_mc", "PersuasionList_mc" };
			for (int i = 0; i < 2; ++i) {
				Value list;
				if (!Child(menu, names[i], list)) {
					continue;
				}
				Value      visible;
				const bool now = SafeGet(&list, "visible", &visible) && visible.IsBoolean() && visible.GetBoolean();
				if (now && !g_options.wasVisible[i]) {
					g_options.shownFor[i] = 0.0f;
					if (a_dump && i == 0 && !g_options.listDumped) {
						g_options.listDumped = true;
						int count = 0;
						DumpTree(list, std::string{ "DialogueMenu.Menu_mc." } + names[i], 2, count);
					}
				}
				g_options.wasVisible[i] = now;
				if (!now || !s.fadeOptions) {
					continue;
				}
				if (g_options.shownFor[i] < s.optionsFade) {
					g_options.shownFor[i] += a_delta;
					const float t = std::clamp(g_options.shownFor[i] / (std::max)(s.optionsFade, 0.01f), 0.0f, 1.0f);
					const Value alpha{ static_cast<double>(t * t * (3.0f - 2.0f * t)) };
					SafeSet(&list, "alpha", &alpha);
				}
			}
		}

		// ---- Letterbox -----------------------------------------------------------------------
		constexpr const char* kTopName = "cccLetterboxTop";
		constexpr const char* kBottomName = "cccLetterboxBottom";

		struct LetterboxState
		{
			const void* movie{ nullptr };  // the dialogue menu movie the bars were added to
			float       ease{ 0.0f };      // 0 = off screen, 1 = fully in
			float       barHeight{ 0.0f }; // stage units
			bool        logged{ false };
		} g_bars;

		bool FindChild(Value& a_parent, const char* a_name, Value& a_out)
		{
			Value name{ a_name };
			return SafeInvoke(&a_parent, "getChildByName", &a_out, &name, 1) && a_out.IsDisplayObject();
		}

		bool MakeBar(MovieRoot* a_root, Value& a_top, const char* a_name, double a_x, double a_y, double a_w, double a_h)
		{
			Value shape;
			if (!SafeCreateObject(a_root, &shape, "flash.display.Shape")) {
				return false;
			}
			Value graphics;
			if (!SafeGet(&shape, "graphics", &graphics) || !graphics.IsObject()) {
				return false;
			}
			const Value fill[2]{ Value{ static_cast<std::uint32_t>(0) }, Value{ 1.0 } };
			const Value rect[4]{ Value{ a_x }, Value{ a_y }, Value{ a_w }, Value{ a_h } };
			if (!SafeInvoke(&graphics, "beginFill", nullptr, fill, 2) || !SafeInvoke(&graphics, "drawRect", nullptr, rect, 4) ||
				!SafeInvoke(&graphics, "endFill", nullptr, nullptr, 0)) {
				return false;
			}
			const Value name{ a_name };
			SafeSet(&shape, "name", &name);
			// Under everything the menu draws (index 0): its subtitles and options stay on top.
			const Value args[2]{ shape, Value{ static_cast<std::int32_t>(0) } };
			return SafeInvoke(&a_top, "addChildAt", nullptr, args, 2);
		}

		void UpdateLetterbox(bool a_want, float a_delta)
		{
			const auto& s = Settings::Get();
			const auto  ref = OpenMovie("DialogueMenu");
			if (!ref) {
				g_bars.ease = 0.0f;
				return;
			}
			Value top;
			if (!RootObject(ref->root, top, "DialogueMenu")) {
				return;
			}
			const float target = a_want ? 1.0f : 0.0f;
			const float step = a_delta / (std::max)(s.letterboxFade, 0.01f);
			g_bars.ease = g_bars.ease < target ? (std::min)(target, g_bars.ease + step) : (std::max)(target, g_bars.ease - step);

			Value upper, lower;
			const bool have = FindChild(top, kTopName, upper) && FindChild(top, kBottomName, lower);
			if (!have) {
				if (!a_want) {
					return;
				}
				// Stage size; draw well past its sides, so a wider screen than the movie's is
				// still covered.
				double width = 1920.0, height = 1080.0;
				Value  stage;
				if (SafeGet(&top, "stage", &stage) && stage.IsObject()) {
					width = Member(stage, "stageWidth", width);
					height = Member(stage, "stageHeight", height);
				}
				const double bar = height * std::clamp(s.letterboxHeight, 0.02f, 0.3f);
				g_bars.barHeight = static_cast<float>(bar);
				if (!MakeBar(ref->root, top, kTopName, -width, -height, width * 3.0, height + bar) ||
					!MakeBar(ref->root, top, kBottomName, -width, height - bar, width * 3.0, height + bar)) {
					if (!g_bars.logged) {
						g_bars.logged = true;
						REX::WARN("HUD: couldn't add the letterbox bars to the dialogue menu");
					}
					return;
				}
				if (!g_bars.logged || g_bars.movie != ref->movie.get()) {
					g_bars.logged = true;
					REX::INFO("HUD: letterbox bars added to the dialogue menu (stage {:.0f} x {:.0f}, bars {:.0f} high)", width, height, bar);
				}
				g_bars.movie = ref->movie.get();
				if (!FindChild(top, kTopName, upper) || !FindChild(top, kBottomName, lower)) {
					return;
				}
			}
			// Slide in from the screen edges.
			const float e = 1.0f - (1.0f - g_bars.ease) * (1.0f - g_bars.ease);
			const Value upY{ static_cast<double>(-g_bars.barHeight * (1.0f - e)) };
			const Value downY{ static_cast<double>(g_bars.barHeight * (1.0f - e)) };
			const Value shown{ g_bars.ease > 0.001f };
			SafeSet(&upper, "y", &upY);
			SafeSet(&lower, "y", &downY);
			SafeSet(&upper, "visible", &shown);
			SafeSet(&lower, "visible", &shown);
		}
	}

	void Update(bool a_filming, float a_delta)
	{
		const auto& s = Settings::Get();
		if (g_broken || (!s.letterbox && !s.dumpMenus && !s.fadeOptions) || !CheckReady()) {
			return;
		}
		static float dialogueOpenFor = 0.0f;
		static bool  dumpedDialogue = false, dumpedHud = false;
		auto*        ui = RE::UI::GetSingleton();
		const bool   dialogueOpen = ui && ui->IsMenuOpen(RE::BSFixedString{ "DialogueMenu" });
		dialogueOpenFor = dialogueOpen ? dialogueOpenFor + a_delta : 0.0f;
		if (s.dumpMenus && dialogueOpenFor > 1.5f) {
			if (!dumpedDialogue) {
				dumpedDialogue = true;
				Dump("DialogueMenu");
			}
			if (!dumpedHud) {
				dumpedHud = true;
				// 0.6.3's full dump of the HUD stopped at 400 elements before the subtitles:
				// search it (and the HUD messages menu) for them instead.
				Search("HUDMenu");
				Search("HUDMessagesMenu");
			}
		}
		if (s.letterbox && !g_broken) {
			UpdateLetterbox(a_filming && dialogueOpen, a_delta);
		}
		if (dialogueOpen && !g_broken && (s.fadeOptions || s.dumpMenus)) {
			if (const auto ref = OpenMovie("DialogueMenu")) {
				Value top;
				if (RootObject(ref->root, top, "DialogueMenu")) {
					UpdateOptions(top, a_delta, s.dumpMenus);
				}
			}
		} else if (!dialogueOpen) {
			g_options.wasVisible[0] = g_options.wasVisible[1] = false;
		}
		if (g_broken) {
			REX::WARN("HUD: a Scaleform call failed; letterbox and subtitle features are off for this session");
		}
	}
}
