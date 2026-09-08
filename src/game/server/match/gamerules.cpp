
#include "game/shared/interfaces/extdll.h"
#include "game/shared/entities/util.h"
#include "game/shared/entities/cbase.h"
#include "game/shared/players/player.h"
#include "game/shared/weapons/weapons.h"
#include "game/shared/entities/gamerules.h"
#include "game/shared/entities/skill.h"
#include "game/server/match/game.h"
#include "game/server/runtime/globals.h"

#include "base/utllinkedlist.h"
#include "game/shared/movement/pm_shared.h"

#include "game/server/match/GameEvent.h"
#include "game/server/ai/navigation/bot_util.h"


#include "game/server/runtime/steam_util.h"
#include "game/server/ai/navigation/bot_manager.h"

#include "game/shared/strings/shared_util.h"
#include "game/server/ai/navigation/bot_profile.h"

#include "game/server/ai/navigation/nav.h"
#include "game/server/ai/navigation/improv.h"
#include "game/server/ai/navigation/nav_node.h"
#include "game/server/ai/navigation/nav_area.h"
#include "game/server/ai/navigation/nav_file.h"
#include "game/server/ai/navigation/nav_path.h"

#include "game/server/ai/navigation/bot_constants.h"
#include "game/server/ai/cs_bot/cs_bot_manager.h"

#include "engine_api/types/cvardef.h"
#include "game/server/match/modes/mods.h"

namespace sv {

/*
* Globals initialization
*/
CHalfLifeMultiplay *g_pGameRules = NULL;

BOOL CGameRules::CanHaveAmmo(CBasePlayer *pPlayer, const char *pszAmmoName, int iMaxCarry)
{
	int iAmmoIndex;

	if (pszAmmoName != NULL) {
		iAmmoIndex = pPlayer->GetAmmoIndex(pszAmmoName);

		if (iAmmoIndex > -1) {
			if (pPlayer->AmmoInventory(iAmmoIndex) < iMaxCarry) {
				// player has room for more of this type of ammo
				return TRUE;
			}
		}
	}

	return FALSE;
}

edict_t *CGameRules::GetPlayerSpawnSpot(CBasePlayer *pPlayer)
{
	// gat valid spawn point
	edict_t *pentSpawnSpot = EntSelectSpawnPoint(pPlayer);

	// Move the player to the place it said.
	pPlayer->pev->origin = VARS(pentSpawnSpot)->origin + Vector(0, 0, 1);

	pPlayer->pev->v_angle = g_vecZero;
	pPlayer->pev->velocity = g_vecZero;
	pPlayer->pev->angles = VARS(pentSpawnSpot)->angles;
	pPlayer->pev->punchangle = g_vecZero;
	pPlayer->pev->fixangle = 1;

	return pentSpawnSpot;
}

BOOL CGameRules::CanHavePlayerItem(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon)
{
	// only living players can have items
	if (pPlayer->pev->deadflag != DEAD_NO) {
		return FALSE;
	}

	CCSBotManager *ctrl = TheCSBots();

	if (pPlayer->IsBot() && ctrl != NULL && !ctrl->IsWeaponUseable(pWeapon)) {
		return FALSE;
	}

	if (pWeapon->pszAmmo1()) {
		if (!CanHaveAmmo(pPlayer, pWeapon->pszAmmo1(), pWeapon->iMaxAmmo1())) {
			// we can't carry anymore ammo for this gun. We can only
			// have the gun if we aren't already carrying one of this type
			if (pPlayer->HasPlayerItem(pWeapon)) {
				return FALSE;
			}
		}
	} else {
		// weapon doesn't use ammo, don't take another if you already have it.
		if (pPlayer->HasPlayerItem(pWeapon)) {
			return FALSE;
		}
	}

	// note: will fall through to here if GetItemInfo doesn't fill the struct!
	return TRUE;
}

void CGameRules::RefreshSkillData()
{
	int iSkill = (int) CVAR_GET_FLOAT("skill");

	if (iSkill < 1)
		iSkill = 1;

	else if (iSkill > 3)
		iSkill = 3;

	gSkillData.iSkillLevel = iSkill;
	ALERT(at_console, "\nGAME SKILL LEVEL:%d\n", iSkill);

	gSkillData.monDmg12MM = 8;
	gSkillData.monDmgMP5 = 3;
	gSkillData.monDmg9MM = 5;
	gSkillData.batteryCapacity = 15;
}

CGameRules *InstallGameRules()
{
	SERVER_COMMAND("exec game.cfg\n");
	SERVER_EXECUTE();

	InstallBteMod(gamemode.string);

	gpGlobals->deathmatch = true;

	//return new CHalfLifeMultiplay;
	return dynamic_cast<CHalfLifeMultiplay *>(g_pModRunning);
}

}
