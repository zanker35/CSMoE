#include "game/client/hud/hud.h"
#include <stdio.h>
#include <wchar.h>
#include "source_sdk/public/tier1/utlsymbol.h"

#include "ui/vgui2/interfaces/vgui/IBorder.h"
#include "ui/vgui2/interfaces/vgui/IInput.h"
#include "ui/vgui2/interfaces/vgui/IScheme.h"
#include "ui/vgui2/interfaces/vgui/ISurface.h"
#include "ui/vgui2/interfaces/vgui/ISystem.h"
#include "ui/vgui2/interfaces/vgui/IVGui.h"
#include "ui/vgui2/interfaces/vgui/ILocalize.h"
#include "ui/vgui2/interfaces/vgui/MouseCode.h"
#include "ui/vgui2/interfaces/vgui/KeyCode.h"
#include "source_sdk/public/tier1/KeyValues.h"

#include "ui/vgui2/vgui_controls/TextImage.h"
#include "ui/vgui2/vgui_controls/ImagePanel.h"
#include "ui/vgui2/vgui_controls/EditablePanel.h"

#include "game/client/menus/CBaseViewport.h"

#include "game/client/menus/csmoe/BuyMenu/cstrikebuymouseoverpanelbutton.h"
#include "game/client/menus/csmoe/BuyMenu/cstrikebuymouseoverpanel.h"

#include <string>

using namespace vgui2;

const Color COL_NONE = { 255, 255, 255, 255 };
const Color COL_CT = { 192, 205, 224, 255 };
const Color COL_TR = { 216, 182, 183, 255 };

CSBuyMouseOverPanelButton::CSBuyMouseOverPanelButton(vgui2::Panel *parent, const char *panelName, vgui2::EditablePanel *panel)
	: BaseClass(parent, panelName, panel)
{
	if (m_pPanel)
		delete m_pPanel;
	m_pPanel = new CSBuyMouseOverPanel(parent, "ItemInfo");
	m_pPanel->SetVisible(false);

	m_pWeaponImage = new WeaponImagePanel(this, "WeaponImage");
	m_pWeaponImage->SetShouldScaleImage(true);
	m_pWeaponImage->SetShouldCenterImage(true);
	m_pWeaponImage->SetMouseInputEnabled(false);
	m_pWeaponImage->SetKeyBoardInputEnabled(false);



}

void CSBuyMouseOverPanelButton::UpdateWeapon(const char *weapon)
{
	if (auto *panel = dynamic_cast<CSBuyMouseOverPanel *>(m_pPanel))
		panel->UpdateWeapon(weapon);
    if(weapon && weapon[0])
    {
        m_pWeaponImage->SetVisible(true);

        m_pWeaponImage->SetWeapon(weapon);
    }
    else
    {
		m_pWeaponImage->SetWeapon(nullptr);
        m_pWeaponImage->SetVisible(false);
    }
}
void CSBuyMouseOverPanelButton::Paint()
{
	Color col(200, 200, 200, 255);
	if (m_iTeam == TERRORIST)
	{
		col = COL_TR;
	}
	else if (m_iTeam == CT)
	{
		col = COL_CT;
	}
	SetFgColor(col);
	BaseClass::Paint();
}

void CSBuyMouseOverPanelButton::PerformLayout()
{
	BaseClass::PerformLayout();
	int  x, y, w, h;
	GetBounds(x, y, w, h);

	int newWide = h * 2.9;
	m_pWeaponImage->SetBounds(w - newWide, 0, newWide, h);


}
