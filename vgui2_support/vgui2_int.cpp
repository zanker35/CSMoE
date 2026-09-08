#include "vgui_api.h"
#include "cdll_int.h"

#include "entity_state.h"
#include "usercmd.h"
#include "ref_params.h"
#include "cl_entity.h"
#include "cdll_exp.h"

#include "winsani_in.h"

#include <FileSystem.h>
#include "tier1/interface.h"
#include "vgui/ISurface.h"
#include "vgui_controls/Controls.h"
#include "render_api.h"
#include "BaseUISurface.h"
#include "GameUI/IGameConsole.h"
#include "GameUI/IGameUI.h"
#include "IBaseUI.h"

#include "winsani_out.h"

#include <dlfcn.h>

namespace vgui2 {
cl_enginefunc_t gEngfuncs;
void VGui_Startup(int width, int height);
extern IGameConsole* staticGameConsole;
extern IGameUI* staticGameUIFuncs;
extern IBaseUI *staticUIFuncs;
extern vguiapi_t *g_api;
extern qboolean g_bScissor;
}
extern BaseUISurface* staticSurface;
extern void RegisterInterface();
extern void RegisterControls();
void UITest_Init();
void UITest_RunFrame();

void VGuiWrap2_Startup();
void VGuiWrap2_Shutdown();
void VGuiWrap2_Paint();

using namespace vgui2;

extern "C" int VGui2_COM_ExpandFileName(const char *fileName, char *nameOutBuffer, int nameOutBufferSize) {
    return g_pFullFileSystem->GetLocalPath(fileName, nameOutBuffer, nameOutBufferSize) != NULL;
}

extern "C" void VGui2_pfnDrawSetTextColor(float r, float g, float b) {
    vgui2::surface()->DrawSetTextColor(r * 255, g * 255, b * 255, 255);
    gEngfuncs.pfnDrawSetTextColor(r, g, b);
}

extern "C" int VGui2_Initialize(cl_enginefunc_t *pEnginefuncs) {
    gEngfuncs = *pEnginefuncs;

	return 0;
}

extern "C" void VGui2_Startup()
{
    VGuiWrap2_Startup();
}

extern "C" int VGui2_VidInit()
{
    if (!staticUIFuncs || !staticSurface)
        return 0;
    extern void VGUI2_Draw_Init();
    VGUI2_Draw_Init();
	return 0;
}

extern "C" void VGui2_Paint()
{
    VGuiWrap2_Paint();
}

extern "C" int VGui2_Shutdown()
{
    VGuiWrap2_Shutdown();
    return 0;
}

extern "C" void VGui2_LoadingFinished(const char *mapName)
{
    if (staticGameUIFuncs)
        staticGameUIFuncs->LoadingFinished("level", mapName ? mapName : "");
}

void VGuiWrap2_Startup()
{
    if( staticUIFuncs )
        return;
    RegisterInterface();
    RegisterControls();

    CreateInterfaceFn pEngineFactory = Sys_GetFactoryThis();
    staticUIFuncs = (IBaseUI *)pEngineFactory(BASEUI_INTERFACE_VERSION, NULL);
    staticUIFuncs->Initialize(&pEngineFactory, 1);
    staticUIFuncs->Start(NULL, 0);
    UITest_Init();
}

void VGuiWrap2_Shutdown()
{
    if( staticUIFuncs )
    {
        staticUIFuncs->Shutdown();
        staticUIFuncs = nullptr;
    }
}

void VGuiWrap2_Paint() {
    if (!staticUIFuncs) {
        return;
    }

    int wide, tall;
    staticSurface->GetScreenSize(wide, tall);
    UITest_RunFrame();
    g_bScissor = true;
    staticUIFuncs->Paint(0, 0, wide, tall);
    g_bScissor = false;
}

extern "C" void VGuiWrap2_HideConsole()
{
    if(staticGameConsole)
        staticGameConsole->Hide();
}

extern "C" int VGuiWrap2_IsConsoleVisible()
{
    return staticGameConsole && staticGameConsole->IsConsoleVisible();
}

extern "C" void VGuiWrap2_ToggleConsole()
{
    if (!staticUIFuncs)
        return;
    if (VGuiWrap2_IsConsoleVisible())
        staticUIFuncs->HideConsole();
    else
        staticUIFuncs->ShowConsole();
}

extern "C" void VGuiWrap2_ClearConsole()
{
    if(staticGameConsole)
        staticGameConsole->Clear();
}

extern "C" void VGuiWrap2_ConPrintf(const char* msg)
{
    if(staticGameConsole)
        staticGameConsole->Printf("%s", msg);
}

extern "C" void VGuiWrap2_ConDPrintf(const char* msg)
{
    if(staticGameConsole)
        staticGameConsole->DPrintf("%s", msg);
}
