/*
weapons_moe_buy.h - CSMoE Gameplay server : Weapon buy command handler
Copyright (C) 2019 Moemod Hymei

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef WEAPONS_MOE_BUY_H
#define WEAPONS_MOE_BUY_H
#ifdef _WIN32
#pragma once
#endif

struct MoEWeaponBuyInfo_s
{
	const char* pszClassName;
	const char* pszDisplayName;
	int iCost;
	InventorySlotType iSlot;
	TeamName team;
};

constexpr MoEWeaponBuyInfo_s g_MoEWeaponBuyInfo[] = {
	{"weapon_infinity",    "Infinity Black/Sliver", 1500, PISTOL_SLOT,         UNASSIGNED},
	{"weapon_infinityex1", "Infinity Red/Sliver",   1500, PISTOL_SLOT,         UNASSIGNED},
	{"weapon_infinityex2", "Infinity Red/Gold",     1500, PISTOL_SLOT,         UNASSIGNED},
	{"weapon_infinityss",  "Infinity Sliver",       600,  PISTOL_SLOT,         UNASSIGNED},
	{"weapon_infinitysr",  "Infinity Red",          600,  PISTOL_SLOT,         UNASSIGNED},
	{"weapon_infinitysb",  "Infinity Black",        600,  PISTOL_SLOT,         UNASSIGNED},
	{"weapon_anaconda",	 "Anaconda",			  650,  PISTOL_SLOT,         UNASSIGNED},
	{"weapon_knife", "Seal Knife",            0,    KNIFE_SLOT,          UNASSIGNED},
	{"weapon_m1887",       "M1887",                 2800, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_k1a",         "K1A",                   1850, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_mp7a1c",      "MP7A1",                 2150, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_m14ebr",      "M14 EBR",               3100, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_m134",      "M134 Minigun",               7000, PRIMARY_WEAPON_SLOT, UNASSIGNED },
	{"weapon_xm8c",        "XM8",                   3250, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_scarl",       "Scar",                  3250, PRIMARY_WEAPON_SLOT, UNASSIGNED},

	{"weapon_mg3",         "MG-3",                  5750, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_m4a1dragon",         "M4A1 Dragon",                  3100, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"weapon_ak47dragon",         "AK47 Dragon",                  2500, PRIMARY_WEAPON_SLOT, UNASSIGNED},
	{"knife_knifedragon",         "Dragon Knife",                  0, KNIFE_SLOT, UNASSIGNED},
	

};

#endif
