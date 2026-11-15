#include "Visibility.h"

namespace CCC::Visibility
{
	namespace
	{
		// Starfield 1.16.244: NiAVObject's "app culled" (hidden) flag and the virtual that
		// sets it. Calling the virtual rather than flipping the bit lets the engine update
		// its render state too. Slot from Player Camera During Dialogue by rrrafau (GPL-3.0).
		constexpr std::size_t   kSetAppCulledVtableIndex = 61;
		constexpr std::uint64_t kAppCulledBit = 0x1;

		constexpr std::array kHeadNodes{ "C_Head", "C_Neck", "C_Neck1", "Head", "NPC Head [Head]" };

		struct Changed
		{
			RE::NiAVObject* object;
			bool            wasCulled;
		};
		std::vector<Changed> g_changed;
		std::mutex           g_mutex;  // called from the dialogue camera hook and the main thread
		bool                 g_loggedOnce = false;

		bool IsCulled(RE::NiAVObject* a_obj)
		{
			return (std::atomic_ref<std::uint64_t>(a_obj->flags).load(std::memory_order_acquire) & kAppCulledBit) != 0;
		}

		void SetCulled(RE::NiAVObject* a_obj, bool a_culled)
		{
			using SetAppCulled_t = void (*)(RE::NiAVObject*, bool);
			auto** vtbl = *reinterpret_cast<void***>(a_obj);
			reinterpret_cast<SetAppCulled_t>(vtbl[kSetAppCulledVtableIndex])(a_obj, a_culled);
		}

		void Reveal(RE::NiAVObject* a_obj, std::string_view a_what)
		{
			if (!a_obj || !IsCulled(a_obj)) {
				return;
			}
			const bool known = std::ranges::any_of(g_changed, [&](const Changed& c) { return c.object == a_obj; });
			if (!known) {
				g_changed.push_back({ a_obj, true });
				if (!g_loggedOnce) {
					REX::INFO("Revealing hidden player {}", a_what);
				}
			}
			SetCulled(a_obj, false);
		}

		RE::NiAVObject* PlayerRoot()
		{
			auto* player = RE::PlayerCharacter::GetSingleton();
			if (!player) {
				return nullptr;
			}
			auto  lock = player->loadedData.LockRead();
			auto* data = *lock;
			return data ? data->data3D.get() : nullptr;
		}
	}

	void RevealPlayer()
	{
		std::scoped_lock lock{ g_mutex };
		auto* root = PlayerRoot();
		if (!root) {
			return;
		}
		Reveal(root, "body");
		for (const auto* name : kHeadNodes) {
			if (auto* node = root->GetObjectByName(RE::BSFixedString{ name })) {
				Reveal(node, name);
				break;
			}
		}
		if (!g_changed.empty()) {
			g_loggedOnce = true;
		}
	}

	void HidePlayer()
	{
		std::scoped_lock lock{ g_mutex };
		auto* root = PlayerRoot();
		if (!root || IsCulled(root)) {
			return;
		}
		const bool known = std::ranges::any_of(g_changed, [&](const Changed& c) { return c.object == root; });
		if (!known) {
			g_changed.push_back({ root, false });
			REX::INFO("Hiding the player (full body unavailable)");
		}
		SetCulled(root, true);
	}

	void Restore(bool a_visible)
	{
		std::scoped_lock lock{ g_mutex };
		auto* root = PlayerRoot();

		for (const auto& c : g_changed) {
			if (!root) {
				break;
			}
			// Only touch objects that still belong to the player's current 3D; if the game
			// rebuilt the player's model in the meantime, the old pointers are stale.
			bool stillOurs = c.object == root;
			if (!stillOurs) {
				for (const auto* name : kHeadNodes) {
					if (root->GetObjectByName(RE::BSFixedString{ name }) == c.object) {
						stillOurs = true;
						break;
					}
				}
			}
			if (!stillOurs) {
				continue;
			}
			// Decided by where the player is going back to, not by what the game is showing
			// right now: the game is still in its dialogue camera at this point, and hiding a
			// third-person player here left one frame without them after the conversation.
			if (IsCulled(c.object) == a_visible) {
				SetCulled(c.object, !a_visible);
			}
		}
		if (!g_changed.empty()) {
			REX::INFO("Player visibility restored ({} objects, {})", g_changed.size(), a_visible ? "shown" : "hidden");
		}
		g_changed.clear();
	}

	namespace
	{
		std::vector<RE::NiAVObject*> g_firstPersonHidden;  // culled by HideFirstPersonBody
		bool                         g_firstPersonSearched = false;  // searched once per filming (the scene graph isn't walked every frame)
		bool                         g_firstPersonLogged = false;

		bool IsFirstPersonName(const char* a_name)
		{
			if (!a_name) {
				return false;
			}
			std::string lower{ a_name };
			std::ranges::transform(lower, lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
			return lower.find("1stperson") != std::string::npos;
		}

		// The first-person body: by name, under the player's own 3D, beside it (its parent's
		// children), or under the camera.
		std::vector<RE::NiAVObject*> FindFirstPersonBody(RE::NiAVObject* a_root, std::string& a_where)
		{
			std::vector<RE::NiAVObject*> found;
			static const RE::BSFixedString name{ "Player1stPersonBody" };
			if (a_root) {
				if (auto* obj = a_root->GetObjectByName(name); obj && obj != a_root) {
					found.push_back(obj);
					a_where = "under your 3D";
				}
				if (auto* parent = a_root->parent) {
					for (auto& child : parent->children) {
						auto* obj = child.get();
						if (obj && obj != a_root && IsFirstPersonName(obj->name.c_str()) && std::ranges::find(found, obj) == found.end()) {
							found.push_back(obj);
							a_where = std::format("beside your 3D ('{}')", obj->name.c_str());
						}
					}
				}
			}
			if (found.empty()) {
				if (auto* camera = RE::PlayerCamera::GetSingleton(); camera && camera->cameraRoot) {
					if (auto* obj = camera->cameraRoot->GetObjectByName(name)) {
						found.push_back(obj);
						a_where = "under the camera";
					}
				}
			}
			return found;
		}
	}

	void HideFirstPersonBody()
	{
		std::scoped_lock lock{ g_mutex };
		if (g_firstPersonSearched) {
			for (auto* obj : g_firstPersonHidden) {
				if (!IsCulled(obj)) {
					SetCulled(obj, true);
				}
			}
			return;
		}
		g_firstPersonSearched = true;
		auto*       root = PlayerRoot();
		std::string where;
		const auto  found = FindFirstPersonBody(root, where);
		if (!g_firstPersonLogged) {
			g_firstPersonLogged = true;
			if (found.empty()) {
				REX::INFO("First-person body: not found by name (your 3D '{}', its parent '{}' with {} children)", root ? root->name.c_str() : "none",
					root && root->parent ? root->parent->name.c_str() : "none", root && root->parent ? root->parent->children.size() : 0u);
			} else {
				REX::INFO("First-person body: found {} ({} objects); hidden while filming from first person", where, found.size());
			}
		}
		for (auto* obj : found) {
			if (!IsCulled(obj)) {
				SetCulled(obj, true);
				if (std::ranges::find(g_firstPersonHidden, obj) == g_firstPersonHidden.end()) {
					g_firstPersonHidden.push_back(obj);
				}
			}
		}
	}

	void RestoreFirstPersonBody()
	{
		std::scoped_lock lock{ g_mutex };
		g_firstPersonSearched = false;
		if (g_firstPersonHidden.empty()) {
			return;
		}
		// Only what can still be found now (the pointers may be stale otherwise).
		std::string where;
		const auto  found = FindFirstPersonBody(PlayerRoot(), where);
		for (auto* obj : found) {
			if (std::ranges::find(g_firstPersonHidden, obj) != g_firstPersonHidden.end() && IsCulled(obj)) {
				SetCulled(obj, false);
			}
		}
		g_firstPersonHidden.clear();
	}
}
