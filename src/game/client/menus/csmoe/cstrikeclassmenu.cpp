#include "game/client/hud/hud.h"
#include "game/client/runtime/cl_util.h"
#include "game/client/menus/csmoe/cstrikeclassmenu.h"
#include "game/shared/data/player_model.h"

#include "ui/vgui2/interfaces/vgui/IScheme.h"
#include "ui/vgui2/interfaces/vgui/ILocalize.h"
#include "ui/vgui2/vgui_controls/Button.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <string>

using namespace vgui2;

namespace
{
const Color COL_CT(192, 205, 224, 255);
const Color COL_TR(216, 182, 183, 255);
constexpr size_t CLASSES_PER_PAGE = 9;

int TeamClassCount(TeamName team)
{
    return team == CT ? cl::PlayerClassManager().PlayerClass_GetNumCT()
                      : cl::PlayerClassManager().PlayerClass_GetNumTR();
}

const char* TeamClassModel(TeamName team, int slot)
{
    auto& classes = cl::PlayerClassManager();
    return classes.PlayerClass_GetModelName(classes.PlayerClass_FromTeamSlot(team, slot));
}

std::string ClassNameToken(const char* model)
{
    return std::string("#CSO_Item_Name_") + model;
}
}

CCSClassMenu::CCSClassMenu(IViewport* pViewPort) : CClassMenu(pViewPort)
{
    m_pTitleLabel = new Label(this, "CaptionLabel", "选择角色");
    m_pShowCT = new NewTabButton(this, "ShowCTWeapon", "#CSO_ClsCT");
    m_pShowTER = new NewTabButton(this, "ShowTERWeapon", "#CSO_ClsTER");
    m_pShowCT->SetTextColor(COL_CT);
    m_pShowTER->SetTextColor(COL_TR);
    m_pShowCT->SetCommand("showctclass");
    m_pShowTER->SetCommand("showterclass");

    m_pPrevBtn = new Button(this, "prevBtn", "#CSO_PrevBuy");
    m_pNextBtn = new Button(this, "nextBtn", "#CSO_NextBuy");
    m_pPrevBtn->SetCommand("prevpage");
    m_pNextBtn->SetCommand("nextpage");
    m_pPrevBtn->SetHotkey('-');
    m_pNextBtn->SetHotkey('=');

    m_pClassImage = new ImagePanel(this, "CharacterPortrait");
    m_pClassImage->SetShouldScaleImage(true);
    m_pClassDesc = new Label(this, "CharacterDescription", "");
    // Label owns a wrap flag in addition to TextImage's flag. Configure both
    // through the normal resource-settings interface so Label does not later
    // collapse the image back to its original narrow content width.
    auto* descriptionSettings = new KeyValues("CharacterDescription");
    descriptionSettings->SetInt("wrap", 1);
    descriptionSettings->SetString("textAlignment", "north-west");
    static_cast<Panel*>(m_pClassDesc)->ApplySettings(descriptionSettings);
    descriptionSettings->deleteThis();

    for (int i = 0; i < 10; ++i)
    {
        char name[16];
        Q_snprintf(name, sizeof(name), "slot%d", i);
        m_pSlotButtons[i] = new NewMouseOverPanelButton(this, name, m_pPanel);
        m_pSlotButtons[i]->GetClassPanel()->SetName("ClassInfo");
        m_pSlotButtons[i]->AddActionSignalTarget(this);
    }

    LoadControlSettings("Resource/UI/cso_classmenu_ver2.res", "GAME");
    SetVisible(false);
    SetTeam(CT);
}

CCSClassMenu::~CCSClassMenu() = default;

const char* CCSClassMenu::GetName()
{
    return PANEL_CLASS;
}

void CCSClassMenu::Reset()
{
    BaseClass::Reset();
}

MouseOverPanelButton* CCSClassMenu::CreateNewMouseOverPanelButton(EditablePanel* panel)
{
    return new NewMouseOverPanelButton(this, "MouseOverPanelButton", panel);
}

void CCSClassMenu::SetTeam(TeamName team)
{
    if (team != CT && team != TERRORIST)
        return;
    m_iTeam = team;
    SetupTeamPage(team, 0);
}

void CCSClassMenu::SetupTeamPage(TeamName team, size_t page)
{
    if (team != CT && team != TERRORIST)
        return;

    const int count = TeamClassCount(team);
    if (count <= 0)
        return;
    const size_t pages = (count + CLASSES_PER_PAGE - 1) / CLASSES_PER_PAGE;
    m_iCurrentPage = std::min(page, pages - 1);
    m_iCurrentTeamPage = team;
    m_pShowCT->SetActive(team == CT);
    m_pShowTER->SetActive(team == TERRORIST);
    m_pPrevBtn->SetVisible(m_iCurrentPage > 0);
    m_pNextBtn->SetVisible(m_iCurrentPage + 1 < pages);

    for (int i = 0; i < 9; ++i)
    {
        const int slot = static_cast<int>(m_iCurrentPage * CLASSES_PER_PAGE) + i + 1;
        auto* button = m_pSlotButtons[i];
        button->SetVisible(slot <= count);
        button->SetEnabled(slot <= count);
        if (slot > count)
            continue;

        const char* model = TeamClassModel(team, slot);
        const std::string token = ClassNameToken(model);
        const wchar_t* text = localize()->Find(token.c_str());
        if (text)
            button->SetText(text);
        else
            button->SetText(model);
        button->SetFgColor(team == CT ? COL_CT : COL_TR);
        button->SetHotkey('1' + i);
        char command[64];
        Q_snprintf(command, sizeof(command), "VGUI_ClassMenu_Select %d", slot);
        button->SetCommand(command);
    }

    m_pSlotButtons[9]->SetText("自动选择");
    m_pSlotButtons[9]->SetVisible(true);
    m_pSlotButtons[9]->SetEnabled(true);
    m_pSlotButtons[9]->SetCommand("VGUI_ClassMenu_Select 0");
    m_pSlotButtons[9]->SetHotkey('0');
    UpdateClass(0);
}

void CCSClassMenu::ShowPanel(bool show)
{
    if (show && (gHUD.m_iIntermission || gEngfuncs.IsSpectateOnly()))
        return;
    BaseClass::ShowPanel(show);
    if (show)
        m_pSlotButtons[0]->RequestFocus();
}

void CCSClassMenu::PaintBackground()
{
    BaseClass::PaintBackground();
}

void CCSClassMenu::PerformLayout()
{
    BaseClass::PerformLayout();
    const auto scaled = [](int value) { return scheme()->GetProportionalScaledValue(value); };
    m_pTitleLabel->SizeToContents();
    m_pTitleLabel->SetPos((GetWide() - m_pTitleLabel->GetWide()) / 2, scaled(12));
    m_pShowTER->SetBounds(scaled(12), scaled(32), scaled(98), scaled(16));
    m_pShowCT->SetBounds(scaled(110), scaled(32), scaled(98), scaled(16));
    m_pClassImage->SetBounds(scaled(255), scaled(58), scaled(300), scaled(225));
    m_pClassDesc->SetBounds(scaled(255), scaled(292), scaled(300), scaled(90));
    m_pClassDesc->SetTextInset(0, 0);
    m_pClassDesc->InvalidateLayout(true);
    // Repainting must preserve the selected team and page.
}

void CCSClassMenu::ApplySchemeSettings(IScheme* schemeData)
{
    BaseClass::ApplySchemeSettings(schemeData);
    if (m_pTitleLabel)
        m_pTitleLabel->SetFont(schemeData->GetFont("Title", IsProportional()));
    if (m_pClassDesc)
    {
        m_pClassDesc->SetFont(schemeData->GetFont("Default", IsProportional()));
        m_pClassDesc->SetFgColor(schemeData->GetColor("MapDescriptionText", Color(255, 255, 255, 255)));
    }
}

void CCSClassMenu::OnUpdateClass(const char* name)
{
    if (!strncmp(name, "slot", 4))
        UpdateClass(atoi(name + 4));
}

void CCSClassMenu::OnCommand(const char* command)
{
    constexpr char selectPrefix[] = "VGUI_ClassMenu_Select ";
    if (!strncmp(command, selectPrefix, sizeof(selectPrefix) - 1))
    {
        OnSelectClass(m_iCurrentTeamPage, command + sizeof(selectPrefix) - 1);
        return;
    }
    if (!Q_stricmp(command, "showctclass") || !Q_stricmp(command, "showterclass"))
    {
        const TeamName team = !Q_stricmp(command, "showctclass") ? CT : TERRORIST;
        if (team == m_iTeam)
            SetupTeamPage(team, 0);
        else
            gEngfuncs.pfnClientCmd(team == CT ? "jointeam 2\n" : "jointeam 1\n");
        // Only the server's MENU_CLASS response confirms a successful team change.
    }
    else if (!Q_stricmp(command, "prevpage"))
        SetupTeamPage(m_iCurrentTeamPage, m_iCurrentPage ? m_iCurrentPage - 1 : 0);
    else if (!Q_stricmp(command, "nextpage"))
        SetupTeamPage(m_iCurrentTeamPage, m_iCurrentPage + 1);
    else
        BaseClass::OnCommand(command);
}

void CCSClassMenu::OnSelectClass(TeamName team, const char* name)
{
    int slot = atoi(name);
    const int count = TeamClassCount(team);
    if (slot == 0)
        slot = gEngfuncs.pfnRandomLong(1, count);
    if (slot < 1 || slot > count)
        return;

    char command[64];
    Q_snprintf(command, sizeof(command), "joinclass %d\n", slot);
    BaseClass::OnCommand("vguicancel");
    gEngfuncs.pfnClientCmd(command);
}

void CCSClassMenu::UpdateGameMode()
{
    const TeamName team = g_iTeamNumber == TEAM_TERRORIST ? TERRORIST : CT;
    SetTeam(team);
    InvalidateLayout();
}

void CCSClassMenu::UpdateClass(int index)
{
    if (index < 0 || index >= 10)
        return;
    if (index == 9)
    {
        m_pClassImage->SetImage(m_iCurrentTeamPage == CT ? "gfx/vgui/ct_random" : "gfx/vgui/t_random");
        m_pClassDesc->SetText("随机选择一名角色");
        return;
    }

    const int slot = static_cast<int>(m_iCurrentPage * CLASSES_PER_PAGE) + index + 1;
    if (slot > TeamClassCount(m_iCurrentTeamPage))
        return;
    const char* model = TeamClassModel(m_iCurrentTeamPage, slot);
    std::string image = std::string("gfx/vgui/") + model;
    m_pClassImage->SetImage(image.c_str());

    std::string name = model;
    name[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(name[0])));
    const std::string description = "#Cstrike_" + name + "_Label";
    if (const wchar_t* text = localize()->Find(description.c_str()))
        m_pClassDesc->SetText(text);
    else if (const wchar_t* text = localize()->Find(ClassNameToken(model).c_str()))
        m_pClassDesc->SetText(text);
    else
        m_pClassDesc->SetText(model);
}
