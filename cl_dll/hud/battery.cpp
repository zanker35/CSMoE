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
	m_iFlags = 0;
	m_enArmorType = Vest;

	HOOK_MESSAGE( Battery );
	HOOK_MESSAGE( ArmorType );
	gHUD.AddHudElem( this );

	return 1;
}

int CHudBattery::VidInit( void )
{
	m_NEWHUD_hEmpty[Vest].SetSpriteByName("suit_empty_new");
	m_NEWHUD_hFull[Vest].SetSpriteByName("suit_full_new");
	m_NEWHUD_hEmpty[VestHelm].SetSpriteByName("suithelmet_empty_new");
	m_NEWHUD_hFull[VestHelm].SetSpriteByName("suithelmet_full_new");

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
		m_iBat = x;
	}

	return 1;
}

int CHudBattery::Draw( float flTime )
{
	if ((gHUD.m_iHideHUDDisplay & HIDEHUD_HEALTH) ||
		!(gHUD.m_iWeaponBits & (1 << WEAPON_SUIT)))
		return 1;
	if (gHUD.m_NEWHUD_number_0 >= 0 && m_NEWHUD_hEmpty[m_enArmorType].spr)
		return DrawNewHudArmor(flTime);
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
