#ifndef MULTIPLAYERADVANCEDDIALOG_H
#define MULTIPLAYERADVANCEDDIALOG_H


#include "ui/vgui2/vgui_controls/Frame.h"
#include "ui/gameui/ScriptObject.h"
#include "ui/vgui2/interfaces/vgui/KeyCode.h"

class CMultiplayerAdvancedDialog : public vgui2::Frame
{
	DECLARE_CLASS_SIMPLE(CMultiplayerAdvancedDialog, vgui2::Frame);

public:
	CMultiplayerAdvancedDialog(vgui2::Panel *parent);
	~CMultiplayerAdvancedDialog(void);

public:
	virtual void Activate(void);

private:

	void CreateControls(void);
	void DestroyControls(void);
	void GatherCurrentValues(void);
	void SaveValues(void);

public:
	CInfoDescription *m_pDescription;
	mpcontrol_t *m_pList;
	CPanelListPanel *m_pListPanel;

public:
	virtual void OnCommand(const char *command);
	virtual void OnClose(void);
	virtual void OnKeyCodeTyped(vgui2::KeyCode code);
};

#endif
