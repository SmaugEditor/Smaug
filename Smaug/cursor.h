#pragma once
#include "actionmanager.h"
#include <glm/vec3.hpp>
#include <bgfx/bgfx.h>

class IModel;
class CCursor
{
public:
	CCursor();

	void Update(float dt);

	void Draw(float scale = 2.5f);

	void SetPosition(glm::vec3 pos);
	void SetSelection(selectionInfo_t info);
	void SetModel(const char* path);

private:
	glm::vec3 m_position;

	float m_cursorSpinTime;
	bool m_selectedObject;

	IModel* m_model;
};


CCursor& GetCursor();