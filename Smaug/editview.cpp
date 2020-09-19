#include "editview.h"

#include "smaugapp.h"

#include <bigg.hpp>
#include <bx/string.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp> 
#include "worldeditor.h"
#include "worldrenderer.h"
#include "utils.h"
#include "shadermanager.h"
#include "editoractions.h"
#include "basicdraw.h"
#include "cursor.h"

#define SCROLL_SPEED 5

void CEditView::Init(bgfx::ViewId viewId, int width, int height, uint32_t clearColor)
{
	CBaseView::Init(viewId, width, height, clearColor);


	m_shaderProgram = ShaderManager::GetShaderProgram(Shader::EDIT_VIEW_SHADER);


	CNode* node = GetWorldEditor().CreateQuad();

	node->m_origin = glm::vec3(0, 0, -20);

	for (int i = 0; i < node->m_sideCount; i++)
		node->m_sides[i].vertex1->origin *= 10;
	node->Update();

	// Zoom and pan
	m_viewZoom = 80;
	m_cameraPos = glm::vec3(0, 10, 0);
	
	GetActionManager().actionMode = ActionMode::NONE;
}


void CEditView::Update(float dt, float mx, float my)
{
	float scrollDelta = ImGui::GetIO().MouseWheel * SCROLL_SPEED;
	m_viewZoom -= scrollDelta;

	// Put the mouse pos into the world
	mx = (mx * 2 - 1) * (m_viewZoom * m_aspectRatio)- m_cameraPos.x;
	my = (my * 2 - 1) * (m_viewZoom) - m_cameraPos.z;

	GetActionManager().Act(glm::vec3(mx, 5, my));

}

void CEditView::Draw(float dt)
{
	CBaseView::Draw(dt);

	float width = m_viewZoom * m_aspectRatio;
	float height = m_viewZoom;


	// Camera
	glm::mat4 view = glm::mat4(1.0f);
	view = glm::rotate(view, glm::radians(90.0f), glm::vec3(1, 0, 0)); // Look down
	view = glm::translate(view, m_cameraPos); // Set our pos to be 10 units up
	glm::mat4 proj = glm::ortho(-(float)width, (float)width, -(float)height, (float)height, -900.0f, 800.0f);
	bgfx::setViewTransform(m_viewId, &view[0][0], &proj[0][0]);


	GetWorldRenderer().Draw2D(m_viewId, Shader::EDIT_VIEW_SHADER);


	float t = glfwGetTime();

	for (int i = 0; i < GetWorldEditor().m_nodes.size(); i++)
	{
		glm::mat4 mtx = glm::identity<glm::mat4>();
		mtx = glm::translate(mtx, GetWorldEditor().m_nodes[i]->m_origin);
		mtx = glm::scale(mtx, glm::vec3(2.5f, 2.5f, 2.5f));
		mtx *= glm::yawPitchRoll(1.37f * t, t, 0.0f);	
		BasicDraw().Cube(mtx);
		bgfx::submit(m_viewId, m_shaderProgram);
	}

	// Cursor
	GetCursor().Draw();
	bgfx::submit(m_viewId, m_shaderProgram);


	// Camera Icon
	glm::vec3 camAngle = GetApp().m_uiView.m_previewView.m_cameraAngle;
	glm::vec3 camPos = GetApp().m_uiView.m_previewView.m_cameraPos;

	// Our movement directions
	glm::vec3 forwardDir;
	glm::vec3 rightDir;
	glm::vec3 upDir;

	Directions(camAngle, &forwardDir, &rightDir, &upDir);

	glm::mat4 mtx;

	// Up
	mtx = glm::identity<glm::mat4>();
	mtx = glm::translate(mtx, camPos + upDir * 5.0f);
	mtx = glm::scale(mtx, glm::vec3(1.5f, 1.5f, 1.5f));
	BasicDraw().Cube(mtx);
	bgfx::submit(m_viewId, m_shaderProgram);

	// Forward
	mtx = glm::identity<glm::mat4>();	
	mtx = glm::translate(mtx, camPos + forwardDir * 5.0f);
	mtx = glm::scale(mtx, glm::vec3(2.5f, 2.5f, 2.5f));
	BasicDraw().Cube(mtx);
	bgfx::submit(m_viewId, m_shaderProgram);

	// Right
	mtx = glm::identity<glm::mat4>();
	mtx = glm::translate(mtx, camPos + rightDir * 5.0f);
	mtx = glm::scale(mtx, glm::vec3(2.5f, 2.5f, 2.5f));
	BasicDraw().Cube(mtx);
	bgfx::submit(m_viewId, m_shaderProgram);

}