#pragma once
#include "baseview.h"
#include <glm/vec3.hpp>
#include "actionmanager.h"

struct nodeSide_t;
class CNode;
class IModel;

class CEditView : public CBaseView
{
public:
	virtual void Init(bgfx::ViewId viewId, int width, int height, uint32_t clearColor);
	virtual void Update(float dt, float mx, float my);
	virtual void Draw(float dt);
	
	glm::vec3 TransformMousePos(float mx, float my);
	glm::vec3 m_editPlaneAngle;

	static glm::vec3 m_cameraPos;
	static float m_viewZoom;
private:

	glm::vec3 TransformMousePos(float mx, float my, glm::vec3 cameraPos);

	bgfx::ProgramHandle m_shaderProgram;


	
	struct
	{
		bool panning;
		glm::vec3 mouseStartPos;
		glm::vec3 cameraStartPos;
	} m_panView;

	static IModel* m_cameraModel;
	static IModel* m_tickModel;


};

