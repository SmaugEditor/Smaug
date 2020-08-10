#pragma once
#include <glm/vec3.hpp>
#include <vector>
#include "worldrenderer.h"

enum class Constraint
{
	NONE,
	LOCKED_TO_PARENT,

};

// Do not change this struct without editing CreateVertexLayout in worldrenderer
struct nodeSide_t
{
	glm::vec3 point1;
	glm::vec3* point2; // Links to the next side's point 1
	
	Constraint constraint;
	std::vector<nodeSide_t> linkedSides;
	nodeSide_t* parentSide;
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
