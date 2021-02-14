#pragma once
#include "actionmanager.h"
#include <glm/vec3.hpp>
#include <bgfx/bgfx.h>

enum class CursorMode
{
	NONE,
	HOVERING,
	EDITING
};

class IModel;
class CCursor
{
public:
	CCursor();

	void Update(float dt);

	void Draw(float scale = 2.5f);

	void SetPosition(glm::vec3 pos);
	glm::vec3 SetSelection(glm::vec3 pos, selectionInfo_t info);
	void SetEditPosition(glm::vec3 pos);
	void SetModel(const char* path);

	void SetMode(CursorMode mode);

private:
	glm::vec3 m_position;

	IModel* m_model;
	
	float m_cursorSpinTime;
	CursorMode m_cursorMode;

};


CCursor& GetCursor();