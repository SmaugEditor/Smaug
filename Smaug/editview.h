#pragma once
#include "baseview.h"
#include <glm/vec3.hpp>

struct nodeSide_t;
class CNode;


enum class ActionMode
{
	NONE,
	DRAG_SIDE,
	DRAG_VERTEX,
	EXTRUDE_SIDE,
	PAN_VIEW,
};

class CEditView : public CBaseView
{
public:
	virtual void Init(bgfx::ViewId viewId, int width, int height, uint32_t clearColor);
	virtual void Update(float dt, float mx, float my);
	virtual void Draw(float dt);
private:

	bgfx::ProgramHandle m_hShaderProgram;
	bgfx::VertexBufferHandle m_hVertexBuf;
	bgfx::IndexBufferHandle m_hIndexBuf;

	float m_viewportWidth;
	float m_viewportHeight;

	glm::vec3 m_cameraPos;

	struct actionData_t
	{
		glm::vec3 mouseStartPos;

		nodeSide_t* selectedSide;
		CNode* selectedNode;

		ActionMode actionMode;
	} m_actionData;

	glm::vec3 m_cursorPos;
	int m_cursorSpin;
};

