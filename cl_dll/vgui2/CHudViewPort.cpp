#include "hud.h"
#include <vgui/IInputInternal.h>
#include <vgui/ISurface.h>
#include <IEngineVgui.h>

#include "CHudViewPort.h"
#include "CClientMOTD.h"
#include "parsemsg.h"
#include "csmoe/BuyMenu/cstrikebuymenu.h"
#include "csmoe/cstriketeammenu.h"
#include "csmoe/cstrikeclassmenu.h"

void CHudViewport::ApplySchemeSettings(vgui2::IScheme* scheme)
{
    BaseClass::ApplySchemeSettings(scheme);
    SetPaintBackgroundEnabled(false);
}

void CHudViewport::Start()
{
    BaseClass::Start();
    gEngfuncs.pfnAddCommand("motd_open", []() {
        auto* viewport = static_cast<CHudViewport*>(g_pViewport);
        if (viewport && viewport->m_pMOTD)
        {
            viewport->ShowPanel(viewport->m_pMOTD, true);
            viewport->m_pMOTD->Activate(gHUD.m_szServerName, viewport->m_szMOTD.c_str());
        }
    });
}

int CHudViewport::MsgFunc_MOTD(const char* name, int size, void* data)
{
    if (m_bGotAllMOTD)
        m_szMOTD.clear();
    BufferReader reader(name, data, size);
    m_bGotAllMOTD = reader.ReadByte() != 0;
    m_szMOTD += reader.ReadString();
    return 1;
}

void CHudViewport::HideScoreBoard()
{
    BaseClass::HideScoreBoard();
    gHUD.m_Scoreboard.UserCmd_HideScores();
}

void CHudViewport::ActivateClientUI()
{
    BaseClass::ActivateClientUI();
    if (gHUD.m_iIntermission)
        gHUD.m_Scoreboard.UserCmd_ShowScores();
}

void CHudViewport::HideClientUI()
{
    BaseClass::HideClientUI();
}

void CHudViewport::CreateDefaultPanels()
{
    AddNewPanel(CreatePanelByName("ClientMOTD"));
    AddNewPanel(CreatePanelByName(PANEL_TEAM));
    AddNewPanel(CreatePanelByName(PANEL_CLASS));
    AddNewPanel(CreatePanelByName(PANEL_BUY));
    HideAllVGUIMenu();
}

void CHudViewport::RemoveAllPanels()
{
    BaseClass::RemoveAllPanels();
    m_pMOTD = nullptr;
    m_pTeamMenu = nullptr;
    m_pClassMenu = nullptr;
    m_pBuyMenu = nullptr;
}

IViewportPanel* CHudViewport::CreatePanelByName(const char* name)
{
    if (!Q_strcmp("ClientMOTD", name))
    {
        if (!m_pMOTD)
            m_pMOTD = new CClientMOTD(this);
        return m_pMOTD;
    }
    if (!Q_strcmp(PANEL_TEAM, name))
    {
        if (!m_pTeamMenu)
        {
            m_pTeamMenu = new CCSTeamMenu(this);
            m_pTeamMenu->UpdateGameMode();
        }
        return m_pTeamMenu;
    }
    if (!Q_strcmp(PANEL_CLASS, name))
    {
        if (!m_pClassMenu)
            m_pClassMenu = new CCSClassMenu(this);
        return m_pClassMenu;
    }
    if (!Q_strcmp(PANEL_BUY, name))
    {
        if (!m_pBuyMenu)
        {
            m_pBuyMenu = new CCSBaseBuyMenu(this);
            m_pBuyMenu->UpdateGameMode();
        }
        return m_pBuyMenu;
    }
    return nullptr;
}

bool CHudViewport::ShowVGUIMenu(int menu)
{
    IViewportPanel* panel = nullptr;
    switch (menu)
    {
    case MENU_TEAM:
        panel = m_pTeamMenu;
        break;
    case MENU_CLASS_T:
    case MENU_CLASS_CT:
        if (!m_pClassMenu)
            return false;
        m_pClassMenu->SetTeam(menu == MENU_CLASS_T ? TERRORIST : CT);
        panel = m_pClassMenu;
        break;
    case MENU_BUY:
    case MENU_BUY_PISTOL:
    case MENU_BUY_SHOTGUN:
    case MENU_BUY_RIFLE:
    case MENU_BUY_SUBMACHINEGUN:
    case MENU_BUY_MACHINEGUN:
    case MENU_BUY_ITEM:
        if (!m_pBuyMenu)
            return false;
        m_pBuyMenu->SetTeam(g_iTeamNumber);
        m_pBuyMenu->ActivateMenu(menu);
        return true;
    default:
        return false;
    }
    if (!panel)
        return false;
    ShowPanel(panel, true);
    return true;
}

bool CHudViewport::HideVGUIMenu(int menu)
{
    IViewportPanel* panel = nullptr;
    switch (menu)
    {
    case MENU_TEAM:
        panel = m_pTeamMenu;
        break;
    case MENU_CLASS_T:
    case MENU_CLASS_CT:
        panel = m_pClassMenu;
        break;
    case MENU_BUY:
    case MENU_BUY_PISTOL:
    case MENU_BUY_SHOTGUN:
    case MENU_BUY_RIFLE:
    case MENU_BUY_SUBMACHINEGUN:
    case MENU_BUY_MACHINEGUN:
    case MENU_BUY_ITEM:
        panel = m_pBuyMenu;
        break;
    default:
        return false;
    }
    if (!panel)
        return false;
    ShowPanel(panel, false);
    return true;
}

void CHudViewport::UpdateGameMode()
{
    if (m_pBuyMenu)
        m_pBuyMenu->UpdateGameMode();
    if (m_pTeamMenu)
        m_pTeamMenu->UpdateGameMode();
    if (m_pClassMenu)
        m_pClassMenu->UpdateGameMode();
}

int CHudViewport::GetAllowSpectators()
{
    return gHUD.m_Menu.m_bAllowSpec;
}

bool CHudViewport::ShowVGUIMenuByName(const char* name)
{
    auto* panel = FindPanelByName(name);
    if (!panel)
    {
        panel = CreatePanelByName(name);
        if (panel && !AddNewPanel(panel))
            return false;
    }
    if (!panel)
        return false;
    ShowPanel(panel, true);
    return true;
}
