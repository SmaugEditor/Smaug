#pragma once

#include "mesh.h"
#include "modelmanager.h"
#include "shadermanager.h"

#include <bgfx/bgfx.h>


class CMeshRenderer
{
public:
	CMeshRenderer(cuttableMesh_t& mesh);
	~CMeshRenderer();

	void RebuildRenderData();
	
	// This does not bgfx::submit!!
	void Render(bgfx::ViewId viewID, bgfx::ProgramHandle shader);
	void Render(CModelTransform trnsfm, bgfx::ViewId viewID, bgfx::ProgramHandle shader);
	
	cuttableMesh_t& Mesh() { return m_mesh; }

private:
	cuttableMesh_t& m_mesh;
	void BuildRenderData(const bgfx::Memory*& vertBuf, const bgfx::Memory*& indexBuf);

	
	bool m_empty;

	struct partData_t
	{
		std::vector<meshPart_t*> parts;
		int indexStart;
		int indexCount;
	};
	std::map<texture_t, partData_t> m_partRenderData;
	bgfx::DynamicVertexBufferHandle m_vertexBuf = BGFX_INVALID_HANDLE;
	bgfx::DynamicIndexBufferHandle m_indexBuf = BGFX_INVALID_HANDLE;
};

