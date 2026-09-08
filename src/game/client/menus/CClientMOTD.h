#ifndef GAME_CLIENT_UI_VGUI2_CCLIENTMOTD_H
#define GAME_CLIENT_UI_VGUI2_CCLIENTMOTD_H

#include "ui/vgui2/vgui_controls/Frame.h"
#include "game/client/menus/IViewportPanel.h"

class IViewport;

namespace vgui2
{
class Button;
class Label;
class RichText;
}

class CClientMOTD : public vgui2::Frame, public IViewportPanel
{
public:
    DECLARE_CLASS_SIMPLE(CClientMOTD, Frame);

    explicit CClientMOTD(IViewport* viewport);
    ~CClientMOTD() override;

    void PerformLayout() override;
    void OnKeyCodeTyped(vgui2::KeyCode key) override;
    void OnCommand(const char* command) override;
    void Close() override;
    void Activate(const char* title, const char* message);

    const char* GetName() override { return "ClientMOTD"; }
    void SetData(KeyValues*) override {}
    void Reset() override;
    void Update() override {}
    bool NeedsUpdate() override { return false; }
    bool HasInputElements() override { return true; }
    void ShowPanel(bool state) override;

    vgui2::VPANEL GetVPanel() override final { return BaseClass::GetVPanel(); }
    bool IsVisible() override final { return BaseClass::IsVisible(); }
    void SetParent(vgui2::VPANEL parent) override final { BaseClass::SetParent(parent); }
    void Init() override final { ShowPanel(false); }
    void VidInit() override final { ShowPanel(false); }

private:
    IViewport* m_pViewport;
    vgui2::RichText* m_pMessage;
    vgui2::Label* m_pServerName;
    vgui2::Button* m_pOkayButton;
};

#endif
