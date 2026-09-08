/*
spectator_gui.cpp - HUD Overlays
Copyright (C) 2015 a1batross

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

In addition, as a special exception, the author gives permission to
link the code of this program with the Half-Life Game Engine ("HL
Engine") and Modified Game Libraries ("MODs") developed by Valve,
L.L.C ("Valve").  You must obey the GNU General Public License in all
respects for all of the code used other than the HL Engine and MODs
from Valve.  If you modify this file, you may extend this exception
to your version of the file, but you are not obligated to do so.  If
you do not wish to do so, delete this exception statement from your
version.
*/

#include <string.h>

#include "game/client/hud/hud.h"
#include "game/client/runtime/cl_util.h"
#include "game/client/runtime/parsemsg.h"

#include "game/client/hud/vgui_parser.h"
#include "engine_api/interfaces/triangleapi.h"
#include "game/client/hud/draw_util.h"

/*
 * We will draw all elements inside a box. It's size 16x10.
 */

#define XPOS( x ) ( (x) / 16.0f )
#define YPOS( y ) ( (y) / 10.0f  )

#define INT_XPOS(x) int(XPOS(x) * ScreenWidth)
#define INT_YPOS(y) int(YPOS(y) * ScreenHeight)

DECLARE_MESSAGE( m_SpectatorGui, SpecHealth )
DECLARE_MESSAGE( m_SpectatorGui, SpecHealth2 )


// close
// help
// settings
// pip
// autodirector
// showscores

// settings
// // chat msgs
// // show status
// // view cone
// // player names

// chase map overview
// free map overview
// first person
// free look
// free chase camera
// locked chase camera

float CHudSpectatorGui::m_fTextScale = 1.0f;

void __CmdFunc_FindNextPlayerReverse( void )
{
	gHUD.m_Spectator.FindNextPlayer(true);
}

void __CmdFunc_FindNextPlayer( void )
{
	gHUD.m_Spectator.FindNextPlayer(false);
}

int CHudSpectatorGui::Init()
{
	HOOK_MESSAGE( SpecHealth );
	HOOK_MESSAGE( SpecHealth2 );

	HOOK_COMMAND( "_spec_find_next_player_reverse", FindNextPlayerReverse );
	HOOK_COMMAND( "_spec_find_next_player", FindNextPlayer );

	gHUD.AddHudElem(this);
	m_iFlags = HUD_DRAW;
	m_fTextScale = 1.0f;
	return 1;
}

int CHudSpectatorGui::VidInit()
{
	if( !g_iXash )
	{
		ConsolePrint("Warning: CHudSpectatorGui is disabled! Dude, are you running me on old GoldSrc?\n");
		m_iFlags = 0;
		return 0;
	}

	m_fTextScale = ScreenWidth / 1024.0f;
	if( m_fTextScale < 1.0f )
		m_fTextScale = 1.0f;
	R_InitTexture(m_hTimerTexture, "gfx/vgui/timer.tga");
	return 1;
}

void CHudSpectatorGui::Shutdown()
{
	m_hTimerTexture = nullptr;
}


int CHudSpectatorGui::Draw( float flTime )
{
	if( !g_iUser1 )
	{

		return 1;
	}

	// check for scoreboard. We will don't draw it, because screen space econodmy
	/*if( gHUD.m_Scoreboard.m_bForceDraw || !(!gHUD.m_Scoreboard.m_bShowscoresHeld && gHUD.m_Health.m_iHealth > 0 && !gHUD.m_iIntermission ))
		return 1;*/

	// function name says it
	CalcAllNeededData( );

	int r = 255, g = 140, b = 0;

	// at first, draw these silly black bars
	int startpos = 0;
	if( gHUD.m_Spectator.m_pip->value != INSET_OFF ) // pip adjust
	{
		startpos = XRES(gHUD.m_Spectator.m_OverviewData.insetWindowWidth) + XRES(gHUD.m_Spectator.m_OverviewData.insetWindowX);
		startpos *= ScreenWidth / TrueWidth; // hud_scale adjust
	}
	FillRGBABlend(startpos, 0, ScreenWidth - startpos, INT_YPOS(2), 0, 0, 0, 153);
	FillRGBABlend(0, ScreenHeight - INT_YPOS(2), ScreenWidth, INT_YPOS(2), 0, 0, 0, 153);

	// divider
	FillRGBABlend( INT_XPOS(12.5), INT_YPOS(2) * 0.25, 1, INT_YPOS(2) * 0.5, r, g, b, 255 );

	{ // mapname. extradata
		DrawUtils::DrawHudString( INT_XPOS(12.5) + 10, INT_YPOS(2) * 0.25, ScreenWidth, label.m_szMap, r, g, b, m_fTextScale );

		if( !m_bBombPlanted ) // timer remaining
		{
			if( m_hTimerTexture )
			{
				m_hTimerTexture->Bind();
				gEngfuncs.pTriAPI->RenderMode( kRenderTransAlpha );
				DrawUtils::Draw2DQuad( (INT_XPOS(12.5) + 10) * gHUD.m_flScale,
									   (INT_YPOS(2) * 0.5) * gHUD.m_flScale,
									   (INT_XPOS(12.5) + 10 + gHUD.GetCharHeight() * m_fTextScale) * gHUD.m_flScale,
									   (INT_YPOS(2) * 0.5 + gHUD.GetCharHeight() * m_fTextScale) * gHUD.m_flScale );
			}
			DrawUtils::DrawHudString( INT_XPOS(12.5) + gHUD.GetCharHeight() * 1.5 * m_fTextScale + gHUD.GetCharWidth('M') * m_fTextScale, INT_YPOS(2) * 0.5, ScreenWidth,
									  label.m_szTimer, r, g, b, m_fTextScale );
		}
	}


	{ // draw team here
		int iLen = DrawUtils::HudStringLen("Counter-Terrorists:", m_fTextScale );

		DrawUtils::DrawHudString( INT_XPOS(12.5) - iLen - 50 , INT_YPOS(2) * 0.25, INT_XPOS(12.5) - 50, "Counter-Terrorists:", r, g, b, m_fTextScale );
		DrawUtils::DrawHudString( INT_XPOS(12.5) - iLen - 50, INT_YPOS(2) * 0.5, INT_XPOS(12.5) - 50, "Terrorists:", r, g, b, m_fTextScale );
		// count
		DrawUtils::DrawHudNumberString( INT_XPOS(12.5) - 10, INT_YPOS(2) * 0.25, INT_XPOS(12.5) - 50, label.m_iCounterTerrorists, r, g, b, m_fTextScale );
		DrawUtils::DrawHudNumberString( INT_XPOS(12.5) - 10, INT_YPOS(2) * 0.5,  INT_XPOS(12.5) - 50, label.m_iTerrorists,        r, g, b, m_fTextScale );
	}



	//if( !label.m_szNameAndHealth[0] )
	//{
		int iLen = DrawUtils::HudStringLen( label.m_szNameAndHealth, m_fTextScale );
		GetTeamColor( r, g, b, g_PlayerExtraInfo[ g_iUser2 ].teamnumber );
		DrawUtils::DrawHudString( ScreenWidth * 0.5 - iLen * 0.5, INT_YPOS(9) - gHUD.GetCharHeight() * 0.5 * m_fTextScale, ScreenWidth,
								  label.m_szNameAndHealth, r, g, b, m_fTextScale );
	//}

	return 1;
}

void CHudSpectatorGui::CalcAllNeededData( )
{
	// mapname
	if( !label.m_szMap[0] )
	{
		static char szMapNameStripped[55];
		const char *szMapName = gEngfuncs.pfnGetLevelName(); //  "maps/%s.bsp"
		strncpy( szMapNameStripped, szMapName + 5, sizeof( szMapNameStripped ) );
		szMapNameStripped[strlen(szMapNameStripped) - 4] = '\0';
		snprintf( label.m_szMap, sizeof( label.m_szMap ), "Map: %s", szMapNameStripped );
	}

	// team
	/*label.m_iTerrorists        = 0;
	label.m_iCounterTerrorists = 0;
	for( int i = 0; i < MAX_PLAYERS; i++ )
	{
		if( g_PlayerExtraInfo[i].dead )
			continue; // show remaining

		switch( g_PlayerExtraInfo[i].teamnumber )
		{
		case TEAM_CT:
			label.m_iCounterTerrorists++;
		case TEAM_TERRORIST:
			label.m_iTerrorists++;
		}
	}*/

	label.m_iCounterTerrorists = 0;
	label.m_iTerrorists = 0;
	for( int i = 0; i < gHUD.m_Scoreboard.m_iNumTeams; i++ )
	{
		switch( g_TeamInfo[i].teamnumber )
		{
		case TEAM_CT:
			label.m_iCounterTerrorists = g_TeamInfo[i].frags;
			break;
		case TEAM_TERRORIST:
			label.m_iTerrorists = g_TeamInfo[i].frags;
			break;
		}
	}

	// timer
	// time must be positive
	if( !m_bBombPlanted )
	{
		int iMinutes = max( 0, (int)( gHUD.m_Timer.m_iTime + gHUD.m_Timer.m_fStartTime - gHUD.m_flTime ) / 60);
		int iSeconds = max( 0, (int)( gHUD.m_Timer.m_iTime + gHUD.m_Timer.m_fStartTime - gHUD.m_flTime ) - (iMinutes * 60));

		sprintf( label.m_szTimer, "%i:%i", iMinutes, iSeconds );
	}

	// player name
	if( g_iUser2 > 0 && g_iUser2 < MAX_PLAYERS )
	{
		hud_player_info_t sInfo;
		GetPlayerInfo( g_iUser2, &sInfo );

		snprintf( label.m_szNameAndHealth, sizeof( label.m_szNameAndHealth ),
				  "%s (%i)",  sInfo.name, g_PlayerExtraInfo[g_iUser2].health );
	}
	else label.m_szNameAndHealth[0] = '\0';
}

void CHudSpectatorGui::InitHUDData()
{
	m_bBombPlanted = false;
	label.m_szMap[0] = '\0';
}

void CHudSpectatorGui::Reset()
{
	m_bBombPlanted = false;

}

int CHudSpectatorGui::MsgFunc_SpecHealth(const char *pszName, int iSize, void *buf)
{
	BufferReader reader( pszName, buf, iSize );

	int health = reader.ReadByte();

	g_PlayerExtraInfo[g_iUser2].health = health;
	m_iPlayerLastPointedAt = g_iUser2;

	return 1;
}

int CHudSpectatorGui::MsgFunc_SpecHealth2(const char *pszName, int iSize, void *buf)
{
	BufferReader reader( pszName, buf, iSize );

	int health = reader.ReadByte();
	int client = reader.ReadByte();

	g_PlayerExtraInfo[client].health = health;
	m_iPlayerLastPointedAt = g_iUser2;

	return 1;
}
