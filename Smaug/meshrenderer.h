#pragma once

#include "mesh.h"
#include "modelmanager.h"

#include <bgfx/bgfx.h>

class CMeshRenderer
{
public:
	CMeshRenderer(cutttableShape_t& shape);
	~CMeshRenderer();

	void RebuildRenderData();
	void Render(CModelTransform& trnsfm);

private:
	cutttableShape_t& m_shape;
	void BuildRenderData(const bgfx::Memory*& vertBuf, const bgfx::Memory*& indexBuf);
	
	bgfx::DynamicVertexBufferHandle m_vertexBuf;
	bgfx::DynamicIndexBufferHandle m_indexBuf;
	int m_indexCount;
};

CMeshRenderer& TestMesh();