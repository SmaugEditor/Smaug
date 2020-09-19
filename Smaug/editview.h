#pragma once
#include "baseview.h"
#include <glm/vec3.hpp>
#include "actionmanager.h"

struct nodeSide_t;
class CNode;

class CEditView : public CBaseView
{
public:
	virtual void Init(bgfx::ViewId viewId, int width, int height, uint32_t clearColor);
	virtual void Update(float dt, float mx, float my);
	virtual void Draw(float dt);
//private:

	bgfx::ProgramHandle m_shaderProgram;

	float m_viewZoom;

	glm::vec3 m_cameraPos;
	

};

