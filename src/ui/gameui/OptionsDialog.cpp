#include "ui/gameui/OptionsDialog.h"

#include "ui/vgui2/vgui_controls/Button.h"
#include "ui/vgui2/vgui_controls/CheckButton.h"
#include "ui/vgui2/vgui_controls/PropertySheet.h"
#include "ui/vgui2/vgui_controls/Label.h"
#include "ui/vgui2/vgui_controls/QueryBox.h"

#include "ui/vgui2/interfaces/vgui/ILocalize.h"
#include "ui/vgui2/interfaces/vgui/ISurface.h"
#include "ui/vgui2/interfaces/vgui/ISystem.h"
#include "ui/vgui2/interfaces/vgui/IVGui.h"

#include "ui/gameui/OptionsSubMultiplayer.h"
#include "ui/gameui/OptionsSubKeyboard.h"
#include "ui/gameui/OptionsSubMouse.h"
#include "ui/gameui/OptionsSubAudio.h"
#include "ui/gameui/OptionsSubVideo.h"
#include "ui/gameui/OptionsSubAdvanced.h"
#include "ui/gameui/OptionsSubMoeSettings.h"
#include "ui/gameui/ModInfo.h"

#include "source_sdk/public/tier1/KeyValues.h"

COptionsDialog::COptionsDialog(vgui2::Panel *parent) : PropertyDialog(parent, "OptionsDialog")
{
	SetBounds(0, 0, 600, 430);
	SetSizeable(false);
	SetTitle("#GameUI_Options", true);

	m_pOptionsSubMultiplayer = NULL;
	m_pOptionsSubKeyboard = NULL;
	m_pOptionsSubMouse = NULL;
	m_pOptionsSubAudio = NULL;
	m_pOptionsSubVideo = NULL;
	//m_pOptionsSubAdvanced = NULL;
    m_pOptionsSubMoeSettings = NULL;

	if ((ModInfo().IsMultiplayerOnly() && !ModInfo().IsSinglePlayerOnly()) || (!ModInfo().IsMultiplayerOnly() && !ModInfo().IsSinglePlayerOnly()))
		m_pOptionsSubMultiplayer = new COptionsSubMultiplayer(this);

	m_pOptionsSubKeyboard = new COptionsSubKeyboard(this);
	m_pOptionsSubMouse = new COptionsSubMouse(this);
	m_pOptionsSubAudio = new COptionsSubAudio(this);
	m_pOptionsSubVideo = new COptionsSubVideo(this);
	if (!ModInfo().IsSinglePlayerOnly())
	{
	}

	//m_pOptionsSubAdvanced = new COptionsSubAdvanced(this);
    m_pOptionsSubMoeSettings = new COptionsSubMoeSettings(this);

    AddPage(m_pOptionsSubMoeSettings, "#GameUI_CSMoESettings");
	if (m_pOptionsSubMultiplayer)
		AddPage(m_pOptionsSubMultiplayer, "#GameUI_Multiplayer");
	AddPage(m_pOptionsSubKeyboard, "#GameUI_Keyboard");
	AddPage(m_pOptionsSubMouse, "#GameUI_Mouse");
	AddPage(m_pOptionsSubAudio, "#GameUI_Audio");
	AddPage(m_pOptionsSubVideo, "#GameUI_Video");
	//AddPage(m_pOptionsSubAdvanced, "#GameUI_Advanced");

	SetApplyButtonVisible(true);
	GetPropertySheet()->SetTabWidth(68);
}

COptionsDialog::~COptionsDialog(void)
{
}

void COptionsDialog::Activate(void)
{
	BaseClass::Activate();
	
	if (m_pOptionsSubMultiplayer)
	{
		if (GetActivePage() != m_pOptionsSubMultiplayer)
			GetPropertySheet()->SetActivePage(m_pOptionsSubMultiplayer);
	}
	else
	{
		if (GetActivePage() != m_pOptionsSubKeyboard)
			GetPropertySheet()->SetActivePage(m_pOptionsSubKeyboard);
	}
	ResetAllData();
	EnableApplyButton(false);
}

void COptionsDialog::Run(void)
{
	SetTitle("#GameUI_Options", true);
	Activate();
}

void COptionsDialog::OnClose(void)
{
	BaseClass::OnClose();
}

void COptionsDialog::OnGameUIHidden(void)
{
	for (int i = 0; i < GetChildCount(); i++)
	{
		Panel *pChild = GetChild(i);

		if (pChild)
			PostMessage(pChild, new KeyValues("GameUIHidden"));
	}
}
