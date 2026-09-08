#include <stdio.h>
#include <wchar.h>
#include "source_sdk/public/tier1/utlsymbol.h"

#include "ui/vgui2/interfaces/vgui/IBorder.h"
#include "ui/vgui2/interfaces/vgui/IInput.h"
#include "ui/vgui2/interfaces/vgui/IScheme.h"
#include "ui/vgui2/interfaces/vgui/ISurface.h"
#include "ui/vgui2/interfaces/vgui/ISystem.h"
#include "ui/vgui2/interfaces/vgui/IVGui.h"
#include "ui/vgui2/interfaces/vgui/ILocalize.h"
#include "ui/vgui2/interfaces/vgui/MouseCode.h"
#include "ui/vgui2/interfaces/vgui/KeyCode.h"
#include "source_sdk/public/tier1/KeyValues.h"

#include "game/client/menus/csmoe/BuyMenu/WeaponImagePanel.h"
#include "game/client/menus/csmoe/BuyMenu/weaponimagepath.h"

using namespace vgui2;

WeaponImagePanel::WeaponImagePanel(Panel *parent, const char *name) : BaseClass(parent, name)
{
	m_bBanned = false;
	m_pBannedImage = vgui2::scheme()->GetImage("gfx/vgui/basket/cannotuse", true);
}

void WeaponImagePanel::SetWeapon(const char *name)
{
	SetWeapon(nullptr);
	if (!name || !name[0])
		return;

	const std::string basket = GetBuyMenuBasketImage(name);
	if (filesystem()->FileExists((basket + ".tga").c_str()) ||
		filesystem()->FileExists((basket + ".bmp").c_str()))
	{
		SetImage(basket.c_str());
	}
}

void WeaponImagePanel::SetWeapon(std::nullptr_t)
{
	// Clear the filename too: ImagePanel otherwise lazily restores the old image.
	BaseClass::SetImage("");
	m_bBanned = false;
}

void WeaponImagePanel::PaintBackground()
{
	BaseClass::PaintBackground();
	
	if (m_bBanned)
	{
		vgui2::IImage *backup = GetImage();
		SetImage(m_pBannedImage);
		BaseClass::PaintBackground();
		SetImage(backup);
	}
	
}
