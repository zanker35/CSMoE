/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
// battery.cpp
//
// implementation of CHudBattery class
//

#include "hud.h"
#include "parsemsg.h"
#include "cl_util.h"
#include "draw_util.h"
#include "triangleapi.h"

DECLARE_MESSAGE( m_Battery, Battery )
DECLARE_MESSAGE( m_Battery, ArmorType )

int CHudBattery::Init( void )
{
	m_iBat = 0;
	m_fFade = 0;
	m_iFlags = 0;
	m_enArmorType = Vest;

	HOOK_MESSAGE( Battery );
	HOOK_MESSAGE( ArmorType );
	gHUD.AddHudElem( this );

	return 1;
}

int CHudBattery::VidInit( void )
{
	m_hEmpty[Vest].SetSpriteByName("suit_empty");
	m_hFull[Vest].SetSpriteByName("suit_full");
	m_hEmpty[VestHelm].SetSpriteByName("suithelmet_empty");
	m_hFull[VestHelm].SetSpriteByName("suithelmet_full");
	m_NEWHUD_hEmpty[Vest].SetSpriteByName("suit_empty_new");
	m_NEWHUD_hFull[Vest].SetSpriteByName("suit_full_new");
	m_NEWHUD_hEmpty[VestHelm].SetSpriteByName("suithelmet_empty_new");
	m_NEWHUD_hFull[VestHelm].SetSpriteByName("suithelmet_full_new");
	R_InitTexture(m_pTexture_Black, "resource/hud/csgo/blackright");
	m_iHeight = m_hFull[Vest].rect.bottom - m_hEmpty[Vest].rect.top;
	m_fFade = 0;

	return 1;
}

void CHudBattery::InitHUDData( void )
{
	m_enArmorType = Vest;
}

int CHudBattery:: MsgFunc_Battery(const char *pszName, int iSize, void *pbuf )
{
	BufferReader reader( pszName, pbuf, iSize );

	m_iFlags |= HUD_DRAW;
	int x = reader.ReadShort();

	if( x != m_iBat )
	{
		m_fFade = FADE_TIME;
		m_iBat = x;
	}

	return 1;
}

int CHudBattery::DrawBar(int x, int y, int width, int height, float f, int& r, int& g, int& b, int& a)
{

	f = bound(0, f, 1);
	int w = f * width;
	if (f)
	{
		if (w <= 0)
			w = 1;
		FillRGBA(x, y, w, height, r, g, b, a);
		x += w;
		width -= w;
	}

	return (x + width);
}

int CHudBattery::Draw( float flTime )
{
	if( gHUD.m_iHideHUDDisplay & HIDEHUD_HEALTH )
		return 1;

	if (!(gHUD.m_iWeaponBits & (1<<(WEAPON_SUIT)) ))
		return 1;
	if (gHUD.m_hudstyle->value == 2 && gHUD.m_NEWHUD_number_0 >= 0 && m_NEWHUD_hEmpty[m_enArmorType].spr)
		return DrawNewHudArmor(flTime);

	int r, g, b, x, y, a, x1;
	int ArmorWidth, ArmorHeight;
	wrect_t rc;

	rc = m_hEmpty[m_enArmorType].rect;

	// battery can go from 0 to 100 so * 0.01 goes from 0 to 1
	rc.top += m_iHeight * ((float)( 100 - ( min( 100, m_iBat ))) * 0.01f );

	DrawUtils::UnpackRGB( r, g, b, (gHUD.m_hudstyle->value == 1)? RGB_WHITE : RGB_YELLOWISH );
	if ((gHUD.m_hudstyle->value == 1))
		a = 255;
	// Has health changed? Flash the health #
	if( m_fFade )
	{
		if( m_fFade > FADE_TIME )
			m_fFade = FADE_TIME;

		m_fFade -= (gHUD.m_flTimeDelta * 20);

		if( m_fFade <= 0 )
		{
			a = 128;
			m_fFade = 0;
		}

		// Fade the health number back to dim
		if (!(gHUD.m_hudstyle->value == 1))
			a = MIN_ALPHA +  (m_fFade / FADE_TIME) * 128;

	}
	else
	{
		if (!(gHUD.m_hudstyle->value == 1))
			a = MIN_ALPHA;
	}

	DrawUtils::ScaleColors( r, g, b, a );

	ArmorWidth = gHUD.GetSpriteRect(gHUD.m_HUD_number_0).right - gHUD.GetSpriteRect(gHUD.m_HUD_number_0).left;
	ArmorHeight = gHUD.GetSpriteRect(gHUD.m_HUD_number_0).bottom - gHUD.GetSpriteRect(gHUD.m_HUD_number_0).top;
	y = ScreenHeight - gHUD.m_iFontHeight - gHUD.m_iFontHeight / 2;
	x = ScreenWidth / 5;

	if ((gHUD.m_hudstyle->value == 1))
	{
		gEngfuncs.pTriAPI->RenderMode(kRenderTransAlpha);
		gEngfuncs.pTriAPI->Color4ub(0, 0, 0, 100);
		m_pTexture_Black->Bind();
		DrawUtils::Draw2DQuadScaled(x - ArmorWidth, y - gHUD.m_iFontHeight / 2, x - ArmorWidth + ScreenWidth / 5 - ArmorWidth, ScreenHeight);
	}

	// make sure we have the right sprite handles
	
	
	SPR_Set( m_hFull[m_enArmorType].spr, r, g, b );
	SPR_DrawAdditive( 0, x, y, &m_hFull[m_enArmorType].rect );

	if( rc.bottom > rc.top )
	{
		SPR_Set( m_hEmpty[m_enArmorType].spr, r, g, b );
		SPR_DrawAdditive( 0, x, y + (rc.top - m_hEmpty[m_enArmorType].rect.top), &rc );
	}



	x += (m_hEmpty[m_enArmorType].rect.right - m_hEmpty[m_enArmorType].rect.left);
	x = DrawUtils::DrawHudNumber( x, y, DHN_3DIGITS|DHN_DRAWZERO, m_iBat, r, g, b );
	if ((gHUD.m_hudstyle->value == 1))
	{
		
		float f = (float)m_iBat / (float)100;
		x = DrawBar(x + ArmorWidth / 2, y + 2.5, ArmorWidth * 5, ArmorHeight * 0.8, f, r, g, b, a);
	}

	return 1;
}

int CHudBattery::DrawNewHudArmor(float flTime)
{
	const auto &empty = m_NEWHUD_hEmpty[m_enArmorType];
	const auto &full = m_NEWHUD_hFull[m_enArmorType];
	const int width = empty.rect.right - empty.rect.left;
	const int height = empty.rect.bottom - empty.rect.top;
	const int x = 29 + gHUD.m_NEWHUD_iFontWidth * 8;
	const int y = ScreenHeight - 15 - gHUD.m_NEWHUD_iFontHeight;
	const int offsetY = abs(gHUD.m_NEWHUD_iFontHeight - height) / 2;
	SPR_Set(empty.spr, 100, 100, 100);
	SPR_DrawAdditive(0, x, y + offsetY, &empty.rect);
	wrect_t rect = full.rect;
	rect.top += height * (100 - max(0, min(100, m_iBat))) / 100;
	if (rect.bottom > rect.top)
	{
		SPR_Set(full.spr, 255, 255, 255);
		SPR_DrawAdditive(0, x, y + offsetY + rect.top - full.rect.top, &rect);
	}
	DrawUtils::DrawNEWHudNumber(0, x + width + 3, y, m_iBat, 255, 255, 255, 255, false, 5);
	return 1;
}

int CHudBattery::MsgFunc_ArmorType(const char *pszName,  int iSize, void *pbuf )
{
	BufferReader reader( pszName, pbuf, iSize );

	m_enArmorType = (armortype_t)reader.ReadByte();

	return 1;
}
