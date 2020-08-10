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
	m_renderData.UpdateVertexBuf();
}

void CNode::LinkSides()
{
	// Link up the sides
	for (int i = 0; i < m_sideCount - 1; i++)
	{
		m_sides[i].point2 = &m_sides[i + 1].point1;
	}
	m_sides[m_sideCount - 1].point2 = &m_sides[0].point1;
}

CQuadNode::CQuadNode()
{
	m_nodeType = NodeType::QUAD;

	m_sideCount = 4;
	m_sides = new nodeSide_t[m_sideCount];

	// Start at the left side
	m_sides[0].point1 = glm::vec3(-1,  0, -1); // Bottom Left
	m_sides[1].point1 = glm::vec3(-1,  0,  1); // Top Left
	m_sides[2].point1 = glm::vec3( 1,  0,  1); // Top Right
	m_sides[3].point1 = glm::vec3( 1,  0, -1); // Bottom Right

	LinkSides();

	m_origin = glm::vec3(0, 0, 0);

	m_renderData.Setup(this);
}

CTriNode::CTriNode()
{
	m_nodeType = NodeType::TRI;

	m_sideCount = 3;
	m_sides = new nodeSide_t[m_sideCount];

	// Start at the left side
	m_sides[0].point1 = glm::vec3( 0,  0, -1); // Bottom Left
	m_sides[1].point1 = glm::vec3(-1,  0,  1); // Top Left
	m_sides[2].point1 = glm::vec3( 1,  0,  1); // Top Right

	LinkSides();

	m_origin = glm::vec3(0, 0, 0);

	m_renderData.Setup(this);
}
