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
// status_icons.cpp
//
#include "game/client/hud/hud.h"
#include "game/client/runtime/cl_util.h"
#include "engine_api/types/const.h"
#include "engine_api/protocol/entity_state.h"
#include "engine_api/types/cl_entity.h"
#include <string.h>
#include <stdio.h>
#include "game/client/runtime/parsemsg.h"
#include "engine_api/interfaces/event_api.h"
#include "game/client/prediction/com_weapons.h"
#include "game/shared/data/mods_const.h"
#include "game/client/hud/draw_util.h"
#include "engine_api/interfaces/triangleapi.h"

using namespace cl;

DECLARE_MESSAGE( m_StatusIcons, StatusIcon )

int CHudStatusIcons::Init( void )
{
	HOOK_MESSAGE( StatusIcon );

	gHUD.AddHudElem( this );

	Reset();


	return 1;
}

int CHudStatusIcons::VidInit( void )
{
	m_NEWHUD_hC4_Off = gHUD.GetSpriteIndex("c4_off_new");
	m_NEWHUD_hC4_On = gHUD.GetSpriteIndex("c4_on_new");
	m_NEWHUD_hDefuser = gHUD.GetSpriteIndex("defuser_new");
	return 1;
}

void CHudStatusIcons::Reset( void )
{
	memset( m_IconList, 0, sizeof m_IconList );
	m_iFlags &= ~HUD_DRAW;
}


// Draw status icons along the left-hand side of the screen
int CHudStatusIcons::Draw( float flTime )
{
	if (gEngfuncs.IsSpectateOnly())
		return 1;
	// find starting position to draw from, along right-hand side of screen
	int x = 5;
	int y = ScreenHeight / 2;

	// loop through icon list, and draw any valid icons drawing up from the middle of screen
	for ( int i = 0; i < MAX_ICONSPRITES; i++ )
	{
		if ( m_IconList[i].spr )
		{
			if (!strcmp(m_IconList[i].szSpriteName, "buyzone"))
			{
				SPR_Set(m_IconList[i].spr, 255, 255, 255);
				SPR_DrawAdditive(0, 4, gHUD.m_Radar.GetRadarBottom() + 7, &m_IconList[i].rc);
				continue;
			}
			int sprite = -1;
			int iconX = ScreenWidth - 150;
			if (!strcmp(m_IconList[i].szSpriteName, "c4"))
				sprite = g_bInBombZone && (static_cast<int>(flTime * 10) % 2) ? m_NEWHUD_hC4_On : m_NEWHUD_hC4_Off;
			else if (!strcmp(m_IconList[i].szSpriteName, "defuser"))
			{
				sprite = m_NEWHUD_hDefuser;
				iconX = ScreenWidth - 185;
			}
			if (sprite >= 0)
			{
				SPR_Set(gHUD.GetSprite(sprite), 255, 255, 255);
				SPR_DrawAdditive(0, iconX, ScreenHeight - 82, &gHUD.GetSpriteRect(sprite));
				continue;
			}
			y -= ( m_IconList[i].rc.bottom - m_IconList[i].rc.top ) + 5;

			SPR_Set(m_IconList[i].spr, m_IconList[i].r, m_IconList[i].g, m_IconList[i].b);
			SPR_DrawAdditive(0, x, y, &m_IconList[i].rc);
		}
	}
	
	return 1;
}

// Message handler for StatusIcon message
// accepts five values:
//		byte   : TRUE = ENABLE icon, FALSE = DISABLE icon
//		string : the sprite name to display
//		byte   : red
//		byte   : green
//		byte   : blue
int CHudStatusIcons::MsgFunc_StatusIcon( const char *pszName, int iSize, void *pbuf )
{
	BufferReader reader( pszName, pbuf, iSize );

	int ShouldEnable = reader.ReadByte();
	char *pszIconName = reader.ReadString();

	if ( ShouldEnable )
	{
		int r = reader.ReadByte();
		int g = reader.ReadByte();
		int b = reader.ReadByte();
		EnableIcon( pszIconName, r, g, b );
		m_iFlags |= HUD_DRAW;
	}
	else
	{
		DisableIcon( pszIconName );
	}

	return 1;
}

// add the icon to the icon list, and set it's drawing color
void CHudStatusIcons::EnableIcon( const char *pszIconName, unsigned char red, unsigned char green, unsigned char blue )
{
	// check to see if the sprite is in the current list
	int i;
	for ( i = 0; i < MAX_ICONSPRITES; i++ )
	{
		if ( !stricmp( m_IconList[i].szSpriteName, pszIconName ) )
			break;
	}

	if ( i == MAX_ICONSPRITES )
	{
		// icon not in list, so find an empty slot to add to
		for ( i = 0; i < MAX_ICONSPRITES; i++ )
		{
			if ( !m_IconList[i].spr )
				break;
		}
	}

	// if we've run out of space in the list, overwrite the first icon
	if ( i == MAX_ICONSPRITES )
	{
		i = 0;
	}

	// Load the sprite and add it to the list
	// the sprite must be listed in hud.txt
	int spr_index = gHUD.GetSpriteIndex( pszIconName );
	m_IconList[i].spr = gHUD.GetSprite( spr_index );
	m_IconList[i].rc = gHUD.GetSpriteRect( spr_index );
	m_IconList[i].r = red;
	m_IconList[i].g = green;
	m_IconList[i].b = blue;
	strncpy( m_IconList[i].szSpriteName, pszIconName, MAX_ICONSPRITENAME_LENGTH );

	// Hack: Play Timer sound when a grenade icon is played (in 0.8 seconds)
	if ( strstr(m_IconList[i].szSpriteName, "grenade") )
	{
		cl_entity_t *pthisplayer = gEngfuncs.GetLocalPlayer();
		gEngfuncs.pEventAPI->EV_PlaySound( pthisplayer->index, pthisplayer->origin, CHAN_STATIC, "weapons/timer.wav", 1.0, ATTN_NORM, 0, PITCH_NORM );
	}
}

void CHudStatusIcons::DisableIcon( const char *pszIconName )
{
	// find the sprite is in the current list
	for ( int i = 0; i < MAX_ICONSPRITES; i++ )
	{
		if ( !stricmp( m_IconList[i].szSpriteName, pszIconName ) )
		{
			// clear the item from the list
			memset( &m_IconList[i], 0, sizeof( icon_sprite_t ) );
			return;
		}
	}
}
