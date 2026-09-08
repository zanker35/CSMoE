#pragma once

#include "game/client/view/r_texture.h"

// Citrus round-result artwork, driven by master's existing round messages.
class CHudShowWin : public CHudBase
{
public:
	int Init() override;
	int VidInit() override;
	int Draw(float time) override;
	void Reset() override;
	void InitHUDData() override;
	void Shutdown() override;

	bool OnTextMessage(const char *message);
	void OnRadioMessage(const char *sentence);

private:
	enum Winner
	{
		CT_WIN,
		T_WIN,
		HUMAN_WIN,
		ZOMBIE_WIN,
		WINNER_COUNT
	};

	bool Show(Winner winner);

	Winner m_winner = CT_WIN;
	float m_displayUntil = 0.0f;
	SharedTexture m_textures[WINNER_COUNT];
};
