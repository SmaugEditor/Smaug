#pragma once

#include <glm/glm.hpp>
#include "worldeditor.h"

enum class ActionMode
{
	NONE,
	DRAG_SIDE,
	DRAG_VERTEX,
	EXTRUDE_SIDE,
	PAN_VIEW,
};

class CActionManager
{
public:

	void Act(glm::vec3 mousePos);

	glm::vec3 cursorPos;

	glm::vec3 mouseStartPos;

	nodeWall_t* selectedWall;
	nodeSide_t* selectedSide;
	nodeVertex_t* selectedVertex;
	CNode* selectedNode;

	ActionMode actionMode;
};

CActionManager& GetActionManager();