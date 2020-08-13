#include "worldeditor.h"


CWorldEditor::CWorldEditor()
{

}

void CWorldEditor::RegisterNode(CNode* node)
{
	m_nodes.push_back(node);
}

CQuadNode* CWorldEditor::CreateQuad()
{
	CQuadNode* node = new CQuadNode();
	RegisterNode(node);
	return node;
}

CTriNode* CWorldEditor::CreateTri()
{
	CTriNode* node = new CTriNode();
	RegisterNode(node);
	return node;
}


CWorldEditor& GetWorldEditor()
{
	static CWorldEditor s_worldEditor;
	return s_worldEditor;
}


void CNode::Update()
{
	// Recenter the origin
	// Not sure how needed this is, but it keeps me sane
	//glm::vec3 startOrigin = m_origin;

	glm::vec3 averageOrigin = glm::vec3(0,0,0);
	for (int i = 0; i < m_sideCount; i++)
	{
		averageOrigin += m_sides[i].vertex1->origin;
	}
	averageOrigin /= m_sideCount;

	// Shift the vertexes
	m_origin += averageOrigin;
	for (int i = 0; i < m_sideCount; i++)
	{
		m_sides[i].vertex1->origin -= averageOrigin;
	}

	m_renderData.UpdateVertexBuf();
}

void CNode::LinkSides()
{
	// Link up the sides
	for (int i = 0; i < m_sideCount - 1; i++)
	{
		m_sides[i].vertex1 = &m_vertexes[i];
		m_sides[i].vertex2 = &m_vertexes[i + 1];
	}
	m_sides[m_sideCount - 1].vertex1 = &m_vertexes[m_sideCount - 1];
	m_sides[m_sideCount - 1].vertex2 = &m_vertexes[0];
}

CQuadNode::CQuadNode()
{
	m_nodeType = NodeType::QUAD;
	m_sideCount = 4;
	m_vertexes = new nodeVertex_t[4];

	// Start at the left side and rotate clockwise
	m_vertexes[0].origin = glm::vec3(-1,  0, -1); // Bottom Left
	m_vertexes[1].origin = glm::vec3(-1,  0,  1); // Top Left
	m_vertexes[2].origin = glm::vec3( 1,  0,  1); // Top Right
	m_vertexes[3].origin = glm::vec3( 1,  0, -1); // Bottom Right

	m_sides = new nodeSide_t[m_sideCount];
	LinkSides();

	m_origin = glm::vec3(0, 0, 0);

	m_renderData.Setup(this);
}

CTriNode::CTriNode()
{
	m_nodeType = NodeType::TRI;

	m_sideCount = 3;
	m_vertexes = new nodeVertex_t[3];

	// Start at the left side
	m_vertexes[0].origin = glm::vec3( 0,  0, -1); // Bottom Left
	m_vertexes[1].origin = glm::vec3(-1,  0,  1); // Top Left
	m_vertexes[2].origin = glm::vec3( 1,  0,  1); // Top Right

	m_sides = new nodeSide_t[m_sideCount];
	LinkSides();

	m_origin = glm::vec3(0, 0, 0);

	m_renderData.Setup(this);
}
