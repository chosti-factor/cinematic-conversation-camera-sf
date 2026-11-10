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

	void Restore()
	{
		// Only touch objects that still belong to the player's current 3D; if the game
		// rebuilt the player's model in the meantime, the old pointers are stale.
		auto* root = PlayerRoot();

		// If the game is already back in third person, the player is meant to be visible;
		// re-hiding them would leave an invisible character. Only re-hide while the game is
		// still in its dialogue or first-person view.
		auto*      camera = RE::PlayerCamera::GetSingleton();
		const bool thirdPerson = camera && camera->IsInThirdPerson();

		for (const auto& c : g_changed) {
			if (!root || thirdPerson) {
				break;
			}
			const bool isRoot = c.object == root;
			bool       stillOurs = isRoot;
			if (!isRoot) {
				for (const auto* name : kHeadNodes) {
					if (root->GetObjectByName(RE::BSFixedString{ name }) == c.object) {
						stillOurs = true;
						break;
					}
				}
			}
			if (stillOurs && c.wasCulled && !IsCulled(c.object)) {
				SetCulled(c.object, true);
			}
		}
		if (!g_changed.empty()) {
			REX::INFO("Player visibility restored ({} objects)", g_changed.size());
		}
		g_changed.clear();
	}
}
