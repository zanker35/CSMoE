#include "hud.h"
#include "teammenu.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <FileSystem.h>
#include <IGameUIFuncs.h>

#include <string>

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/RichText.h>

#include "game_controls/mouseoverpanelbutton.h"

using namespace vgui2;

const char *GetStringTeamColor(int i)
{
	switch (i)
	{
		case 0: return "team0";
		case 1: return "team1";
		case 2: return "team2";
		case 3: return "team3";
		case 4:
		default: return "team4";
	}
}

CTeamMenu::CTeamMenu(IViewport* pViewPort) : Frame(NULL, PANEL_TEAM), m_pViewPort(pViewPort)
{
	SetTitle("#Cstrike_Select_Team", true);
	SetScheme("ClientScheme");
	SetMoveable(false);
	SetSizeable(false);

	SetTitleBarVisible(false);
	SetProportional(true);

	m_pPanel = new EditablePanel(this, "ClassInfo");
	m_pMapInfo = new RichText(this, "MapInfo");
#if defined (ENABLE_HTML_WINDOW)
	m_pMapInfoHTML = new HTML(this, "MapInfoHTML");
#endif
	LoadControlSettings("Resource/UI/TeamMenu.res", "GAME");
	InvalidateLayout();

	m_szMapName[0] = 0;
}

CTeamMenu::~CTeamMenu(void)
{
}

MouseOverPanelButton *CTeamMenu::CreateNewMouseOverPanelButton(EditablePanel *panel)
{
	return new MouseOverPanelButton(this, "MouseOverPanelButton", panel);
}

Panel *CTeamMenu::CreateControlByName(const char *controlName)
{
	if (!Q_stricmp("MouseOverPanelButton", controlName))
	{
		MouseOverPanelButton *newButton = CreateNewMouseOverPanelButton(m_pPanel);
		m_mouseoverButtons.AddToTail(newButton);
		return newButton;
	}
	else
	{
		return BaseClass::CreateControlByName(controlName);
	}
}

void CTeamMenu::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	m_pMapInfo->SetFgColor(pScheme->GetColor("MapDescriptionText", Color(255, 255, 255, 0)));

	if (*m_szMapName)
		LoadMapPage(m_szMapName);
}

void CTeamMenu::AutoAssign(void)
{
	gEngfuncs.pfnClientCmd("jointeam 5");

	OnClose();
}

void CTeamMenu::ShowPanel(bool bShow)
{
	if (BaseClass::IsVisible() == bShow)
		return;

	if (bShow)
	{
		Activate();
		SetMouseInputEnabled(true);

		for (int i = 0; i < m_mouseoverButtons.Count(); ++i)
		{
			if (i == 0)
				m_mouseoverButtons[i]->ShowPage();
			else
				m_mouseoverButtons[i]->HidePage();
		}
	}
	else
	{
		SetVisible(false);
		SetMouseInputEnabled(false);
	}

	m_pViewPort->ShowBackGround(bShow);
}

void CTeamMenu::Update(void)
{
	char mapname[32];
	Q_FileBase(gEngfuncs.pfnGetLevelName(), mapname, sizeof(mapname));

	if (Q_stricmp(m_szMapName, mapname))
	{
		SetLabelText("mapname", mapname);
		LoadMapPage(mapname);
	}
}

void CTeamMenu::LoadMapPage(const char *mapName)
{
	Q_strncpy(m_szMapName, mapName, sizeof(m_szMapName));

	char mapRES[MAX_PATH];
	
	m_pMapInfo->SetVisible(true);
#if defined (ENABLE_HTML_WINDOW)
	m_pMapInfoHTML->SetVisible(false);
#endif
	
	Q_snprintf(mapRES, sizeof(mapRES), "maps/%s.txt", mapName);

	if (!filesystem()->FileExists(mapRES))
	{
		if (filesystem()->FileExists("maps/default.txt"))
		{
			Q_snprintf(mapRES, sizeof(mapRES), "maps/default.txt");
		}
		else
		{
			m_pMapInfo->SetText("");
			return;
		}
	}

	FileHandle_t f = filesystem()->Open(mapRES, "rb");
	if (f == FILESYSTEM_INVALID_HANDLE)
		return;
	std::string text(filesystem()->Size(f), '\0');
	const int bytesRead = filesystem()->Read(text.data(), static_cast<int>(text.size()), f);
	filesystem()->Close(f);
	text.resize(bytesRead > 0 ? bytesRead : 0);

	if (text.size() >= 2 && static_cast<unsigned char>(text[0]) == 0xff && static_cast<unsigned char>(text[1]) == 0xfe)
	{
		// Map descriptions use UTF-16LE, while macOS wchar_t is four bytes.
		std::wstring wide;
		for (size_t i = 2; i + 1 < text.size(); i += 2)
		{
			unsigned value = static_cast<unsigned char>(text[i]) | (static_cast<unsigned char>(text[i + 1]) << 8);
			if (sizeof(wchar_t) > 2 && value >= 0xd800 && value <= 0xdbff && i + 3 < text.size())
			{
				const unsigned low = static_cast<unsigned char>(text[i + 2]) | (static_cast<unsigned char>(text[i + 3]) << 8);
				if (low >= 0xdc00 && low <= 0xdfff)
				{
					value = 0x10000 + ((value - 0xd800) << 10) + low - 0xdc00;
					i += 2;
				}
			}
			wide.push_back(static_cast<wchar_t>(value));
		}
		m_pMapInfo->SetText(wide.c_str());
	}
	else
		m_pMapInfo->SetText(text.c_str());
	m_pMapInfo->GotoTextStart();

	InvalidateLayout();
	Repaint();
}

void CTeamMenu::SetLabelText(const char *textEntryName, const char *text)
{
	Label *entry = dynamic_cast<Label *>(FindChildByName(textEntryName));

	if (entry)
		entry->SetText(text);
}

void CTeamMenu::Reset(void)
{
	for (int i = 0; i < GetChildCount(); ++i)
	{
		MouseOverPanelButton *pPanel = dynamic_cast<MouseOverPanelButton *>(GetChild(i));

		if (pPanel)
			pPanel->HidePage();
	}

	for (int i = 0; i < m_mouseoverButtons.Count(); ++i)
		m_mouseoverButtons[i]->HidePage();
}
