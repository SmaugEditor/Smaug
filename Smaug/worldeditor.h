#pragma once
#include "worldrenderer.h"
#include "mesh.h"
#include "meshrenderer.h"

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <vector>
#include <unordered_map>

// World References
//  - These function as semi-safe references to objects in the world, 
//    these allow the edit history to function without imploding.
//  - If you need more than 65k nodes with 65k parts and 65k hes per part, please let me know

typedef uint16_t nodeId_t;
#define MAX_NODE_ID UINT16_MAX
#define INVALID_NODE_ID MAX_NODE_ID


typedef uint16_t meshId_t;
#define MAX_MESH_ID UINT16_MAX
#define INVALID_MESH_ID MAX_MESH_ID

// Safe reference to a node
class CNode;
class CNodeRef
{
public:
	CNodeRef();
	CNodeRef(nodeId_t id);
	CNodeRef(CNode* node);

	bool IsValid();
	nodeId_t ID() { return m_targetId; }

	CNode* operator->() const;
	operator CNode* () const;
	void operator=(const CNodeRef& ref);

private:
	nodeId_t m_targetId;

};

class CNodeMeshPartRef
{
public:
	CNodeMeshPartRef();
	CNodeMeshPartRef(meshPart_t* part, CNodeRef node);

	bool IsValid();

	meshPart_t* operator->() const;
	operator meshPart_t* () const;
	void operator=(const CNodeMeshPartRef& ref);

private:
	CNodeRef m_node;

	meshId_t m_partId;
};

class CNodeHalfEdgeRef
{
public:
	CNodeHalfEdgeRef();
	CNodeHalfEdgeRef(halfEdge_t* he, CNodeRef node);

	bool IsValid();

	halfEdge_t* operator->() const;
	operator halfEdge_t* () const;
	void operator=(const CNodeHalfEdgeRef& ref);

private:
	CNodeMeshPartRef m_part;

	meshId_t m_heId;

};


class CNodeVertexRef
{
public:
	CNodeVertexRef();
	CNodeVertexRef(vertex_t* vertex, CNodeRef node);

	bool IsValid();

	vertex_t* operator->() const;
	operator vertex_t* () const;
	void operator=(const CNodeVertexRef& ref);

private:
	CNodeMeshPartRef m_part;

	meshId_t m_vertId;

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

	// Nodes should not be manually deleted!
	~CNode() {}

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
	nodeId_t m_id = INVALID_NODE_ID;

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

	CNode* GetNode(nodeId_t id);
	void RegisterNode(CNode* node);
	// Returns true on success
	bool AssignID(CNode* node, nodeId_t id);
	void DeleteNode(CNode* node);

	CQuadNode* CreateQuad();
	//CTriNode* CreateTri();
	

//private:
	std::unordered_map<nodeId_t, CNode*> m_nodes;

	// This lets us create unique ids
	// Ideally, we should never decrement this, but if we never do, we'll run out of space due to edit history...
	// TODO: somehow cull out edit history or make something better!
	nodeId_t m_currentNodeId;

};

CWorldEditor& GetWorldEditor();
