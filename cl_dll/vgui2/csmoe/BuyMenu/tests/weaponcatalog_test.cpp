#include "../weaponcatalog.h"

#include <cassert>
#include <iostream>
#include <set>

int main()
{
	const auto &weapons = GetBuyMenuWeapons();
	std::set<std::string> names;
	for (const auto &weapon : weapons)
	{
		assert(names.insert(weapon.pszClassName).second);
		assert(weapon.pszDisplayName && weapon.pszDisplayName[0]);
		assert(weapon.iMenu >= BUY_PISTOL && weapon.iMenu <= BUY_KNIFE);
		assert(weapon.iCost >= 0);
		assert(!GetBuyMenuWeaponCommand(weapon.pszClassName).empty());
	}

	// Every extension uses the server's own price, slot and accepted command.
	for (const auto &weapon : g_MoEWeaponBuyInfo)
	{
		const auto *item = FindBuyMenuWeapon(weapon.pszClassName);
		assert(item && item->iCost == weapon.iCost && item->iSlot == weapon.iSlot);
		assert(GetBuyMenuWeaponCommand(weapon.pszClassName) == std::string("moe_buy ") + weapon.pszClassName);
	}

	// These standard weapons are NOT accepted by master's moe_buy handler.
	assert(GetBuyMenuWeaponCommand("weapon_ak47") == "ak47");
	assert(GetBuyMenuWeaponCommand("weapon_m4a1") == "m4a1");
	assert(GetBuyMenuWeaponCommand("weapon_glock18") == "glock");
	assert(GetBuyMenuWeaponCommand("weapon_elite") == "elites");
	assert(GetBuyMenuWeaponCommand("weapon_mp5navy") == "mp5");
	assert(GetBuyMenuWeaponCommand("weapon_hegrenade") == "hegrenade");
	assert(GetBuyMenuWeaponCommand("weapon_knife") == "moe_buy weapon_knife");
	assert(FindBuyMenuWeapon("weapon_ak47")->iMenu == BUY_RIFLE);
	assert(FindBuyMenuWeapon("weapon_mp5navy")->iMenu == BUY_SMG);
	assert(FindBuyMenuWeapon("weapon_gatling")->iMenu == BUY_SHOTGUN);
	assert(FindBuyMenuWeapon("weapon_balrog7")->iMenu == BUY_MG);
	assert(FindBuyMenuWeapon("knife_dualsword")->iMenu == BUY_KNIFE);

	for (const auto &preset : g_BuyMenuDefaultFavorites)
	{
		const auto *primary = FindBuyMenuWeapon(preset[0]);
		const auto *secondary = FindBuyMenuWeapon(preset[1]);
		assert(primary && primary->iSlot == PRIMARY_WEAPON_SLOT && primary->team == UNASSIGNED);
		assert(secondary && secondary->iSlot == PISTOL_SLOT && secondary->team == UNASSIGNED);
	}
	assert(FindBuyMenuWeapon("weapon_knife")->iSlot == KNIFE_SLOT);
	assert(FindBuyMenuWeapon("weapon_hegrenade")->iSlot == GRENADE_SLOT);

	assert(FindBuyMenuWeapon(nullptr) == nullptr);
	assert(GetBuyMenuWeaponCommand("").empty());
	assert(GetBuyMenuWeaponCommand("weapon_unknown").empty());
	assert(GetBuyMenuWeaponCommand("weapon_ak47;quit").empty());
	std::cout << "Validated " << weapons.size() << " buy-menu entries, master command routing and 5 team-neutral presets\n";
}
