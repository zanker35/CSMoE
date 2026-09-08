#ifndef CSTRIKEBUYMOUSEOVERPANELBUTTON_H
#define CSTRIKEBUYMOUSEOVERPANELBUTTON_H


#include "ui/vgui2/interfaces/vgui/IScheme.h"
#include "ui/vgui2/vgui_controls/Button.h"
#include "ui/vgui2/interfaces/vgui/KeyCode.h"
#include "source_sdk/public/FileSystem.h"
#include "game/shared/interfaces/maintypes.h"
#include "game/shared/strings/shared_util.h"
#include "game/shared/data/player_const.h"

#include "game/client/menus/game_controls/mouseoverpanelbutton.h"
#include "game/client/menus/csmoe/BuyMenu/buymouseoverpanelbutton.h"
#include "game/client/menus/csmoe/BuyMenu/WeaponImagePanel.h"

class CSBuyMouseOverPanelButton : public BuyMouseOverPanelButton
{
private:
	typedef BuyMouseOverPanelButton BaseClass;
public:
	CSBuyMouseOverPanelButton(vgui2::Panel *parent, const char *panelName, vgui2::EditablePanel *panel);

	virtual void Paint() override;
	virtual void PerformLayout() override;

	void SetTeam(TeamName team)
	{
		m_iTeam = team;
	}
    TeamName m_iTeam = UNASSIGNED;
	void UpdateWeapon(const char *weapon = "");

	WeaponImagePanel *m_pWeaponImage;


};

#endif
