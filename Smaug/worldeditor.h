#pragma once
#include "worldrenderer.h"
#include "mesh.h"
#include "meshrenderer.h"

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <vector>
#include <unordered_map>

typedef uint16_t nodeId_t;
#define MAX_NODE_ID UINT16_MAX
#define INVALID_NODE_ID MAX_NODE_ID

// Safe reference to a node
class CNode;
class CNodeRef
{
public:
	CNodeRef();
	CNodeRef(nodeId_t id);
	CNodeRef(CNode* node);

	bool IsValid();
	CNode* operator->() const;
	operator CNode* () const;
	void operator=(const CNodeRef& ref);

private:
	nodeId_t m_targetId;

};

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
	nodeId_t NodeID() { return m_id; }
	CNodeRef Ref() { return { m_id }; }

protected:
	//void LinkSides();
	void CalculateAABB();
public:

	cuttableMesh_t m_mesh;
	CMeshRenderer m_renderData;
	
	std::vector<CNodeRef> m_cutting;
	std::vector<CNodeRef> m_cutters;

protected:
	aabb_t m_aabb;
	bool m_visible;
	nodeId_t m_id;

	friend class CWorldEditor;
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
	
	// Returns true on success
	bool AssignID(CNode* node, nodeId_t id);

	CQuadNode* CreateQuad();
	//CTriNode* CreateTri();
	
	CNode* GetNode(nodeId_t id);

//private:
	std::unordered_map<nodeId_t, CNode*> m_nodes;

	// This lets us create unique ids
	// Ideally, we should never decrement this, but if we never do, we'll run out of space due to edit history...
	// TODO: somehow cull out edit history or make something better!
	nodeId_t m_currentNodeId;

};

CWorldEditor& GetWorldEditor();
