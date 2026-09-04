
#pragma once

#include "hud_sub.h"

class CHudScoreBoardLegacy : public IBaseHudSub
{
public:
	CHudScoreBoardLegacy();
	~CHudScoreBoardLegacy() override;
	int VidInit(void) override;
	int Draw(float time) override;
	void Reset(void) override;		// called every time a server is connected to

	// Shared with the citrus-style round timer.
	static void BuildNumberRC(wrect_t (&rect)[10], int width, int height);
	static void DrawTexturePart(const CTextureRef &texture, const wrect_t &rect, int x, int y,
		float scale = 1.0f, byte r = 255, byte g = 255, byte b = 255, byte a = 255);
	static int DrawTexturedNumbers(const CTextureRef &texture, const wrect_t (&rect)[10],
		int number, int x, int y, int flags, int gap = 0, float scale = 1.0f,
		byte r = 255, byte g = 255, byte b = 255, byte a = 255);

protected:
	bool DrawNewHud(float time);
	void BuildHudNumberRect(int moe, wrect_t *prc, int w, int h, int xOffset, int yOffset);
	int DrawHudNumber(int moe, wrect_t *prc, int x, int y, int iFlags, int iNumber, int r, int g, int b);
	int GetHudNumberWidth(int moe, wrect_t *prc, int iFlags, int iNumber);

private:
	int m_iBGIndex;
	int m_iTextIndex;
	int m_iTTextIndex, m_iCTTextIndex;

	wrect_t m_rcNumber_Large[10];
	wrect_t m_rcNumber_Small[10];
	wrect_t m_rcNumber_csgo[10];

	int m_iOriginalBG;
	int m_iTeamDeathBG;
	int m_iUnitehBG;
	int m_iNum_L;
	int m_iNum_S;
	int m_iText_CT;
	int m_iText_T;
	int m_iText_TR;
	int m_iText_HM;
	int m_iText_ZB;
	int m_iText_1st;
	int m_iText_Kill;
	int m_iText_Round;
	int m_iNum_csgo;
	UniqueTexture m_pTexture_Board;

	UniqueTexture m_pNewBackground;
	UniqueTexture m_pNewZombieBackground;
	UniqueTexture m_pNewDeathmatchBackground;
	UniqueTexture m_pNewLargeRed;
	UniqueTexture m_pNewLargeBlue;
	UniqueTexture m_pNewLargeWhite;
	UniqueTexture m_pNewCenterNumbers;
	UniqueTexture m_pNewSmallRed;
	UniqueTexture m_pNewSmallBlue;
	UniqueTexture m_pNewTRIcon;
	UniqueTexture m_pNewCTIcon;
	UniqueTexture m_pNewZombieIcon;
	UniqueTexture m_pNewHumanIcon;
	UniqueTexture m_pNewWinIcon;
	UniqueTexture m_pNewRoundIcon;
	UniqueTexture m_pNewSlash;
	UniqueTexture m_pNewFirstIcon;
	UniqueTexture m_pNewMyIcon;
	UniqueTexture m_pNewKillIcon;
	UniqueTexture m_pNewTeamKillIcon;
	wrect_t m_rcNewLarge[10];
	wrect_t m_rcNewCenter[10];
	wrect_t m_rcNewSmall[10];

	int m_HUD_timer;
	int m_iTime;
	float m_fStartTime;
	bool m_bPanicColorChange;
	float m_flPanicTime;
	int m_closestRight;
};
