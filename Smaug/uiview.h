#pragma once
#include "baseview.h"
#include "3dview.h"
#include "editview.h"
#include "selectedview.h"
#include "toolbox.h"
#include "settingsmenu.h"

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
	bool m_drawSelectedView;
	CSelectedView m_selectedView;
	CToolBox m_toolBox;
	CSettingsMenu m_settingsMenu;
};