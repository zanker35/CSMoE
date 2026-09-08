#include "ui/gameui/MoeSettings.h"

#include "ui/vgui2/vgui_controls/Button.h"
#include "ui/vgui2/vgui_controls/CheckButton.h"
#include "ui/vgui2/vgui_controls/PropertySheet.h"
#include "ui/vgui2/vgui_controls/Label.h"
#include "ui/vgui2/vgui_controls/QueryBox.h"

#include "ui/vgui2/interfaces/vgui/ILocalize.h"
#include "ui/vgui2/interfaces/vgui/ISurface.h"
#include "ui/vgui2/interfaces/vgui/ISystem.h"
#include "ui/vgui2/interfaces/vgui/IVGui.h"

#include "ui/gameui/OptionsSubMoeSettings.h"
#include "ui/gameui/ModInfo.h"

#include "source_sdk/public/tier1/KeyValues.h"

CMoeSettings::CMoeSettings(vgui2::Panel *parent) : PropertyDialog(parent, "OptionsDialog")
{
	int offset = 40;
	int swide, stall;

	vgui2::surface()->GetScreenSize(swide, stall);

	SetBounds(offset, offset,
		600, 400);
	SetSizeable(false);
	SetTitle("#GameUI_CSMoESettings", true);

	m_pOptionsSubMoeSettings = NULL;

	m_pOptionsSubMoeSettings = new COptionsSubMoeSettings(this);

	AddPage(m_pOptionsSubMoeSettings, "#GameUI_CSMoESettings");

	SetApplyButtonVisible(true);
	GetPropertySheet()->SetTabWidth(150);
}

CMoeSettings::~CMoeSettings(void)
{
}

void CMoeSettings::Activate(void)
{
	BaseClass::Activate();
	
	ResetAllData();
	EnableApplyButton(true);
}

void CMoeSettings::Run(void)
{
	SetTitle("#GameUI_TouchButtonSettings", true);
	Activate();
}

void CMoeSettings::OnClose(void)
{
	BaseClass::OnClose();
}

void CMoeSettings::OnGameUIHidden(void)
{
	for (int i = 0; i < GetChildCount(); i++)
	{
		Panel *pChild = GetChild(i);

		if (pChild)
			PostMessage(pChild, new KeyValues("GameUIHidden"));
	}
}