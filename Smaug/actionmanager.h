#pragma once

#include <glm/glm.hpp>
#include "worldeditor.h"
#include <typeinfo>

#define ACT_SELECT_NONE 0
#define ACT_SELECT_NODE	1
#define ACT_SELECT_VERT 2
#define ACT_SELECT_SIDE 4
#define ACT_SELECT_WALL 8

struct selectionInfo_t
{
	int selected = ACT_SELECT_NONE;
	CNode* node = nullptr;
	nodeVertex_t* vertex = nullptr;
	nodeSide_t* side = nullptr;
	nodeWall_t* wall = nullptr;
};


// Actions are single moves performed on single objects one at a time

class IAction
{
public:
	virtual int GetSelectionType() = 0;

	virtual void Select(selectionInfo_t selectInfo) = 0;
	virtual void Preview(glm::vec3 moveDelta) = 0;
	virtual void Act(glm::vec3 moveDelta) = 0;
	virtual void Cancel() = 0;

	virtual void Undo() = 0; // THESE WONT WORK ATM
	virtual void Redo() = 0;
};


class CBaseAction : public IAction
{
public:

	virtual void Select(selectionInfo_t selectionInfo)
	{
		m_selectInfo = selectionInfo;
		m_node = selectionInfo.node;
	}

//protected:
	selectionInfo_t m_selectInfo;
	CNode* m_node;
	
	glm::vec2 m_mouseDelta;
};

class CActionManager
{
public:

	
	bool FindFlags(glm::vec3 mousePos, selectionInfo_t& info, int findFlags);


	IAction* m_selectedAction;
};

CActionManager& GetActionManager();
