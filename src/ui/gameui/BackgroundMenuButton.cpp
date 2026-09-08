#include "ui/gameui/BackgroundMenuButton.h"

#include "source_sdk/public/tier1/KeyValues.h"
#include "ui/vgui2/interfaces/vgui/IImage.h"
#include "ui/vgui2/interfaces/vgui/IScheme.h"
#include "ui/vgui2/interfaces/vgui/ISurface.h"
#include "ui/vgui2/vgui_controls/Menu.h"
#include "ui/vgui2/vgui_controls/MenuItem.h"

CBackgroundMenuButton::CBackgroundMenuButton(vgui2::Panel *parent, const char *name) : BaseClass(parent, name, "")
{
	m_pImage = NULL;
	m_pMouseOverImage = NULL;
}

CBackgroundMenuButton::~CBackgroundMenuButton(void)
{
}

void CBackgroundMenuButton::SetVisible(bool state)
{
	BaseClass::SetVisible(true);
}

void CBackgroundMenuButton::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetFgColor(Color(255, 255, 255, 255));
	SetBgColor(Color(0, 0, 0, 0));
	SetDefaultColor(Color(255, 255, 255, 255), Color(0, 0, 0, 0));
	SetArmedColor(Color(255, 255, 0, 255), Color(0, 0, 0, 0));
	SetDepressedColor(Color(255, 255, 0, 255), Color(0, 0, 0, 0));
	SetContentAlignment(Label::a_west);
	SetBorder(NULL);
	SetDefaultBorder(NULL);
	SetDepressedBorder(NULL);
	SetKeyFocusBorder(NULL);
	SetTextInset(0, 0);
	SetAlpha(0);
}

void CBackgroundMenuButton::OnKillFocus(void)
{
	BaseClass::OnKillFocus();
}

void CBackgroundMenuButton::OnCommand(const char *command)
{
	BaseClass::OnCommand(command);
}