#ifndef CSTRIKEBUYMOUSEOVERPANEL_H
#define CSTRIKEBUYMOUSEOVERPANEL_H


#include "ui/vgui2/interfaces/vgui/IScheme.h"
#include "ui/vgui2/vgui_controls/Button.h"
#include "ui/vgui2/interfaces/vgui/KeyCode.h"
#include "source_sdk/public/FileSystem.h"
#include "game/shared/interfaces/maintypes.h"
#include "game/shared/strings/shared_util.h"

#include "game/client/menus/game_controls/mouseoverpanelbutton.h"
#include "game/client/menus/csmoe/newmouseoverpanelbutton.h"

class WeaponImagePanel;

class CSBuyMouseOverPanel : public NewMouseOverPanel
{
	typedef NewMouseOverPanel BaseClass;

public:
	CSBuyMouseOverPanel(vgui2::Panel *parent, const char *panelName);

	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme) override;
	virtual void PerformLayout(void) override;
	void UpdateWeapon(const char *weapon = "");

public:
	vgui2::Label *infolabel;

	vgui2::Label *pricelabel;
	vgui2::Label *calibrelabel;
	vgui2::Label *clipcapacitylabel;
	vgui2::Label *rateoffirelabel;
	vgui2::Label *weightloadedlabel;

	vgui2::Label *price;
	vgui2::Label *calibre;
	vgui2::Label *clipcapacity;
	vgui2::Label *rateoffire;
	vgui2::Label *weightempty;


	vgui2::ImagePanel *imageBG;
	WeaponImagePanel *classimage;
};

#endif
