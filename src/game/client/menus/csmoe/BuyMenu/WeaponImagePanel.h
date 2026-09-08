#ifndef WEAPONIMAGEPANEL_H
#define WEAPONIMAGEPANEL_H


#include "ui/vgui2/interfaces/vgui/IScheme.h"
#include "ui/vgui2/vgui_controls/Button.h"
#include "ui/vgui2/vgui_controls/ImagePanel.h"
#include "ui/vgui2/interfaces/vgui/KeyCode.h"
#include "source_sdk/public/FileSystem.h"
#include <cstddef>

class WeaponImagePanel : public vgui2::ImagePanel
{
private:
	typedef vgui2::ImagePanel BaseClass;
public:
	WeaponImagePanel(Panel *parent, const char *name);

	virtual void PaintBackground() override;

	void SetWeapon(const char *weapon);
	void SetWeapon(std::nullptr_t);
private:
	virtual void SetImage(vgui2::IImage *image) override { return BaseClass::SetImage(image); }
	virtual void SetImage(const char *imageName) override { return BaseClass::SetImage(imageName); }

	bool m_bBanned;
	vgui2::IImage *m_pBannedImage;
};

#endif
