#pragma once
#include "baseview.h"
#include "3dview.h"
#include "editview.h"

class CUIView : public CBaseView
{
public:
	virtual void Init(bgfx::ViewId viewId, int width, int height, uint32_t clearColor);
	virtual void Draw(float dt);
	virtual void Update(float dt, float mx, float my);
//private:
	bool m_drawEditView;
	CEditView m_editView;
	bool m_drawPreviewView;
	C3DView m_previewView;
};
