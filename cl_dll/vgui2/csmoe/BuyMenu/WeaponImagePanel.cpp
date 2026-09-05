#include "cl_dll.h"
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
#include "triangleapi.h"
#include <vgui/IPanel.h>
#include <algorithm>

using namespace vgui2;

WeaponImagePanel::WeaponImagePanel(Panel *parent, const char *name) : BaseClass(parent, name)
{
	m_bBanned = false;
	m_pMissingImage = new Label(this, "MissingWeaponImage", L"暂无图标");
	m_pMissingImage->SetContentAlignment(Label::a_center);
	m_pMissingImage->SetMouseInputEnabled(false);
	m_pMissingImage->SetVisible(false);
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
		return;
	}

	// Some ports ship only a weapon HUD sprite, not a basket TGA. Use that
	// weapon's own icon instead of binding a missing texture (a white block).
	const std::string manifest = std::string("sprites/") + name + ".txt";
	int count = 0;
	client_sprite_t *sprites = filesystem()->FileExists(manifest.c_str())
		? gEngfuncs.pfnSPR_GetList(manifest.c_str(), &count) : nullptr;
	const client_sprite_t *icon = nullptr;
	for (int i = 0; i < count; ++i)
		if (!strcmp(sprites[i].szName, "weapon") && (!icon || sprites[i].iRes > icon->iRes))
			icon = &sprites[i];
	if (icon)
	{
		const std::string path = std::string("sprites/") + icon->szSprite + ".spr";
		if (filesystem()->FileExists(path.c_str()) && gEngfuncs.pfnSPR_Load(path.c_str()))
		{
			m_HudSprite = path;
			m_HudRect = icon->rc;
			return;
		}
	}

	// Keep the catalogue entry usable when neither art resource was supplied.
	m_pMissingImage->SetVisible(true);
	gEngfuncs.Con_DPrintf("Buy menu: no basket or HUD icon for %s\n", name);
}

void WeaponImagePanel::SetWeapon(std::nullptr_t)
{
	// Clear the filename too: ImagePanel otherwise lazily restores the old image.
	BaseClass::SetImage("");
	m_HudSprite.clear();
	m_pMissingImage->SetVisible(false);
	m_bBanned = false;
}

void WeaponImagePanel::PaintBackground()
{
	BaseClass::PaintBackground();
	if (!m_HudSprite.empty())
		PaintHudSprite();
	
	if (m_bBanned)
	{
		vgui2::IImage *backup = GetImage();
		SetImage(m_pBannedImage);
		BaseClass::PaintBackground();
		SetImage(backup);
	}
	
}

void WeaponImagePanel::PerformLayout()
{
	BaseClass::PerformLayout();
	m_pMissingImage->SetBounds(0, 0, GetWide(), GetTall());
}

void WeaponImagePanel::PaintHudSprite()
{
	// Sprite handles belong to the engine; resolve through its cache so a map
	// or video reset cannot leave this long-lived menu holding an old handle.
	const HSPRITE sprite = gEngfuncs.pfnSPR_Load(m_HudSprite.c_str());
	const auto *model = gEngfuncs.GetSpritePointer(sprite);
	const int textureWidth = gEngfuncs.pfnSPR_Width(sprite, 0);
	const int textureHeight = gEngfuncs.pfnSPR_Height(sprite, 0);
	const int iconWidth = m_HudRect.right - m_HudRect.left;
	const int iconHeight = m_HudRect.bottom - m_HudRect.top;
	if (!model || textureWidth <= 0 || textureHeight <= 0 || iconWidth <= 0 || iconHeight <= 0)
		return;

	int x = 0, y = 0;
	LocalToScreen(x, y);
	const float fit = std::min(float(GetWide()) / iconWidth, float(GetTall()) / iconHeight);
	const float width = iconWidth * fit, height = iconHeight * fit;
	const float left = x + (GetWide() - width) * 0.5f;
	const float top = y + (GetTall() - height) * 0.5f;
	int clipLeft, clipTop, clipRight, clipBottom;
	ipanel()->GetClipRect(GetVPanel(), clipLeft, clipTop, clipRight, clipBottom);
	const float x1 = std::max(left, float(clipLeft)), y1 = std::max(top, float(clipTop));
	const float x2 = std::min(left + width, float(clipRight)), y2 = std::min(top + height, float(clipBottom));
	if (x2 <= x1 || y2 <= y1)
		return;

	const float u1 = (m_HudRect.left + (x1 - left) / fit) / textureWidth;
	const float v1 = (m_HudRect.top + (y1 - top) / fit) / textureHeight;
	const float u2 = (m_HudRect.left + (x2 - left) / fit) / textureWidth;
	const float v2 = (m_HudRect.top + (y2 - top) / fit) / textureHeight;
	int screenWidth, screenHeight;
	surface()->GetScreenSize(screenWidth, screenHeight);
	if (screenWidth <= 0 || screenHeight <= 0)
		return;
	const float sx = float(gRenderAPI.RenderGetParm(PARM_SCREEN_WIDTH, 0)) / screenWidth;
	const float sy = float(gRenderAPI.RenderGetParm(PARM_SCREEN_HEIGHT, 0)) / screenHeight;

	// Flush VGUI's text batch and invalidate its binding before drawing a sprite.
	// The next VGUI image will then rebind its own texture normally.
	surface()->DrawFlushText();
	surface()->DrawSetTexture(0);
	auto *tri = gEngfuncs.pTriAPI;
	tri->RenderMode(kRenderTransAdd);
	tri->CullFace(TRI_NONE);
	const Color color = GetDrawColor();
	tri->Color4ub(color[0], color[1], color[2], color[3]);
	tri->SpriteTexture(const_cast<model_s *>(model), 0);
	tri->Begin(TRI_QUADS);
	tri->TexCoord2f(u1, v1); tri->Vertex3f(x1 * sx, y1 * sy, 0);
	tri->TexCoord2f(u1, v2); tri->Vertex3f(x1 * sx, y2 * sy, 0);
	tri->TexCoord2f(u2, v2); tri->Vertex3f(x2 * sx, y2 * sy, 0);
	tri->TexCoord2f(u2, v1); tri->Vertex3f(x2 * sx, y1 * sy, 0);
	tri->End();
	tri->RenderMode(kRenderNormal);
}
