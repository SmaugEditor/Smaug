#pragma once

#include <bgfx/bgfx.h>
#include "shadermanager.h"
#include <glm/vec3.hpp>

class CWorldRenderer
{
public:
	void Init();
	void Draw2D(bgfx::ViewId viewId, Shader shader);
	void Draw3D(bgfx::ViewId viewId, Shader shader);
	
private:
	bgfx::UniformHandle m_colorUniform;
};

/*
class CNode;

struct CNodeRenderData
{
	void Setup(CNode* node);
	void UpdateBufs();
	void Update3DBufs();
	void Update2DBufs();
	void GenerateWallBufs(const bgfx::Memory*& vertBuf, const bgfx::Memory*& indexBuf);

	void Draw2D();
	void Draw2D(glm::vec3 origin);

	void Draw3D();
	void Draw3D(glm::vec3 origin);

	void Shutdown();


	bgfx::DynamicVertexBufferHandle m_vertexBuf2D;
	bgfx::IndexBufferHandle m_indexBuf2D;

	bgfx::DynamicVertexBufferHandle m_vertexBuf3D;
	bgfx::DynamicIndexBufferHandle m_indexBuf3D;
	int m_indexCount3D;

	CNode* m_parentNode;
};

*/

CWorldRenderer& GetWorldRenderer();