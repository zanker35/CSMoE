
#ifndef BUTTONGLOW_H
#define BUTTONGLOW_H


#include "ui/vgui2/interfaces/vgui/IBorder.h"
#include "ui/vgui2/interfaces/vgui/IScheme.h"
#include "source_sdk/public/tier1/KeyValues.h"

#include "ui/vgui2/vgui_controls/Button.h"
#include "ui/vgui2/vgui_controls/ImagePanel.h"

class ButtonGlow : public vgui2::Button
{
	DECLARE_CLASS_SIMPLE(ButtonGlow, Button);

public:
	ButtonGlow(vgui2::Panel *parent, const char *panelName, const char *text) :
		Button(parent, panelName, text) {}

	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme) override
	{
		BaseClass::ApplySchemeSettings(pScheme);

		_imageBackground = true;
		_depressedImage[0] = vgui2::scheme()->GetImage("resource/control/button_glow/btn_glow_left@c", true);
		_depressedImage[1] = vgui2::scheme()->GetImage("resource/control/button_glow/btn_glow_center@c", true);
		_depressedImage[2] = vgui2::scheme()->GetImage("resource/control/button_glow/btn_glow_right@c", true);
		_defaultImage[0] = vgui2::scheme()->GetImage("resource/control/button_glow/btn_glow_left@n", true);
		_defaultImage[1] = vgui2::scheme()->GetImage("resource/control/button_glow/btn_glow_center@n", true);
		_defaultImage[2] = vgui2::scheme()->GetImage("resource/control/button_glow/btn_glow_right@n", true);
		_armedImage[0] = vgui2::scheme()->GetImage("resource/control/button_glow/btn_glow_left@o", true);
		_armedImage[1] = vgui2::scheme()->GetImage("resource/control/button_glow/btn_glow_center@o", true);
		_armedImage[2] = vgui2::scheme()->GetImage("resource/control/button_glow/btn_glow_right@o", true);
	}
	virtual Color GetButtonFgColor() override
	{
		return { 192, 205, 224, 255 };
	}
};

#endif
