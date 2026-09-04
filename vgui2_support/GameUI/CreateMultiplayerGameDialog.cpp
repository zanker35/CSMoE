//========= Copyright ?1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "CreateMultiplayerGameDialog.h"
#include "CreateMultiplayerGameServerPage.h"
#include "CreateMultiplayerGameGameplayPage.h"
#include "CreateMultiplayerGameBotPage.h"

#include "EngineInterface.h"
#include "ModInfo.h"

#include <stdio.h>

//#include "Taskbar.h"
//extern CTaskbar *g_pTaskbar;

#include <vgui/ILocalize.h>

#include "FileSystem.h"
#include <KeyValues.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

// CS
#include <vgui_controls/RadioButton.h>

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CCreateMultiplayerGameDialog::CCreateMultiplayerGameDialog(vgui2::Panel *parent) : PropertyDialog(parent, "CreateMultiplayerGameDialog")
{
	SetSize(348, 460);

	m_bBotsEnabled = false;
	if (1/*!stricmp( ModInfo().GetGameDescription(), "Condition Zero" )*/)
	{
		m_bBotsEnabled = true;
	}

	//g_pTaskbar->AddTask(GetVPanel());

	SetTitle("创建游戏", true);
	SetOKButtonText("#GameUI_Start");

	m_pServerPage = new CCreateMultiplayerGameServerPage(this, "ServerPage");
	m_pGameplayPage = new CCreateMultiplayerGameGameplayPage(this, "GameplayPage");
	m_pBotPage = NULL;

	AddPage(m_pServerPage, "房间");
	AddPage(m_pGameplayPage, "游戏规则");

	// create KeyValues object to load/save bot options
	m_pBotSavedData = new KeyValues( "CSBotConfig" );

	// load the bot config data
	if (m_pBotSavedData)
	{
		if ( !m_pBotSavedData->LoadFromFile( vgui2::filesystem(), "CSBotConfig.vdf", "CONFIG" ) )
		{
			m_pBotSavedData->SetString( "bot_difficulty", "0" );
			m_pBotSavedData->SetString( "bot_join_after_player", "1" );
			m_pBotSavedData->SetString( "bot_allow_rogues", "1" );
			m_pBotSavedData->SetString( "bot_allow_pistols", "1" );
			m_pBotSavedData->SetString( "bot_allow_shotguns", "1" );
			m_pBotSavedData->SetString( "bot_allow_sub_machine_guns", "1" );
			m_pBotSavedData->SetString( "bot_allow_machine_guns", "1" );
			m_pBotSavedData->SetString( "bot_allow_rifles", "1" );
			m_pBotSavedData->SetString( "bot_allow_snipers", "1" );
			m_pBotSavedData->SetString( "bot_allow_grenades", "1" );
			m_pBotSavedData->SetString( "bot_allow_shield", "1" );
			m_pBotSavedData->SetString( "bot_join_team", "any" );
			m_pBotSavedData->SetString( "bot_quota", "4" );
			m_pBotSavedData->SetString( "bot_defer_to_human", "0" );
			m_pBotSavedData->SetString( "bot_chatter", "normal" );
		}
	}
	
	if (1/*!stricmp( ModInfo().GetGameDescription(), "Condition Zero" )*/)
	{
		// add a page of advanced bot controls
		// NOTE: These controls will use the bot keys to initialize their values
		m_pBotPage = new CCreateMultiplayerGameBotPage( this, "BotPage", m_pBotSavedData );
		AddPage(m_pBotPage, "机器人");
	}

	int botDiff = 0;

	// do bot cvars exist yet?
	if (engine->pfnGetCvarPointer( "bot_difficulty" ))
	{
		// yes, use the current value of the cvar
		botDiff = (int)(engine->pfnGetCvarFloat( "bot_difficulty" ));
	}
	else
	{
		// no, use the current key value loaded from the config file
		botDiff = m_pBotSavedData->GetInt( "bot_difficulty" );
	}

	if (engine->pfnGetCvarPointer( "bot_quota" ))
	{
		// yes, use the current value of the cvar
		m_pServerPage->SetBotQuota( (int)(engine->pfnGetCvarFloat( "bot_quota" )) );
	}
	else
	{
		// no, use the current key value loaded from the config file
		m_pServerPage->SetBotQuota( atoi( m_pBotSavedData->GetString( "bot_quota" ) ) );
	}

	if ( !m_bBotsEnabled )
	{
		botDiff = -1;
	}
	m_pServerPage->SetBotsEnabled( botDiff >= 0 );

	const char *startMap = m_pBotSavedData->GetString("map", "");
	if (startMap[0])
	{
		m_pServerPage->SetMap(startMap);
	}

	const char* startGameMode = m_pBotSavedData->GetString("mp_gamemode", "none");
	if (startGameMode[0])
	{
		m_pServerPage->SetGameMode(startGameMode);
	}

	// set bot difficulty radio buttons
	static const char *buttonName[] = 
	{
		"SkillLevelEasy",
		"SkillLevelNormal",
		"SkillLevelHard",
		"SkillLevelExpert"
	};
	if (botDiff < 0)
		botDiff = 0;
	else if (botDiff > 3)
		botDiff = 3;
	vgui2::RadioButton *button = static_cast<vgui2::RadioButton *>( m_pServerPage->FindChildByName( buttonName[ botDiff ] ) );
	if (button)
		button->SetSelected( true );
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CCreateMultiplayerGameDialog::~CCreateMultiplayerGameDialog()
{
	if (m_pBotSavedData)
	{
		m_pBotSavedData->deleteThis();
		m_pBotSavedData = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Overrides the base class so it can setup the taskbar title properly
//-----------------------------------------------------------------------------
void CCreateMultiplayerGameDialog::SetTitle(const char *title, bool surfaceTitle)
{
	BaseClass::SetTitle(title,surfaceTitle);
	/*if (g_pTaskbar)
	{
		wchar_t *wTitle;
		wchar_t w_szTitle[1024];

		wTitle = vgui2::localize()->Find(title);

		if(!wTitle)
		{
			vgui2::localize()->ConvertANSIToUnicode(title,w_szTitle,sizeof(w_szTitle));
			wTitle = w_szTitle;
		}

		g_pTaskbar->SetTitle(GetVPanel(), wTitle);
	}*/


}

//-----------------------------------------------------------------------------
// Purpose: runs the server when the OK button is pressed
//-----------------------------------------------------------------------------
bool CCreateMultiplayerGameDialog::OnOK(bool applyOnly)
{
	BaseClass::OnOK(applyOnly);

	// get these values from m_pServerPage and store them temporarily
	char szMapName[64], szHostName[64], szPassword[64], szGameModeName[64];
	const char *pszMapName = m_pServerPage->GetMapName();

	if (!pszMapName)
		return false;
	Q_strncpy(szMapName, pszMapName, sizeof(szMapName));
	Q_strncpy(szHostName, m_pGameplayPage->GetHostName(), sizeof(szHostName));
	Q_strncpy(szPassword, m_pGameplayPage->GetPassword(), sizeof(szPassword));
	Q_strncpy(szGameModeName, m_pServerPage->GetGameMode(), sizeof(szGameModeName));

	char szMapCommand[1024];

	//
	// Changes for Counter-strike game dialog - MSB
	//
	if ( m_bBotsEnabled )
	{
		// get bot difficulty level
		float botSkill = -1.0f;

		if ( m_pServerPage->GetBotsEnabled() )
		{
			if ( m_pServerPage->GetControlInt( "SkillLevelEasy", 0 ) )
				botSkill = 0.0f;

			if ( m_pServerPage->GetControlInt( "SkillLevelNormal", 0 ) )
				botSkill = 1.0f;

			if ( m_pServerPage->GetControlInt( "SkillLevelHard", 0 ) )
				botSkill = 2.0f;

			if ( m_pServerPage->GetControlInt( "SkillLevelExpert", 0 ) )
				botSkill = 3.0f;
		}

		// update keys
		m_pBotSavedData->SetInt( "bot_difficulty", (int)botSkill );

		if ( m_pBotPage )
			m_pBotPage->UpdateKeys( m_pBotSavedData );

		m_pBotSavedData->SetInt( "bot_quota", m_pServerPage->GetBotQuota() );

		// if don't want bots (difficulty == -1), override and set the quota to zero
		if (m_pBotSavedData->GetInt( "bot_difficulty" ) < 0)
		{
			m_pBotSavedData->SetInt( "bot_quota", 0 );
		}

		if (m_pServerPage->IsRandomMapSelected())
		{
			// it's set to random map, just save an
			m_pBotSavedData->SetString("map", "");
		}
		else
		{
			m_pBotSavedData->SetString("map", szMapName);
		}

		m_pBotSavedData->SetString("mp_gamemode", m_pServerPage->GetGameMode());

		// save bot config to a file
		m_pBotSavedData->SaveToFile( vgui2::filesystem(), "CSBotConfig.vdf", "CONFIG" );

		// Apply BOT options before the map command, including string values
		// such as name prefixes without routing them through command parsing.
		for( KeyValues *key = m_pBotSavedData->GetFirstSubKey(); key; key = key->GetNextKey() )
		{
			if (!stricmp(key->GetName(), "map"))
				continue;
			if (!stricmp(key->GetName(), "mp_gamemode"))
				continue;

			if (key->GetDataType() == KeyValues::TYPE_STRING)
				engine->Cvar_Set(key->GetName(), key->GetString());
			else
				engine->Cvar_SetValue(key->GetName(), key->GetInt());
		}

		engine->pfnClientCmd( "mp_autoteambalance 0\n" );
		engine->pfnClientCmd( "mp_limitteams 0\n" );
	}

    engine->Cvar_Set("defaultmap", szMapName);
    engine->Cvar_Set("sv_nat", "0");
    engine->Cvar_Set("mp_gamemode", szGameModeName);
    engine->Cvar_Set("sv_password", szPassword);
    engine->Cvar_Set("hostname", szHostName);

    // master map handles teardown and restart itself. Its disconnect command
    // raises Host_Error, which would discard the queued map. Keep maxplayers
    // as a normal console assignment so an existing server latches it safely.
    snprintf(szMapCommand, sizeof(szMapCommand),
        "deathmatch 1\nsv_lan 1\nmaxplayers %i\nmenu_connectionprogress localserver\nmap %s\n",
        Clamp(m_pGameplayPage->GetMaxPlayers(), 2, 32), szMapName);
    engine->pfnClientCmd(szMapCommand);
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: deletes the dialog when it gets closed
//-----------------------------------------------------------------------------
void CCreateMultiplayerGameDialog::OnClose()
{
	MarkForDeletion();
	BaseClass::OnClose();
}
