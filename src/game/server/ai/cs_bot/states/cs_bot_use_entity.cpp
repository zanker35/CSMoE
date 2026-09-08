
#include "game/shared/interfaces/extdll.h"
#include "game/shared/entities/util.h"
#include "game/shared/entities/cbase.h"
#include "game/shared/entities/monsters.h"
#include "game/shared/weapons/weapons.h"
#include "game/shared/players/player.h"
#include "game/shared/entities/gamerules.h"
#include "engine_api/protocol/hltv.h"
#include "game/server/match/game.h"
#include "game/server/entities/trains.h"
#include "game/server/entities/vehicle.h"
#include "game/server/runtime/globals.h"

#include "game/server/ai/bot_include.h"

namespace sv {

// Face the entity and "use" it
// NOTE: This state assumes we are standing in range of the entity to be used, with no obstructions.

void UseEntityState::OnEnter(CCSBot *me)
{
	;
}

void UseEntityState::OnUpdate(CCSBot *me)
{
	// in the very rare situation where two or more bots "used" a hostage at the same time,
	// one bot will fail and needs to time out of this state
	constexpr auto useTimeout = 5.0s;
	if (me->GetStateTimestamp() - gpGlobals->time > useTimeout)
	{
		me->Idle();
		return;
	}

	// look at the entity
	Vector pos = m_entity->pev->origin + Vector(0, 0, HumanHeight * 0.5f);
	me->SetLookAt("Use entity", &pos, PRIORITY_HIGH);

	// if we are looking at the entity, "use" it and exit
	if (me->IsLookingAtPosition(&pos))
	{
		if (TheCSBots()->GetScenario() == CCSBotManager::SCENARIO_RESCUE_HOSTAGES
			&& me->m_iTeam == CT
			&& me->GetTask() == CCSBot::COLLECT_HOSTAGES)
		{
			// we are collecting a hostage, assume we were successful - the update check will correct us if we weren't
			me->IncreaseHostageEscortCount();
		}

		me->UseEnvironment();
		me->Idle();
	}
}

void UseEntityState::OnExit(CCSBot *me)
{
	me->ClearLookAt();
	me->ResetStuckMonitor();
}

}
