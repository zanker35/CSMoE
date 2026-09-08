#include "source_sdk/public/tier1/interface.h"

#include "source_sdk/public/icvar.h"
#include "ui/vgui2/interfaces/GameUI/IGameConsole.h"
#include "ui/vgui2/interfaces/GameUI/IGameUI.h"
#include "ui/vgui2/interfaces/vgui/ILocalize.h"
#include "ui/vgui2/interfaces/vgui/IInput.h"
#include "ui/vgui2/interfaces/vgui/IInputInternal.h"
#include "ui/vgui2/interfaces/vgui/IScheme.h"
#include "ui/vgui2/interfaces/vgui/ISchemeManager.h"
#include "ui/vgui2/interfaces/vgui/IVGui.h"
#include "ui/vgui2/interfaces/vgui/IKeyValues.h"
#include "ui/vgui2/interfaces/vgui/IPanel.h"
#include "ui/vgui2/interfaces/vgui/ISystem.h"
#include "ui/vgui2/interfaces/vgui/ISurface.h"
#include "ui/vgui2/interfaces/vgui/IBorder.h"
#include "ui/vgui2/interfaces/vgui/IClientVGUI.h"
#include "ui/vgui2/interfaces/IBaseUI.h"
#include "ui/vgui2/interfaces/IEngineVGui.h"
#include "ui/vgui2/interfaces/IGameUIFuncs.h"
#include "source_sdk/public/FileSystem.h"

#define REGISTER_INTERFACE_FN(functionName, interfaceName, versionName) \
    void* functionName(); \
    new InterfaceReg(functionName, versionName);
#define REGISTER_INTERFACE(className) \
    void* __Create##className##_interface();  \
    new InterfaceReg(functionName, versionName);
#define REGISTER_SINGLE_INTERFACE_GLOBALVAR_WITH_NAMESPACE(className, interfaceNamespace, interfaceName, versionName, globalVarName) \
	void* __Create##className##interfaceName##_interface(); \
    new InterfaceReg(__Create##className##interfaceName##_interface, versionName);
#define REGISTER_SINGLE_INTERFACE_GLOBALVAR(className, interfaceName, versionName, globalVarName) \
    REGISTER_SINGLE_INTERFACE_GLOBALVAR_WITH_NAMESPACE(className, , interfaceName, versionName, globalVarName)
#define REGISTER_SINGLE_INTERFACE(className, interfaceName, versionName) \
	REGISTER_SINGLE_INTERFACE_GLOBALVAR(className, interfaceName, versionName, __g_##className##_singleton)

// tier1
void RegisterInterface()
{
    
REGISTER_SINGLE_INTERFACE_GLOBALVAR_WITH_NAMESPACE(CLocalizedStringTable, vgui2::, ILocalize, VGUI_LOCALIZE_INTERFACE_VERSION, g_StringTable);

// vstdlib
REGISTER_SINGLE_INTERFACE_GLOBALVAR(CCvar, ICvar, CVAR_INTERFACE_VERSION, s_Cvar);

// GameUI
REGISTER_SINGLE_INTERFACE_GLOBALVAR(CGameConsole, IGameConsole, GAMECONSOLE_INTERFACE_VERSION, g_GameConsole);
REGISTER_SINGLE_INTERFACE_GLOBALVAR(CGameUI, IGameUI, GAMEUI_INTERFACE_VERSION, g_GameUI);

// vgui2
REGISTER_SINGLE_INTERFACE_GLOBALVAR(CInputWin32, IInput, VGUI_INPUT_INTERFACE_VERSION, g_Input);
REGISTER_SINGLE_INTERFACE_GLOBALVAR(CInputWin32, IInputInternal, VGUI_INPUTINTERNAL_INTERFACE_VERSION, g_Input);

// vgui2
REGISTER_SINGLE_INTERFACE_GLOBALVAR(CSchemeManager, ISchemeManager, VGUI_SCHEME_INTERFACE_VERSION, g_Scheme);
REGISTER_SINGLE_INTERFACE(CVGui, IVGui, VGUI_IVGUI_INTERFACE_VERSION);
REGISTER_SINGLE_INTERFACE(CVGuiKeyValues, IKeyValues, KEYVALUES_INTERFACE_VERSION);
REGISTER_SINGLE_INTERFACE(VPanelWrapper, IPanel, VGUI_PANEL_INTERFACE_VERSION);
REGISTER_SINGLE_INTERFACE_GLOBALVAR(CSystem, ISystem, VGUI_SYSTEM_INTERFACE_VERSION, g_System);

// engine
REGISTER_SINGLE_INTERFACE(BaseUISurface, ISurface, VGUI_SURFACE_INTERFACE_VERSION);
REGISTER_SINGLE_INTERFACE(CBaseUI, IBaseUI, BASEUI_INTERFACE_VERSION);
REGISTER_SINGLE_INTERFACE(CEngineVGui, IEngineVGui, VENGINE_VGUI_VERSION);
REGISTER_SINGLE_INTERFACE(CGameUIFuncs, IGameUIFuncs, ENGINE_GAMEUIFUNCS_INTERFACE_VERSION);
REGISTER_SINGLE_INTERFACE_GLOBALVAR( CXashFileSystem, IFileSystem, FILESYSTEM_INTERFACE_VERSION, fs );

// client
REGISTER_SINGLE_INTERFACE_GLOBALVAR( CClientVGUI, IClientVGUI, CLIENTVGUI_INTERFACE_VERSION, g_ClientVGUI );
}