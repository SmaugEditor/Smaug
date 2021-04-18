#pragma once
#include "actionmanager.h"


class CVertDragAction : public CBaseDragAction
{
public:
	virtual const char* GetName() { return "Drag Vertex"; }

	virtual void Select(selectionInfo_t selectInfo)
	{
		CBaseDragAction::Select(selectInfo);
		m_originalPos = *m_selectInfo.vertex->vert;
	}

	virtual void Preview()
	{
		*m_selectInfo.vertex->vert = m_originalPos + m_moveDelta;
		m_node->PreviewUpdate();
	}

	virtual void Act()
	{
		*m_selectInfo.vertex->vert = m_originalPos + m_moveDelta;
		m_node->Update();
		m_finalMoveDelta = m_moveDelta;
	}

	virtual void Undo()
	{
		*m_selectInfo.vertex->vert -= m_finalMoveDelta;
		m_node->Update();
	}

	virtual void Redo()
	{
		*m_selectInfo.vertex->vert += m_finalMoveDelta;
		m_node->Update();
	}

	virtual void Cancel()
	{
		*m_selectInfo.vertex->vert = m_originalPos;
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

	CNode* CreateExtrusion();

	virtual void Preview();
	virtual void Act();
	virtual void Undo();
	virtual void Redo();

	virtual int GetSelectionType()
	{
		return ACT_SELECT_SIDE;
	}

	CNodeRef m_quad;
};

class CSideDragAction : public CBaseDragAction
{
public:
	~CSideDragAction()
	{
		if (m_originalPos)
			delete[] m_originalPos;
	}

	virtual const char* GetName() { return "Drag Side"; }
	
	virtual void Select(selectionInfo_t selectInfo)
	{
		CBaseDragAction::Select(selectInfo);
		if (!m_originalPos)
		{

			m_originalPos = new glm::vec3[selectInfo.side->verts.size()];
		}
		else
			printf("Yo! Bad call!");

		for (int i = 0; auto v : selectInfo.side->verts)
			m_originalPos[i++] = *v->vert;
	}

	virtual void Preview()
	{
		for (int i = 0; auto v : m_selectInfo.side->verts)
			*v->vert = m_originalPos[i++] + m_moveDelta;

		m_node->PreviewUpdate();
	}

	virtual void Act()
	{
		for (int i = 0; auto v : m_selectInfo.side->verts)
			*v->vert = m_originalPos[i++] + m_moveDelta;
		m_node->Update();

		m_finalMoveDelta = m_moveDelta;
	}

	virtual void Undo()
	{
		for (int i = 0; auto v : m_selectInfo.side->verts)
			*v->vert -= m_finalMoveDelta;
		m_node->Update();
	}

	virtual void Redo()
	{
		for (int i = 0; auto v : m_selectInfo.side->verts)
			*v->vert += m_finalMoveDelta;
		m_node->Update();
	}

	virtual void Cancel()
	{
		for (int i = 0; auto v : m_selectInfo.side->verts)
			*v->vert = m_originalPos[i++];
		m_node->Update();
	}

	virtual int GetSelectionType()
	{
		return ACT_SELECT_SIDE;
	}

	glm::vec3* m_originalPos = nullptr;
	glm::vec3 m_finalMoveDelta;

};
