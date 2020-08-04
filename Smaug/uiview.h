#pragma once
#include "baseview.h"
#include "3dview.h"

class CUIView : public CBaseView
{
public:

	virtual void Init(bgfx::ViewId viewId, int width, int height, uint32_t clearColor);
	virtual void Update(float dt);
private:
	C3DView m_previewView;
};
