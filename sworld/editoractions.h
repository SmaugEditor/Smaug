#pragma once
#include "actionmanager.h"
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

	virtual bool Act()
	{
		if (m_originalPos.size() == 0)
			return false;

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

		return true;
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

	virtual void Select(std::vector<selectionInfo_t> selectInfo);
	virtual void Cancel();
	virtual void Preview();
	virtual bool Act();
	virtual void Undo();
	virtual void Redo();

	virtual SelectionFlag GetSelectionType()
	{
		return SelectionFlag::SF_PART;
	}

	// the uint16 is a pos within m_selectionInfo
	std::vector<std::pair<CNodeRef, unsigned short>> m_quads = {};
};



class CPaintAction : public CBaseSelectionAction
{
public:

	virtual const char* GetName() { return "Paint Side"; };


	virtual void Select(std::vector<selectionInfo_t> selectInfo)
	{
		CBaseSelectionAction::Select(selectInfo);

		// Cache all the old paints
		for (auto si : selectInfo)
			if (si.part.IsValid())
				m_originalPaint.push_back(si.part->txData);
	}

	virtual void Preview() { for(auto& si : m_selectInfo) Paint(si, m_newPaint); }

	virtual bool Act()
	{
		if (m_selectInfo.size() == 0)
			return false;
		
		bool success = false;
		for (auto& si : m_selectInfo)
			success |= Paint(si, m_newPaint);
	
		return success;
	}

	virtual void Undo() { for (int i = 0; auto & si : m_selectInfo) Paint(si, m_originalPaint[i++]); }

	virtual void Redo() { Preview(); }

	virtual SelectionFlag GetSelectionType() { return SelectionFlag::SF_PART; }

	bool Paint(selectionInfo_t& si, textureMeshPartData_t& tx)
	{
		if (si.part->txData == tx)
			return false;

		si.part->txData = tx;
		if(si.node->m_renderData)
			si.node->m_renderData->Rebuild();
		return true;
	}

	std::vector<textureMeshPartData_t> m_originalPaint;
	textureMeshPartData_t m_newPaint;
};