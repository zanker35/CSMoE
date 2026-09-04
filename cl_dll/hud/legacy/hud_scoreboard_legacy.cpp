#include "hud.h"
#include "cl_util.h"
#include "draw_util.h"
#include "triangleapi.h"

#include "hud_scoreboard_legacy.h"

#include "gamemode/mods_const.h"

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
	: m_iBGIndex(-1), m_iTextIndex(-1), m_iTTextIndex(-1), m_iCTTextIndex(-1),
	  m_iOriginalBG(-1), m_iTeamDeathBG(-1), m_iUnitehBG(-1),
	  m_iNum_L(-1), m_iNum_S(-1), m_iText_CT(-1), m_iText_T(-1),
	  m_iText_TR(-1), m_iText_HM(-1), m_iText_ZB(-1), m_iText_1st(-1),
	  m_iText_Kill(-1), m_iText_Round(-1), m_iNum_csgo(-1)
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
	m_iOriginalBG = gHUD.GetSpriteIndex("SBOriginalBG");
	m_iTeamDeathBG = gHUD.GetSpriteIndex("SBTeamDeathBG");
	m_iUnitehBG = gHUD.GetSpriteIndex("SBUnitehBG");
	m_iNum_L = gHUD.GetSpriteIndex("SBNum_L");
	m_iNum_S = gHUD.GetSpriteIndex("SBNum_S");
	m_iNum_csgo = gHUD.GetSpriteIndex("csgo_number");
	m_iText_CT = gHUD.GetSpriteIndex("SBText_CT");
	m_iText_T = gHUD.GetSpriteIndex("SBText_T");
	m_iText_TR = gHUD.GetSpriteIndex("SBText_TR");
	m_iText_HM = gHUD.GetSpriteIndex("SBText_HM");
	m_iText_ZB = gHUD.GetSpriteIndex("SBText_ZB");
	m_iText_1st = gHUD.GetSpriteIndex("SBText_1st");
	m_iText_Kill = gHUD.GetSpriteIndex("SBText_Kill");
	m_iText_Round = gHUD.GetSpriteIndex("SBText_Round");
	R_InitTexture(m_pTexture_Board, "resource/hud/csgo/board");
	BuildHudNumberRect(m_iNum_csgo, m_rcNumber_csgo, 21, 30, 1, 1);
	BuildHudNumberRect(m_iNum_L, m_rcNumber_Large, 13, 13, 1, 1);
	BuildHudNumberRect(m_iNum_S, m_rcNumber_Small, 10, 10, 1, 1);
	R_InitTexture(m_pNewBackground, "resource/hud/hud_scoreboard_bg");
	R_InitTexture(m_pNewZombieBackground, "resource/hud/hud_scoreboard_bg_gundeath");
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
	Reset();

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

	if (gHUD.m_hudstyle->value == 2 && DrawNewHud(flTime))
		return 1;

	if (m_iBGIndex == -1)
		return 1;

	HSPRITE bgSprite = gHUD.GetSprite(m_iBGIndex);
	wrect_t bgRect = gHUD.GetSpriteRect(m_iBGIndex);
	int bgHeight = (bgRect.bottom - bgRect.top);
	int bgWidth = (bgRect.right - bgRect.left);
	int bgY = 2;
	int bgX = (ScreenWidth - bgWidth) / 2;

	int aliveCT = gHUD.m_Scoreboard.m_iTeamAlive_CT;
	int aliveT = gHUD.m_Scoreboard.m_iTeamAlive_T;
	int textWidth_CTAlive = GetHudNumberWidth(m_iNum_S, m_rcNumber_Small, DHN_2DIGITS | DHN_DRAWZERO, aliveCT);
	int textWidth_TAlive = GetHudNumberWidth(m_iNum_S, m_rcNumber_Small, DHN_2DIGITS | DHN_DRAWZERO, aliveT);
	int scoreCT = gHUD.m_Scoreboard.m_iTeamScore_CT;
	int scoreT = gHUD.m_Scoreboard.m_iTeamScore_T;
	int scoreMax = gHUD.m_Scoreboard.m_iTeamScore_Max;
	int roundNumber = scoreMax ? scoreMax : scoreT + scoreCT + 1;

	if (gHUD.m_hudstyle->value == 1 &&
		gHUD.m_iModRunning == MOD_NONE && m_pTexture_Board)
	{
		int x = ScreenWidth / 2;
		bgY = 5;
		int y = 5;
		const float flScale = 0.0f;

		gEngfuncs.pTriAPI->RenderMode(kRenderTransAlpha);
		gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255);
		m_pTexture_Board->Bind();
		DrawUtils::Draw2DQuadScaled(x - 240 / 2, y, x + 240 / 2, y + 70);

		if (scoreCT >= 100)
		{
			int textWidth_CT = GetHudNumberWidth(m_iNum_L, m_rcNumber_Large, DHN_3DIGITS | DHN_2DIGITS | DHN_DRAWZERO, scoreCT);

			if (textWidth_CT > 0)

				DrawHudNumber(m_iNum_L, m_rcNumber_Large, (ScreenWidth) / 2 - 39, bgY + 50, DHN_3DIGITS | DHN_DRAWZERO, scoreCT, 100, 134, 142);
		}
		else if (scoreCT >= 10)
		{
			int textWidth_CT = GetHudNumberWidth(m_iNum_L, m_rcNumber_Large, DHN_2DIGITS | DHN_DRAWZERO, scoreCT);

			if (textWidth_CT > 0)

				DrawHudNumber(m_iNum_L, m_rcNumber_Large, (ScreenWidth) / 2 - 39, bgY + 50, DHN_2DIGITS | DHN_DRAWZERO, scoreCT, 100, 134, 142);
		}
		else
		{
			int textWidth_CT = GetHudNumberWidth(m_iNum_L, m_rcNumber_Large, DHN_DRAWZERO, scoreCT);

			if (textWidth_CT > 0)

				DrawHudNumber(m_iNum_L, m_rcNumber_Large, (ScreenWidth) / 2 - 33, bgY + 50, DHN_DRAWZERO, scoreCT, 100, 134, 142);
		}

		if (scoreT >= 100)
		{
			int textWidth_T = GetHudNumberWidth(m_iNum_L, m_rcNumber_Large, DHN_3DIGITS | DHN_DRAWZERO, scoreT);

			if (textWidth_T > 0)
				DrawHudNumber(m_iNum_L, m_rcNumber_Large, ((ScreenWidth) / 2) + 39 - (textWidth_T / 2), bgY + 50, DHN_3DIGITS | DHN_DRAWZERO, scoreT, 172, 154, 111);
		}
		else if (scoreT >= 10)
		{
			int textWidth_T = GetHudNumberWidth(m_iNum_L, m_rcNumber_Large, DHN_2DIGITS | DHN_DRAWZERO, scoreT);

			if (textWidth_T > 0)
				DrawHudNumber(m_iNum_L, m_rcNumber_Large, ((ScreenWidth) / 2) + 39 - textWidth_T, bgY + 50, DHN_2DIGITS | DHN_DRAWZERO, scoreT, 172, 154, 111);
		}
		else
		{
			int textWidth_T = GetHudNumberWidth(m_iNum_L, m_rcNumber_Large,  DHN_DRAWZERO, scoreT);

			if (textWidth_T > 0)
				DrawHudNumber(m_iNum_L, m_rcNumber_Large, ((ScreenWidth) / 2) + 39 - textWidth_T, bgY + 50, DHN_DRAWZERO, scoreT, 172, 154, 111);
		}

		textWidth_CTAlive = GetHudNumberWidth(m_iNum_csgo, m_rcNumber_csgo, DHN_2DIGITS | DHN_DRAWZERO, aliveCT);
		textWidth_TAlive = GetHudNumberWidth(m_iNum_csgo, m_rcNumber_csgo, DHN_2DIGITS | DHN_DRAWZERO, aliveT);
		if (textWidth_TAlive > 0)
		{
			if(aliveCT >= 10)
				DrawHudNumber(m_iNum_csgo, m_rcNumber_csgo, (ScreenWidth) / 2 - 111, bgY + 20, DHN_2DIGITS | DHN_DRAWZERO, aliveCT, 128 * 255, 128 * 255, 128 * 255);
			else
				DrawHudNumber(m_iNum_csgo, m_rcNumber_csgo, (ScreenWidth) / 2 - 100, bgY + 20, DHN_DRAWZERO, aliveCT, 128 * 255, 128 * 255, 128 * 255);
		}
	
		if (textWidth_CTAlive > 0)
		{
			if (aliveT >= 10)
				DrawHudNumber(m_iNum_csgo, m_rcNumber_csgo, (ScreenWidth) / 2 + 111 - textWidth_TAlive, bgY + 20, DHN_2DIGITS | DHN_DRAWZERO, aliveT, 128 * 255, 128 * 255, 128 * 255);
			else
			{
				DrawHudNumber(m_iNum_csgo, m_rcNumber_csgo, (ScreenWidth) / 2 + 100 - 19, bgY + 20, DHN_DRAWZERO, aliveT, 128 * 255, 128 * 255, 128 * 255);
			}
		}

		return 1;
	}

	if (bgSprite)
	{
		SPR_Set(bgSprite, 255, 255, 255);
		SPR_DrawHoles(0, bgX, bgY, &bgRect);
	}

	HSPRITE textSprite = gHUD.GetSprite(m_iTextIndex);

	if (textSprite)
	{
		wrect_t textRect = gHUD.GetSpriteRect(m_iTextIndex);

		SPR_Set(textSprite, 128, 128, 128);
		SPR_DrawAdditive(0, (ScreenWidth - (textRect.right - textRect.left)) / 2, bgY + 29, &textRect);
	}

	HSPRITE textSprite_T = gHUD.GetSprite(m_iTTextIndex);

	if (textSprite_T)
	{
		wrect_t textRect = gHUD.GetSpriteRect(m_iTTextIndex);

		SPR_Set(textSprite_T, 128, 128, 128);
		SPR_DrawAdditive(0, (ScreenWidth) / 2 - 50, bgY + 11, &textRect);
	}

	HSPRITE textSprite_CT = gHUD.GetSprite(m_iCTTextIndex);

	if (textSprite_CT)
	{
		wrect_t textRect = gHUD.GetSpriteRect(m_iCTTextIndex);

		SPR_Set(textSprite_CT, 128, 128, 128);
		SPR_DrawAdditive(0, (ScreenWidth) / 2 + 27, bgY + 11, &textRect);
	}

//Caculate HudNumber
	
	if (gHUD.m_iModRunning == MOD_DM)
	{
		int best_player = gHUD.m_Scoreboard.FindBestPlayer();
		scoreCT = g_PlayerExtraInfo[gEngfuncs.GetLocalPlayer()->index].frags;
		scoreT = best_player ? g_PlayerExtraInfo[best_player].frags : 0;

		roundNumber = scoreMax ? scoreMax : 0;
	}

	if (roundNumber >= 1000)
	{
		int textWidth = GetHudNumberWidth(m_iNum_S, m_rcNumber_Small, DHN_4DIGITS | DHN_3DIGITS | DHN_2DIGITS | DHN_DRAWZERO, roundNumber);

		if (textWidth > 0)
			DrawHudNumber(m_iNum_S, m_rcNumber_Small, (ScreenWidth - textWidth) / 2, bgY + 10, DHN_4DIGITS | DHN_3DIGITS | DHN_2DIGITS | DHN_DRAWZERO, roundNumber, 128, 128, 128);
	}
	else if (roundNumber >= 100)
	{
		int textWidth = GetHudNumberWidth(m_iNum_L, m_rcNumber_Large, DHN_3DIGITS | DHN_2DIGITS | DHN_DRAWZERO, roundNumber);

		if (textWidth > 0)
			DrawHudNumber(m_iNum_L, m_rcNumber_Large, (ScreenWidth - textWidth) / 2, bgY + 10, DHN_3DIGITS | DHN_2DIGITS | DHN_DRAWZERO, roundNumber, 128, 128, 128);
	}
	else
	{
		int textWidth = GetHudNumberWidth(m_iNum_L, m_rcNumber_Large, DHN_2DIGITS | DHN_DRAWZERO, roundNumber);

		if (textWidth > 0)
			DrawHudNumber(m_iNum_L, m_rcNumber_Large, (ScreenWidth - textWidth) / 2, bgY + 10, DHN_2DIGITS | DHN_DRAWZERO, roundNumber, 128, 128, 128);
	}

	if (scoreT >= 1000)
	{
		int textWidth_T = GetHudNumberWidth(m_iNum_S, m_rcNumber_Small, DHN_4DIGITS | DHN_3DIGITS | DHN_2DIGITS | DHN_DRAWZERO, scoreT);

		if (textWidth_T > 0)
			DrawHudNumber(m_iNum_S, m_rcNumber_Small, (ScreenWidth) / 2 - 90, bgY + 11, DHN_4DIGITS | DHN_3DIGITS | DHN_2DIGITS | DHN_DRAWZERO, scoreT, 128, 128, 128);
	}
	else if (scoreT >= 100)
	{
		int textWidth_T = GetHudNumberWidth(m_iNum_L, m_rcNumber_Large, DHN_3DIGITS | DHN_DRAWZERO, scoreT);

		if (textWidth_T > 0)
			DrawHudNumber(m_iNum_L, m_rcNumber_Large, (ScreenWidth) / 2 - 89, bgY + 10, DHN_3DIGITS | DHN_DRAWZERO, scoreT, 128, 128, 128);
	}
	else
	{
		int textWidth_T = GetHudNumberWidth(m_iNum_L, m_rcNumber_Large, DHN_2DIGITS | DHN_DRAWZERO, scoreT);

		if (textWidth_T > 0)
			DrawHudNumber(m_iNum_L, m_rcNumber_Large, (ScreenWidth) / 2 - 89, bgY + 10, DHN_2DIGITS | DHN_DRAWZERO, scoreT, 128, 128, 128);
	}

	if (scoreCT >= 1000)
	{
		int textWidth_CT = GetHudNumberWidth(m_iNum_S, m_rcNumber_Small, DHN_4DIGITS | DHN_3DIGITS | DHN_2DIGITS | DHN_DRAWZERO, scoreCT);

		if (textWidth_CT > 0)
			DrawHudNumber(m_iNum_S, m_rcNumber_Small, ((ScreenWidth) / 2) + 71 - (textWidth_CT / 2), bgY + 11, DHN_4DIGITS | DHN_3DIGITS | DHN_2DIGITS | DHN_DRAWZERO, scoreCT, 128, 128, 128);
	}
	else if (scoreCT >= 100)
	{
		int textWidth_CT = GetHudNumberWidth(m_iNum_L, m_rcNumber_Large, DHN_3DIGITS | DHN_2DIGITS | DHN_DRAWZERO, scoreCT);

		if (textWidth_CT > 0)
			DrawHudNumber(m_iNum_L, m_rcNumber_Large, ((ScreenWidth) / 2) + 70 - (textWidth_CT / 2), bgY + 10, DHN_3DIGITS | DHN_DRAWZERO, scoreCT, 128, 128, 128);
	}
	else
	{
		int textWidth_CT = GetHudNumberWidth(m_iNum_L, m_rcNumber_Large, DHN_2DIGITS | DHN_DRAWZERO, scoreCT);

		if (textWidth_CT > 0)
			DrawHudNumber(m_iNum_L, m_rcNumber_Large, ((ScreenWidth) / 2) + 73 - (textWidth_CT / 2), bgY + 10, DHN_2DIGITS | DHN_DRAWZERO, scoreCT, 128, 128, 128);
	}

	if (m_iBGIndex != m_iTeamDeathBG)
	{
		if (textWidth_TAlive > 0)
			DrawHudNumber(m_iNum_S, m_rcNumber_Small, (ScreenWidth) / 2 - 69, bgY + 30, DHN_2DIGITS | DHN_DRAWZERO, aliveT, 128, 128, 128);

		if (textWidth_CTAlive > 0)
			DrawHudNumber(m_iNum_S, m_rcNumber_Small, (ScreenWidth) / 2 + 47, bgY + 30, DHN_2DIGITS | DHN_DRAWZERO, aliveCT, 128, 128, 128);
	}

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
	const bool zombie = gHUD.m_iModRunning == MOD_ZB1 || gHUD.m_iModRunning == MOD_ZB2 ||
		gHUD.m_iModRunning == MOD_ZB3;
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

void CHudScoreBoardLegacy::Reset(void)
{
	m_iBGIndex = m_iOriginalBG;
	m_iTextIndex = m_iText_Round;
	m_iTTextIndex = m_iText_TR;
	m_iCTTextIndex = m_iText_CT;

	switch (gHUD.m_iModRunning)
	{
	case MOD_NONE:
	{
		// nothing to change
		break;
	}
	case MOD_TDM:
	{
		m_iBGIndex = m_iTeamDeathBG;
		m_iTextIndex = m_iText_Kill;
		break;
	}
	case MOD_DM:
	{
		m_iBGIndex = m_iTeamDeathBG;
		m_iTextIndex = m_iText_Kill;
		m_iTTextIndex = m_iText_1st;
		m_iCTTextIndex = m_iText_Kill;
		break;
	}
	case MOD_ZB3:
	case MOD_ZB2:
	case MOD_ZB1:
	{
		m_iTTextIndex = m_iText_ZB;
		m_iCTTextIndex = m_iText_HM;
		break;
	}
	case MOD_ZBS:
	{
		// already drawn in zbs/zbs_scoreboard.cpp
		m_iBGIndex = -1;
		break;
	}
	default:
	{
		// shut clang warnings
		break;
	}
	}
}


void CHudScoreBoardLegacy::BuildHudNumberRect(int moe, wrect_t *prc, int w, int h, int xOffset, int yOffset)
{
	wrect_t rc = gHUD.GetSpriteRect(moe);
	int x = rc.left;
	int y = rc.top;

	for (int i = 0; i < 10; i++)
	{
		prc[i].left = x;
		prc[i].top = 0;
		prc[i].right = prc[i].left + w + xOffset;
		prc[i].bottom = h + yOffset;

		x += w;
		y += h;
	}
}

int CHudScoreBoardLegacy::DrawHudNumber(int moe, wrect_t *prc, int x, int y, int iFlags, int iNumber, int r, int g, int b)
{
	int iWidth = prc[0].right - prc[0].left;
	int k;
	wrect_t rc;

	if (iNumber >= 10000)
	{
		k = iNumber / 10000;
		SPR_Set(gHUD.GetSprite(moe), r, g, b);
		SPR_DrawAdditive(0, x, y, &prc[k]);
		x += iWidth;
	}
	else if (iFlags & (DHN_5DIGITS))
	{
		if (iFlags & DHN_DRAWZERO)
		{
			SPR_Set(gHUD.GetSprite(moe), r, g, b);
			SPR_DrawAdditive(0, x, y, &prc[0]);
		}

		x += iWidth;
	}

	if (iNumber >= 1000)
	{
		k = (iNumber % 10000) / 1000;
		SPR_Set(gHUD.GetSprite(moe), r, g, b);
		SPR_DrawAdditive(0, x, y, &prc[k]);
		x += iWidth;
	}
	else if (iFlags & (DHN_5DIGITS | DHN_4DIGITS))
	{
		if (iFlags & DHN_DRAWZERO)
		{
			SPR_Set(gHUD.GetSprite(moe), r, g, b);
			SPR_DrawAdditive(0, x, y, &prc[0]);
		}

		x += iWidth;
	}

	if (iNumber >= 100)
	{
		k = (iNumber % 1000) / 100;
		SPR_Set(gHUD.GetSprite(moe), r, g, b);
		SPR_DrawAdditive(0, x, y, &prc[k]);
		x += iWidth;
	}
	else if (iFlags & (DHN_5DIGITS | DHN_4DIGITS | DHN_3DIGITS))
	{
		if (iFlags & DHN_DRAWZERO)
		{
			SPR_Set(gHUD.GetSprite(moe), r, g, b);
			SPR_DrawAdditive(0, x, y, &prc[0]);
		}

		x += iWidth;
	}

	if (iNumber >= 10)
	{
		k = (iNumber % 100) / 10;
		rc = prc[k];
		SPR_Set(gHUD.GetSprite(moe), r, g, b);
		SPR_DrawAdditive(0, x, y, &rc);
		x += iWidth;
	}
	else if (iFlags & (DHN_5DIGITS | DHN_4DIGITS | DHN_3DIGITS | DHN_2DIGITS))
	{
		if (iFlags & DHN_DRAWZERO)
		{
			SPR_Set(gHUD.GetSprite(moe), r, g, b);
			SPR_DrawAdditive(0, x, y, &prc[0]);
		}

		x += iWidth;
	}

	k = iNumber % 10;
	SPR_Set(gHUD.GetSprite(moe), r, g, b);
	SPR_DrawAdditive(0, x, y, &prc[k]);
	x += iWidth;

	return x;
}

int CHudScoreBoardLegacy::GetHudNumberWidth(int moe, wrect_t *prc, int iFlags, int iNumber)
{
	int iWidth = prc[0].right - prc[0].left;
	int k;
	wrect_t rc;
	int x = 0;

	if (iNumber >= 10000)
	{
		k = iNumber / 10000;
		x += iWidth;
	}
	else if (iFlags & (DHN_5DIGITS))
		x += iWidth;

	if (iNumber >= 1000)
	{
		k = (iNumber % 10000) / 1000;
		x += iWidth;
	}
	else if (iFlags & (DHN_5DIGITS | DHN_4DIGITS))
		x += iWidth;

	if (iNumber >= 100)
	{
		k = (iNumber % 1000) / 100;
		x += iWidth;
	}
	else if (iFlags & (DHN_5DIGITS | DHN_4DIGITS | DHN_3DIGITS))
		x += iWidth;

	if (iNumber >= 10)
	{
		k = (iNumber % 100) / 10;
		rc = prc[k];
		x += iWidth;
	}
	else if (iFlags & (DHN_5DIGITS | DHN_4DIGITS | DHN_3DIGITS | DHN_2DIGITS))
		x += iWidth;

	k = iNumber % 10;
	x += iWidth;

	return x;
}
