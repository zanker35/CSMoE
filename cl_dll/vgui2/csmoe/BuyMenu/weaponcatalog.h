#ifndef CSO_BUY_WEAPONCATALOG_H
#define CSO_BUY_WEAPONCATALOG_H

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

#include "player/player_const.h"
#include "weapons_const.h"
#include "weapons_buy.h"
#include "weapons_moe_buy.h"

enum MoEWeaponBuyType
{
	BUY_NONE = -1,
	BUY_PISTOL,
	BUY_SHOTGUN,
	BUY_SMG,
	BUY_RIFLE,
	BUY_MG,
	BUY_EQUIP,
	BUY_KNIFE
};

struct BuyMenuWeaponInfo
{
	const char *pszClassName;
	const char *pszDisplayName;
	int iCost;
	InventorySlotType iSlot;
	TeamName team;
	MoEWeaponBuyType iMenu;
};

// Team-neutral starter presets. Saved slots, including empty ones, take priority.
inline constexpr const char *g_BuyMenuDefaultFavorites[5][2] = {
	{"weapon_mp5navy", "weapon_usp"},
	{"weapon_awp", "weapon_deagle"},
	{"weapon_mg3", "weapon_infinityex2"},
	{"weapon_m14ebr", "weapon_infinity"},
	{"weapon_m134", "weapon_anaconda"}
};

// Classic weapons use master's buy aliases; extensions are copied from the
// exact list accepted by the server's MoE_HandleBuyCommands.
inline const std::vector<BuyMenuWeaponInfo> &GetBuyMenuWeapons()
{
	static const std::vector<BuyMenuWeaponInfo> weapons = [] {
		std::vector<BuyMenuWeaponInfo> result = {
			{"weapon_usp", "USP .45", USP_PRICE, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL},
			{"weapon_glock18", "Glock-18", GLOCK18_PRICE, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL},
			{"weapon_p228", "P228", P228_PRICE, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL},
			{"weapon_deagle", "Desert Eagle", DEAGLE_PRICE, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL},
			{"weapon_elite", "Dual Elites", ELITE_PRICE, PISTOL_SLOT, TERRORIST, BUY_PISTOL},
			{"weapon_fiveseven", "Five-Seven", FIVESEVEN_PRICE, PISTOL_SLOT, CT, BUY_PISTOL},
			{"weapon_m3", "M3", M3_PRICE, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN},
			{"weapon_xm1014", "XM1014", XM1014_PRICE, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SHOTGUN},
			{"weapon_mac10", "MAC-10", MAC10_PRICE, PRIMARY_WEAPON_SLOT, TERRORIST, BUY_SMG},
			{"weapon_tmp", "TMP", TMP_PRICE, PRIMARY_WEAPON_SLOT, CT, BUY_SMG},
			{"weapon_mp5navy", "MP5", MP5NAVY_PRICE, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG},
			{"weapon_ump45", "UMP-45", UMP45_PRICE, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG},
			{"weapon_p90", "P90", P90_PRICE, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_SMG},
			{"weapon_galil", "Galil", GALIL_PRICE, PRIMARY_WEAPON_SLOT, TERRORIST, BUY_RIFLE},
			{"weapon_famas", "Famas", FAMAS_PRICE, PRIMARY_WEAPON_SLOT, CT, BUY_RIFLE},
			{"weapon_ak47", "AK-47", AK47_PRICE, PRIMARY_WEAPON_SLOT, TERRORIST, BUY_RIFLE},
			{"weapon_scout", "Scout", SCOUT_PRICE, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE},
			{"weapon_m4a1", "M4A1", M4A1_PRICE, PRIMARY_WEAPON_SLOT, CT, BUY_RIFLE},
			{"weapon_aug", "AUG", AUG_PRICE, PRIMARY_WEAPON_SLOT, CT, BUY_RIFLE},
			{"weapon_sg552", "SG552", SG552_PRICE, PRIMARY_WEAPON_SLOT, TERRORIST, BUY_RIFLE},
			{"weapon_sg550", "SG550", SG550_PRICE, PRIMARY_WEAPON_SLOT, CT, BUY_RIFLE},
			{"weapon_awp", "AWP", AWP_PRICE, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_RIFLE},
			{"weapon_g3sg1", "G3SG1", G3SG1_PRICE, PRIMARY_WEAPON_SLOT, TERRORIST, BUY_RIFLE},
			{"weapon_m249", "M249", M249_PRICE, PRIMARY_WEAPON_SLOT, UNASSIGNED, BUY_MG},
			{"weapon_hegrenade", "#Cstrike_HE_Grenade", HEGRENADE_PRICE, GRENADE_SLOT, UNASSIGNED, BUY_EQUIP}
		};

		const char *smgs[] = {"weapon_k1a", "weapon_mp7a1c"};
		const char *shotguns[] = {"weapon_m1887"};
		const char *machineguns[] = {"weapon_m134", "weapon_mg3"};

		for (const auto &weapon : g_MoEWeaponBuyInfo)
		{
			MoEWeaponBuyType menu = BUY_RIFLE;
			auto isWeapon = [&weapon](const char *name) { return std::strcmp(weapon.pszClassName, name) == 0; };
			if (weapon.iSlot == PISTOL_SLOT)
				menu = BUY_PISTOL;
			else if (weapon.iSlot == KNIFE_SLOT)
				menu = BUY_KNIFE;
			else if (weapon.iSlot == GRENADE_SLOT)
				menu = BUY_EQUIP;
			else if (std::any_of(std::begin(smgs), std::end(smgs), isWeapon))
				menu = BUY_SMG;
			else if (std::any_of(std::begin(shotguns), std::end(shotguns), isWeapon))
				menu = BUY_SHOTGUN;
			else if (std::any_of(std::begin(machineguns), std::end(machineguns), isWeapon))
				menu = BUY_MG;
			result.push_back({weapon.pszClassName, weapon.pszDisplayName, weapon.iCost, weapon.iSlot, weapon.team, menu});
		}
		return result;
	}();
	return weapons;
}

inline const BuyMenuWeaponInfo *FindBuyMenuWeapon(const char *name)
{
	if (!name || !name[0])
		return nullptr;
	const auto &weapons = GetBuyMenuWeapons();
	const auto found = std::find_if(weapons.begin(), weapons.end(), [name](const BuyMenuWeaponInfo &weapon) {
		return std::strcmp(weapon.pszClassName, name) == 0;
	});
	return found == weapons.end() ? nullptr : &*found;
}

inline std::string GetBuyMenuWeaponCommand(const char *name)
{
	if (!FindBuyMenuWeapon(name))
		return {};
	for (const auto &weapon : g_MoEWeaponBuyInfo)
		if (std::strcmp(weapon.pszClassName, name) == 0)
			return std::string("moe_buy ") + name;
	if (std::strcmp(name, "weapon_glock18") == 0)
		return "glock";
	if (std::strcmp(name, "weapon_elite") == 0)
		return "elites";
	if (std::strcmp(name, "weapon_mp5navy") == 0)
		return "mp5";
	if (std::strcmp(name, "weapon_hegrenade") == 0)
		return "hegren";
	return name + std::strlen("weapon_");
}

#endif
