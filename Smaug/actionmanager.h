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
	vertex_t* vertex = nullptr;
	meshPart_t* side = nullptr;
	face_t* wall = nullptr;
};

enum class SolveToLine2DSnap;
glm::vec3 GetSelectionPos(selectionInfo_t info);
glm::vec3 SolvePosToSelection(selectionInfo_t info, glm::vec3 pos, SolveToLine2DSnap* snap = nullptr);


class CActionManager;

// Actions are single moves performed on single objects one at a time

class IAction
{
public:
	virtual ~IAction() {};
	virtual const char* GetName() = 0;

	virtual void Preview() = 0;
	//virtual void Cancel() = 0;

protected:
	virtual void Act() = 0;

	virtual void Undo() = 0; // THESE WONT WORK ATM
	virtual void Redo() = 0;

	friend CActionManager;
};


class CBaseSelectionAction : public IAction
{
public:
	virtual int GetSelectionType() = 0;

	virtual void Select(selectionInfo_t selectionInfo)
	{
		m_selectInfo = selectionInfo;
		m_node = selectionInfo.node;
	}

protected:
	selectionInfo_t m_selectInfo;
	CNodeRef m_node;
};


class CBaseDragAction : public CBaseSelectionAction
{
public:


	virtual void SetMoveDelta(glm::vec3 moveDelta) { m_moveDelta = moveDelta; }

protected:

	glm::vec3 m_moveDelta;
};


class CActionManager
{
public:

	void CommitAction(IAction* action);
	bool FindFlags(glm::vec3 mousePos, selectionInfo_t& info, int findFlags, glm::vec3* outPointOfIntersect = nullptr);

	void Undo();
	void Redo();

	void Update();

	std::vector<IAction*> m_actionHistory;
	std::vector<IAction*> m_redoStack;
	IAction* m_selectedAction;
};

CActionManager& GetActionManager();
