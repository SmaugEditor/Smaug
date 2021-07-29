#pragma once
#include "ibaseview.h"
#include "viewlist.h"
#include "editorinterface.h"
#include <glm/mat4x4.hpp>

class CBaseView : public IBaseView
{
public:
	virtual void Init(uint16_t viewId, int width, int height, uint32_t clearColor);

	virtual void Draw(float dt);
	virtual void Update(float dt, float mx, float my) {}

	virtual ImTextureID GetImTextureID();

	renderTarget_t m_renderTarget;
	

	int m_width;
	int m_height;
	uint32_t m_clearColor;

	// Width/Height
	float m_aspectRatio;

	bool m_focused;
};
