#pragma once

#include "actionmanager.h"
#include <glm/vec3.hpp>


class CBaseTool
{
public:

	virtual const char* GetName() = 0;

	// When this key is pressed, this tool becomes active
	virtual int GetToggleKey() = 0;

	// While this key is held, this tool is active
	virtual int GetHoldKey() = 0;

	virtual void Enable() {};
	virtual void Update(float dt, glm::vec3 mousePos) {};
	virtual void Disable() {};

};


// This base tool implements basic element selection
class CBaseSelectionTool : public CBaseTool
{
public:

	// Override this with your ACT_SELECT_ to request types for selection
	virtual int GetSelectionType() = 0;
};

// This base tool implements basic mouse dragging
class CBaseDragTool : public CBaseSelectionTool
{
public:

	virtual void Enable();
	virtual void Update(float dt, glm::vec3 mousePos);

	virtual void StartDrag() = 0;
	virtual void EndDrag() = 0;

	bool m_inDrag;
	glm::vec3 m_mouseStartDragPos;
	glm::vec3 m_mouseDragDelta;
	selectionInfo_t m_selectionInfo;
};