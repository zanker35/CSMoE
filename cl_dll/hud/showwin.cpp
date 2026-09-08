/*
 * Round-result banners adapted from citrus's hud/showwin.cpp.
 * Copyright (C) 2015-2016 a1batross
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * In addition, as a special exception, the author gives permission to
 * link the code of this program with the Half-Life Game Engine ("HL
 * Engine") and Modified Game Libraries ("MODs") developed by Valve,
 * L.L.C ("Valve"). You must obey the GNU General Public License in all
 * respects for all of the code used other than the HL Engine and MODs
 * from Valve. If you modify this file, you may extend this exception
 * to your version of the file, but you are not obligated to do so. If
 * you do not wish to do so, delete this exception statement from your
 * version.
 */

#include "hud.h"
#include "cl_util.h"
#include "draw_util.h"
#include "triangleapi.h"

#include <cstring>

int CHudShowWin::Init()
{
	gHUD.AddHudElem(this);
	Reset();
	return 1;
}

int CHudShowWin::VidInit()
{
	R_InitTexture(m_textures[CT_WIN], "resource/basic/ctwin.tga");
	R_InitTexture(m_textures[T_WIN], "resource/basic/trwin.tga");
	R_InitTexture(m_textures[HUMAN_WIN], "resource/zombi/humanwin.tga");
	R_InitTexture(m_textures[ZOMBIE_WIN], "resource/zombi/zombiewin.tga");
	Reset();
	return 1;
}

void CHudShowWin::Reset()
{
	m_iFlags = HUD_DRAW;
	m_displayUntil = 0.0f;
	m_winner = CT_WIN;
}

void CHudShowWin::InitHUDData()
{
	Reset();
}

bool CHudShowWin::Show(Winner winner)
{
	if (!m_textures[winner])
		return false;

	// A normal round ends with both SendAudio and TextMsg. Do not restart
	// the same animation or leave it visible longer for the second event.
	if (m_displayUntil <= gHUD.m_flTime || m_winner != winner)
	{
		m_winner = winner;
		m_displayUntil = gHUD.m_flTime + 5.0f;
	}
	return true;
}

bool CHudShowWin::OnTextMessage(const char *message)
{
	if (!std::strcmp(message, "#CTs_Win"))
		return Show(gHUD.IsZombieMod() ? HUMAN_WIN : CT_WIN);
	if (!std::strcmp(message, "#Terrorists_Win"))
		return Show(gHUD.IsZombieMod() ? ZOMBIE_WIN : T_WIN);

	// Master's zombie modes send these literal TextMsg strings instead of
	// the classic victory radio sentences or citrus's ShowWin message.
	if (!std::strcmp(message, "HumanWin"))
		return Show(HUMAN_WIN);
	if (!std::strcmp(message, "Zombie Win"))
		return Show(ZOMBIE_WIN);

	return false;
}

void CHudShowWin::OnRadioMessage(const char *sentence)
{
	// Also covers bomb, hostage and VIP outcomes whose TextMsg describes
	// the objective rather than saying "CTs Win" or "Terrorists Win".
	if (!std::strcmp(sentence, "%!MRAD_ctwin"))
		Show(gHUD.IsZombieMod() ? HUMAN_WIN : CT_WIN);
	else if (!std::strcmp(sentence, "%!MRAD_terwin"))
		Show(gHUD.IsZombieMod() ? ZOMBIE_WIN : T_WIN);
}

int CHudShowWin::Draw(float time)
{
	if (time >= m_displayUntil || !m_textures[m_winner])
		return 1;

	const bool zombieResult = m_winner == HUMAN_WIN || m_winner == ZOMBIE_WIN;
	const float width = zombieResult ? 450.0f : 330.0f;
	const float height = zombieResult ? 183.0f : 181.0f;
	const float x = (ScreenWidth - width) / 2.0f;
	const float y = (ScreenHeight - height) * 0.175f;
	const float remaining = m_displayUntil - time;
	const int alpha = remaining < 0.5f ? static_cast<int>(remaining * 510.0f) : 255;

	gEngfuncs.pTriAPI->RenderMode(kRenderTransTexture);
	gEngfuncs.pTriAPI->Color4ub(255, 255, 255, alpha);
	m_textures[m_winner]->Bind();
	DrawUtils::Draw2DQuadScaled(x, y, x + width, y + height);
	gEngfuncs.pTriAPI->RenderMode(kRenderNormal);

	return 1;
}

void CHudShowWin::Shutdown()
{
	for (auto &texture : m_textures)
		texture = nullptr;
	Reset();
}
