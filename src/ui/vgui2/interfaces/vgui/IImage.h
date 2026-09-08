#ifndef IIMAGE_H
#define IIMAGE_H


#include "ui/vgui2/interfaces/vgui/VGUI.h"
#include "source_sdk/public/Color.h"

namespace vgui2
{

class IImage
{
public:
	virtual void Paint(void) = 0;
	virtual void SetPos(int x, int y) = 0;
	virtual void GetContentSize(int &wide, int &tall) = 0;
	virtual void GetSize(int &wide, int &tall) = 0;
	virtual void SetSize(int wide, int tall) = 0;
	virtual void SetColor(Color color) = 0;
};
}

#endif
