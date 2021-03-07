#pragma once

#include "mesh.h"
#include "modelmanager.h"

#include <bgfx/bgfx.h>

class CMeshRenderer
{
public:
	CMeshRenderer(cuttableMesh_t& mesh);
	~CMeshRenderer();

	void RebuildRenderData();
	void Render();
	void Render(CModelTransform& trnsfm);
	cuttableMesh_t& Mesh() { return m_mesh; }

private:
	cuttableMesh_t& m_mesh;
	void BuildRenderData(const bgfx::Memory*& vertBuf, const bgfx::Memory*& indexBuf);
	
	bgfx::DynamicVertexBufferHandle m_vertexBuf = BGFX_INVALID_HANDLE;
	bgfx::DynamicIndexBufferHandle m_indexBuf = BGFX_INVALID_HANDLE;
	int m_indexCount;
};

