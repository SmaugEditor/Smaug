#pragma once
#include "actionmanager.h"


class CVertDragAction : public CBaseDragAction
{
public:
	virtual const char* GetName() { return "Drag Vertex"; }

	virtual void Select(selectionInfo_t selectInfo)
	{
		CBaseDragAction::Select(selectInfo);
		m_originalPos = m_selectInfo.vertex->origin;
	}

	virtual void Preview()
	{
		m_selectInfo.vertex->origin = m_originalPos + m_moveDelta;
		m_node->Update();
	}

	virtual void Act()
	{
		m_selectInfo.vertex->origin += m_moveDelta;
		m_node->Update();
		m_finalMoveDelta = m_moveDelta;
	}

	virtual void Undo()
	{
		m_selectInfo.vertex->origin -= m_finalMoveDelta;
		m_node->Update();
	}

	virtual void Redo()
	{
		m_selectInfo.vertex->origin += m_finalMoveDelta;
		m_node->Update();
	}

	virtual void Cancel()
	{
		m_selectInfo.vertex->origin = m_originalPos;
		m_node->Update();
	}

private:
	glm::vec3 m_originalPos;
	glm::vec3 m_finalMoveDelta;

	virtual int GetSelectionType() override;
};


class CWallExtrudeAction : public CBaseDragAction
{
public:
	virtual const char* GetName() { return "Extrude Wall"; }

	virtual void Preview();
	virtual void Act();
	virtual void Undo();
	virtual void Redo();

	virtual int GetSelectionType()
	{
		return ACT_SELECT_WALL;
	}

};

class CSideDragAction : public CBaseDragAction
{
public:

	virtual const char* GetName() { return "Drag Side"; }

	virtual void Preview()
	{
	}

	virtual void Act()
	{
		m_selectInfo.side->vertex1->origin += m_moveDelta;
		m_selectInfo.side->vertex2->origin += m_moveDelta;
		m_node->Update();
	}

	virtual void Undo()
	{
	}

	virtual void Redo()
	{
	}

	virtual void Cancel()
	{
	}

	virtual int GetSelectionType()
	{
		return ACT_SELECT_SIDE;
	}
};
