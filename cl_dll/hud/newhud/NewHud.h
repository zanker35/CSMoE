/* citrus HUD character and weapon backgrounds, adapted for the master protocol. */
#pragma once

#include <string>

class CHudNewHud : public CHudBase
{
public:
	int Init() override;
	int VidInit() override;
	int Draw(float flTime) override;
	void InitHUDData() override;
	void Shutdown() override;

private:
	UniqueTexture m_iCharacterBG;
	UniqueTexture m_iCharacterBG_New_Bottom;
	UniqueTexture m_iWeaponBG;
	UniqueTexture m_iCharacter;
	std::string m_szLastModel;
};
