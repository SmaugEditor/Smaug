#pragma once

#include <bgfx/bgfx.h>

class CWorldRenderer
{
public:
	void Init();
	void Draw(bgfx::ViewId viewId, bgfx::ProgramHandle shaderProgram);

};


class CNode;

struct CNodeRenderData
{
	void Setup(CNode* node);
	void UpdateVertexBuf();
	void Draw();
	void Shutdown();
	bgfx::DynamicVertexBufferHandle m_vertexBuf;
	bgfx::IndexBufferHandle m_indexBuf;
	CNode* m_parentNode;
};


CWorldRenderer& GetWorldRenderer();