#pragma once

#include "editorinterface.h"
#include "modelmanager.h"
#include "shadermanager.h"

#include <bgfx/bgfx.h>

class CNode;
struct cuttableMesh_t;
struct meshPart_t;
class CMeshRenderer : public INodeRenderer
{
public:
	CMeshRenderer() {};

	virtual void SetOwner(CNode* node);
	virtual void Destroy();
	virtual void Rebuild();
	
	// This does not bgfx::submit!!
	void Draw(bgfx::ViewId viewID, bgfx::ProgramHandle shader);
	void Draw(CTransform trnsfm, bgfx::ViewId viewID, bgfx::ProgramHandle shader);
	void Draw() {};

	bool Empty();
private:
	CNode* m_node;
	cuttableMesh_t* m_mesh;
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

