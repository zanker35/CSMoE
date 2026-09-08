

#include "ui/vgui2/interfaces/vgui/VGUI.h"
#include "ui/vgui2/interfaces/vgui/ISurface.h"
#include "ui/vgui2/interfaces/vgui/ILocalize.h"
#include "source_sdk/public/tier1/KeyValues.h"

#include "ui/vgui2/vgui_controls/Controls.h"

#include "ui/vgui2/interfaces/IEngineVGui.h"
#include "ui/vgui2/interfaces/vgui/IPanel.h"

void GetHudSize(int &w, int &h)
{
	vgui2::VPANEL hudParent = engineVgui()->GetPanel(PANEL_CLIENTDLL);

	if (hudParent)
		vgui2::ipanel()->GetSize(hudParent, w, h);
	else
		vgui2::surface()->GetScreenSize(w, h);
}