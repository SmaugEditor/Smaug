#pragma once

#include "mesh.h"
#include "modelmanager.h"

#include <bgfx/bgfx.h>

class CMeshRenderer
{
public:
	CMeshRenderer(meshPart_t& part);
	~CMeshRenderer();

	void RebuildRenderData();
	void Render();
	void Render(CModelTransform& trnsfm);

private:
	meshPart_t& m_part;
	void BuildRenderData(const bgfx::Memory*& vertBuf, const bgfx::Memory*& indexBuf);
	
	bgfx::DynamicVertexBufferHandle m_vertexBuf = BGFX_INVALID_HANDLE;
	bgfx::DynamicIndexBufferHandle m_indexBuf = BGFX_INVALID_HANDLE;
	int m_indexCount;
};

