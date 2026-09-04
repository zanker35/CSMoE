
#include "CGameUIFuncs.h"
#include "menu_int.h"
#include "kbutton.h"
#include "keydefs.h"
#include "vgui_key_translation.h"
#include <stdio.h>
#include <vector>
#include <SDL.h>

namespace ui {
    extern ui_enginefuncs_t engfuncs;
    extern ui_globalvars_t *gpGlobals;
}

bool CGameUIFuncs::IsKeyDown(const char *keyname, bool &isdown) {
	const auto key = static_cast<kbutton_t *>(ui::engfuncs.pfnKeyGetState(keyname));
	isdown = key && (key->state & 1) != 0;
	return key != nullptr;
}

const char *CGameUIFuncs::Key_NameForKey(int keynum) {
	return ui::engfuncs.pfnKeynumToString(keynum);
}

const char *CGameUIFuncs::Key_BindingForKey(int keynum) {
	return ui::engfuncs.pfnKeyGetBinding(keynum);
}

KeyCode CGameUIFuncs::GetVGUI2KeyCodeForBind(const char *bind) {
    for(int keynum = 0; keynum < 256; ++keynum)
    {
        auto bind2 = Key_BindingForKey(keynum);
        if(bind2 && !strcmp(bind, bind2))
        {
            return KeyCode_EngineKeyToVGUI(keynum);
        }
    }
	return vgui2::KEY_NONE;
}

void CGameUIFuncs::GetVideoModes(vmode_t **liststart, int *count) {
    static std::vector<vmode_t> modes;
    if (modes.empty())
    {
        for (int mode = 0; const char *description = ui::engfuncs.pfnGetModeString(mode); ++mode)
        {
            vmode_t entry = {0, 0, 32};
            if (sscanf(description, "%d x %d", &entry.width, &entry.height) == 2)
                modes.push_back(entry);
        }
        vmode_t current;
        GetCurrentVideoMode(&current.width, &current.height, &current.bpp);
        bool found = false;
        for (const auto &mode : modes)
            found |= mode.width == current.width && mode.height == current.height;
        if (!found && current.width > 0 && current.height > 0)
            modes.push_back(current);
    }
    if (liststart) *liststart = modes.data();
    if (count) *count = static_cast<int>(modes.size());
}

void CGameUIFuncs::GetCurrentVideoMode(int *wide, int *tall, int *bpp) {
    int width = ui::gpGlobals ? ui::gpGlobals->scrWidth : 640;
    int height = ui::gpGlobals ? ui::gpGlobals->scrHeight : 480;
    if (SDL_Window *window = SDL_GetKeyboardFocus())
        SDL_GetWindowSize(window, &width, &height);
    if (wide) *wide = width;
    if (tall) *tall = height;
    if (bpp) *bpp = 32;
}

void CGameUIFuncs::GetCurrentRenderer(char *name, int namelen, int *windowed, int *hdmodels, int *addons, int *level) {
    if (name && namelen > 0) snprintf(name, namelen, "gl");
    if (windowed) *windowed = ui::engfuncs.pfnGetCvarFloat("fullscreen") == 0;
    if (hdmodels) *hdmodels = 0;
    if (addons) *addons = 0;
    if (level) *level = 0;
}

bool CGameUIFuncs::IsConnectedToVACSecureServer() {
	return false;
}

int CGameUIFuncs::Key_KeyStringToKeyNum(const char *name) {
    for(int keynum = 0; keynum < 256; ++keynum)
    {
        auto name2 = Key_NameForKey(keynum);
        if(name2 && !strcmp(name2, name))
        {
            return KeyCode_VirtualKeyToVGUI(keynum);
        }
    }
	return 0;
}

EXPOSE_SINGLE_INTERFACE(CGameUIFuncs, IGameUIFuncs, ENGINE_GAMEUIFUNCS_INTERFACE_VERSION);
