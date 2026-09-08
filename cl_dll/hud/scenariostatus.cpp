#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "cl_util.h"
#include "vgui_parser.h"
#include "draw_util.h"
#include "triangleapi.h"
DECLARE_MESSAGE(m_scenarioStatus, Scenario)

int CHudScenarioStatus::Init(void)
{
	HOOK_MESSAGE(Scenario);

	m_iFlags = 0;

	gHUD.AddHudElem(this);
	return 1;
}

int CHudScenarioStatus::VidInit(void)
{
	m_alpha = 100;
	m_hSprite = 0;
	m_iRepeatTimes = 0;
	return 1;
}

void CHudScenarioStatus::Reset(void)
{
	m_iFlags &= ~HUD_ACTIVE;
	m_hSprite = 0;
	m_iRepeatTimes = 0;
	m_nextFlash = 0;
}

int CHudScenarioStatus::Draw(float fTime)
{
	if ((gHUD.m_iHideHUDDisplay & HIDEHUD_HEALTH) || g_iUser1)
		return 1;

	if (gEngfuncs.IsSpectateOnly())
		return 1;

	if (!m_hSprite)
		return 1;

	if (m_iFlags & HUD_ACTIVE)
	{
		int r = 255, g = 255, b = 255;
		const int width = m_rect.right - m_rect.left;
		int x = (ScreenWidth - m_iRepeatTimes * width) / 2;
		const int y = 85;
		DrawUtils::ScaleColors(r, g, b, m_alpha);
		if (m_alpha > 100)
			m_alpha *= 0.9;
		for (int i = 0; i < m_iRepeatTimes; ++i)
		{
			SPR_Set(m_hSprite, r, g, b);
			SPR_DrawAdditive(0, x, y, &m_rect);
			x += width;
		}
	}

	if (m_flashInterval && fTime >= m_nextFlash)
	{
		m_nextFlash = m_flashInterval + fTime;
		m_alpha = m_flashAlpha;
	}

	return 1;
}

int CHudScenarioStatus::MsgFunc_Scenario(const char *pszName, int iSize, void *pbuf)
{
	BufferReader reader(pszName, pbuf, iSize);

	bool wasActive = m_iFlags & HUD_ACTIVE;
	const char *spriteName;
	int sprIndex;
	int alpha;

	if (!reader.ReadByte())
	{
		m_iFlags &= ~HUD_ACTIVE;
		return 1;
	}

	m_iFlags |= HUD_ACTIVE;

	spriteName = reader.ReadString();
	if (sscanf(spriteName, "hostage%d", &m_iRepeatTimes) == 1)
	{
		spriteName = "hostage";
	}
	else
	{
		m_iRepeatTimes = 1;
	}
	sprIndex = gHUD.GetSpriteIndex(spriteName);

	m_hSprite = gHUD.GetSprite(sprIndex);
	m_rect = gHUD.GetSpriteRect(sprIndex);

	alpha = reader.ReadByte();

	if (alpha < MIN_ALPHA)
		alpha = MIN_ALPHA;

	m_flashAlpha = alpha;

	if (wasActive)
		alpha = m_alpha;
	else
		m_alpha = alpha;

	if (alpha == MIN_ALPHA)
		return 1;

	m_flashInterval = reader.ReadShort() * 0.01;

	if (!m_nextFlash)
		m_nextFlash = gHUD.m_flTime;

	m_nextFlash += reader.ReadShort() * 0.01;
	return 1;
}
