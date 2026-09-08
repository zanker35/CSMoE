/*
money.cpp -- Money HUD Widget
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
#include "triangleapi.h"
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "gamemode/mods_const.h"
#include <string.h>
#include "vgui_parser.h"
#include "draw_util.h"
DECLARE_MESSAGE( m_Money, Money )
DECLARE_MESSAGE( m_Money, BlinkAcct )

int CHudMoney::Init( )
{
	HOOK_MESSAGE( Money );
	HOOK_MESSAGE( BlinkAcct );
	gHUD.AddHudElem(this);
	m_fFade = 0;
	m_iFlags = 0;
	return 1;
}

int CHudMoney::VidInit()
{
	R_InitTexture(m_iDollarBG, "resource/hud/hud_dollar_bg");
	m_NEWHUD_hDollar = gHUD.GetSpriteIndex("dollar_new");
	m_NEWHUD_hMinus = gHUD.GetSpriteIndex("minus_new");

	return 1;
}

int CHudMoney::Draw(float flTime)
{
	if ((gHUD.m_iHideHUDDisplay & HIDEHUD_HEALTH) ||
		!(gHUD.m_iWeaponBits & (1 << WEAPON_SUIT)))
		return 1;
	if (m_iDollarBG && m_NEWHUD_hDollar >= 0 && gHUD.m_NEWHUD_dollar_number_0 >= 0)
		return DrawNewHudMoney(flTime);
	return 1;
}

int CHudMoney::MsgFunc_Money(const char *pszName, int iSize, void *pbuf)
{
	BufferReader buf( pszName, pbuf, iSize );
	int iOldCount = m_iMoneyCount;
	m_iMoneyCount = buf.ReadLong();
	m_iDelta = m_iMoneyCount - iOldCount;
	m_fFade = 5.0f; //fade for 5 seconds
	m_iFlags |= HUD_DRAW;
	return 1;
}

int CHudMoney::DrawNewHudMoney(float flTime)
{
	const int width = m_iDollarBG->w();
	const int height = m_iDollarBG->h();
	const int y = gHUD.m_Radar.GetRadarBottom() + 7;
	const int numberY = y + max(0, (height - gHUD.m_NEWHUD_iFontHeight_Dollar) / 2);
	m_iDollarBG->Draw2DQuadScaled(0, y, width, y + height);

	m_fFade = max(0.0f, m_fFade - static_cast<float>(gHUD.m_flTimeDelta));
	if (m_fFade == 0)
		m_iDelta = 0;
	int r = 255, g = 255, b = 255;
	if (m_iBlinkAmt)
	{
		m_fBlinkTime += gHUD.m_flTimeDelta;
		if (m_fBlinkTime > 0.5f)
			DrawUtils::UnpackRGB(r, g, b, RGB_REDISH);
		if (m_fBlinkTime > 1.0f)
		{
			m_fBlinkTime = 0;
			--m_iBlinkAmt;
		}
	}

	SPR_Set(gHUD.GetSprite(m_NEWHUD_hDollar), r, g, b);
	SPR_DrawAdditive(0, 40, numberY, &gHUD.GetSpriteRect(m_NEWHUD_hDollar));
	const int right = 40 + 6 * gHUD.m_NEWHUD_iFontWidth_Dollar;
	DrawUtils::DrawNEWHudNumber(1, right - DrawUtils::GetNEWHudNumberWidth(1, m_iMoneyCount, false, 5), numberY,
		m_iMoneyCount, r, g, b, 255, false, 5);
	if (m_iDelta)
	{
		DrawUtils::UnpackRGB(r, g, b, m_iDelta < 0 ? RGB_REDISH : RGB_GREENISH);
		const int alpha = static_cast<int>(255 * m_fFade / 5);
		const int sign = m_iDelta < 0 ? m_NEWHUD_hMinus : gHUD.m_NEWHUD_hPlus;
		DrawUtils::ScaleColors(r, g, b, alpha);
		if (sign >= 0)
		{
			SPR_Set(gHUD.GetSprite(sign), r, g, b);
			SPR_DrawAdditive(0, 40, numberY + height, &gHUD.GetSpriteRect(sign));
		}
		DrawUtils::DrawNEWHudNumber(1, right - DrawUtils::GetNEWHudNumberWidth(1, abs(m_iDelta), false, 5), numberY + height,
			abs(m_iDelta), r, g, b, 255, false, 5);
	}
	return 1;
}

int CHudMoney::MsgFunc_BlinkAcct(const char *pszName, int iSize, void *pbuf)
{
	BufferReader buf( pszName, pbuf, iSize );

	m_iBlinkAmt = buf.ReadByte();
	m_fBlinkTime = 0;
	return 1;
}
