#include <assert.h>
#include "ui/vgui2/vgui_controls/ScrollBar.h"
#include "ui/vgui2/vgui_controls/Label.h"
#include "ui/vgui2/vgui_controls/Button.h"

#include "source_sdk/public/tier1/KeyValues.h"
#include "ui/vgui2/interfaces/vgui/MouseCode.h"
#include "ui/vgui2/interfaces/vgui/KeyCode.h"
#include "ui/vgui2/interfaces/vgui/IInput.h"
#include "ui/vgui2/interfaces/vgui/IScheme.h"
#include "ui/vgui2/interfaces/vgui/ISurface.h"
#include "ui/gameui/PanelListPanel.h"

class VScrollBarReversedButtons : public vgui2::ScrollBar
{
public:
	VScrollBarReversedButtons(Panel *parent, const char *panelName, bool vertical);
	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme);
};

VScrollBarReversedButtons::VScrollBarReversedButtons(Panel *parent, const char *panelName, bool vertical) : ScrollBar(parent, panelName, vertical)
{
}

void VScrollBarReversedButtons::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	ScrollBar::ApplySchemeSettings(pScheme);
}

CPanelListPanel::CPanelListPanel(vgui2::Panel *parent, char const *panelName, bool inverseButtons) : Panel(parent, panelName)
{
	SetBounds(0, 0, 100, 100);
	_sliderYOffset = 0;

	if (inverseButtons)
		_vbar = new VScrollBarReversedButtons(this, "CPanelListPanelVScroll", true);
	else
		_vbar = new vgui2::ScrollBar(this, "CPanelListPanelVScroll", true);

	_vbar->SetBounds(0, 0, 20, 20);
	_vbar->SetVisible(false);
	_vbar->AddActionSignalTarget(this);

	_embedded = new Panel(this, "PanelListEmbedded");
	_embedded->SetBounds(0, 0, 20, 20);
	_embedded->SetPaintBackgroundEnabled(false);
	_embedded->SetPaintBorderEnabled(false);
}

CPanelListPanel::~CPanelListPanel(void)
{
	DeleteAllItems();
}

int CPanelListPanel::computeVPixelsNeeded(void)
{
	int pixels = 0;
	DATAITEM *item;
	Panel *panel;

	for (int i = 0; i < _dataItems.GetCount(); i++)
	{
		item = _dataItems[i];

		if (!item)
			continue;

		panel = item->panel;

		if (!panel)
			continue;

		int w, h;
		panel->GetSize(w, h);

		pixels += h;
	}

	pixels += 5;
	return pixels;
}

vgui2::Panel *CPanelListPanel::GetCellRenderer(int row)
{
	DATAITEM *item = _dataItems[row];

	if (item)
	{
		Panel *panel = item->panel;
		return panel;
	}

	return NULL;
}

int CPanelListPanel::AddItem(Panel *panel)
{
	InvalidateLayout();

	DATAITEM *newitem = new DATAITEM;
	newitem->panel = panel;
	panel->SetParent(_embedded);
	return _dataItems.PutElement(newitem);
}

int CPanelListPanel::GetItemCount(void)
{
	return _dataItems.GetCount();
}

vgui2::Panel *CPanelListPanel::GetItem(int itemIndex)
{
	if (itemIndex < 0 || itemIndex >= _dataItems.GetCount())
		return NULL;

	return _dataItems[itemIndex]->panel;
}

CPanelListPanel::DATAITEM *CPanelListPanel::GetDataItem(int itemIndex)
{
	if (itemIndex < 0 || itemIndex >= _dataItems.GetCount())
		return NULL;

	return _dataItems[itemIndex];
}

void CPanelListPanel::RemoveItem(int itemIndex)
{
	DATAITEM *item = _dataItems[itemIndex];
	delete item->panel;
	delete item;

	_dataItems.RemoveElementAt(itemIndex);

	InvalidateLayout();
}

void CPanelListPanel::DeleteAllItems(void)
{
	for (int i = 0; i < _dataItems.GetCount(); i++)
	{
		if (_dataItems[i])
			delete _dataItems[i]->panel;

		delete _dataItems[i];
	}

	_dataItems.RemoveAll();
	InvalidateLayout();
}

void CPanelListPanel::OnMouseWheeled(int delta)
{
	int val = _vbar->GetValue();
	val -= (delta * 3 * 5);
	_vbar->SetValue(val);
}

void CPanelListPanel::PerformLayout(void)
{
	int wide, tall;
	GetSize(wide, tall);

	int vpixels = computeVPixelsNeeded();

	_vbar->SetVisible(true);
	_vbar->SetEnabled(vpixels > tall);
	_vbar->SetRange(0, max(vpixels, tall));
	_vbar->SetRangeWindow(max(1, tall));
	_vbar->SetValue(min(_vbar->GetValue(), max(0, vpixels - tall)));
	_vbar->SetButtonPressedScrollValue(24);
	_vbar->SetPos(wide - 20, _sliderYOffset);
	_vbar->SetSize(18, tall - 2 - _sliderYOffset);
	_vbar->InvalidateLayout();

	int top = _vbar->GetValue();

	_embedded->SetPos(0, -top);
	_embedded->SetSize(wide - 20, vpixels);

	int y = 0;
	int h = 0;

	for (int i = 0; i < _dataItems.GetCount(); i++, y += h)
	{
		DATAITEM *item = _dataItems[i];

		if (!item || !item->panel)
			continue;

		h = item->panel->GetTall();
		item->panel->SetBounds(8, y, wide - 36, h);
	}
}

void CPanelListPanel::PaintBackground(void)
{
	Panel::PaintBackground();
}

void CPanelListPanel::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	Panel::ApplySchemeSettings(pScheme);

	SetBorder(pScheme->GetBorder("ButtonDepressedBorder"));
	SetBgColor(GetSchemeColor("Label.BgColor", GetBgColor(), pScheme));
}

void CPanelListPanel::OnSliderMoved(int position)
{
	InvalidateLayout();
	Repaint();
}

void CPanelListPanel::SetSliderYOffset(int pixels)
{
	_sliderYOffset = pixels;
}
