/*
timer.cpp -- HUD timer, progress bars, etc
Copyright (C) 2015-2016 a1batross
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

#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#include "game/client/hud/hud.h"
#include "game/client/runtime/cl_util.h"
#include "game/client/runtime/parsemsg.h"
#include "game/client/hud/vgui_parser.h"
#include <string.h>
#include "game/client/hud/draw_util.h"
#include "engine_api/interfaces/triangleapi.h"
#include "game/shared/data/mods_const.h"
#include "game/client/hud/legacy/hud_scoreboard_legacy.h"

DECLARE_MESSAGE( m_Timer, RoundTime )
DECLARE_MESSAGE( m_Timer, ShowTimer )

CHudTimer::CHudTimer()
{
	CHudScoreBoardLegacy::BuildNumberRC(m_iNum_BottomC, 8, 12);
}

int CHudTimer::Init()
{
	HOOK_MESSAGE( RoundTime );
	HOOK_MESSAGE( ShowTimer );
	m_iFlags = 0;
	m_bPanicColorChange = false;
	gHUD.AddHudElem(this);	
	return 1;
}

void CHudTimer::Reset(void)
{
	m_closestRight = ScreenWidth;
}

int CHudTimer::VidInit()
{
	R_InitTexture(m_iNum_Bottom, "resource/hud/hud_sb_num_bottom");
	R_InitTexture(m_iColon_Bottom, "resource/hud/hud_sb_num_bottom_colon");
	return 1;
}

int CHudTimer::Draw( float fTime )
{
	if ((gHUD.m_iHideHUDDisplay & HIDEHUD_HEALTH) ||
		!(gHUD.m_iWeaponBits & (1 << WEAPON_SUIT)))
		return 1;
	if (m_iNum_Bottom && m_iColon_Bottom)
		return DrawNEWHudTimer(fTime);
	return 1;
}

int CHudTimer::MsgFunc_RoundTime(const char *pszName, int iSize, void *pbuf)
{
	BufferReader reader( pszName, pbuf, iSize );
	m_iTime = reader.ReadShort();
	m_fStartTime = gHUD.m_flTime;
	m_iFlags = HUD_DRAW;
	return 1;
}

int CHudTimer::MsgFunc_ShowTimer(const char *pszName, int iSize, void *pbuf)
{
	m_iFlags = HUD_DRAW;
	return 1;
}

#define UPDATE_BOTPROGRESS 0
#define CREATE_BOTPROGRESS 1
#define REMOVE_BOTPROGRESS 2

DECLARE_MESSAGE( m_ProgressBar, BarTime )
DECLARE_MESSAGE( m_ProgressBar, BarTime2 )
DECLARE_MESSAGE( m_ProgressBar, BotProgress )

int CHudProgressBar::Init()
{
	HOOK_MESSAGE( BarTime );
	HOOK_MESSAGE( BarTime2 );
	HOOK_MESSAGE( BotProgress );
	m_iFlags = 0;
	m_szLocalizedHeader = NULL;
	m_szHeader[0] = '\0';
	m_fStartTime = m_fPercent = 0.0f;

	gHUD.AddHudElem(this);
	return 1;
}

int CHudProgressBar::VidInit()
{
	return 1;
}

int CHudProgressBar::Draw( float flTime )
{
	// allow only 0.0..1.0
	if( (m_fPercent < 0.0f) || (m_fPercent > 1.0f) )
	{
		m_iFlags = 0;
		m_fPercent = 0.0f;
		return 1;
	}

	if( m_szLocalizedHeader && m_szLocalizedHeader[0] )
	{
		int r, g, b;
		DrawUtils::UnpackRGB( r, g, b, RGB_YELLOWISH );
		DrawUtils::DrawHudString( ScreenWidth / 4, ScreenHeight / 2, ScreenWidth, (char*)m_szLocalizedHeader, r, g, b );

		DrawUtils::DrawRectangle( ScreenWidth/ 4, ScreenHeight / 2 + gHUD.GetCharHeight(), ScreenWidth/2, ScreenHeight/30 );
		FillRGBA( ScreenWidth/4+2, ScreenHeight/2 + gHUD.GetCharHeight() + 2, m_fPercent * (ScreenWidth/2-4), ScreenHeight/30-4, 255, 140, 0, 255 );
		return 1;
	}

	// prevent SIGFPE
	if( m_iDuration != 0.0f )
	{
		m_fPercent = ((flTime - m_fStartTime) / m_iDuration);
	}
	else
	{
		m_fPercent = 0.0f;
		m_iFlags = 0;
		return 1;
	}

	DrawUtils::DrawRectangle( ScreenWidth/4, ScreenHeight*2/3, ScreenWidth/2, 10 );
	FillRGBA( ScreenWidth/4+2, ScreenHeight*2/3+2, m_fPercent * (ScreenWidth/2-4), 6, 255, 140, 0, 255 );

	return 1;
}

int CHudProgressBar::MsgFunc_BarTime(const char *pszName, int iSize, void *pbuf)
{
	BufferReader reader( pszName, pbuf, iSize );

	m_iDuration = reader.ReadShort();
	m_fPercent = 0.0f;

	m_fStartTime = gHUD.m_flTime;

	m_iFlags = HUD_DRAW;
	return 1;
}

int CHudProgressBar::MsgFunc_BarTime2(const char *pszName, int iSize, void *pbuf)
{
	BufferReader reader( pszName, pbuf, iSize );

	m_iDuration = reader.ReadShort();
	m_fPercent = m_iDuration * (float)reader.ReadShort() / 100.0f;

	m_fStartTime = gHUD.m_flTime;

	m_iFlags = HUD_DRAW;
	return 1;
}

int CHudTimer::DrawNEWHudTimer(float fTime)
{
	const int remaining = max(0, static_cast<int>(m_iTime + m_fStartTime - gHUD.m_flTime));
	const int minutes = remaining / 60;
	const int seconds = remaining % 60;
	int r = 255, g = 255, b = 255;
	if (remaining <= 20)
	{
		m_flPanicTime += gHUD.m_flTimeDelta;
		if (m_flPanicTime > seconds / 40.0f + 0.1f)
		{
			m_flPanicTime = 0;
			m_bPanicColorChange = !m_bPanicColorChange;
		}
		if (m_bPanicColorChange)
			DrawUtils::UnpackRGB(r, g, b, RGB_REDISH);
	}
	const int y = 60;
	const int colonX = ScreenWidth / 2 - 1;
	CHudScoreBoardLegacy::DrawTexturedNumbers(*m_iNum_Bottom, m_iNum_BottomC, minutes, ScreenWidth / 2 - 20, y, DHN_2DIGITS, 1, 1.0, r, g, b);
	m_iColon_Bottom->Draw2DQuadScaled(colonX, y, colonX + m_iColon_Bottom->w(), y + m_iColon_Bottom->h(), 0, 0, 1, 1, r, g, b);
	CHudScoreBoardLegacy::DrawTexturedNumbers(*m_iNum_Bottom, m_iNum_BottomC, seconds, colonX + m_iColon_Bottom->w() + 1, y, DHN_2DIGITS, 1, 1.0, r, g, b);
	m_closestRight = colonX + m_iColon_Bottom->w() + 23;
	return 1;
}

int CHudProgressBar::MsgFunc_BotProgress(const char *pszName, int iSize, void *pbuf)
{
	BufferReader reader( pszName, pbuf, iSize );
	m_iDuration = 0.0f; // don't update our progress bar
	m_iFlags = HUD_DRAW;

	float fNewPercent;
	int flag = reader.ReadByte();
	switch( flag )
	{
	case UPDATE_BOTPROGRESS:
	case CREATE_BOTPROGRESS:
		fNewPercent = (float)reader.ReadByte() / 100.0f;
		// cs behavior:
		// just don't decrease percent values
		if( m_fPercent < fNewPercent )
		{
			m_fPercent = fNewPercent;
		}
		strncpy(m_szHeader, reader.ReadString(), sizeof(m_szHeader));
		if( m_szHeader[0] == '#' )
			m_szLocalizedHeader = Localize(m_szHeader + 1);
		else
			m_szLocalizedHeader = m_szHeader;
		break;
	case REMOVE_BOTPROGRESS:
	default:
		m_fPercent = 0.0f;
		m_szHeader[0] = '\0';
		m_iFlags = 0;
		m_szLocalizedHeader = NULL;
		break;
	}

	return 1;
}
