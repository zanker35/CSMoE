#include "game/client/hud/hud.h"
#include "game/client/runtime/cl_util.h"
#include "game/client/hud/draw_util.h"
#include "engine_api/interfaces/triangleapi.h"

#include "game/client/hud/legacy/hud_scoreboard_legacy.h"

#include "game/shared/data/mods_const.h"

namespace
{
int NumberDigits(int number, int flags)
{
	int digits = 1;
	for (int remaining = max(0, number); remaining >= 10; remaining /= 10)
		++digits;

	if (flags & DHN_5DIGITS)
		return max(5, digits);
	if (flags & DHN_4DIGITS)
		return max(4, digits);
	if (flags & DHN_3DIGITS)
		return max(3, digits);
	if (flags & DHN_2DIGITS)
		return max(2, digits);
	return digits;
}

int NumberWidth(const wrect_t (&rect)[10], int number, int flags, int gap)
{
	return NumberDigits(number, flags) * (rect[0].right - rect[0].left + gap) - gap;
}
}

CHudScoreBoardLegacy::CHudScoreBoardLegacy()
{
	BuildNumberRC(m_rcNewLarge, 18, 22);
	BuildNumberRC(m_rcNewCenter, 11, 13);
	BuildNumberRC(m_rcNewSmall, 8, 11);
}

CHudScoreBoardLegacy::~CHudScoreBoardLegacy()
{
	
}

int CHudScoreBoardLegacy::VidInit(void)
{
	R_InitTexture(m_pNewBackground, "resource/hud/hud_scoreboard_bg");
	R_InitTexture(m_pNewZombieBackground, "resource/hud/hud_scoreboard_bg_zombie");
	R_InitTexture(m_pNewDeathmatchBackground, "resource/hud/hud_scoreboard_bg_zombie-gaint");
	R_InitTexture(m_pNewLargeRed, "resource/hud/hud_sb_num_big_red");
	R_InitTexture(m_pNewLargeBlue, "resource/hud/hud_sb_num_big_blue");
	R_InitTexture(m_pNewLargeWhite, "resource/hud/hud_sb_num_big_white");
	R_InitTexture(m_pNewCenterNumbers, "resource/hud/hud_sb_num_center");
	R_InitTexture(m_pNewSmallRed, "resource/hud/hud_sb_num_small_red");
	R_InitTexture(m_pNewSmallBlue, "resource/hud/hud_sb_num_small_blue");
	R_InitTexture(m_pNewTRIcon, "resource/hud/hud_text_icon_tr_right");
	R_InitTexture(m_pNewCTIcon, "resource/hud/hud_text_icon_ct_left");
	R_InitTexture(m_pNewZombieIcon, "resource/hud/hud_text_icon_zb_right");
	R_InitTexture(m_pNewHumanIcon, "resource/hud/hud_text_icon_hm_left");
	R_InitTexture(m_pNewWinIcon, "resource/hud/hud_text_icon_win_center");
	R_InitTexture(m_pNewRoundIcon, "resource/hud/hud_text_icon_round");
	R_InitTexture(m_pNewSlash, "resource/hud/hud_sb_num_center_slash");
	R_InitTexture(m_pNewFirstIcon, "resource/hud/hud_text_icon_1st");
	R_InitTexture(m_pNewMyIcon, "resource/hud/hud_text_icon_my");
	R_InitTexture(m_pNewKillIcon, "resource/hud/hud_text_icon_kill_center");
	R_InitTexture(m_pNewTeamKillIcon, "resource/hud/hud_text_icon_team-kill");

	return 1;
}

int CHudScoreBoardLegacy::Draw(float flTime)
{
	if (g_iUser1)
		return 1;

	const auto *localPlayer = gEngfuncs.GetLocalPlayer();
	if (!localPlayer || localPlayer->index < 1 || localPlayer->index > MAX_PLAYERS)
		return 1;
	int idx = localPlayer->index;

	if (g_PlayerExtraInfo[idx].dead == true)
		return 1;

	DrawNewHud(flTime);
	return 1;
}

void CHudScoreBoardLegacy::BuildNumberRC(wrect_t (&rect)[10], int width, int height)
{
	for (int i = 0; i < 10; ++i)
	{
		rect[i].left = i * width;
		rect[i].top = 0;
		rect[i].right = (i + 1) * width;
		rect[i].bottom = height;
	}
}

void CHudScoreBoardLegacy::DrawTexturePart(const CTextureRef &texture, const wrect_t &rect,
	int x, int y, float scale, byte r, byte g, byte b, byte a)
{
	const float width = texture.w();
	const float height = texture.h();
	if (width <= 0 || height <= 0)
		return;

	texture.Draw2DQuadScaled(x, y,
		x + (rect.right - rect.left) * scale, y + (rect.bottom - rect.top) * scale,
		rect.left / width, rect.top / height, rect.right / width, rect.bottom / height,
		r, g, b, a);
}

int CHudScoreBoardLegacy::DrawTexturedNumbers(const CTextureRef &texture,
	const wrect_t (&rect)[10], int number, int x, int y, int flags, int gap, float scale,
	byte r, byte g, byte b, byte a)
{
	number = max(0, number);
	const int digits = NumberDigits(number, flags);
	const float advance = (rect[0].right - rect[0].left + gap) * scale;
	for (int i = digits - 1; i >= 0; --i)
	{
		DrawTexturePart(texture, rect[number % 10], x + i * advance, y, scale, r, g, b, a);
		number /= 10;
	}
	return x + digits * advance - gap * scale;
}

bool CHudScoreBoardLegacy::DrawNewHud(float)
{
	const bool zombie = gHUD.m_iModRunning == MOD_ZB1 || gHUD.m_iModRunning == MOD_ZB2;
	const bool deathmatch = gHUD.m_iModRunning == MOD_DM;
	const bool teamDeathmatch = gHUD.m_iModRunning == MOD_TDM;
	if (gHUD.m_iModRunning != MOD_NONE && !zombie && !deathmatch && !teamDeathmatch)
		return false;

	const CTextureRef *background = zombie ? m_pNewZombieBackground.get() :
		deathmatch ? m_pNewDeathmatchBackground.get() : m_pNewBackground.get();
	const CTextureRef *leftIcon = zombie ? m_pNewZombieIcon.get() :
		deathmatch ? m_pNewFirstIcon.get() : m_pNewTRIcon.get();
	const CTextureRef *rightIcon = zombie ? m_pNewHumanIcon.get() :
		deathmatch ? m_pNewMyIcon.get() : m_pNewCTIcon.get();
	const CTextureRef *centerIcon = zombie ? m_pNewRoundIcon.get() :
		deathmatch ? m_pNewKillIcon.get() :
		teamDeathmatch ? m_pNewTeamKillIcon.get() : m_pNewWinIcon.get();
	const CTextureRef *leftNumbers = deathmatch ? m_pNewLargeWhite.get() : m_pNewLargeRed.get();
	const CTextureRef *rightNumbers = deathmatch ? m_pNewLargeWhite.get() : m_pNewLargeBlue.get();

	if (!background || !leftIcon || !rightIcon || !centerIcon || !leftNumbers ||
		!rightNumbers || !m_pNewCenterNumbers ||
		(!deathmatch && (!m_pNewSmallRed || !m_pNewSmallBlue)) || (zombie && !m_pNewSlash))
		return false;

	const int centerX = ScreenWidth / 2;
	const int backgroundX = (ScreenWidth - background->w()) / 2;
	background->Draw2DQuadScaled(backgroundX, 0, backgroundX + background->w(), background->h());
	leftIcon->Draw2DQuadScaled(centerX - 40 - leftIcon->w(), 10,
		centerX - 40, 10 + leftIcon->h());
	rightIcon->Draw2DQuadScaled(centerX + 40, 10,
		centerX + 40 + rightIcon->w(), 10 + rightIcon->h());

	const int centerLabelY = 10 + max(leftIcon->h(), rightIcon->h());
	const int centerLabelX = centerX - centerIcon->w() / 2;
	centerIcon->Draw2DQuadScaled(centerLabelX, centerLabelY,
		centerLabelX + centerIcon->w(), centerLabelY + centerIcon->h());
	const int centerNumberY = centerLabelY + centerIcon->h() + 2;

	int leftScore = max(0, gHUD.m_Scoreboard.m_iTeamScore_T);
	int rightScore = max(0, gHUD.m_Scoreboard.m_iTeamScore_CT);
	const int scoreLimit = max(0, gHUD.m_Scoreboard.m_iTeamScore_Max);
	int centerNumber = leftScore + rightScore + 1;
	if (deathmatch)
	{
		const int bestPlayer = gHUD.m_Scoreboard.FindBestPlayer();
		leftScore = bestPlayer ? max(0, (int)g_PlayerExtraInfo[bestPlayer].frags) : 0;
		rightScore = max(0, (int)g_PlayerExtraInfo[gEngfuncs.GetLocalPlayer()->index].frags);
		centerNumber = scoreLimit;
	}
	else if (teamDeathmatch)
		centerNumber = scoreLimit;

	if (zombie && scoreLimit > 0)
	{
		const int slashX = centerX - m_pNewSlash->w() / 2;
		m_pNewSlash->Draw2DQuadScaled(slashX, centerNumberY,
			slashX + m_pNewSlash->w(), centerNumberY + m_pNewSlash->h());
		DrawTexturedNumbers(*m_pNewCenterNumbers, m_rcNewCenter, centerNumber,
			slashX - 4 - NumberWidth(m_rcNewCenter, centerNumber, DHN_2DIGITS, 2),
			centerNumberY, DHN_2DIGITS, 2);
		DrawTexturedNumbers(*m_pNewCenterNumbers, m_rcNewCenter, scoreLimit,
			slashX + m_pNewSlash->w() + 4, centerNumberY, DHN_2DIGITS, 2);
	}
	else
	{
		DrawTexturedNumbers(*m_pNewCenterNumbers, m_rcNewCenter, centerNumber,
			centerX - NumberWidth(m_rcNewCenter, centerNumber, DHN_2DIGITS, 2) / 2,
			centerNumberY, DHN_2DIGITS, 2);
	}

	const int scoreY = 15 + max(leftIcon->h(), rightIcon->h());
	const int leftWidth = NumberWidth(m_rcNewLarge, leftScore, DHN_2DIGITS, 3);
	const int rightWidth = NumberWidth(m_rcNewLarge, rightScore, DHN_2DIGITS, 3);
	const float leftScale = min(1.0f, 70.0f / leftWidth);
	const float rightScale = min(1.0f, 70.0f / rightWidth);
	DrawTexturedNumbers(*leftNumbers, m_rcNewLarge, leftScore,
		centerX - 74 - leftWidth * leftScale / 2, scoreY, DHN_2DIGITS, 3, leftScale);
	DrawTexturedNumbers(*rightNumbers, m_rcNewLarge, rightScore,
		centerX + 74 - rightWidth * rightScale / 2, scoreY, DHN_2DIGITS, 3, rightScale);

	if (!deathmatch)
	{
		const int leftAlive = max(0, gHUD.m_Scoreboard.m_iTeamAlive_T);
		const int rightAlive = max(0, gHUD.m_Scoreboard.m_iTeamAlive_CT);
		DrawTexturedNumbers(*m_pNewSmallRed, m_rcNewSmall, leftAlive,
			centerX - 75, 60, DHN_2DIGITS, 1);
		DrawTexturedNumbers(*m_pNewSmallBlue, m_rcNewSmall, rightAlive,
			centerX + 75 - NumberWidth(m_rcNewSmall, rightAlive, DHN_2DIGITS, 1),
			60, DHN_2DIGITS, 1);
	}
	return true;
}
