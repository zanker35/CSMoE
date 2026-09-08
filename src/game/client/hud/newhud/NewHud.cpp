/***
* Copyright (c) 1996-2002, Valve LLC. All rights reserved.
* This product contains software technology licensed from Id Software, Inc.
* Use, distribution, and modification are restricted to non-commercial
* enhancements to products from Valve LLC. See the project license.
****/

#include "game/client/hud/hud.h"
#include "game/client/runtime/cl_util.h"

int CHudNewHud::Init()
{
	gHUD.AddHudElem(this);
	m_iFlags = HUD_DRAW;
	return 1;
}

int CHudNewHud::VidInit()
{
	R_InitTexture(m_iCharacterBG, "resource/hud/hud_character_bg");
	R_InitTexture(m_iCharacterBG_New_Bottom, "resource/hud/hud_character_bg_new_bottom");
	R_InitTexture(m_iWeaponBG, "resource/hud/hud_weapon_bg");
	return 1;
}

void CHudNewHud::InitHUDData()
{
	m_szLastModel.clear();
	m_iCharacter.reset();
}

int CHudNewHud::Draw(float flTime)
{
	if ((gHUD.m_iHideHUDDisplay & (HIDEHUD_ALL | HIDEHUD_HEALTH)) ||
		gHUD.m_iIntermission || gEngfuncs.IsSpectateOnly() || g_iUser1 ||
		!(gHUD.m_iWeaponBits & (1 << WEAPON_SUIT)))
		return 0;

	const int bottom = ScreenHeight - 5;
	const auto &background = gHUD.IsZombieMod() ? m_iCharacterBG : m_iCharacterBG_New_Bottom;
	if (background)
		background->Draw2DQuadScaled(0, bottom - background->h(), background->w(), bottom);
	if (m_iWeaponBG && !(gHUD.m_iHideHUDDisplay & HIDEHUD_WEAPONS))
		m_iWeaponBG->Draw2DQuadScaled(ScreenWidth - m_iWeaponBG->w(), bottom - m_iWeaponBG->h(), ScreenWidth, bottom);

	const auto *player = gEngfuncs.GetLocalPlayer();
	if (!player || player->index < 1 || player->index >= MAX_PLAYERS)
		return 1;
	gEngfuncs.pfnGetPlayerInfo(player->index, &g_PlayerInfoList[player->index]);
	const char *model = g_PlayerInfoList[player->index].model;
	if (model && m_szLastModel != model)
	{
		m_szLastModel = model;
		const std::string path = "resource/hud/portrait/hud_" + m_szLastModel;
		m_iCharacter = R_LoadTextureUnique(path.c_str());
	}
	if (m_iCharacter)
		m_iCharacter->Draw2DQuadScaled(0, ScreenHeight - 44 - m_iCharacter->h(), m_iCharacter->w(), ScreenHeight - 44);
	return 1;
}

void CHudNewHud::Shutdown()
{
	m_iCharacterBG.reset();
	m_iCharacterBG_New_Bottom.reset();
	m_iWeaponBG.reset();
	m_iCharacter.reset();
	m_szLastModel.clear();
}
