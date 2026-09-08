#ifndef TOOLBAR_H
#define TOOLBAR_H


#include "ui/vgui2/vgui_controls/Frame.h"
#include "ui/vgui2/vgui_controls/HTML.h"

class CToolBar : public vgui2::Panel
{
	typedef vgui2::Panel BaseClass;

public:
	CToolBar(vgui2::Panel *parent, const char *panelName);
	~CToolBar(void);

public:
	void ApplySchemeSettings(vgui2::IScheme *pScheme);
	void PerformLayout(void);
	void PaintBackground(void);
};

#endif
