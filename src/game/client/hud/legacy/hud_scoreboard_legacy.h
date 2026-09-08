
#pragma once

#include "game/client/hud/hud_sub.h"

class CHudScoreBoardLegacy : public IBaseHudSub
{
public:
	CHudScoreBoardLegacy();
	~CHudScoreBoardLegacy() override;
	int VidInit(void) override;
	int Draw(float time) override;

	// Shared with the citrus-style round timer.
	static void BuildNumberRC(wrect_t (&rect)[10], int width, int height);
	static void DrawTexturePart(const CTextureRef &texture, const wrect_t &rect, int x, int y,
		float scale = 1.0f, byte r = 255, byte g = 255, byte b = 255, byte a = 255);
	static int DrawTexturedNumbers(const CTextureRef &texture, const wrect_t (&rect)[10],
		int number, int x, int y, int flags, int gap = 0, float scale = 1.0f,
		byte r = 255, byte g = 255, byte b = 255, byte a = 255);

protected:
	bool DrawNewHud(float time);

private:
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

};
