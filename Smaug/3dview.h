#pragma once
#include "baseview.h"
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"

class C3DView : public CBaseView
{
public:
	virtual void Init(bgfx::ViewId viewId, int width, int height, uint32_t clearColor);

	virtual void Draw(float dt);
	virtual void Update(float dt, float mx, float my);

	bool m_controllingCamera;

	glm::vec3 m_cameraAngle;
	glm::vec3 m_cameraPos;
	

	struct
	{
		bool panning;
		glm::vec2 mouseStartPos;
		glm::vec3 cameraStartPos;
	} m_panView;
};


