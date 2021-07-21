#pragma once
#include "mesh.h"
#include "worldeditor.h"
#include "raytest.h"

enum SelectionFlag
{
	SF_NONE = 0,
	SF_NODE	= 1,
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

class CSelectionManager
{
public:
	
	void Update2D(glm::vec3 mousePos);
	void Update3D(testWorld_t worldTest);

	void Draw();

	SelectionFlag SelectionMode() { return SelectionFlag::SF_PART; }

	bool MultiselectMode();

	// Are we still selecting something at the moment?
	bool BusySelecting() { return MultiselectMode() || m_selected.size() == 0; }
private:
	// If replace current is true, it will discard all currently selected objects in place of the new selection
	// Returns true if something was selected
	bool SelectAtPoint(glm::vec3 point, SelectionFlag search);

	// If not in multiselect mode, this will overtake the previous selection
	void AddSelection(selectionInfo_t& si);

public:
	std::vector<selectionInfo_t> m_selected;

private:
	aabb_t m_selectionBox;
};

CSelectionManager& SelectionManager();