/*
mod_zb1.cpp - CSMoE Gameplay server : Zombie Mod
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
#include "game/shared/weapons/weapons.h"
#include "game/server/match/game.h"
#include "game/server/players/client.h"
#include "game/server/entities/bmodels.h"
#include "game/server/entities/triggers.h"

#include "game/server/match/modes/mod_zb1.h"

#include <algorithm>
#include <vector>
#include <random>

#include "game/server/ai/bot_include.h"
#include "game/server/players/player_zombie.h"

#include "game/shared/types/u_range.hpp"


namespace sv {

namespace {

bool IsZombieRoundParticipant(const CBasePlayer *player)
{
	return player->m_iJoiningState == JOINED && !player->m_bJustConnected
		&& (player->m_iTeam == CT || player->m_iTeam == TERRORIST)
		&& !(player->pev->flags & (FL_DORMANT | FL_SPECTATOR));
}

bool CanBecomeZombieOrigin(CBasePlayer *player)
{
	return IsZombieRoundParticipant(player) && player->IsAlive()
		&& player->pev->solid == SOLID_SLIDEBOX && !player->IsObserver()
		&& !player->m_bIsZombie;
}

}

CMod_Zombi::CMod_Zombi() // precache
	: m_Countdown (this, std::unique_ptr<CZB1CountdownDelegate>(new CZB1CountdownDelegate(this)) )
{
	m_Countdown.SetCounts(20);

	PRECACHE_SOUND("zombi/human_death_01.wav");
	PRECACHE_SOUND("zombi/human_death_02.wav");
	PRECACHE_GENERIC("sound/Zombi_Ambience.mp3");

	CVAR_SET_FLOAT("sv_maxspeed", 390);
}

void CMod_Zombi::CheckMapConditions()
{
	Base::CheckMapConditions();
	CVAR_SET_STRING("sv_skyname", "hk"); // it should work, but...
//	CVAR_SET_FLOAT("sv_skycolor_r", 150);
//	CVAR_SET_FLOAT("sv_skycolor_g", 150);
//	CVAR_SET_FLOAT("sv_skycolor_b", 150);

	// create fog, however it doesnt work...
	CBaseEntity *fog = nullptr;
	while ((fog = UTIL_FindEntityByClassname(fog, "env_fog")) != nullptr)
	{
		REMOVE_ENTITY(fog->edict());
	}
	CClientFog *newfog = GetClassPtr<CClientFog>(nullptr);
	MAKE_STRING_CLASS("env_fog", newfog->pev);
	newfog->Spawn();
	newfog->m_fDensity = 0.0016f;
	newfog->pev->rendercolor = { 0,0,0 };

	// light
	LIGHT_STYLE(0, "g"); // previous one is "f"
}

void CMod_Zombi::UpdateGameMode(CBasePlayer *pPlayer)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgGameMode, nullptr, pPlayer->edict());
	WRITE_BYTE(MOD_ZB1);
	WRITE_BYTE(0); // Reserved. (weapon restriction? )
	WRITE_BYTE(static_cast<int>(maxrounds.value)); // MaxRound (mp_roundlimit)
	WRITE_BYTE(0); // Reserved. (MaxTime?)

	MESSAGE_END();
}

BOOL CMod_Zombi::ClientConnected(edict_t *pEntity, const char *pszName, const char *pszAddress, char *szRejectReason)
{
	CLIENT_COMMAND(pEntity, "mp3 loop sound/Zombi_Ambience.mp3\n");

	return CHalfLifeMultiplay::ClientConnected(pEntity, pszName, pszAddress, szRejectReason);
}

void CMod_Zombi::ClientDisconnected(edict_t *pClient)
{
	CLIENT_COMMAND(pClient, "mp3 stop\n");

	return CHalfLifeMultiplay::ClientDisconnected(pClient);
}

void CMod_Zombi::Think()
{
	TeamCheck();
	if (!FInfectionStarted())
		m_Countdown.Think();

	if (CheckGameOver())   // someone else quit the game already
		return;

	if (CheckTimeLimit())
		return;

	if (IsFreezePeriod())
	{
		CheckFreezePeriodExpired();
	}

	if (m_fTeamCount != invalid_time_point && m_fTeamCount <= gpGlobals->time)
	{
		RestartRound();
	}

	CheckLevelInitialized();

	if (gpGlobals->time > m_tmNextPeriodicThink)
	{
		CheckRestartRound();
		m_tmNextPeriodicThink = gpGlobals->time + 1.0s;
		// A server may reach the countdown while everyone is still choosing a
		// team. Start infection only after real, spawned participants arrive.
		if (!FInfectionStarted() && m_Countdown.IsExpired() && !m_bRoundTerminating && !IsFreezePeriod())
			PickZombieOrigin();

		if (g_psv_accelerate->value != 5.0f)
		{
			CVAR_SET_FLOAT("sv_accelerate", 5.0);
		}

		if (g_psv_friction->value != 4.0f)
		{
			CVAR_SET_FLOAT("sv_friction", 4.0);
		}

		if (g_psv_stopspeed->value != 75.0f)
		{
			CVAR_SET_FLOAT("sv_stopspeed", 75.0);
		}

		m_iMaxRounds = (int)maxrounds.value;

		if (m_iMaxRounds < 0)
		{
			m_iMaxRounds = 0;
			CVAR_SET_FLOAT("mp_maxrounds", 0);
		}

		m_iMaxRoundsWon = (int)winlimit.value;

		if (m_iMaxRoundsWon < 0)
		{
			m_iMaxRoundsWon = 0;
			CVAR_SET_FLOAT("mp_winlimit", 0);
		}
	}

	if (FInfectionStarted() && TimeRemaining() <= 0s && !m_bRoundTerminating && !m_bFreezePeriod)
		HumanWin();
}

void CMod_Zombi::HumanWin()
{
	//Broadcast("ctwin");
	for(CBasePlayer *player : moe::range::PlayersList())
		CLIENT_COMMAND(player->edict(), "spk win_human\n");

	EndRoundMessage("HumanWin", ROUND_CTS_WIN);
	TerminateRound(5s, WINSTATUS_CTS);
	RoundEndScore(WINSTATUS_CTS);

	++m_iNumCTWins;
	UpdateTeamScores();
}

void CMod_Zombi::ZombieWin()
{
	//Broadcast("terwin");
	for(CBasePlayer *player : moe::range::PlayersList())
		CLIENT_COMMAND(player->edict(), "spk win_zombi\n");

	EndRoundMessage("Zombie Win", ROUND_TERRORISTS_WIN);
	TerminateRound(5s, WINSTATUS_TERRORISTS);
	RoundEndScore(WINSTATUS_TERRORISTS);

	++m_iNumTerroristWins;
	UpdateTeamScores();
}

void CMod_Zombi::CheckWinConditions()
{
	// If a winner has already been determined and game of started.. then get the heck out of here
	if (m_bFirstConnected && m_iRoundWinStatus != WINNER_NONE)
	{
		return;
	}

	int NumDeadCT, NumDeadTerrorist, NumAliveTerrorist, NumAliveCT;
	InitializePlayerCounts(NumAliveTerrorist, NumAliveCT, NumDeadTerrorist, NumDeadCT);

	if (!FInfectionStarted())
		return;
	
	if (!NumAliveTerrorist)
	{
		HumanWin();
	}
	else if (!NumAliveCT)
	{
		ZombieWin();
	}

}

BOOL CMod_Zombi::FInfectionStarted()
{
	return m_bInfectionStarted;
}

void CMod_Zombi::RoundEndScore(int iWinStatus)
{
	for(CBasePlayer *player : moe::range::PlayersList())
	{
		if (player->m_iTeam == TEAM_UNASSIGNED || player->m_iTeam == TEAM_SPECTATOR)
			continue;

		if (iWinStatus == WINSTATUS_CTS)
		{
			if (player->IsAlive() && !player->m_bIsZombie)
			{
				player->pev->frags += 3;

				MESSAGE_BEGIN(MSG_BROADCAST, gmsgScoreInfo);
				WRITE_BYTE(ENTINDEX(player->edict()));
				WRITE_SHORT((int)player->pev->frags);
				WRITE_SHORT(player->m_iDeaths);
				WRITE_SHORT(0);
				WRITE_SHORT(player->m_iTeam);
				MESSAGE_END();
			}
		}
		else if (iWinStatus == WINSTATUS_TERRORISTS)
		{
			if (player->m_bIsZombie)
			{
				player->pev->frags += 1;

				MESSAGE_BEGIN(MSG_BROADCAST, gmsgScoreInfo);
				WRITE_BYTE(ENTINDEX(player->edict()));
				WRITE_SHORT((int)player->pev->frags);
				WRITE_SHORT(player->m_iDeaths);
				WRITE_SHORT(0);
				WRITE_SHORT(player->m_iTeam);
				MESSAGE_END();
			}
		}

		
	}

}

int CMod_Zombi::IPointsForKill(CBasePlayer *pAttacker, CBasePlayer *pKilled)
{
	if (!pAttacker->m_bIsZombie && pKilled->m_bIsZombie)
		return 3;

	return 0;
}

void CMod_Zombi::PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor)
{
	// additional death when zombie being killed.
	if (pVictim->m_bIsZombie)
	{
		pVictim->m_iDeaths++;
	}
	return CHalfLifeMultiplay::PlayerKilled(pVictim, pKiller, pInflictor);
}

void CMod_Zombi::InstallPlayerModStrategy(CBasePlayer *player)
{
	std::unique_ptr<CPlayerModStrategy_ZB1> up(new CPlayerModStrategy_ZB1(player, this));
	player->m_pModStrategy = std::move(up);
}

bool CPlayerModStrategy_ZB1::CanPlayerBuy(bool display)
{
	// is the player alive?
	if (m_pPlayer->pev->deadflag != DEAD_NO)
		return false;

	return !m_pPlayer->m_bIsZombie;
}

int CPlayerModStrategy_ZB1::ComputeMaxAmmo(const char *szAmmoClassName, int iOriginalMax)
{
	int ret = iOriginalMax * 2;

	// do not *2 for machine-guns.
	if (Q_strstr(szAmmoClassName, "box"))
		ret = iOriginalMax;

	return ret;
}

void CPlayerModStrategy_ZB1::OnSpawn()
{
	BecomeHuman();
	return CPlayerModStrategy_Default::OnSpawn();
}

void CPlayerModStrategy_ZB1::Event_OnBecomeZombie(CBasePlayer *who, ZombieLevel iEvolutionLevel)
{
	if (m_pPlayer != who || !IsZombieRoundParticipant(who) || !who->IsAlive())
		return;

	BecomeZombie(iEvolutionLevel);
	if (m_pPlayer->m_bIsZombie)
		m_pPlayer->OnBecomeZombie(iEvolutionLevel);
}

void CPlayerModStrategy_ZB1::BecomeZombie(ZombieLevel iEvolutionLevel)
{
	m_pCharacter = std::make_shared<CZombie_ZB1>(m_pPlayer, iEvolutionLevel);
	EquipZombie();
}

bool CPlayerModStrategy_ZB1::EquipZombie()
{
	// Install the new character before GiveDefaultItems can auto-deploy a
	// weapon through the strategy's virtual OnWeaponDeploy hook.
	m_pPlayer->GiveDefaultItems();
	CBasePlayerItem *knife = m_pPlayer->m_rgpPlayerItems[KNIFE_SLOT];
	if (!knife || knife->m_iId != WEAPON_KNIFE || !m_pPlayer->SwitchWeapon(knife))
	{
		ALERT(at_error, "ZB: aborting infection of %s: zombie knife delivery failed (join=%d dead=%d team=%d)\n",
			STRING(m_pPlayer->pev->netname), m_pPlayer->m_iJoiningState,
			m_pPlayer->pev->deadflag, m_pPlayer->m_iTeam);
		// An unarmed zombie is not a successful infection. Restore a playable
		// human; the round selector can retry once participants are ready.
		BecomeHuman();
		m_pPlayer->GiveDefaultItems();
		m_pPlayer->SetPlayerModel(false);
		return false;
	}
	m_pPlayer->m_bNightVisionOn = false;
	m_pPlayer->ClientCommand("nightvision");
	UTIL_LogPrintf("\"%s<%i><%s>\" triggered \"Became_ZOMBIE\" (weapon \"%s\")\n",
		STRING(m_pPlayer->pev->netname), GETPLAYERUSERID(m_pPlayer->edict()),
		GETPLAYERAUTHID(m_pPlayer->edict()), STRING(knife->pev->classname));
	return true;
}

void CPlayerModStrategy_ZB1::BecomeHuman()
{
	m_pCharacter = std::make_shared<CHuman_ZB1>(m_pPlayer);
}

CPlayerModStrategy_ZB1::CPlayerModStrategy_ZB1(CBasePlayer *player, CMod_Zombi *mp)
	:   CPlayerModStrategy_Zombie(player),
	    m_eventBecomeZombieListener(mp->m_eventBecomeZombie.subscribe(&CPlayerModStrategy_ZB1::Event_OnBecomeZombie, this))
{

}

float CPlayerModStrategy_ZB1::AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType)
{
	flDamage = m_pCharacter->AdjustDamageTaken(pevInflictor, pevAttacker, flDamage, bitsDamageType);
	return CPlayerModStrategy_Zombie::AdjustDamageTaken(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

void CPlayerModStrategy_ZB1::Pain(int m_LastHitGroup, bool HasArmour)
{
	if(m_pPlayer->m_bIsZombie)
		return m_pCharacter->Pain_Zombie(m_LastHitGroup, HasArmour);
	return CPlayerModStrategy_Zombie::Pain(m_LastHitGroup, HasArmour);
}

void CPlayerModStrategy_ZB1::DeathSound()
{
	if(m_pPlayer->m_bIsZombie)
		return m_pCharacter->DeathSound_Zombie();
	return CPlayerModStrategy_Zombie::DeathSound();
}

size_t CMod_Zombi::ZombieOriginNum()
{
	moe::range::PlayersList list;
	const size_t participants = std::count_if(list.begin(), list.end(), CanBecomeZombieOrigin);
	return participants ? participants / 10 + 1 : 0;
}

void CMod_Zombi::PickZombieOrigin()
{
	if (FInfectionStarted() || m_bRoundTerminating || IsFreezePeriod())
		return;

	// build alive player list
	moe::range::PlayersList list;
	std::vector<CBasePlayer *> players {list.begin(), list.end()};
	players.erase(std::remove_if(players.begin(), players.end(), [](CBasePlayer *player) { return !CanBecomeZombieOrigin(player); }), players.end());
	// One participant cannot form opposing sides. Keep waiting without awarding
	// a round; Think retries after additional players finish joining/spawning.
	if (players.size() < 2)
		return;
	const size_t iNumPlayers = players.size();
	const size_t iNumZombies = std::min(ZombieOriginNum(), iNumPlayers - 1);
	if (!iNumZombies)
		return;

	// randomize player list
	std::shuffle(players.begin(), players.end(), std::random_device());

	// pick them
	bool infected = false;
	for (size_t i = 0; i < iNumZombies; ++i)
	{
		MakeZombie(players[i], ZOMBIE_LEVEL_ORIGIN);
		if (!players[i]->m_bIsZombie)
			continue;
		players[i]->pev->health = players[i]->pev->max_health = 1000.0f * iNumPlayers / iNumZombies + 1000.0f;
		players[i]->pev->armorvalue = 1100;
		infected = true;
	}
	if (!infected)
		return;

	TeamCheck();
	if (m_Countdown.IsExpired())
	{
		// Time spent waiting in the menus must not consume the playable round.
		m_fRoundCount = gpGlobals->time;
		for (CBasePlayer *player : moe::range::PlayersList())
			if (IsZombieRoundParticipant(player))
				player->SyncRoundTimer();
	}
	m_bInfectionStarted = true;
	// sound effect
	InfectionSound();
	CheckWinConditions();
}

void CMod_Zombi::HumanInfectionByZombie(CBasePlayer *player, CBasePlayer *attacker)
{
	if (!CanBecomeZombieOrigin(player) || !IsZombieRoundParticipant(attacker) || !attacker->IsAlive() || !attacker->m_bIsZombie)
		return;
	MakeZombie(player, ZOMBIE_LEVEL_HOST);
	if (!player->m_bIsZombie)
		return;
	player->pev->health = player->pev->max_health = std::max(1000, static_cast<int>(attacker->pev->health * 0.5f));
	player->pev->armorvalue = std::max(100, static_cast<int>(attacker->pev->armorvalue * 0.5f));

	InfectionSound();
	PRECACHE_SOUND("zombi/human_death_01.wav");
	PRECACHE_SOUND("zombi/human_death_02.wav");
	EMIT_SOUND(ENT(player->pev), CHAN_BODY, RANDOM_LONG(0, 1) ? "zombi/human_death_01.wav" : "zombi/human_death_02.wav", VOL_NORM, ATTN_NORM);


	DeathNotice(player, attacker->pev, attacker->pev);
	SetScoreAttrib(player, player);
	TeamCheck();
	CheckWinConditions();

	player->m_iDeaths += 1;
	player->AddPoints(0, FALSE);
	attacker->AddPoints(1, FALSE);
}

void CMod_Zombi::InfectionSound()
{
	for(CBasePlayer *player : moe::range::PlayersList())
		CLIENT_COMMAND(player->edict(), "spk zombi_coming_%d\n", RANDOM_LONG(1, 2));
}

void CMod_Zombi::RestartRound()
{
	m_bInfectionStarted = false;
	for(CBasePlayer *player : moe::range::PlayersList())
		player->m_bIsZombie = false;

	TeamCheck();

	CVAR_SET_FLOAT("mp_autoteambalance", 0.0f);

	CHalfLifeMultiplay::RestartRound();
	m_bTCantBuy = false;
}

void CMod_Zombi::TeamCheck()
{
	for(CBasePlayer *player : moe::range::PlayersList())
	{
		// Team assignment must not bypass the team/class selection lifecycle.
		if (!IsZombieRoundParticipant(player))
			continue;
		if ((player->m_bIsZombie && player->m_iTeam != TERRORIST) || (!player->m_bIsZombie && player->m_iTeam != CT))
		{
			player->m_iTeam = player->m_bIsZombie ? TERRORIST :CT;
			TeamChangeUpdate(player, player->m_iTeam);

			TheBots->OnEvent(EVENT_PLAYER_CHANGED_TEAM, player);
		}
	}
}

void CMod_Zombi::PlayerSpawn(CBasePlayer *pPlayer)
{
	pPlayer->m_bIsZombie = false;
	pPlayer->m_bNotKilled = false;
	CHalfLifeMultiplay::PlayerSpawn(pPlayer);
	pPlayer->AddAccount(16000);

	// Open buy menu on spawn
	//ShowVGUIMenu(pPlayer, VGUI_Menu_Buy, (MENU_KEY_1 | MENU_KEY_2 | MENU_KEY_3 | MENU_KEY_4 | MENU_KEY_5 | MENU_KEY_6 | MENU_KEY_7 | MENU_KEY_8 | MENU_KEY_0), "#Buy");
	//pPlayer->m_iMenu = Menu_Buy;
}

BOOL CMod_Zombi::FPlayerCanRespawn(CBasePlayer *pPlayer)
{
	if (!FInfectionStarted())
	{
		// The classic CS twenty-second late-join limit must not strand a
		// player in spectator mode while this round is still waiting to start.
		return pPlayer->m_iJoiningState == JOINED && !pPlayer->m_bJustConnected
			&& pPlayer->m_iNumSpawns == 0 && pPlayer->m_iMenu != Menu_ChooseAppearance
			&& (pPlayer->m_iTeam == CT || pPlayer->m_iTeam == TERRORIST);
	}
	return CHalfLifeMultiplay::FPlayerCanRespawn(pPlayer);
}

BOOL CMod_Zombi::FPlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity *pAttacker)
{
	int iReturn = FALSE;

	if (!FInfectionStarted() || m_bRoundTerminating)
		return FALSE;

	if (!pAttacker || PlayerRelationship(pPlayer, pAttacker) != GR_TEAMMATE)
	{
		iReturn = TRUE;
	}

	if (CVAR_GET_FLOAT("mp_friendlyfire") != 0 || pAttacker == pPlayer)
	{
		iReturn = TRUE;
	}

	CBasePlayer *pAttackerPlayer = dynamic_ent_cast<CBasePlayer *>(pAttacker);
	if (pAttackerPlayer)
	{
		if (pAttackerPlayer->m_bIsZombie && !pPlayer->m_bIsZombie)
		{
			HumanInfectionByZombie(pPlayer, pAttackerPlayer);
			iReturn = false;
		}
	}
	

	return iReturn;
}

void CZB1CountdownDelegate::OnCountdownStart()
{
	for (CBasePlayer* player : moe::range::PlayersList())
		CLIENT_COMMAND(player->edict(), "spk zombi_start\n");
}

inline void CZB1CountdownDelegate::OnCountdownChanged(int iCurrentCount)
{
	UTIL_ClientPrintAll(HUD_PRINTCENTER, "Time Remaining for Zombie Selection: %s1 Sec", UTIL_dtos1(iCurrentCount)); // #CSO_ZombiSelectCount
}

inline void CZB1CountdownDelegate::OnCountdownEnd()
{
	// select zombie
	m_pMod->PickZombieOrigin();
}

}
