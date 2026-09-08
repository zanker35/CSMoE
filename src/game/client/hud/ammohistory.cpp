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
//  ammohistory.cpp
//


#include "game/client/hud/hud.h"
#include "game/client/runtime/cl_util.h"
#include "game/client/runtime/parsemsg.h"

#include <string.h>
#include <stdio.h>

#include "game/client/hud/ammohistory.h"
#include "game/client/hud/draw_util.h"
#ifdef XASH_VGUI2
#include "ui/vgui2/vgui_controls/Controls.h"
#include "ui/vgui2/interfaces/vgui/ILocalize.h"
#endif

HistoryResource gHR;

#define AMMO_PICKUP_GAP (gHR.iHistoryGap+5)
#define AMMO_PICKUP_PICK_HEIGHT		(gHUD.m_iFontHeight * 3 + (gHR.iHistoryGap * 2))
#define AMMO_PICKUP_HEIGHT_MAX		(ScreenHeight - 100)

#define MAX_ITEM_NAME	32
int HISTORY_DRAW_TIME = 5;

// keep a list of items
struct ITEM_INFO
{
	char szName[MAX_ITEM_NAME];
	HSPRITE spr;
	wrect_t rect;
};

void HistoryResource :: AddToHistory( int iType, int iId, int iCount )
{
	if ( iType == HISTSLOT_AMMO && !iCount )
		return;  // no amount, so don't add

	if ( (((AMMO_PICKUP_GAP * iCurrentHistorySlot) + AMMO_PICKUP_PICK_HEIGHT) > AMMO_PICKUP_HEIGHT_MAX) || (iCurrentHistorySlot >= MAX_HISTORY) )
	{	// the pic would have to be drawn too high
		// so start from the bottom
		iCurrentHistorySlot = 0;
	}
	
	HIST_ITEM *freeslot = &rgAmmoHistory[iCurrentHistorySlot++];  // default to just writing to the first slot
	HISTORY_DRAW_TIME = gHUD.m_Ammo.m_pHud_DrawHistory_Time->value;

	freeslot->type = iType;
	freeslot->iId = iId;
	freeslot->iCount = iCount;
	freeslot->DisplayTime = gHUD.m_flTime + HISTORY_DRAW_TIME;
}

void HistoryResource :: AddToHistory( int iType, const char *szName, int iCount )
{
	if ( iType != HISTSLOT_ITEM )
		return;

	if ( (((AMMO_PICKUP_GAP * iCurrentHistorySlot) + AMMO_PICKUP_PICK_HEIGHT) > AMMO_PICKUP_HEIGHT_MAX) || (iCurrentHistorySlot >= MAX_HISTORY) )
	{	// the pic would have to be drawn too high
		// so start from the bottom
		iCurrentHistorySlot = 0;
	}

	HIST_ITEM *freeslot = &rgAmmoHistory[iCurrentHistorySlot++];  // default to just writing to the first slot

	// I am really unhappy with all the code in this file
	// I am too, -- a1batross

	int i = gHUD.GetSpriteIndex( szName );
	if ( i == -1 )
		return;  // unknown sprite name, don't add it to history

	freeslot->iId = i;
	freeslot->type = iType;
	freeslot->iCount = iCount;

	HISTORY_DRAW_TIME = gHUD.m_Ammo.m_pHud_DrawHistory_Time->value;
	freeslot->DisplayTime = gHUD.m_flTime + HISTORY_DRAW_TIME;
}


void HistoryResource :: CheckClearHistory( void )
{
	for ( int i = 0; i < MAX_HISTORY; i++ )
	{
		if ( rgAmmoHistory[i].type )
			return;
	}

	iCurrentHistorySlot = 0;
}

void HistoryResource::VidInit()
{
	m_weaponGetSprite = gHUD.GetSpriteIndex("weapon_get_bg_new");
}

// Citrus pickup layout, using the same history ring as the classic HUD.
int HistoryResource::DrawNEWHudAmmoHistory(float flTime)
{
	for (auto &item : rgAmmoHistory)
	{
		if (item.type == HISTSLOT_EMPTY)
			continue;
		item.DisplayTime = min(item.DisplayTime, gHUD.m_flTime + HISTORY_DRAW_TIME);
		if (item.DisplayTime <= flTime)
			memset(&item, 0, sizeof(item));
	}
	CheckClearHistory();

	int ammoRows = 0;
	int weaponRows = 0;
	for (int step = 0; step < MAX_HISTORY; ++step)
	{
		const int index = (iCurrentHistorySlot + MAX_HISTORY - 1 - step) % MAX_HISTORY;
		const HIST_ITEM &item = rgAmmoHistory[index];
		if (item.type == HISTSLOT_EMPTY)
			continue;

		const int alpha = min(255, static_cast<int>((item.DisplayTime - flTime) * 80.0f));
		if (item.type == HISTSLOT_AMMO || item.type == HISTSLOT_ITEM)
		{
			if (ammoRows >= 4)
				continue;

			wrect_t iconRect{};
			HSPRITE icon = 0;
			if (item.type == HISTSLOT_AMMO)
			{
				if (HSPRITE *sprite = gWR.GetAmmoPicFromWeapon(item.iId, iconRect))
					icon = *sprite;
			}
			else
			{
				iconRect = gHUD.GetSpriteRect(item.iId);
				icon = gHUD.GetSprite(item.iId);
			}

			const int iconHeight = max(24, iconRect.bottom - iconRect.top);
			const int y = ScreenHeight - 47 - 24 * ammoRows++ - iconHeight;
			int x = ScreenWidth - 224;
			if (icon)
			{
				SPR_Set(icon, alpha, alpha, alpha);
				SPR_DrawAdditive(0, x, y, &iconRect);
			}

			const int count = item.type == HISTSLOT_AMMO ? item.iCount : max(1, item.iCount);
			const int numberY = y + (iconHeight - gHUD.m_NEWHUD_iFontHeight_Dollar) / 2;
			x -= DrawUtils::GetNEWHudNumberWidth(1, count, false, 4);
			DrawUtils::DrawNEWHudNumber(1, x, numberY, count, 255, 255, 255, alpha, false, 4);
			if (gHUD.m_NEWHUD_hPlus >= 0)
			{
				const wrect_t &plusRect = gHUD.GetSpriteRect(gHUD.m_NEWHUD_hPlus);
				SPR_Set(gHUD.GetSprite(gHUD.m_NEWHUD_hPlus), alpha, alpha, alpha);
				SPR_DrawAdditive(0, x - (plusRect.right - plusRect.left), numberY, &plusRect);
			}
		}
		else if (item.type == HISTSLOT_WEAP)
		{
			if (weaponRows >= 4 || item.iId <= 0 || item.iId >= MAX_WEAPONS)
				continue;
			WEAPON *weapon = gWR.GetWeapon(item.iId);
			if (!weapon->iId)
				continue;

			const int height = 23;
			const int width = 170;
			const int x = ScreenWidth - width - 5;
			const int y = ScreenHeight - 160 - 23 * weaponRows++ - height;
			if (m_weaponGetSprite >= 0)
			{
				int r = 255, g = 255, b = 255;
				if (weapon->iSlot != 2 && !gWR.HasAmmo(weapon))
					DrawUtils::UnpackRGB(r, g, b, RGB_REDISH);
				DrawUtils::ScaleColors(r, g, b, alpha);
				SPR_Set(gHUD.GetSprite(m_weaponGetSprite), r, g, b);
				SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_weaponGetSprite));
			}

			const char *name = weapon->szName;
			if (!strncmp(name, "weapon_", 7))
				name += 7;
			else if (!strncmp(name, "knife_", 6))
				name += 6;
#ifdef XASH_VGUI2
			char translated[128];
			const char *tokenName = name;
			if (!strcmp(name, "usp")) tokenName = "USP45";
			else if (!strcmp(name, "deagle")) tokenName = "DesertEagle";
			else if (!strcmp(name, "hegrenade")) tokenName = "HE_Grenade";
			char token[128];
			snprintf(token, sizeof(token), "#CSO_%s", tokenName);
			if (vgui2::localize())
			{
				if (const wchar_t *localized = vgui2::localize()->Find(token))
				{
					vgui2::localize()->ConvertUnicodeToANSI(localized, translated, sizeof(translated));
					name = translated;
				}
			}
#endif
			int textWidth, textHeight;
			gEngfuncs.pfnDrawConsoleStringLen(name, &textWidth, &textHeight);
			const float textColor = alpha / 255.0f;
			gEngfuncs.pfnDrawSetTextColor(textColor, textColor, textColor);
			gEngfuncs.pfnDrawConsoleString(x + 4, y + (height - textHeight) / 2, name);
		}
	}
	return 1;
}

//
// Draw Ammo pickup history
//
