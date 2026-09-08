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
// death notice
//
#include "game/client/hud/hud.h"
#include "game/client/runtime/cl_util.h"
#include "game/client/runtime/parsemsg.h"

#include <string.h>
#include <stdio.h>
#include "game/client/hud/draw_util.h"

#include "engine_api/interfaces/triangleapi.h"
#include <vector>
#ifdef XASH_VGUI2
#include "ui/vgui2/vgui_controls/Controls.h"
#include "ui/vgui2/interfaces/vgui/ILocalize.h"
#endif

float color[3];

DECLARE_MESSAGE( m_DeathNotice, DeathMsg )
DECLARE_MESSAGE( m_DeathNotice, CombatReport )

enum DrawBgType
{
	DB_NONE, 
	DB_KILL,
	DB_DEATH
};

struct DeathNoticeItem 
{
	char szKiller[MAX_PLAYER_NAME_LENGTH*2];
	char szVictim[MAX_PLAYER_NAME_LENGTH*2];
	int iId;	// the index number of the associated sprite
	bool bSuicide;
	bool bTeamKill;
	bool bNonPlayerKill;
	float flDisplayTime;
	float *KillerColor;
	float *VictimColor;
	int iHeadShotId;

	DrawBgType DrawBg;
};

#define MAX_DEATHNOTICES	4
static int DEATHNOTICE_DISPLAY_TIME = 6;
static int KILLEFFECT_DISPLAY_TIME = 4;
static int KILLICON_DISPLAY_TIME = 1;

#define DEATHNOTICE_TOP		32

DeathNoticeItem rgDeathNoticeList[ MAX_DEATHNOTICES + 1 ];

int CHudDeathNotice :: Init( void )
{
	gHUD.AddHudElem( this );

	HOOK_MESSAGE( DeathMsg );
	HOOK_MESSAGE( CombatReport );

	hud_deathnotice_time = CVAR_CREATE( "hud_deathnotice_time", "6", 0 );
	m_iFlags = 0;

	return 1;
}

void CHudDeathNotice::Reset(void)
{
	m_killNums = 0;
	m_multiKills = 0;
	m_showIcon = false;
	m_showKill = false;
	m_iconIndex = 0;
	m_killEffectTime = 0;
	m_killIconTime = 0;
}

void CHudDeathNotice :: InitHUDData( void )
{
	memset( rgDeathNoticeList, 0, sizeof(rgDeathNoticeList) );
	for (auto &report : m_combatReports)
		report = {};
}


int CHudDeathNotice :: VidInit( void )
{
	m_HUD_d_skull = gHUD.GetSpriteIndex( "d_skull" );
	m_HUD_d_headshot = gHUD.GetSpriteIndex("d_headshot");

	m_KM_Number0 = gHUD.GetSpriteIndex("KM_Number0");
	m_KM_Number1 = gHUD.GetSpriteIndex("KM_Number1");
	m_KM_Number2 = gHUD.GetSpriteIndex("KM_Number2");
	m_KM_Number3 = gHUD.GetSpriteIndex("KM_Number3");
	m_KM_KillText = gHUD.GetSpriteIndex("KM_KillText");
	m_KM_Icon_Head = gHUD.GetSpriteIndex("KM_Icon_Head");
	m_KM_Icon_Knife = gHUD.GetSpriteIndex("KM_Icon_knife");
	m_KM_Icon_Frag = gHUD.GetSpriteIndex("KM_Icon_Frag");
	R_InitTexture(m_NewHud_killBg[0], "resource/hud/deathnotice/killbg_left_new.tga");
	R_InitTexture(m_NewHud_killBg[1], "resource/hud/deathnotice/killbg_center_new.tga");
	R_InitTexture(m_NewHud_killBg[2], "resource/hud/deathnotice/killbg_right_new.tga");
	R_InitTexture(m_NewHud_deathBg[0], "resource/hud/deathnotice/deathbg_left_new.tga");
	R_InitTexture(m_NewHud_deathBg[1], "resource/hud/deathnotice/deathbg_center_new.tga");
	R_InitTexture(m_NewHud_deathBg[2], "resource/hud/deathnotice/deathbg_right_new.tga");
	return 1;
}

void CHudDeathNotice::Shutdown(void)
{
	std::fill(std::begin(m_NewHud_killBg), std::end(m_NewHud_killBg), nullptr);
	std::fill(std::begin(m_NewHud_deathBg), std::end(m_NewHud_deathBg), nullptr);
}

int CHudDeathNotice :: Draw( float flTime )
{
	DrawCombatReports(flTime);
	int x, y, r, g, b, i;

	for( i = 0; i < MAX_DEATHNOTICES; i++ )
	{
		if ( rgDeathNoticeList[i].iId == 0 )
			break;  // we've gone through them all

		if ( rgDeathNoticeList[i].flDisplayTime < flTime )
		{ // display time has expired
			// remove the current item from the list
			memmove( &rgDeathNoticeList[i], &rgDeathNoticeList[i+1], sizeof(DeathNoticeItem) * (MAX_DEATHNOTICES - i) );
			i--;  // continue on the next item;  stop the counter getting incremented
			continue;
		}

		rgDeathNoticeList[i].flDisplayTime = min( rgDeathNoticeList[i].flDisplayTime, flTime + DEATHNOTICE_DISPLAY_TIME );

		// Hide when scoreboard drawing. It will break triapi
		//if ( gViewPort && gViewPort->AllowedToPrintText() )
		//if ( !gHUD.m_iNoConsolePrint )
		{
			// Draw the death notice
			if( !g_iUser1 )
			{
				y = YRES(DEATHNOTICE_TOP) + 2 + (20 * i);  //!!!
			}
			else
			{
				y = ScreenHeight / 5 + 2 + (20 * i);
			}

			int id = (rgDeathNoticeList[i].iId == -1) ? m_HUD_d_skull : rgDeathNoticeList[i].iId;
			x = ScreenWidth - DrawUtils::ConsoleStringLen(rgDeathNoticeList[i].szVictim) - (gHUD.GetSpriteRect(id).right - gHUD.GetSpriteRect(id).left) - (YRES(5) * 3);
			if( rgDeathNoticeList[i].iHeadShotId )
				x -= gHUD.GetSpriteRect(m_HUD_d_headshot).right - gHUD.GetSpriteRect(m_HUD_d_headshot).left;

			int xMin = x, xOffset = 3;
			if (!rgDeathNoticeList[i].bSuicide)
				xMin -= (5 + DrawUtils::ConsoleStringLen(rgDeathNoticeList[i].szKiller));

			gEngfuncs.pTriAPI->RenderMode(kRenderTransTexture);
			gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255);

			SharedTexture (*DrawBg)[3] = nullptr;
			switch (rgDeathNoticeList[i].DrawBg)
			{
			case DB_KILL:
				DrawBg = &m_NewHud_killBg; break;
			case DB_DEATH:
				DrawBg = &m_NewHud_deathBg; break;
			default:
				break;
			}

			if (DrawBg)
			{
				if((*DrawBg)[0])
				{
					(*DrawBg)[0]->Bind();
					DrawUtils::Draw2DQuadScaled(xMin - 3 - xOffset, y, xMin - 3 - xOffset + 3, y + 16);
				}

				if ((*DrawBg)[1])
				{
					(*DrawBg)[1]->Bind();
					DrawUtils::Draw2DQuadScaled(xMin - 3 - xOffset + 3, y, ScreenWidth - (YRES(5) * 3), y + 16);
				}


				if ((*DrawBg)[2])
				{
					(*DrawBg)[2]->Bind();
					DrawUtils::Draw2DQuadScaled(ScreenWidth - (YRES(5) * 3), y, ScreenWidth - (YRES(5) * 3) + 3, y + 16);
				}
			}

			if ( !rgDeathNoticeList[i].bSuicide )
			{
				x -= (5 + DrawUtils::ConsoleStringLen( rgDeathNoticeList[i].szKiller ) );

				// Draw killers name
				if ( rgDeathNoticeList[i].KillerColor )
					DrawUtils::SetConsoleTextColor( rgDeathNoticeList[i].KillerColor[0], rgDeathNoticeList[i].KillerColor[1], rgDeathNoticeList[i].KillerColor[2] );

				x = 5 + DrawUtils::DrawConsoleString( x, y, rgDeathNoticeList[i].szKiller );
			}
			

			r = 255;
			g = 255;
			b = 255;
			if ( rgDeathNoticeList[i].bTeamKill )
			{
				r = 10;	g = 240; b = 10;  // display it in sickly green
			}

			// Draw death weapon
			SPR_Set( gHUD.GetSprite(id), r, g, b );
			SPR_DrawAdditive( 0, x, y, &gHUD.GetSpriteRect(id) );

			x += (gHUD.GetSpriteRect(id).right - gHUD.GetSpriteRect(id).left);

			if( rgDeathNoticeList[i].iHeadShotId)
			{
				SPR_Set( gHUD.GetSprite(m_HUD_d_headshot), r, g, b );
				SPR_DrawAdditive( 0, x, y, &gHUD.GetSpriteRect(m_HUD_d_headshot));
				x += (gHUD.GetSpriteRect(m_HUD_d_headshot).right - gHUD.GetSpriteRect(m_HUD_d_headshot).left);
			}

			// Draw victims name (if it was a player that was killed)
			if (!rgDeathNoticeList[i].bNonPlayerKill)
			{
				if ( rgDeathNoticeList[i].VictimColor )
					DrawUtils::SetConsoleTextColor( rgDeathNoticeList[i].VictimColor[0], rgDeathNoticeList[i].VictimColor[1], rgDeathNoticeList[i].VictimColor[2] );
				x = DrawUtils::DrawConsoleString( x, y, rgDeathNoticeList[i].szVictim );
			}
		}
	}

	if (m_showKill)
	{
		m_killEffectTime = min(m_killEffectTime, gHUD.m_flTime + KILLEFFECT_DISPLAY_TIME);

		if (gHUD.m_flTime < m_killEffectTime)
		{
			int r = 255, g = 255, b = 255;
			float alpha = (m_killEffectTime - gHUD.m_flTime) / KILLEFFECT_DISPLAY_TIME;
			int numIndex = -1;

			if (alpha > 0)
			{
				r *= alpha;
				g *= alpha;
				b *= alpha;

				switch (m_multiKills)
				{
				case 1:
				{
					numIndex = m_KM_Number0;
					break;
				}

				case 2:
				{
					numIndex = m_KM_Number1;
					break;
				}

				case 3:
				{
					numIndex = m_KM_Number2;
					break;
				}

				case 4:
				{
					numIndex = m_KM_Number3;
					break;
				}
				}

				if (numIndex != -1)
				{
					int numWidth, numHeight;
					int textWidth, textHeight;
					int iconWidth, iconHeight;

					numWidth = gHUD.GetSpriteRect(numIndex).right - gHUD.GetSpriteRect(numIndex).left;
					numHeight = gHUD.GetSpriteRect(numIndex).bottom - gHUD.GetSpriteRect(numIndex).top;
					textWidth = gHUD.GetSpriteRect(m_KM_KillText).right - gHUD.GetSpriteRect(m_KM_KillText).left;
					textHeight = gHUD.GetSpriteRect(m_KM_KillText).bottom - gHUD.GetSpriteRect(m_KM_KillText).top;
					iconWidth = gHUD.GetSpriteRect(m_KM_Icon_Head).right - gHUD.GetSpriteRect(m_KM_Icon_Head).left;
					iconHeight = gHUD.GetSpriteRect(m_KM_Icon_Head).bottom - gHUD.GetSpriteRect(m_KM_Icon_Head).top;

					if (m_multiKills == 1)
						numWidth += 10;

					y = (25.0 * 0.01 * ScreenHeight) - (iconHeight + textHeight) * 0.5;
					x = (50.0 * 0.01 * ScreenWidth) - (numHeight + textWidth) * 0.5;

					SPR_Set(gHUD.GetSprite(numIndex), r, g, b);
					SPR_DrawAdditive(0, x, y - (gHUD.GetSpriteRect(numIndex).bottom + gHUD.GetSpriteRect(m_KM_KillText).top - gHUD.GetSpriteRect(m_KM_KillText).bottom - gHUD.GetSpriteRect(numIndex).top) * 0.6, &gHUD.GetSpriteRect(numIndex));

					SPR_Set(gHUD.GetSprite(m_KM_KillText), r, g, b);
					SPR_DrawAdditive(0, x + numWidth, y, &gHUD.GetSpriteRect(m_KM_KillText));

					x = (50.0 * 0.01 * ScreenWidth) - (iconWidth) * 0.5;
					y = y + textHeight;

					m_killIconTime = min(m_killIconTime, gHUD.m_flTime + KILLICON_DISPLAY_TIME);

					if (m_showIcon)
					{
						alpha = (m_killIconTime - gHUD.m_flTime) / KILLICON_DISPLAY_TIME;

						if (alpha > 0)
						{
							r *= alpha;
							g *= alpha;
							b *= alpha;

							switch (m_iconIndex)
							{
							case 1:
							{
								SPR_Set(gHUD.GetSprite(m_KM_Icon_Head), r, g, b);
								SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_KM_Icon_Head));
								break;
							}

							case 2:
							{
								SPR_Set(gHUD.GetSprite(m_KM_Icon_Knife), r, g, b);
								SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_KM_Icon_Knife));
								break;
							}

							case 3:
							{
								SPR_Set(gHUD.GetSprite(m_KM_Icon_Frag), r, g, b);
								SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_KM_Icon_Frag));
								break;
							}
							}
						}
					}
				}
			}
		}
		else
		{
			m_showKill = false;
			m_showIcon = false;
		}
	}

	if (i == 0 && m_combatReports[0].expires <= flTime && m_combatReports[1].expires <= flTime)
		m_iFlags &= ~HUD_DRAW; // disable hud item

	return 1;
}

// This message handler may be better off elsewhere
namespace {
std::string CombatWeaponName(const std::string &weapon)
{
	const char *name = weapon.c_str();
	if (!strncmp(name, "weapon_", 7)) name += 7;
	else if (!strncmp(name, "knife_", 6)) name += 6;
#ifdef XASH_VGUI2
	const char *tokenName = name;
	if (!strcmp(name, "usp")) tokenName = "USP45";
	else if (!strcmp(name, "deagle")) tokenName = "DesertEagle";
	else if (!strcmp(name, "hegrenade") || !strcmp(name, "grenade")) tokenName = "HE_Grenade";
	char token[128];
	snprintf(token, sizeof(token), "#CSO_%s", tokenName);
	if (vgui2::localize())
	{
		if (const wchar_t *localized = vgui2::localize()->Find(token))
		{
			char translated[256];
			vgui2::localize()->ConvertUnicodeToANSI(localized, translated, sizeof(translated));
			return translated;
		}
	}
#endif
	return name;
}

// Wrap at UTF-8 boundaries, including names and localized custom weapons.
void CombatReportLine(std::vector<std::string> &lines, const std::string &text, int width)
{
	std::string line;
	for (size_t i = 0; i < text.size();)
	{
		size_t end = i + 1;
		while (end < text.size() && (static_cast<unsigned char>(text[end]) & 0xc0) == 0x80)
			++end;
		const auto next = text.substr(i, end - i);
		if (!line.empty() && DrawUtils::ConsoleStringLen((line + next).c_str()) > width)
		{
			lines.push_back(line);
			line.clear();
		}
		line += next;
		i = end;
	}
	if (!line.empty()) lines.push_back(line);
}
}

int CHudDeathNotice::MsgFunc_CombatReport(const char *pszName, int iSize, void *pbuf)
{
	if (iSize < 2) return 0;
	BufferReader reader(pszName, pbuf, iSize);
	const int type = reader.ReadByte();
	const int death = reader.ReadByte();
	if (death > 1) return 0;
	auto &report = m_combatReports[death];
	if (type == combat_report::Begin)
	{
		report = {};
		report.opponent = reader.ReadString();
		report.distance = reader.ReadLong();
		report.expires = gHUD.m_flTime + 8.0f;
		m_iFlags |= HUD_DRAW;
	}
	else if (type == combat_report::Weapon && report.expires > gHUD.m_flTime)
	{
		const std::string weapon = reader.ReadString();
		combat_report::Parts parts{};
		for (auto &part : parts)
		{
			part.hits = reader.ReadLong();
			part.damage = reader.ReadLong();
			if (part.hits < 0 || part.damage < 0) return 0;
		}
		report.weapons[weapon] = parts;
	}
	return 1;
}

void CHudDeathNotice::DrawCombatReports(float time)
{
	static const char *partNames[] = {"其他", "头部", "胸部", "腹部", "左臂", "右臂", "左腿", "右腿"};
	const int padding = 8;
	const int width = min(520, ScreenWidth / 2 - 16);
	int fontWidth = 0, fontHeight = 0;
	DrawUtils::ConsoleStringSize("伤害", &fontWidth, &fontHeight);
	const int lineHeight = max(14, fontHeight) + 2;
	std::vector<std::string> lines[2];
	for (int death = 0; death < 2; ++death)
	{
		const auto &report = m_combatReports[death];
		if (report.expires <= time || report.weapons.empty()) continue;
		auto add = [&](const std::string &line) { CombatReportLine(lines[death], line, width - padding * 2); };
		add(std::string(death ? "被击杀：" : "击杀：") + report.opponent);
		combat_report::Parts total{};
		for (const auto &weapon : report.weapons)
			combat_report::Accumulate(total, weapon.second);
		char line[512];
		snprintf(line, sizeof(line), "距离：%.1f 米", report.distance / 10.0f);
		add(line);
		for (const auto &weapon : report.weapons)
		{
			snprintf(line, sizeof(line), "武器：%s    伤害：%d", CombatWeaponName(weapon.first).c_str(), combat_report::Total(weapon.second));
			add(line);
		}
		for (int part = 0; part < combat_report::kHitGroups; ++part)
		{
			if (!total[part].hits) continue;
			snprintf(line, sizeof(line), "%s：%d 次命中    %d 伤害", partNames[part], total[part].hits, total[part].damage);
			add(line);
		}
	}
	const int bottom = ScreenHeight - max(90, ScreenHeight / 6);
	const int combinedHeight = (lines[0].size() + lines[1].size()) * lineHeight + padding * 5;
	// Normally stack on the right; long simultaneous reports use two columns.
	const bool columns = !lines[0].empty() && !lines[1].empty() && combinedHeight > bottom - 40;
	int cursor = bottom;
	for (int death = 1; death >= 0; --death)
	{
		if (lines[death].empty()) continue;
		const int height = lines[death].size() * lineHeight + padding * 2;
		const int x = (columns && death == 0) ? 16 : ScreenWidth - width - 16;
		const int y = max(32, (columns ? bottom : cursor) - height);
		int rowY = y + padding;
		for (const auto &line : lines[death])
		{
			gEngfuncs.pfnDrawSetTextColor(death ? 1.0f : 0.65f, death ? 0.72f : 0.85f, death ? 0.67f : 1.0f);
			DrawUtils::DrawConsoleString(x + padding, rowY, line.c_str());
			rowY += lineHeight;
		}
		cursor = y - padding;
	}
	gEngfuncs.pfnDrawSetTextColor(1, 1, 1);
}

int CHudDeathNotice :: MsgFunc_DeathMsg( const char *pszName, int iSize, void *pbuf )
{
	m_iFlags |= HUD_DRAW;

	BufferReader reader( pszName, pbuf, iSize );

	int killer = reader.ReadByte();
	int victim = reader.ReadByte();
	int headshot = reader.ReadByte();
	int multiKills = 0;
	int idx = gEngfuncs.GetLocalPlayer()->index;

	char killedwith[32];
	strncpy( killedwith, "d_", sizeof(killedwith) );
	strcat( killedwith, reader.ReadString() );

	//if (gViewPort)
	//	gViewPort->DeathMsg( killer, victim );
	gHUD.m_Scoreboard.DeathMsg( killer, victim );

	gHUD.m_Spectator.DeathMessage(victim);

	for (int j = 0; j < MAX_DEATHNOTICES; j++)
	{
		if (rgDeathNoticeList[j].iId == 0)
			break;

		if (rgDeathNoticeList[j].DrawBg == DB_KILL)
			multiKills++;
	}

	if (1/*cl_killmessage->value*/)
	{
		if (killer == idx && victim != idx)
		{
			m_killNums++;
			m_showIcon = false;

			if (headshot)
			{
				if (!multiKills)
					gEngfuncs.pfnClientCmd("speak \"HeadShot\"\n");

				m_showIcon = true;
				m_iconIndex = 1;
				m_killIconTime = gHUD.m_flTime + KILLICON_DISPLAY_TIME;
			}

			if (!strcmp(killedwith, "d_grenade"))
			{
				if (!multiKills)
					gEngfuncs.pfnClientCmd("speak \"GotIt\"\n");

				m_showIcon = true;
				m_iconIndex = 3;
				m_killIconTime = gHUD.m_flTime + KILLICON_DISPLAY_TIME;
			}
		}

		if (!strcmp(killedwith, "d_knife") && !g_PlayerExtraInfo[killer].zombie)
		{
			if (killer == idx)
			{
				if(!multiKills)
					gEngfuncs.pfnClientCmd("speak \"Humililation\"\n");

				m_showIcon = true;
				m_iconIndex = 2;
				m_killIconTime = gHUD.m_flTime + KILLICON_DISPLAY_TIME;
			}

			if (victim == idx)
			{
				gEngfuncs.pfnClientCmd("speak \"OhNo\"\n");

				m_showIcon = true;
				m_iconIndex = 2;
				m_killIconTime = gHUD.m_flTime + KILLICON_DISPLAY_TIME;
			}
		}

		if (killer == idx && victim != idx)
		{
			switch (multiKills)
			{
			case 0:
			{
				m_showKill = true;
				m_multiKills = 1;
				m_killEffectTime = gHUD.m_flTime + KILLEFFECT_DISPLAY_TIME;
				break;
			}

			case 1:
			{
				gEngfuncs.pfnClientCmd("speak \"DoubleKill\"\n");

				m_showKill = true;
				m_multiKills = 2;
				m_killEffectTime = gHUD.m_flTime + KILLEFFECT_DISPLAY_TIME;
				break;
			}

			case 2:
			{
				gEngfuncs.pfnClientCmd("speak \"TripleKill\"\n");

				m_showKill = true;
				m_multiKills = 3;
				m_killEffectTime = gHUD.m_flTime + KILLEFFECT_DISPLAY_TIME;
				break;
			}

			default:
			{
				gEngfuncs.pfnClientCmd("speak \"MultiKill\"\n");

				m_showKill = true;
				m_multiKills = 4;
				m_killEffectTime = gHUD.m_flTime + KILLEFFECT_DISPLAY_TIME;
				break;
			}
			}

			switch (m_killNums)
			{
			case 5:
			{
				gEngfuncs.pfnClientCmd("speak \"Excellent\"\n");
				break;
			}

			case 10:
			{
				gEngfuncs.pfnClientCmd("speak \"Incredible\"\n");
				break;
			}

			case 15:
			{
				gEngfuncs.pfnClientCmd("speak \"Crazy\"\n");
				break;
			}

			case 20:
			{
				gEngfuncs.pfnClientCmd("speak \"CantBelive\"\n");
				break;
			}

			case 25:
			{
				gEngfuncs.pfnClientCmd("speak \"OutofWorld\"\n");
				break;
			}
			}
		}
	}

	int i;
	for ( i = 0; i < MAX_DEATHNOTICES; i++ )
	{
		if ( rgDeathNoticeList[i].iId == 0 )
			break;
	}
	if ( i == MAX_DEATHNOTICES )
	{ // move the rest of the list forward to make room for this item
		memmove( rgDeathNoticeList, rgDeathNoticeList+1, sizeof(DeathNoticeItem) * MAX_DEATHNOTICES );
		i = MAX_DEATHNOTICES - 1;
	}

	//if (gViewPort)
		//gViewPort->GetAllPlayersInfo();
	gHUD.m_Scoreboard.GetAllPlayersInfo();

	// Get the Killer's name
	const char *killer_name = g_PlayerInfoList[ killer ].name;
	if ( !killer_name )
	{
		killer_name = "";
		rgDeathNoticeList[i].szKiller[0] = 0;
	}
	else
	{
		rgDeathNoticeList[i].KillerColor = GetClientColor( killer );
		strncpy( rgDeathNoticeList[i].szKiller, killer_name, MAX_PLAYER_NAME_LENGTH );
		rgDeathNoticeList[i].szKiller[MAX_PLAYER_NAME_LENGTH-1] = 0;
	}

	// Get the Victim's name
	const char *victim_name = NULL;
	// If victim is -1, the killer killed a specific, non-player object (like a sentrygun)
	if ( ((signed char)victim) != -1 )
		victim_name = g_PlayerInfoList[ victim ].name;
	if ( !victim_name )
	{
		victim_name = "";
		rgDeathNoticeList[i].szVictim[0] = 0;
	}
	else
	{
		rgDeathNoticeList[i].VictimColor = GetClientColor( victim );
		strncpy( rgDeathNoticeList[i].szVictim, victim_name, MAX_PLAYER_NAME_LENGTH );
		rgDeathNoticeList[i].szVictim[MAX_PLAYER_NAME_LENGTH-1] = 0;
	}

	// Is it a non-player object kill?
	if ( ((signed char)victim) == -1 )
	{
		rgDeathNoticeList[i].bNonPlayerKill = true;

		// Store the object's name in the Victim slot (skip the d_ bit)
		strncpy( rgDeathNoticeList[i].szVictim, killedwith+2, sizeof(killedwith) );
	}
	else
	{
		if ( killer == victim || killer == 0 )
			rgDeathNoticeList[i].bSuicide = true;

		if ( !strncmp( killedwith, "d_teammate", sizeof(killedwith)  ) )
			rgDeathNoticeList[i].bTeamKill = true;
	}

	rgDeathNoticeList[i].iHeadShotId = headshot;

	// Find the sprite in the list
	int spr = gHUD.GetSpriteIndex( killedwith );

	rgDeathNoticeList[i].iId = spr;

	rgDeathNoticeList[i].flDisplayTime = gHUD.m_flTime + hud_deathnotice_time->value;


	if (victim == idx)
		rgDeathNoticeList[i].DrawBg = DB_DEATH;
	else if (killer == idx)
		rgDeathNoticeList[i].DrawBg = DB_KILL;
	else
		rgDeathNoticeList[i].DrawBg = DB_NONE;

	if (rgDeathNoticeList[i].bNonPlayerKill)
	{
		ConsolePrint( rgDeathNoticeList[i].szKiller );
		ConsolePrint( " killed a " );
		ConsolePrint( rgDeathNoticeList[i].szVictim );
		ConsolePrint( "\n" );
	}
	else
	{
		// record the death notice in the console
		if ( rgDeathNoticeList[i].bSuicide )
		{
			ConsolePrint( rgDeathNoticeList[i].szVictim );

			if ( !strncmp( killedwith, "d_world", sizeof(killedwith)  ) )
			{
				ConsolePrint( " died" );
			}
			else
			{
				ConsolePrint( " killed self" );
			}
		}
		else if ( rgDeathNoticeList[i].bTeamKill )
		{
			ConsolePrint( rgDeathNoticeList[i].szKiller );
			ConsolePrint( " killed his teammate " );
			ConsolePrint( rgDeathNoticeList[i].szVictim );
		}
		else
		{
			if( headshot )
				ConsolePrint( "*** ");
			ConsolePrint( rgDeathNoticeList[i].szKiller );
			ConsolePrint( " killed " );
			ConsolePrint( rgDeathNoticeList[i].szVictim );
		}

		if ( *killedwith && (*killedwith > 13 ) && strncmp( killedwith, "d_world", sizeof(killedwith) ) && !rgDeathNoticeList[i].bTeamKill )
		{
			if ( headshot )
				ConsolePrint(" with a headshot from ");
			else
				ConsolePrint(" with ");

			ConsolePrint( killedwith+2 ); // skip over the "d_" part
		}

		if( headshot ) ConsolePrint( " ***");
		ConsolePrint( "\n" );
	}

	return 1;
}
