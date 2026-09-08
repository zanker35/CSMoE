#ifndef CSCLASSMENU_H
#define CSCLASSMENU_H

#include "game/shared/data/player_const.h"
#include "game/client/menus/game_controls/classmenu.h"
#include "ui/vgui2/vgui_controls/ImagePanel.h"
#include "ui/vgui2/cso_controls/NewTabButton.h"
#include "game/client/menus/csmoe/newmouseoverpanelbutton.h"

class CCSClassMenu : public CClassMenu
{
    DECLARE_CLASS_SIMPLE(CCSClassMenu, CClassMenu);
public:
    explicit CCSClassMenu(IViewport* viewport);
    ~CCSClassMenu() override;
    const char* GetName() override;
    void Reset() override;
    void ShowPanel(bool show) override;
    void PaintBackground() override;
    void PerformLayout() override;
    void ApplySchemeSettings(vgui2::IScheme* scheme) override;
    void OnCommand(const char* command) override;
    void SetTeam(TeamName team);
    void SetupTeamPage(TeamName team, size_t page);
    void UpdateGameMode();
    void UpdateClass(int index);
    void OnSelectClass(TeamName team, const char* name);
    MESSAGE_FUNC_CHARPTR(OnUpdateClass, "UpdateClass", name);

protected:
    MouseOverPanelButton* CreateNewMouseOverPanelButton(vgui2::EditablePanel* panel) override;

private:
    NewTabButton* m_pShowCT = nullptr;
    NewTabButton* m_pShowTER = nullptr;
    NewMouseOverPanelButton* m_pSlotButtons[10] = {};
    vgui2::Button* m_pPrevBtn = nullptr;
    vgui2::Button* m_pNextBtn = nullptr;
    vgui2::Label* m_pTitleLabel = nullptr;
    vgui2::ImagePanel* m_pClassImage = nullptr;
    vgui2::Label* m_pClassDesc = nullptr;
    size_t m_iCurrentPage = 0;
    TeamName m_iCurrentTeamPage = CT;
};

#endif
