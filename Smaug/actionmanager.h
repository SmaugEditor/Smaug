#pragma once

#include "worldeditor.h"
#include "selectionmanager.h"
#include <glm/glm.hpp>
#include <typeinfo>

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
	//bool FindFlags(glm::vec3 mousePos, selectionInfo_t& info, int findFlags, glm::vec3* outPointOfIntersect = nullptr);

	void Clear();

	void Undo();
	void Redo();

	void Update();

	std::vector<IAction*> m_actionHistory;
	std::vector<IAction*> m_redoStack;
};

CActionManager& GetActionManager();
