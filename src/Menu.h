#pragma once

// The in-game settings page (0.9.12), through SFSE Menu Framework (QTR-Modding; optional).
// Mostly a mirror of the INI: each setting shows its current value (your _user.ini's if
// you've changed it, the main INI's otherwise), and a change is written to the _user.ini,
// which the mod reads every couple of seconds outside conversations. Without the framework
// installed nothing happens and the INI works as before.
namespace CCC::Menu
{
	void Register();  // at SFSE's kPostLoad
}
