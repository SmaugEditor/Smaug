#include "worldrenderer.h"
#include "worldeditor.h"

#include <bgfx/bgfx.h>
#include <glm/gtc/matrix_transform.hpp>


bgfx::VertexLayout CreateVertexLayout()
{
	bgfx::VertexLayout layout;
	layout.begin()
		.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		.skip(sizeof(nodeVertex_t) - sizeof(glm::vec3)) // This value depends on nodeSide_t's layout. Make sure it's correct!
		.end();
	return layout;
}

static bgfx::VertexLayout g_vertexLayout;

static const uint16_t g_quadTriIndexList[] = { 0, 1, 2, 2, 3, 0 };
static bgfx::IndexBufferHandle g_quadTriIndexBuffer;
static const uint16_t g_triTriIndexList[] = { 0, 1, 2 };
static bgfx::IndexBufferHandle g_triTriIndexBuffer;


void CWorldRenderer::Init()
{
	g_vertexLayout = CreateVertexLayout();
	g_quadTriIndexBuffer = bgfx::createIndexBuffer(bgfx::makeRef(g_quadTriIndexList, sizeof(g_quadTriIndexList)));
	g_triTriIndexBuffer = bgfx::createIndexBuffer(bgfx::makeRef(g_triTriIndexList, sizeof(g_triTriIndexList)));
}

void CWorldRenderer::Draw(bgfx::ViewId viewId, bgfx::ProgramHandle shaderProgram)
{
	CWorldEditor& world = GetWorldEditor();
	for (int i = 0; i < world.m_nodes.size(); i++)
	{
		world.m_nodes[i]->m_renderData.Draw();
		bgfx::submit(viewId, shaderProgram);
	}
}

void CNodeRenderData::Setup(CNode* node)
{
	m_parentNode = node;

	m_vertexBuf = bgfx::createDynamicVertexBuffer(bgfx::makeRef(node->m_vertexes, sizeof(nodeVertex_t) * node->m_sideCount), g_vertexLayout);

	switch (node->m_nodeType)
	{
	case NodeType::QUAD:
		m_indexBuf = g_quadTriIndexBuffer;
		break;
	case NodeType::TRI:
		m_indexBuf = g_triTriIndexBuffer;
	default:
		break;
	}
}

void CNodeRenderData::UpdateVertexBuf()
{
	bgfx::update(m_vertexBuf, 0, bgfx::makeRef(m_parentNode->m_vertexes, sizeof(nodeVertex_t) * m_parentNode->m_sideCount));
}

void CNodeRenderData::Draw()
{

	glm::mat4 mtx = glm::identity<glm::mat4>();
	mtx = glm::translate(mtx, m_parentNode->m_origin);

	bgfx::setTransform(&mtx[0][0]);
	bgfx::setVertexBuffer(0, m_vertexBuf);
	bgfx::setIndexBuffer(m_indexBuf);
	bgfx::setState(BGFX_STATE_DEFAULT);
	// This does not bgfx::submit!!
}

void CNodeRenderData::Shutdown()
{
}


CWorldRenderer& GetWorldRenderer()
{
	static CWorldRenderer worldRenderer;
	return worldRenderer;
}
