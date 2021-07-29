#pragma once
#include "baseview.h"
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

class IModel;
class C3DView : public CBaseView
{
public:
	virtual void Init(uint16_t viewId, int width, int height, uint32_t clearColor);

	virtual void Draw(float dt);
	virtual void Update(float dt, float mx, float my);

	bool m_controllingCamera;

	glm::vec3 m_cameraAngle;
	glm::vec3 m_cameraPos;
	
	IModel* m_gridCenter;

	struct
	{
		bool panning;
		glm::vec2 mouseStartPos;
		glm::vec3 cameraStartPos;
	} m_panView;
private:
	glm::mat4 m_view;
	glm::mat4 m_proj;
};


