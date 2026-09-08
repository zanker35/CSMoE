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

#include "ui/vgui2/vgui_controls/TextImage.h"
#include "ui/vgui2/vgui_controls/ImagePanel.h"
#include "ui/vgui2/vgui_controls/EditablePanel.h"

#include "game/client/menus/csmoe/BuyMenu/cstrikebuymouseoverpanel.h"
#include "game/client/menus/csmoe/BuyMenu/weaponcatalog.h"
#include "game/client/menus/csmoe/BuyMenu/WeaponImagePanel.h"

#include <string>

using namespace vgui2;

CSBuyMouseOverPanel::CSBuyMouseOverPanel(vgui2::Panel *parent, const char *panelName) : BaseClass(parent, panelName)
{
	SetMouseInputEnabled(false);
	SetKeyBoardInputEnabled(false);

	infolabel = new Label(this, "infolabel", "");

	pricelabel = new Label(this, "pricelabel", "#CStrike_PriceLabel");
	calibrelabel = new Label(this, "calibrelabel", "#CStrike_CalibreLabel");
	clipcapacitylabel = new Label(this, "clipcapacitylabel", "#CStrike_ClipCapacityLabel");
	rateoffirelabel = new Label(this, "rateoffirelabel", "#CStrike_RateOfFireLabel");
	weightloadedlabel = new Label(this, "weightloadedlabel", "#CStrike_WeightLoadedLabel");

	price = new Label(this, "price", "");
	calibre = new Label(this, "calibre", "");
	clipcapacity = new Label(this, "clipcapacity", "");
	rateoffire = new Label(this, "rateoffire", "");
	weightempty = new Label(this, "weightempty", "");

	imageBG = new ImagePanel(this, "imageBG");
	imageBG->SetShouldScaleImage(true);

	classimage = new WeaponImagePanel(this, "classimage");
	classimage->SetShouldScaleImage(true);
}

void CSBuyMouseOverPanel::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	vgui2::Label *info_label[] = { pricelabel, calibrelabel, clipcapacitylabel, rateoffirelabel, weightloadedlabel };
	vgui2::Label *info[] = { price, calibre, clipcapacity, rateoffire, weightempty };
	for (auto p : info_label)
		p->SetFgColor({ 161,128,25,255 });
	for (auto p : info)
		p->SetFgColor({ 161,128,25,255 });

	pricelabel->SetFgColor({ 236,224,148,255 });
	price->SetFgColor({ 236,224,148,255 });

	imageBG->SetImage("resource/Control/basket/basket_blank_slot");
}

void CSBuyMouseOverPanel::PerformLayout(void)
{
	BaseClass::PerformLayout();
	int w, h;
	GetParent()->GetSize(w, h);
	float scale = h / 420.0;

	//SetBounds(216 * scale, 60 * scale, 152 * scale, 145 * scale);

	imageBG->SetBounds(5 * scale, 0 * scale, 147 * scale, 51 * scale);
	classimage->SetBounds(5 * scale, 0 * scale, 147 * scale, 51 * scale);

	vgui2::Label *info_label[] = { pricelabel, calibrelabel, clipcapacitylabel, rateoffirelabel, weightloadedlabel };
	vgui2::Label *info[] = { price, calibre, clipcapacity, rateoffire, weightempty };
	for (int i = 0; i < 5; ++i)
	{
		info_label[i]->SetBounds(5 * scale, (60 + i * 17) * scale, 60 * scale, 14 * scale);
		info[i]->SetBounds(68 * scale, (60 + i * 17) * scale, 60 * scale, 14 * scale);
	}
}

void CSBuyMouseOverPanel::UpdateWeapon(const char *weapon)
{
	bool bEnabled = weapon && weapon[0];

	infolabel->SetVisible(false);

	vgui2::Label *info_label[] = { pricelabel, calibrelabel, clipcapacitylabel, rateoffirelabel, weightloadedlabel };
	vgui2::Label *info[] = { price, calibre, clipcapacity, rateoffire, weightempty };
	for(auto p : info_label)
		p->SetVisible(bEnabled);
	for (auto p : info)
		p->SetVisible(bEnabled);
	classimage->SetVisible(bEnabled);
	imageBG->SetVisible(bEnabled);

	if (!bEnabled)
		return;

	int cost = -1;
	if (const auto *entry = FindBuyMenuWeapon(weapon))
		cost = entry->iCost;
	else
	{
		const struct { const char *name; int cost; } equipment[] = {
			{"vest", KEVLAR_PRICE}, {"vesthelm", ASSAULTSUIT_PRICE},
			{"flash", FLASHBANG_PRICE}, {"hegrenade", HEGRENADE_PRICE},
			{"sgren", SMOKEGRENADE_PRICE}, {"defuser", DEFUSEKIT_PRICE}, {"nvgs", NVG_PRICE}
		};
		for (const auto &item : equipment)
			if (!strcmp(item.name, weapon))
				cost = item.cost;
	}
	price->SetText(cost >= 0 ? std::to_string(cost).c_str() : "-");
	// Master does not expose reliable calibre, weight or rate metadata here.
	for (int i = 1; i < 5; ++i)
	{
		info_label[i]->SetVisible(false);
		info[i]->SetVisible(false);
	}
	
	classimage->SetWeapon(weapon);
}
