#include "worldeditor.h"
#include <glm/geometric.hpp>
#include <glm/common.hpp>

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


void CNode::Init()
{

	m_sides = new nodeSide_t[m_sideCount];
	LinkSides();

	m_origin = glm::vec3(0, 0, 0);

	ConstructWalls();

	CalculateAABB();

	m_renderData.Setup(this);
}

void CNode::Update()
{

	// Make sure our new position fits our constraints
	for (int i = 0; i < m_constrainedToCount; i++)
	{
		m_constrainedTo[i].Update();
	}



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

	// Update our walls
	ConstructWalls();

	m_renderData.UpdateVertexBuf();

	// Now that we've updated ourself, we have to update our constraints.
	for (int i = 0; i < m_constraining.size(); i++)
	{
		m_constraining[i]->Update();
		m_constraining[i]->m_childNode->Update();
	}

	CalculateAABB();
}

static float FindMaxConstraintHeight(CNode* otherNode, nodeSide_t* otherSide, CNode* ourNode, nodeSide_t* ourSide)
{
	// Highest point on the other side's wall + how high the node is + how high the ceiling is. Absolute Max Y for the other side.
	float constraintMaxY = glm::max(otherSide->vertex1->origin.y, otherSide->vertex2->origin.y) + otherNode->m_origin.y + otherNode->m_nodeHeight;

	// Now that we have how tall the other side is, we have to make it relative to the lowest point on our side
	constraintMaxY -= glm::max(ourSide->vertex1->origin.y, ourSide->vertex2->origin.y) + ourNode->m_origin.y;

	return constraintMaxY;
}

void CNode::ConstructWalls()
{
	// We start with discarding our walls and then creating new ones for each side. We then have to cut out of the walls where side->side constraints exist
	
	// Figure out how tall our node needs to be based on our connected nodes
	// Sometimes the user might have a height in mind already.
	if (m_useCalculatedNodeHeight)
	{
		float maxY = FLT_MIN;

		// Check what we're constrained to
		for (int i = 0; i < m_constrainedToCount; i++)
		{
			CConstraint* constraint = &m_constrainedTo[i];
			
			float constraintMaxY = FindMaxConstraintHeight(constraint->m_parentNode, constraint->m_parentSide, constraint->m_childNode, constraint->m_childSide);

			if (constraintMaxY > maxY)
				maxY = constraintMaxY;
		}

		// Check what we're constraining
		for (int i = 0; i < m_constraining.size(); i++)
		{
			CConstraint* constraint = m_constraining[i];

			float constraintMaxY = FindMaxConstraintHeight(constraint->m_parentNode, constraint->m_parentSide, constraint->m_childNode, constraint->m_childSide);

			if (constraintMaxY > maxY)
				maxY = constraintMaxY;
		}

		// Only change the height if we calculated a new one.
		if (maxY > FLT_MIN)
		{
			m_nodeHeight = maxY;
		}

	}



	// Create the walls
	for (int i = 0; i < m_sideCount; i++)
	{
		// Out with the old
		m_sides[i].walls.clear();

		nodeWall_t wall;

		wall.bottomPoints[0] = m_sides[i].vertex1->origin;
		wall.bottomPoints[1] = m_sides[i].vertex2->origin;

		wall.topPoints[0] = wall.bottomPoints[0];
		wall.topPoints[1] = wall.bottomPoints[1];

		// Push the top points up
		wall.topPoints[0].y += m_nodeHeight;
		wall.topPoints[1].y += m_nodeHeight;

		// In with the new
		m_sides[i].walls.push_back(wall);
	}


	// Cut the walls owned 

	// Order from Left to right
	// Punch holes in order
	// Easy peasy

	// for Constraining
	// for ConstrainedBy
}

bool CNode::IsPointInAABB(glm::vec2 point)
{
	point.x -= m_origin.x;
	point.y -= m_origin.z;

	return point.x <= m_aabb.topRight.x && point.y <= m_aabb.topRight.y && point.x >= m_aabb.bottomLeft.x && point.y >= m_aabb.bottomLeft.y;
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

void CNode::CalculateAABB()
{
	glm::vec2 topRight = { FLT_MIN, FLT_MIN };
	glm::vec2 bottomLeft = { FLT_MAX, FLT_MAX };
	for (int i = 0; i < m_sideCount; i++)
	{
		if (m_vertexes[i].origin.x > topRight.x)
			topRight.x = m_vertexes[i].origin.x;
		if (m_vertexes[i].origin.z > topRight.y)
			topRight.y = m_vertexes[i].origin.z;

		if (m_vertexes[i].origin.x < bottomLeft.x)
			bottomLeft.x = m_vertexes[i].origin.x;
		if(m_vertexes[i].origin.z < bottomLeft.y)
			bottomLeft.y = m_vertexes[i].origin.z;
	}
	m_aabb.bottomLeft = bottomLeft;
	m_aabb.topRight = topRight;

	m_aabbLength = glm::length(topRight - bottomLeft);

	printf("AABB - TR:{%f, %f} - BL:{%f, %f}\n", m_aabb.topRight.x, m_aabb.topRight.y, m_aabb.bottomLeft.x, m_aabb.bottomLeft.y);
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

	Init();
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

	Init();
}

void CConstraint::Update()
{
	if(m_parentType == ConstraintPairType::NONE || m_childType == ConstraintPairType::NONE)
		return; // No constraint. Let's dip.

	if(m_parentType == ConstraintPairType::SIDE && m_childType == ConstraintPairType::VERTEX)
	{
		// I'm really tired while writing this, so this is probably going to need some revamping at some point.
		// But this is how it works. We can solve the vertex to the line using two ways: solving to the line using the vert's X, or solving using the Z.
		// This code solves for both, uses the one with least distance to the line as the solution for snapping, and takes that position's Y for its own.


		// If we don't add the home node's origin, all of our math will be totally broken since the verts use local positioning
		glm::vec3 newOrigin = m_childVertex->origin + m_childNode->m_origin;

		glm::vec3 parentVert1 = m_parentSide->vertex1->origin + m_parentNode->m_origin;
		glm::vec3 parentVert2 = m_parentSide->vertex2->origin + m_parentNode->m_origin;


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

		// Lock the vertex to the side
		// This is probably terrible. Revisit when not tired.

		glm::vec3 midpoint = (parentVert1 + parentVert2) / 2.0f;
		float linePercent = glm::length(newOrigin - midpoint) / glm::length(parentVert2 - midpoint);
		
		if (linePercent > 1.0f)
		{
			// Out of line. Find nearest and lock to it.
			// Not how I'd want to do this...

			float lenTo1 = glm::length(parentVert1 - newOrigin);
			float lenTo2 = glm::length(parentVert2 - newOrigin);

			if (lenTo1 < lenTo2)
			{
				newOrigin = parentVert1;
			}
			else
			{
				newOrigin = parentVert2;
			}
		}

		printf("origin - (%f, %f)\tnew - (%f, %f)\n", m_childVertex->origin.x, m_childVertex->origin.z, newOrigin.x, newOrigin.z);
		
		// Since our origin is local the the home origin, we have to remove it 
		m_childVertex->origin = newOrigin - m_childNode->m_origin;
		return;
	}

	if (m_parentType == ConstraintPairType::VERTEX && m_childType == ConstraintPairType::VERTEX)
	{
		m_childVertex->origin = m_parentVertex->origin + m_parentNode->m_origin - m_childNode->m_origin;

		return;
	}


	if (m_parentType == ConstraintPairType::SIDE && m_childType == ConstraintPairType::SIDE)
	{
		// We have to solve the child side's verts to have the same slope and y-intercept as the parent's side while maintaining the same dist between the verts



	}



}
