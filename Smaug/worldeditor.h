#pragma once
#include "worldrenderer.h"
#include "mesh.h"
#include "meshrenderer.h"

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <vector>


class CNode
{
public:
	CNode();

	void Init();
	void PreviewUpdate();
	void Update();
	void UpdateThisOnly();

	//void ConstructWalls();
	bool IsPointInAABB(glm::vec3 point);

	// Absolute to world
	aabb_t GetAbsAABB();
	// Local to node's origin
	aabb_t GetLocalAABB() { return m_aabb; }

	glm::vec3 Origin() { return m_mesh.origin; }

	void SetVisible(bool visible) { m_visible = visible; }
	bool IsVisible() { return m_visible; }

protected:
	//void LinkSides();
	void CalculateAABB();
public:

	cuttableMesh_t m_mesh;
	CMeshRenderer m_renderData;
	
	std::vector<CNode*> m_cutting;

protected:
	aabb_t m_aabb;
	bool m_visible;
};

// I've been told hammer only likes triangles and quads. How sad!

class CQuadNode : public CNode
{
public:
	CQuadNode();

};

/*
class CTriNode : public CNode
{
public:
	CTriNode();

};
*/


class CWorldEditor
{
public:
	CWorldEditor();

	void RegisterNode(CNode* node);
	CQuadNode* CreateQuad();
	//CTriNode* CreateTri();

	std::vector<CNode*> m_nodes;


};

CWorldEditor& GetWorldEditor();
