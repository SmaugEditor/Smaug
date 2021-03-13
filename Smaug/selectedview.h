#pragma once
#include "baseview.h"

class CNode;
class CSelectedView : public CBaseView
{
public:
	virtual void Init(bgfx::ViewId viewId, int width, int height, uint32_t clearColor);
	virtual void Draw(float dt);
	void SetSelection(CNode* node);
	bool Show();
private:
	CNode* m_selectedNode;
	float m_aabbLength;
};

CSelectedView& SelectedView();
