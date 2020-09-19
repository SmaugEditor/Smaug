#pragma once
#include "actionmanager.h"
#include <glm/vec3.hpp>
#include <bgfx/bgfx.h>

class CCursor
{
public:
	CCursor();

	void Update(float dt);

	// Does not call bgfx::submit!
	void Draw();

	void SetPosition(glm::vec3 pos);
	void SetSelection(selectionInfo_t info);

private:
	glm::vec3 m_position;


	float m_cursorSpinTime;
	bool m_selectedObject;
};


CCursor& GetCursor();