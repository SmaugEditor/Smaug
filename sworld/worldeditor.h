#pragma once
#include "mesh.h"

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <vector>
#include <unordered_map>
#include <unordered_set>

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

	bool IsValid() const;
	nodeId_t ID() const { return m_targetId; }
	CNode* Node() const;

	CNode* operator->() const;
	void operator=(const CNodeRef& ref);
	friend bool operator==(const CNodeRef& p1, const CNodeRef& p2) { return p1.m_targetId == p2.m_targetId; }
	friend bool operator==(const CNodeRef* p1, const CNodeRef& p2) { return p1->m_targetId == p2.m_targetId; }
	friend bool operator==(const CNodeRef& p1, const CNodeRef* p2) { return p1.m_targetId == p2->m_targetId; }

private:
	nodeId_t m_targetId;

};


class CNodeMeshPartRef
{
public:
	CNodeMeshPartRef();
	CNodeMeshPartRef(meshPart_t* part, CNodeRef node);

	const bool IsValid();
	meshId_t ID() { return m_partId; }

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

	const bool IsValid();
	meshId_t ID() { return m_heId; }

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
	meshId_t ID() { return m_vertId; }

	vertex_t* operator->() const;
	operator vertex_t* () const;
	void operator=(const CNodeVertexRef& ref);

private:
	CNodeMeshPartRef m_part;

	meshId_t m_vertId;

};


namespace std
{
	template <>
	struct hash<CNodeRef>
	{
		inline size_t operator()(const CNodeRef& v) const
		{
			std::hash<nodeId_t> hasher;
			return hasher(v.ID());
		}
	};
}


// Nodes
class INodeRenderer
{
public:
	virtual void SetOwner(CNode* node) = 0;
	virtual void Rebuild() = 0;
	virtual void Destroy() = 0;
};
void SupplyNodeRenderer(INodeRenderer*(*func)());


class CNode
{
public:
	CNode();

	void Init();

	void MarkDirty() { m_dirty = 2; }
	void MarkDirtyPreview() { if(m_dirty >= 0 && m_dirty < 2) m_dirty = 1; }
	bool IsNewBorn() { return m_dirty == -1; }

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

	void ConnectTo(CNodeRef node);
	void DisconnectFrom(CNodeRef node);

protected:
	// Nodes should not be manually deleted!
	~CNode() { if (m_renderData) m_renderData->Destroy(); }

	void PreviewUpdate();
	void PreviewUpdateThisOnly();
	void Update();
	void UpdateThisOnly();


	//void LinkSides();
	void CalculateAABB();
public:

	cuttableMesh_t m_mesh;
	INodeRenderer* m_renderData;
	
	std::unordered_set<CNodeRef> m_cutting;
	std::unordered_set<CNodeRef> m_cutters;

protected:
	aabb_t m_aabb;
	bool m_visible;
	nodeId_t m_id = INVALID_NODE_ID;

	// Does this node need updating?
	// -1 : Just born, 0 : Content, 1 : Preview, 2 : Full update
	int m_dirty = 0;

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

	void Update();
	void Clear();

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
