#pragma once

#include "worldeditor.h"
#include <glm/glm.hpp>
#include <typeinfo>

enum SelectionFlag
{
	SF_NONE = 0,
	SF_NODE = 1,
	//SF_VERT = 2,
	SF_EDGE = 4,
	SF_PART = 8
};
MAKE_BITFLAG(SelectionFlag);

struct selectionInfo_t
{
	SelectionFlag    selected = SelectionFlag::SF_NONE;
	CNodeRef         node;
	//CNodeVertexRef   vert;
	CNodeMeshPartRef part;
	CNodeVertexRef   edge;
};

class CActionManager;

// Actions are single moves performed on single objects one at a time

class IAction
{
public:
	virtual ~IAction() {};
	virtual const char* GetName() = 0;

	virtual void Preview() = 0;
	virtual void Cancel() {};

protected:
	virtual void Act() = 0;

	virtual void Undo() = 0;
	virtual void Redo() = 0;

	friend CActionManager;
};


class CBaseSelectionAction : public IAction
{
public:
	virtual SelectionFlag GetSelectionType() = 0;

	virtual void Select(std::vector<selectionInfo_t> selectionInfo)
	{
		m_selectInfo.clear();
		for(auto i : selectionInfo)
			m_selectInfo.push_back(i);
	}

protected:
	std::vector<selectionInfo_t> m_selectInfo;
};


class CBaseDragAction : public CBaseSelectionAction
{
public:

	virtual void SetMoveStart(glm::vec3 moveStart) { m_moveStart = moveStart; }
	virtual void SetMoveDelta(glm::vec3 moveDelta) { m_moveDelta = moveDelta; }

protected:

	glm::vec3 m_moveStart;
	glm::vec3 m_moveDelta;
};


class CActionManager
{
public:

	void CommitAction(IAction* action);

	void Clear();

	void Undo();
	void Redo();

	std::vector<IAction*> m_actionHistory;
	std::vector<IAction*> m_redoStack;
};

CActionManager& GetActionManager();
