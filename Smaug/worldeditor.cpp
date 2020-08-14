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

	// Now that we've updated ourself, we have to update our vertexes' constraints.
	for (int i = 0; i < m_sideCount; i++)
	{
		nodeSide_t& side = m_sides[i];
		for (int j = 0; j < side.children.size(); j++)
		{
			side.children[j]->Constrain();
			side.children[j]->homeNode->Update(); // Update its verts
		}
	}


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

	for (int i = 0; i < m_sideCount; i++)
	{
		m_vertexes[i].homeNode = this;
	}
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

void nodeVertex_t::Constrain()
{
	switch (constraint)
	{
	case Constraint::NONE:
		return; // No constraint. Let's dip.
	case Constraint::SIDE:
	{
		// I'm really tired while writing this, so this is probably going to need some revamping at some point.
		// But this is how it works. We can solve the vertex to the line using two ways: solving to the line using the vert's X, or solving using the Z.
		// This code solves for both, uses the one with least distance to the line as the solution for snapping, and takes that position's Y for its own.


		// If we don't add the home node's origin, all of our math will be totally broken since the verts use local positioning
		glm::vec3 newOrigin = origin + homeNode->m_origin;

		glm::vec3 parentVert1 = parentSide->vertex1->origin + parentSide->vertex1->homeNode->m_origin;
		glm::vec3 parentVert2 = parentSide->vertex2->origin + parentSide->vertex2->homeNode->m_origin;


		glm::vec3 line = parentVert2 - parentVert1;

		// Solving for Z, keeping X
		float slope = line.z / line.x;
		float zDelta = (newOrigin.x - parentVert1.x) * slope + parentVert1.z - newOrigin.z;

		// Solving for X, keeping Z
		slope = line.x / line.z;
		float xDelta = (newOrigin.z - parentVert1.z) * slope + parentVert1.x - newOrigin.x;

		if (abs(xDelta) < abs(zDelta))
		{
			// Shortest path to snap is X
			newOrigin.x += xDelta;
			printf("solved for X\n");
		}
		else
		{
			// Shortest path to snap is Z
			newOrigin.z += zDelta;
			printf("solved for Z\n");
		}

		// Find the y for our new X Z
		slope = line.y / line.x;
		//newOrigin.y = (newOrigin.x - parentVert1.x) * slope + parentVert1.y;

		printf("origin - (%f, %f)\tnew - (%f, %f)\n", origin.x, origin.z, newOrigin.x, newOrigin.z);
		
		// Since our origin is local the the home origin, we have to remove it 
		origin = newOrigin - homeNode->m_origin;
		return;
	}
	default:
		break;
	}
}
