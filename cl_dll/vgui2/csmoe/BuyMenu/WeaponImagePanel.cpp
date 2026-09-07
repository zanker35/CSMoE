#include <stdio.h>
#include <wchar.h>
#include <UtlSymbol.h>

#include <vgui/IBorder.h>
#include <vgui/IInput.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/IVGui.h>
#include <vgui/ILocalize.h>
#include <vgui/MouseCode.h>
#include <vgui/KeyCode.h>
#include <KeyValues.h>

#include "WeaponImagePanel.h"
#include "weaponimagepath.h"

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
