/*
mods.cpp - CSMoE Server Gameplay : gamemode object factory
Copyright (C) 2018 Moemod Hyakuya

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "game/shared/interfaces/extdll.h"
#include "game/shared/entities/util.h"
#include "game/shared/entities/cbase.h"
#include "game/shared/players/player.h"
#include "game/server/match/modes/mods.h"

#include <tuple>
#include <type_traits>

#include "game/server/match/modes/mod_none.h"
#include "game/server/match/modes/mod_dm.h"
#include "game/server/match/modes/mod_tdm.h"
#include "game/server/match/modes/mod_zb1.h"
#include "game/server/match/modes/mod_zb2.h"

namespace sv {

DLL_GLOBAL IBaseMod *g_pModRunning = nullptr;

template<class T>
IBaseMod *DefaultFactory()
{
	return new T;
}

std::pair<const char *, IBaseMod *(*)()> g_FindList[] = {
	{ "", DefaultFactory<CMod_None> }, // default
	{ "none", DefaultFactory<CMod_None> }, // BTE_MOD_NONE
	{ "dm", DefaultFactory<CMod_DeathMatch> },
	{ "tdm", DefaultFactory<CMod_TeamDeathMatch> },
	{ "zb1", DefaultFactory<CMod_Zombi> },
	{ "zb2", DefaultFactory<CMod_ZombieMod2> },
};

void InstallBteMod(const char *name)
{
	for (auto p : g_FindList)
	{
		if (!strcasecmp(name, p.first))
		{
			g_pModRunning = p.second();
			return;
		}
	}
	CVAR_SET_STRING("mp_gamemode", "none");
	g_pModRunning = g_FindList[0].second(); // default
	return;
}

}
