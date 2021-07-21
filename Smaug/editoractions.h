#pragma once
#include "actionmanager.h"
#include "selectionmanager.h"
#include <vector>

class CDragAction : public CBaseDragAction
{
public:
	virtual const char* GetName() { return "Drag"; }

	virtual void Select(std::vector<selectionInfo_t> selectInfo)
	{
		CBaseDragAction::Select(selectInfo);
		for (auto i : m_selectInfo)
			if (i.edge.IsValid())
			{
				m_originalPos.push_back(*i.edge->vert);
				m_originalPos.push_back(*i.edge->edge->vert->vert);
			}
			else if (i.part.IsValid())
			{
				for (auto v : i.part->verts)
					m_originalPos.push_back(*v->vert);
			}
			else
			{
				m_originalPos.push_back(i.node->Origin());
			}
	}

	virtual void Preview()
	{
		for (int j = 0; auto i : m_selectInfo)
		{
			if (i.edge.IsValid())
			{
				*i.edge->vert = m_originalPos[j++] + m_moveDelta;
				*i.edge->edge->vert->vert = m_originalPos[j++] + m_moveDelta;
			}
			else if (i.part.IsValid())
			{
				for(auto v : i.part->verts)
					*v->vert = m_originalPos[j++] + m_moveDelta;
			}
			else
			{
				i.node->m_mesh.origin = m_originalPos[j++] + m_moveDelta;
			}
			i.node->MarkDirtyPreview();
		}
	}

	virtual void Act()
	{
		for (int j = 0; auto i : m_selectInfo)
		{
			if (i.edge.IsValid())
			{
				*i.edge->vert = m_originalPos[j++] + m_moveDelta;
				*i.edge->edge->vert->vert = m_originalPos[j++] + m_moveDelta;
			}
			else if (i.part.IsValid())
			{
				for (auto v : i.part->verts)
					*v->vert = m_originalPos[j++] + m_moveDelta;
			}
			else
			{
				i.node->m_mesh.origin = m_originalPos[j++] + m_moveDelta;
			}
			i.node->MarkDirty();
		}
		m_finalMoveDelta = m_moveDelta;

		// Save on some memory
		m_originalPos.clear();
	}

	virtual void Undo()
	{
		for (int j = 0; auto i : m_selectInfo)
		{
			if (i.edge.IsValid())
			{
				*i.edge->vert -= m_finalMoveDelta;
				*i.edge->edge->vert->vert -= m_finalMoveDelta;
			}
			else if (i.part.IsValid())
			{
				for (auto v : i.part->verts)
					*v->vert -= m_finalMoveDelta;
			}
			else
			{
				i.node->m_mesh.origin -= m_finalMoveDelta;
			}
			i.node->MarkDirty();
		}
	}

	virtual void Redo()
	{
		for (int j = 0; auto i : m_selectInfo)
		{
			if (i.edge.IsValid())
			{
				*i.edge->vert += m_finalMoveDelta;
				*i.edge->edge->vert->vert += m_finalMoveDelta;
			}
			else if (i.part.IsValid())
			{
				for (auto v : i.part->verts)
					*v->vert += m_finalMoveDelta;
			}
			else
			{
				i.node->m_mesh.origin += m_finalMoveDelta;
			}
			i.node->MarkDirty();
		}
	}

	virtual void Cancel()
	{
		for (int j = 0; auto i : m_selectInfo)
		{
			if (i.edge.IsValid())
			{
				*i.edge->vert = m_originalPos[j++];
				*i.edge->edge->vert->vert = m_originalPos[j++];
			}
			else if (i.part.IsValid())
			{
				for (auto v : i.part->verts)
					*v->vert = m_originalPos[j++];
			}
			else
			{
				i.node->m_mesh.origin += m_originalPos[j++];
			}
			i.node->MarkDirty();
		}
	}

private:
	std::vector<glm::vec3> m_originalPos = {};
	glm::vec3 m_finalMoveDelta;

	virtual SelectionFlag GetSelectionType() override
	{
		return (SelectionFlag)(SelectionFlag::SF_EDGE | SelectionFlag::SF_PART | SelectionFlag::SF_NODE);
	}
};


class CWallExtrudeAction : public CBaseDragAction
{
public:
	virtual const char* GetName() { return "Extrude Wall"; }

	CNode* CreateExtrusion(CNode* node, meshPart_t* part);

	virtual void Preview();
	virtual void Act();
	virtual void Undo();
	virtual void Redo();

	virtual SelectionFlag GetSelectionType()
	{
		return SelectionFlag::SF_PART;
	}

	std::vector<CNodeRef> m_quads = {};
};
