#include "worldeditor.h"
#include "raytest.h"
#include "tessellate.h"
#include "slice.h"

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

	applyCuts(m_mesh);


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
