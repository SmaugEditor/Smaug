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

	nodeSide_t* selectedSide;
	CNode* selectedNode;

	ActionMode actionMode;
};

CActionManager& GetActionManager();