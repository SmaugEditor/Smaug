#pragma once
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <vector>
#include "worldrenderer.h"

struct nodeSide_t;
class CNode;
struct nodeVertex_t;

enum class ConstraintPairType
{
	NONE,
	VERTEX,
	SIDE,
};

class CConstraint
{
public:
	void Update();

	void SetParent(CNode* node, nodeSide_t* side) { m_parentType = ConstraintPairType::SIDE; m_parentNode = node; m_parentSide = side; }
	void SetParent(CNode* node, nodeVertex_t* vertex) { m_parentType = ConstraintPairType::VERTEX; m_parentNode = node; m_parentVertex = vertex; }

	void SetChild(CNode* node, nodeSide_t* side) { m_childType = ConstraintPairType::SIDE; m_childNode = node; m_childSide = side; }
	void SetChild(CNode* node, nodeVertex_t* vertex) { m_childType = ConstraintPairType::VERTEX; m_childNode = node; m_childVertex = vertex; }
	
	ConstraintPairType m_parentType;
	CNode* m_parentNode;
	union
	{
		nodeSide_t* m_parentSide;
		nodeVertex_t* m_parentVertex;
	};

	ConstraintPairType m_childType;
	CNode* m_childNode;
	union
	{
		nodeSide_t* m_childSide;
		nodeVertex_t* m_childVertex;
	};

};

// Do not change this struct without editing CreateVertexLayout in worldrenderer
struct nodeVertex_t
{
	glm::vec3 origin;
};


struct nodeWall_t
{
	glm::vec3 bottomPoints[2];
	glm::vec3 topPoints[2];
};


struct nodeSide_t
{
	nodeVertex_t* vertex1;
	nodeVertex_t* vertex2; // Links to the next side's point 1

	std::vector<nodeWall_t> walls;
};

enum class NodeType
{
	NONE,
	QUAD,
	TRI,
};

struct boundingBox2D_t
{
	glm::vec2 topRight;
	glm::vec2 bottomLeft;

};


class CNode
{
public:
	void Init();
	void Update();
	void ConstructWalls();
	bool IsPointInAABB(glm::vec2 point);
protected:
	void LinkSides();
	void CalculateAABB();
public:

	nodeVertex_t* m_vertexes;
	nodeSide_t* m_sides;
	int m_sideCount;

	NodeType m_nodeType;

	glm::vec3 m_origin;

	CNodeRenderData m_renderData;

	std::vector<CConstraint*> m_constraining; // We parent these constraints
	
	// Temp Hack!!
	CConstraint m_constrainedTo[16]; // We are constrained by these constraints 
	int m_constrainedToCount = 0;

	boundingBox2D_t m_aabb;
	float m_aabbLength;

	bool m_useCalculatedNodeHeight;
	float m_nodeHeight;
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
