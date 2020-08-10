#pragma once
#include "baseview.h"
#include <glm/vec3.hpp>

struct nodeSide_t;
class CNode;


enum class SelectionMode
{
	NONE,
	SIDE,
	NODE,
	VERTEX,
};

class CEditView : public CBaseView
{
public:
	virtual void Init(bgfx::ViewId viewId, int width, int height, uint32_t clearColor);
	virtual void Update(float dt, float mx, float my);
private:

	bgfx::ProgramHandle m_hShaderProgram;
	bgfx::VertexBufferHandle m_hVertexBuf;
	bgfx::IndexBufferHandle m_hIndexBuf;

	float m_viewportWidth;
	float m_viewportHeight;

	glm::vec3 m_cameraPos;

	struct selectionData_t
	{
		nodeSide_t* m_selectedSide;
		CNode* m_selectedNode;
		glm::vec3 m_mouseStartPos;

	} m_selectData;
};

