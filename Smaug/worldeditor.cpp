#include "worldeditor.h"
#include "raytest.h"
#include "tessellate.h"
#include "slice.h"
#include "sassert.h"

#include <glm/geometric.hpp>
#include <glm/common.hpp>



CNodeRef::CNodeRef() : m_targetId(INVALID_NODE_ID) {}
CNodeRef::CNodeRef(nodeId_t id) : m_targetId(id) {}
CNodeRef::CNodeRef(CNode* node) : m_targetId(node->NodeID()) {}
bool CNodeRef::IsValid() { return GetWorldEditor().GetNode(m_targetId) != nullptr; }
CNode* CNodeRef::operator->() const { return GetWorldEditor().GetNode(m_targetId); }
CNodeRef::operator CNode* () const { return GetWorldEditor().GetNode(m_targetId); }
void CNodeRef::operator=(const CNodeRef& ref) { m_targetId = ref.m_targetId;}


CWorldEditor::CWorldEditor()
{
	m_currentNodeId = 0;
}

void CWorldEditor::RegisterNode(CNode* node)
{
	m_nodes.emplace(m_currentNodeId, node);

	node->m_id = m_currentNodeId;
	m_currentNodeId++;

	// If this ever happens, it'll be awful.
	SASSERT(m_currentNodeId != INVALID_NODE_ID);
}

bool CWorldEditor::AssignID(CNode* node, nodeId_t id)
{
	// If we're one ahead of the cur, increment it. It'll make life easier
	if (id == m_currentNodeId + 1)
		m_currentNodeId++;
	
	// We're assigning an arbitrary id. Check if it's not in use.
	if (m_nodes.contains(id))
	{
		// Failure!
		return false;
	}

	// Is this free?
	nodeId_t oldId = node->m_id;
	if (oldId != INVALID_NODE_ID)
	{
		if (m_nodes.contains(oldId))
		{
			// Remove the existing ref
			m_nodes.erase(oldId);

		}
	}

	m_nodes.emplace(id, node);
	node->m_id = id;
}

CQuadNode* CWorldEditor::CreateQuad()
{
	CQuadNode* node = new CQuadNode();
	RegisterNode(node);
	return node;
}

CNode* CWorldEditor::GetNode(nodeId_t id)
{
	if(!m_nodes.contains(id))
		return nullptr;
	return m_nodes[id];
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


CNode::CNode() : m_renderData(m_mesh), m_id(MAX_NODE_ID), m_visible(true)
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

	for(auto p : m_mesh.parts)
		defineMeshPartFaces(*p);
	CalculateAABB();
}

void CNode::PreviewUpdate()
{
	m_renderData.RebuildRenderData();

}

void CNode::Update()
{
	
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
		triangluateMeshPartFaces(*pa, pa->fullFaces);
	}

	//applyCuts(m_mesh);


	for (auto pa : m_mesh.parts)
	{
		triangluateMeshPartFaces(*pa, pa->cutFaces);
	}

	m_renderData.RebuildRenderData();
}


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


void CNode::CalculateAABB()
{
	m_aabb = meshAABB(m_mesh);

	//m_aabbLength = glm::length(m_aabb.max - m_aabb.min);

	printf("AABB - Max:{%f, %f, %f} - Min:{%f, %f, %f}\n", m_aabb.max.x, m_aabb.max.y, m_aabb.max.z, m_aabb.min.x, m_aabb.min.y, m_aabb.min.z);
}

CQuadNode::CQuadNode() : CNode()
{
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

