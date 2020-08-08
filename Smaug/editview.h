#pragma once
#include "baseview.h"
class CEditView : public CBaseView
{
public:
	virtual void Init(bgfx::ViewId viewId, int width, int height, uint32_t clearColor);
	virtual void Update(float dt);
private:

	bgfx::ProgramHandle m_hShaderProgram;
	bgfx::VertexBufferHandle m_hVertexBuf;
	bgfx::IndexBufferHandle m_hIndexBuf;
};

