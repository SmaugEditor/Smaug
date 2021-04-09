#include "worldeditor.h"
#include "raytest.h"

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

/*
CTriNode* CWorldEditor::CreateTri()
{
	CTriNode* node = new CTriNode();
	RegisterNode(node);
	return node;
}
*/


CWorldEditor& GetWorldEditor()
{
	static CWorldEditor s_worldEditor;
	return s_worldEditor;
}


CNode::CNode() : m_renderData(m_mesh)
{
}

void CNode::Init()
{
	/*
	m_sides = new nodeSide_t[m_sideCount];
	LinkSides();

	m_origin = glm::vec3(0, 0, 0);
	
	m_nodeHeight = 10;
	ConstructWalls();
	*/

	CalculateAABB();
}

void CNode::PreviewUpdate()
{
	m_renderData.RebuildRenderData();

}

void CNode::Update()
{
#if 0
	// Make sure our new position fits our constraints
	for (int i = 0; i < m_constrainedToCount; i++)
	{
		m_constrainedTo[i].Update();
	}
#endif

	

#if 0
	// Update our walls
	ConstructWalls();

	m_renderData.UpdateBufs();

	// Now that we've updated ourself, we have to update our constraints.
	for (int i = 0; i < m_constraining.size(); i++)
	{
		m_constraining[i]->Update();
		m_constraining[i]->m_childNode->Update();
	}

	CalculateAABB();

	// Since we've updated, we have to notify our parents's walls
	for (int i = 0; i < m_constrainedToCount; i++)
	{
		m_constrainedTo[i].m_parentNode->ConstructWalls();
		m_constrainedTo[i].m_parentNode->m_renderData.Update3DBufs();
	}
#endif
	
	UpdateThisOnly();

	// Update what we're cutting
	for (auto m : m_cutting)
	{
		m->UpdateThisOnly();
	}

}
void CNode::UpdateThisOnly()
{
	recenterMesh(m_mesh);
	CalculateAABB();

	for (auto pa : m_mesh.parts)
	{
		defineMeshPartFaces(*pa);
		triangluateMeshPartFaces(*pa, pa->fullFaces);
	}

	applyCuts(m_mesh);


	for (auto pa : m_mesh.parts)
	{
		triangluateMeshPartFaces(*pa, pa->cutFaces);
	}

	m_renderData.RebuildRenderData();
}


#if 0
static float FindMaxConstraintHeight(CNode* otherNode, nodeSide_t* otherSide, CNode* ourNode, nodeSide_t* ourSide)
{
	// Highest point on the other side's wall + how high the node is + how high the ceiling is. Absolute Max Y for the other side.
	float constraintMaxY = glm::max(otherSide->vertex1->origin.y, otherSide->vertex2->origin.y) + otherNode->m_origin.y + otherNode->m_nodeHeight;

	// Now that we have how tall the other side is, we have to make it relative to the lowest point on our side
	constraintMaxY -= glm::max(ourSide->vertex1->origin.y, ourSide->vertex2->origin.y) + ourNode->m_origin.y;

	return constraintMaxY;
}

// COMPARED ITEMS MUST HAVE THE SAME PARENT!
int SortSideSideConstraints(const void* a, const void* b)
{
	const CConstraint* arg1 = *(const CConstraint**)a;
	const CConstraint* arg2 = *(const CConstraint**)b;
	
	// This is probably slow...

	// If we're not comparing with the same parents, what are we even doing...
	glm::vec3 parentVert = arg1->m_parentSide->vertex1->origin + arg1->m_parentNode->m_origin;

	// Check our distance from the left side of our parent
	float arg1Dist = glm::length(parentVert - (arg1->m_childSide->vertex2->origin + arg1->m_childNode->m_origin));
	float arg2Dist = glm::length(parentVert - (arg2->m_childSide->vertex2->origin + arg2->m_childNode->m_origin));

	if (arg1Dist < arg2Dist) return -1;
	if (arg1Dist > arg2Dist) return 1;
	return 0;
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

			// If it's not a side->side constraint, continue
			if (constraint->m_childType != ConstraintPairType::SIDE || constraint->m_parentType != ConstraintPairType::SIDE)
				continue;

			float constraintMaxY = FindMaxConstraintHeight(constraint->m_childNode, constraint->m_childSide, constraint->m_parentNode, constraint->m_parentSide);

			if (constraintMaxY > maxY)
				maxY = constraintMaxY;
		}

		// Check what we're constraining
		for (int i = 0; i < m_constraining.size(); i++)
		{
			CConstraint* constraint = m_constraining[i];

			// If it's not a side->side constraint, continue
			if (constraint->m_childType != ConstraintPairType::SIDE || constraint->m_parentType != ConstraintPairType::SIDE)
				continue;

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


	// If we're constrained to something, we need to delete its walls
	for (int i = 0; i < m_constrainedToCount; i++)
	{
		CConstraint* cst = &m_constrainedTo[i];

		// If it's a side->side constraint, delete the walls
		if (cst->m_childType == ConstraintPairType::SIDE && cst->m_parentType == ConstraintPairType::SIDE)
		{
			cst->m_childSide->walls.clear();
		}
	}


	// Figure out what we're constraining and organize them from left to right
	std::vector<CConstraint*> constraints;

	// Check what we're constraining
	for (int i = 0; i < m_constraining.size(); i++)
	{
		CConstraint* cst = m_constraining[i];

		// If it's a side->side constraint, store it
		if (cst->m_childType == ConstraintPairType::SIDE && cst->m_parentType == ConstraintPairType::SIDE)
			constraints.push_back(cst);
	}

	// Sort the vector from left to right
	std::qsort(constraints.data(), constraints.size(), sizeof(CConstraint*), SortSideSideConstraints);


	// Loop our constraints and cut holes in our walls
	for (int i = 0; i < constraints.size(); i++)
	{
		CConstraint* cst = constraints[i];

		nodeSide_t* parentSide = cst->m_parentSide;
		nodeSide_t* childSide = cst->m_childSide;

		// If we somehow have no side due to having a parent, bail
		if (parentSide->walls.size() == 0)
			continue;

		// Get our right most wall
		nodeWall_t* lastWall = &parentSide->walls.back();
		
		nodeWall_t newWall;

		// Use our last's rightmost's right points for ours
		newWall.bottomPoints[1] = lastWall->bottomPoints[1];
		newWall.topPoints[1] = lastWall->topPoints[1];

		// Use our constraint's child's rightmost points as our new leftmost
		newWall.bottomPoints[0] = childSide->vertex1->origin + cst->m_childNode->m_origin - cst->m_parentNode->m_origin;
		newWall.topPoints[0] = newWall.bottomPoints[0];
		newWall.topPoints[0].y += m_nodeHeight; // Gotta make sure we have it high enough!

		// Store it
		parentSide->walls.push_back(newWall);

		// Use our constraint's child's leftmost points as our last's rightmost
		lastWall->bottomPoints[1] = childSide->vertex2->origin + cst->m_childNode->m_origin - cst->m_parentNode->m_origin;
		lastWall->topPoints[1] = lastWall->bottomPoints[1];
		lastWall->topPoints[1].y += m_nodeHeight; // Gotta make sure we have it high enough!

	}

}
#endif

bool CNode::IsPointInAABB(glm::vec3 point)
{
	return testPointInAABB(point, GetAbsAABB(), 1.25f);
}

aabb_t CNode::GetAbsAABB()
{
	aabb_t aabb = m_aabb;
	aabb.min += m_mesh.origin;
	aabb.max += m_mesh.origin;
	return aabb;
}

#if 0
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
#endif

void CNode::CalculateAABB()
{
	m_aabb = meshAABB(m_mesh);

	//m_aabbLength = glm::length(m_aabb.max - m_aabb.min);

	printf("AABB - Max:{%f, %f, %f} - Min:{%f, %f, %f}\n", m_aabb.max.x, m_aabb.max.y, m_aabb.max.z, m_aabb.min.x, m_aabb.min.y, m_aabb.min.z);
}

CQuadNode::CQuadNode() : CNode()
{
#if 0
	m_nodeType = NodeType::QUAD;
	m_sideCount = 4;
	m_vertexes = new nodeVertex_t[4];

	// Start at the left side and rotate clockwise
	m_vertexes[0].origin = glm::vec3(-1,  0, -1); // Bottom Left
	m_vertexes[1].origin = glm::vec3(-1,  0,  1); // Top Left
	m_vertexes[2].origin = glm::vec3( 1,  0,  1); // Top Right
	m_vertexes[3].origin = glm::vec3( 1,  0, -1); // Bottom Right
#endif
	glm::vec3 points[] = {

	{-1,-1,-1}, // bottom back left
	{ 1,-1,-1}, // bottom back right
	{ 1, 1,-1}, // top back right
	{-1, 1,-1}, // top back left

	{-1,-1, 1}, // bottom front left
	{ 1,-1, 1}, // bottom front right
	{ 1, 1, 1}, // top front right
	{-1, 1, 1}, // top front left

	};

	auto p = addMeshVerts(m_mesh, &points[0], 8);

	glm::vec3* front[] = { p[7], p[6], p[5], p[4] };
	addMeshFace(m_mesh, front, 4);

	glm::vec3* back[] = { p[0], p[1], p[2], p[3] };
	addMeshFace(m_mesh, &back[0], 4);

	glm::vec3* left[] = { p[3], p[7], p[4], p[0] };
	addMeshFace(m_mesh, &left[0], 4);

	glm::vec3* right[] = { p[2], p[1], p[5], p[6] };
	addMeshFace(m_mesh, &right[0], 4);

	glm::vec3* bottom[] = { p[4], p[5], p[1], p[0] };
	addMeshFace(m_mesh, &bottom[0], 4);

	glm::vec3* top[] = { p[3], p[2], p[6], p[7] };
	addMeshFace(m_mesh, &top[0], 4);
	Init();
}

/*
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
*/
#if 0
void CConstraint::Update()
{
	if(m_parentType == ConstraintPairType::NONE || m_childType == ConstraintPairType::NONE)
		return; // No constraint. Let's dip.

	if(m_parentType == ConstraintPairType::SIDE && m_childType == ConstraintPairType::VERTEX)
	{
		SideVertConstrain();
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

		// HACK HACK
		// Temp set our child to be the verts and constrain them to the parent
		nodeSide_t* side = m_childSide;

		m_childVertex = side->vertex1;
		SideVertConstrain();

		m_childVertex = side->vertex2;
		SideVertConstrain();

		m_childSide = side;

	}



}

void CConstraint::SwapParentAndChild()
{
	ConstraintPairType tempParentType = m_parentType;
	CNode* tempParentNode = m_parentNode;
	void* tempParent = m_parent;
	
	m_parentType = m_childType;
	m_parentNode = m_childNode;
	m_parent = m_child;

	m_childType = tempParentType;
	m_childNode = tempParentNode;
	m_child = tempParent;
}

void CConstraint::SideVertConstrain()
{
	// I'm really tired while writing this, so this is probably going to need some revamping at some point.
	// But this is how it works. We can solve the vertex to the line using two ways: solving to the line using the vert's X, or solving using the Z.
	// This code solves for both, uses the one with least distance to the line as the solution for snapping, and takes that position's Y for its own.


	// If we don't add the home node's origin, all of our math will be totally broken since the verts use local positioning
	glm::vec3 newOrigin = m_childVertex->origin + m_childNode->m_origin;

	glm::vec3 parentVert1 = m_parentSide->vertex1->origin + m_parentNode->m_origin;
	glm::vec3 parentVert2 = m_parentSide->vertex2->origin + m_parentNode->m_origin;


	glm::vec3 line = parentVert2 - parentVert1;

	// Straight line checking
	if (line.x == 0)
	{
		// Line is straight. Take a shortcut
		newOrigin.x = parentVert1.x;
	}
	else if (line.z == 0)
	{
		// Line is straight. Take a shortcut
		newOrigin.z = parentVert1.z;
	}
	else
	{
		// The line is not straight. Solve for the new position using the slope

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
		//slope = line.y / line.x;
		//newOrigin.y = (newOrigin.x - parentVert1.x) * slope + parentVert1.y;

	}

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
}
#endif