
#ifndef COLOREDBUTTON_H
#define COLOREDBUTTON_H


#include "ui/vgui2/interfaces/vgui/IBorder.h"
#include "ui/vgui2/interfaces/vgui/IScheme.h"
#include "source_sdk/public/tier1/KeyValues.h"

#include "ui/vgui2/vgui_controls/Button.h"
#include "ui/vgui2/vgui_controls/ImagePanel.h"

class ColoredButton : public vgui2::Button
{
	DECLARE_CLASS_SIMPLE(ColoredButton, Button);

	Color _replaceColor;
public:
	ColoredButton(vgui2::Panel *parent, const char *panelName, const char *text) :
		Button(parent, panelName, text) {}

	void SetTextColor(Color col)
	{
		_replaceColor = col;
	}

	virtual Color GetButtonFgColor() override
	{
		return _replaceColor;
	}
};

#endif
