#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/RichText.h>

#include "IViewport.h"
#include "CClientMOTD.h"

using namespace vgui2;

CClientMOTD::CClientMOTD(IViewport* viewport)
    : BaseClass(nullptr, "ClientMOTD"), m_pViewport(viewport)
{
    SetTitle("服务器信息", true);
    SetScheme("ClientScheme");
    SetMoveable(false);
    SetSizeable(false);
    SetProportional(true);
    SetAutoDelete(false);

    // This client runs offline without a Chrome/WebKit backend. Build native
    // controls directly: MOTD.res contains HTML and must not instantiate it.
    m_pServerName = new Label(this, "serverName", "");
    m_pMessage = new RichText(this, "TextMessage");
    m_pMessage->SetVerticalScrollbar(true);
    m_pOkayButton = new Button(this, "OkayButton", "#GameUI_OK", this, "okay");
    SetVisible(false);
}

CClientMOTD::~CClientMOTD() = default;

void CClientMOTD::PerformLayout()
{
    BaseClass::PerformLayout();
    const auto scaled = [](int value) { return scheme()->GetProportionalScaledValue(value); };
    SetSize(scaled(580), scaled(400));
    MoveToCenterOfScreen();
    m_pServerName->SetBounds(scaled(20), scaled(34), scaled(540), scaled(26));
    m_pMessage->SetBounds(scaled(20), scaled(65), scaled(540), scaled(278));
    m_pOkayButton->SetBounds(scaled(452), scaled(358), scaled(108), scaled(26));
}

void CClientMOTD::OnKeyCodeTyped(KeyCode key)
{
    if (key == KEY_PAD_ENTER || key == KEY_ENTER || key == KEY_ESCAPE)
        Close();
    else
        BaseClass::OnKeyCodeTyped(key);
}

void CClientMOTD::OnCommand(const char* command)
{
    if (!stricmp(command, "okay") || !stricmp(command, "Close"))
        Close();
    else
        BaseClass::OnCommand(command);
}

void CClientMOTD::Close()
{
    BaseClass::Close();
    m_pViewport->ShowBackGround(false);
}

void CClientMOTD::Activate(const char* title, const char* message)
{
    m_pServerName->SetText(title ? title : "");
    m_pMessage->SetText(message ? message : "");
    m_pMessage->GotoTextStart();
    ShowPanel(true);
    m_pOkayButton->RequestFocus();
}

void CClientMOTD::Reset()
{
    m_pServerName->SetText("");
    m_pMessage->SetText("");
}

void CClientMOTD::ShowPanel(bool state)
{
    m_pViewport->ShowBackGround(state);
    if (state)
    {
        SetMouseInputEnabled(true);
        SetKeyBoardInputEnabled(true);
        BaseClass::Activate();
    }
    else
    {
        SetVisible(false);
        SetMouseInputEnabled(false);
        SetKeyBoardInputEnabled(false);
    }
}
