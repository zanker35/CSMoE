#include <algorithm>
#include <cstdlib>
#include <string>
#include <vector>

#include "xash3d_types.h"
#include "cdll_int.h"
#include "vgui/IClientPanel.h"
#include "vgui/IInputInternal.h"
#include "vgui/IPanel.h"
#include "vgui/ISurface.h"
#include "vgui/IVGui.h"
#include "vgui_controls/Controls.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/TextEntry.h"
#include "tier1/KeyValues.h"
#include "tier1/strtools.h"

namespace vgui2 {
extern cl_enginefunc_t gEngfuncs;
extern IInputInternal *g_pInputInternal;
}

namespace {
using namespace vgui2;

bool enabled = false;

struct VisiblePanel {
    VPANEL handle;
    std::string path;
    int depth;
};

struct PendingClick {
    int stage = -1;
    int x = 0;
    int y = 0;
    VPANEL expected = 0;
    std::string path;
    bool hoverOnly = false;
} pending;

struct PendingType {
    int stage = -1;
    std::string path;
    std::basic_string<uchar32> text;
    size_t position = 0;
} typing;

void CollectVisible(VPANEL panel, const std::string &parent, int depth, std::vector<VisiblePanel> &result)
{
    if (!panel || !ipanel()->IsVisible(panel) || result.size() >= 1024 || depth >= 40)
        return;
    const char *name = ipanel()->GetName(panel);
    std::string path = parent + "/" + (name && *name ? name : "<unnamed>");
    result.push_back({panel, path, depth});
    for (int i = 0; i < ipanel()->GetChildCount(panel); ++i)
        CollectVisible(ipanel()->GetChild(panel, i), path, depth + 1, result);
}

std::vector<VisiblePanel> VisiblePanels()
{
    std::vector<VisiblePanel> panels;
    if (surface() && ipanel())
        CollectVisible(surface()->GetEmbeddedPanel(), "", 0, panels);
    return panels;
}

bool Matches(const VisiblePanel &panel, const char *query)
{
    if (!query || !*query)
        return false;
    if (panel.path == query || !strcmp(ipanel()->GetName(panel.handle), query))
        return true;
    std::string suffix = std::string("/") + query;
    return panel.path.size() >= suffix.size() &&
        panel.path.compare(panel.path.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool Resolve(const char *query, VisiblePanel &result)
{
    int matches = 0;
    for (const auto &panel : VisiblePanels())
    {
        if (Matches(panel, query))
        {
            result = panel;
            ++matches;
        }
    }
    if (matches != 1)
    {
        gEngfuncs.Con_Printf("[ui_test] resolve query=\"%s\" matches=%d; use a unique visible path\n", query, matches);
        return false;
    }
    return true;
}

std::string PanelText(VPANEL panel)
{
    char text[512] = {};
    auto client = ipanel()->Client(panel);
    auto control = client ? client->GetPanel() : nullptr;
    if (auto entry = dynamic_cast<TextEntry *>(control))
    {
        KeyValues *settings = new KeyValues("settings");
        static_cast<Panel *>(entry)->GetSettings(settings);
        bool hidden = settings->GetInt("textHidden") != 0;
        settings->deleteThis();
        if (hidden)
            return "<hidden>";
        entry->GetText(text, sizeof(text));
    }
    else if (auto label = dynamic_cast<Label *>(control))
        label->GetText(text, sizeof(text));
    for (char *character = text; *character; ++character)
        if (*character == '\n' || *character == '\r' || *character == '\t')
            *character = ' ';
    return text;
}

void TreeCommand()
{
    const char *query = gEngfuncs.Cmd_Argc() > 1 ? gEngfuncs.Cmd_Argv(1) : "";
    auto panels = VisiblePanels();
    std::vector<std::string> roots;
    if (*query)
        for (const auto &panel : panels)
            if (Matches(panel, query))
                roots.push_back(panel.path);
    int printed = 0;
    for (const auto &panel : panels)
    {
        bool selected = !*query;
        for (const auto &root : roots)
            selected |= panel.path == root || panel.path.compare(0, root.size() + 1, root + "/") == 0;
        if (!selected)
            continue;
        int x, y, wide, tall;
        ipanel()->GetAbsPos(panel.handle, x, y);
        ipanel()->GetSize(panel.handle, wide, tall);
        gEngfuncs.Con_Printf("[ui_test] panel path=\"%s\" class=%s bounds=%d,%d,%d,%d enabled=%d mouse=%d text=\"%s\"\n",
            panel.path.c_str(), ipanel()->GetClassName(panel.handle), x, y, wide, tall,
            ipanel()->IsEnabled(panel.handle), ipanel()->IsMouseInputEnabled(panel.handle), PanelText(panel.handle).c_str());
        ++printed;
    }
    gEngfuncs.Con_Printf("[ui_test] tree panels=%d visible_total=%d%s\n", printed, static_cast<int>(panels.size()),
        panels.size() == 1024 ? " truncated=1" : "");
}

void ClickCommand()
{
    if ((pending.stage >= 0 && !pending.hoverOnly) || typing.stage >= 0)
    {
        gEngfuncs.Con_Printf("[ui_test] mouse sequence busy\n");
        return;
    }
    if (gEngfuncs.Cmd_Argc() == 3)
    {
        char *xEnd, *yEnd;
        pending.x = static_cast<int>(strtol(gEngfuncs.Cmd_Argv(1), &xEnd, 10));
        pending.y = static_cast<int>(strtol(gEngfuncs.Cmd_Argv(2), &yEnd, 10));
        if (*xEnd || *yEnd)
        {
            gEngfuncs.Con_Printf("[ui_test] usage: ui_test_click <unique path> | <x> <y>\n");
            return;
        }
        pending.expected = 0;
        pending.path = "<coordinates>";
    }
    else if (gEngfuncs.Cmd_Argc() == 2)
    {
        VisiblePanel panel;
        if (!Resolve(gEngfuncs.Cmd_Argv(1), panel))
            return;
        int x, y, wide, tall, left, top, right, bottom;
        ipanel()->GetAbsPos(panel.handle, x, y);
        ipanel()->GetSize(panel.handle, wide, tall);
        ipanel()->GetClipRect(panel.handle, left, top, right, bottom);
        left = std::max(left, x);
        top = std::max(top, y);
        right = std::min(right, x + wide);
        bottom = std::min(bottom, y + tall);
        if (!ipanel()->IsEnabled(panel.handle) || !ipanel()->IsMouseInputEnabled(panel.handle) || left >= right || top >= bottom)
        {
            gEngfuncs.Con_Printf("[ui_test] panel not clickable path=\"%s\"\n", panel.path.c_str());
            return;
        }
        pending.x = (left + right) / 2;
        pending.y = (top + bottom) / 2;
        pending.expected = panel.handle;
        pending.path = panel.path;
    }
    else
    {
        gEngfuncs.Con_Printf("[ui_test] usage: ui_test_click <unique path> | <x> <y>\n");
        return;
    }
    pending.hoverOnly = false;
    pending.stage = 0;
    gEngfuncs.Con_Printf("[ui_test] queued mouse path=\"%s\" position=%d,%d\n", pending.path.c_str(), pending.x, pending.y);
}

void ControlCommand()
{
    if (pending.hoverOnly)
        pending.stage = -1;
    if (gEngfuncs.Cmd_Argc() != 3)
    {
        gEngfuncs.Con_Printf("[ui_test] usage: ui_test_command <unique path> <command>\n");
        return;
    }
    VisiblePanel panel;
    if (!Resolve(gEngfuncs.Cmd_Argv(1), panel))
        return;
    if (!ipanel()->IsEnabled(panel.handle))
    {
        gEngfuncs.Con_Printf("[ui_test] command refused: disabled panel\n");
        return;
    }
    ivgui()->PostMessage(panel.handle, new KeyValues("Command", "command", gEngfuncs.Cmd_Argv(2)), 0);
    gEngfuncs.Con_Printf("[ui_test] queued command path=\"%s\" command=\"%s\"\n", panel.path.c_str(), gEngfuncs.Cmd_Argv(2));
}

void TypeCommand()
{
    if (gEngfuncs.Cmd_Argc() != 3 || (pending.stage >= 0 && !pending.hoverOnly) || typing.stage >= 0)
    {
        gEngfuncs.Con_Printf("[ui_test] usage (when input is idle): ui_test_type <TextEntry path> <text>\n");
        return;
    }
    VisiblePanel panel;
    if (!Resolve(gEngfuncs.Cmd_Argv(1), panel))
        return;
    auto client = ipanel()->Client(panel.handle);
    auto entry = client ? dynamic_cast<TextEntry *>(client->GetPanel()) : nullptr;
    if (!entry || !entry->IsEnabled() || !entry->IsEditable())
    {
        gEngfuncs.Con_Printf("[ui_test] type refused: target is not an enabled editable TextEntry\n");
        return;
    }
    const char *text = gEngfuncs.Cmd_Argv(2);
    if (strlen(text) > 1024)
    {
        gEngfuncs.Con_Printf("[ui_test] type refused: text too long\n");
        return;
    }
    int bytes = Q_UTF8ToUTF32(text, nullptr, 0);
    typing.text.assign(bytes / sizeof(uchar32), 0);
    Q_UTF8ToUTF32(text, typing.text.data(), bytes);
    if (!typing.text.empty() && typing.text.back() == 0)
        typing.text.pop_back();
    typing.path = panel.path;
    typing.position = 0;
    typing.stage = 0;
    pending.stage = -1;
    entry->RequestFocus();
    entry->SelectAllText(true);
    gEngfuncs.Con_Printf("[ui_test] queued typing path=\"%s\" characters=%d\n", typing.path.c_str(), static_cast<int>(typing.text.size()));
}

void HoverCommand()
{
    if (gEngfuncs.Cmd_Argc() != 2 || typing.stage >= 0 || (pending.stage >= 0 && !pending.hoverOnly))
    {
        gEngfuncs.Con_Printf("[ui_test] usage (when input is idle): ui_test_hover <unique visible path>\n");
        return;
    }
    VisiblePanel panel;
    if (!Resolve(gEngfuncs.Cmd_Argv(1), panel))
        return;
    int x, y, wide, tall, left, top, right, bottom;
    ipanel()->GetAbsPos(panel.handle, x, y);
    ipanel()->GetSize(panel.handle, wide, tall);
    ipanel()->GetClipRect(panel.handle, left, top, right, bottom);
    left = std::max(left, x);
    top = std::max(top, y);
    right = std::min(right, x + wide);
    bottom = std::min(bottom, y + tall);
    if (!ipanel()->IsMouseInputEnabled(panel.handle) || left >= right || top >= bottom)
    {
        gEngfuncs.Con_Printf("[ui_test] hover refused: panel has no visible mouse area\n");
        return;
    }
    pending.x = (left + right) / 2;
    pending.y = (top + bottom) / 2;
    pending.expected = panel.handle;
    pending.path = panel.path;
    pending.hoverOnly = true;
    pending.stage = 0;
}

void RunTyping()
{
    VisiblePanel panel;
    if (!Resolve(typing.path.c_str(), panel))
    {
        typing.stage = -1;
        return;
    }
    if (typing.stage == 0)
    {
        ++typing.stage; // let VGUI settle normal keyboard focus before key events
        return;
    }
    if (g_pInputInternal->GetFocus() != panel.handle)
    {
        gEngfuncs.Con_Printf("[ui_test] type refused: requested TextEntry does not own keyboard focus\n");
        typing.stage = -1;
        return;
    }
    if (typing.text.empty() && typing.stage == 1)
    {
        g_pInputInternal->InternalKeyCodePressed(KEY_BACKSPACE);
        g_pInputInternal->InternalKeyCodeTyped(KEY_BACKSPACE);
        g_pInputInternal->InternalKeyCodeReleased(KEY_BACKSPACE);
        ++typing.stage;
        return;
    }
    if (typing.position < typing.text.size())
    {
        g_pInputInternal->InternalKeyTyped(typing.text[typing.position++]);
        return;
    }
    gEngfuncs.Con_Printf("[ui_test] type_complete path=\"%s\" value=\"%s\"\n", typing.path.c_str(), PanelText(panel.handle).c_str());
    typing.stage = -1;
}
}

void UITest_Init()
{
    if (!vgui2::gEngfuncs.CheckParm("-uitest", nullptr))
        return;
    enabled = true;
    vgui2::gEngfuncs.pfnAddCommand("ui_test_tree", TreeCommand);
    vgui2::gEngfuncs.pfnAddCommand("ui_test_click", ClickCommand);
    vgui2::gEngfuncs.pfnAddCommand("ui_test_command", ControlCommand);
    vgui2::gEngfuncs.pfnAddCommand("ui_test_type", TypeCommand);
    vgui2::gEngfuncs.pfnAddCommand("ui_test_hover", HoverCommand);
    vgui2::gEngfuncs.Con_Printf("[ui_test] enabled; input events use the normal VGUI dispatcher\n");
}

void UITest_RunFrame()
{
    using namespace vgui2;
    if (!enabled || !g_pInputInternal)
        return;
    if (typing.stage >= 0)
        RunTyping();
    if (pending.stage < 0)
        return;
    g_pInputInternal->InternalCursorMoved(pending.x, pending.y);
    g_pInputInternal->UpdateMouseFocus(pending.x, pending.y);
    if (pending.hoverOnly)
    {
        bool visible = false;
        for (const auto &panel : VisiblePanels())
            visible |= panel.handle == pending.expected;
        if (!visible)
        {
            pending.stage = -1;
            return;
        }
        if (pending.stage == 0)
        {
            VPANEL hit = g_pInputInternal->GetMouseOver();
            gEngfuncs.Con_Printf("[ui_test] hover path=\"%s\" hit=%s position=%d,%d\n", pending.path.c_str(),
                hit ? ipanel()->GetName(hit) : "<none>", pending.x, pending.y);
            pending.stage = 1;
        }
        return; // keep the VGUI cursor here for inspection; never inject a press
    }
    if (pending.stage == 1)
    {
        VPANEL hit = g_pInputInternal->GetMouseOver();
        bool expectedExists = !pending.expected;
        for (const auto &panel : VisiblePanels())
            expectedExists |= panel.handle == pending.expected;
        if (!hit || !expectedExists || (pending.expected && hit != pending.expected && !ipanel()->HasParent(hit, pending.expected)))
        {
            gEngfuncs.Con_Printf("[ui_test] mouse refused: target obscured or removed; hit=%s\n", hit ? ipanel()->GetName(hit) : "<none>");
            pending.stage = -1;
            return;
        }
        gEngfuncs.Con_Printf("[ui_test] mouse_press hit=%s class=%s\n", ipanel()->GetName(hit), ipanel()->GetClassName(hit));
        g_pInputInternal->InternalMousePressed(MOUSE_LEFT);
    }
    else if (pending.stage == 2)
    {
        g_pInputInternal->InternalMouseReleased(MOUSE_LEFT);
        gEngfuncs.Con_Printf("[ui_test] mouse_release position=%d,%d; inspect resulting UI/game state\n", pending.x, pending.y);
        pending.stage = -1;
        return;
    }
    ++pending.stage;
}
