#pragma once
#include <glm/vec3.hpp>
#include <vector>
#include "worldrenderer.h"

enum class Constraint
{
	NONE,
	VERTEX_SIDE, // Vertex to side constraint
	VERTEX_VERTEX,

};

struct nodeSide_t;
class CNode;

// Do not change this struct without editing CreateVertexLayout in worldrenderer
struct nodeVertex_t
{
	glm::vec3 origin;
	Constraint constraint;
	union
	{
		nodeVertex_t* parentVertex;
		nodeSide_t* parentSide;
	};


	CNode* homeNode;

	// I'm probably a criminal to some for having a function in a struct, but oh well. Maybe I'll make it a class later.
	void Constrain();
};

struct nodeSide_t
{
	nodeVertex_t* vertex1;
	nodeVertex_t* vertex2; // Links to the next side's point 1
	
	std::vector<nodeVertex_t*> children;
	
};

enum class NodeType
{
	NONE,
	QUAD,
	TRI,
};

class CNode
{
public:
	void Update();
protected:
	void LinkSides();
public:

	nodeVertex_t* m_vertexes;
	nodeSide_t* m_sides;
	int m_sideCount;

	NodeType m_nodeType;

	glm::vec3 m_origin;

	CNodeRenderData m_renderData;
};

// I've been told hammer only likes triangles and quads. How sad!

class CQuadNode : public CNode
{
public:
	CQuadNode();

};

class CTriNode : public CNode
{
public:
	CTriNode();

};


class CWorldEditor
{
public:
	CWorldEditor();

	void RegisterNode(CNode* node);
	CQuadNode* CreateQuad();
	CTriNode* CreateTri();

	std::vector<CNode*> m_nodes;


};

CWorldEditor& GetWorldEditor();
