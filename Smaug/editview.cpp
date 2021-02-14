#include "editview.h"

#include "smaugapp.h"

#include "worldeditor.h"
#include "worldrenderer.h"
#include "utils.h"
#include "shadermanager.h"
#include "editoractions.h"
#include "basicdraw.h"
#include "cursor.h"
#include "svarex.h"
#include "modelmanager.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp> 
#include <numbers>

BEGIN_SVAR_TABLE(CEditViewSettings)
	DEFINE_TABLE_SVAR(scrollSpeed,              5.0f)
	DEFINE_TABLE_SVAR(proportionalScroll,       true)
	DEFINE_TABLE_SVAR(proportionalScrollScale,  100.0f)
	// Add Pan Button option!
END_SVAR_TABLE()

static CEditViewSettings s_editViewSettings;
DEFINE_SETTINGS_MENU("Edit View", s_editViewSettings);


void CEditView::Init(bgfx::ViewId viewId, int width, int height, uint32_t clearColor)
{
	CBaseView::Init(viewId, width, height, clearColor);


	m_shaderProgram = ShaderManager::GetShaderProgram(Shader::EDIT_VIEW_SHADER);
	m_cameraModel = ModelManager().LoadModel("assets/camera.obj");

	CNode* node = GetWorldEditor().CreateQuad();

	node->m_origin = glm::vec3(0, 0, -20);

	for (int i = 0; i < node->m_sideCount; i++)
		node->m_sides[i].vertex1->origin *= 10;
	node->Update();

	// Zoom and pan
	m_viewZoom = 80;
	m_cameraPos = glm::vec3(0, 10, 0);
	
	m_panView.panning = false;
}


void CEditView::Update(float dt, float mx, float my)
{
	
	ImGuiIO& io = ImGui::GetIO();

	// View zooming
	float scrollDelta = io.MouseWheel * s_editViewSettings.scrollSpeed.GetValue();
	if (scrollDelta != 0)
	{
		// Scroll and solve mouse to stay in the same pos
		glm::vec3 startMouseStartPos = TransformMousePos(mx, my);

		if(s_editViewSettings.proportionalScroll.GetValue())
			m_viewZoom -= m_viewZoom / s_editViewSettings.proportionalScrollScale.GetValue() * scrollDelta;
		else
			m_viewZoom -= scrollDelta;


		// Run TransformMousePos backwards to keep our mouse pos stable
		m_cameraPos.x = (mx * 2 - 1) * (m_viewZoom * m_aspectRatio) - startMouseStartPos.x;
		m_cameraPos.z = (my * 2 - 1) * (m_viewZoom) - startMouseStartPos.z;

	}

	if (m_viewZoom <= 0)
		m_viewZoom = 0.01;


	// View panning
	if (!m_panView.panning)
	{
		// Should we pan the view?
		if (io.MouseDown[GLFW_MOUSE_BUTTON_3])
		{
			m_panView.mouseStartPos = TransformMousePos(mx,my);
			m_panView.cameraStartPos = m_cameraPos;
			m_panView.panning = true;
		}
	}
	else
	{
		if (io.MouseDown[GLFW_MOUSE_BUTTON_3])
		{
			// Preview the pan
			m_cameraPos = m_panView.cameraStartPos + (TransformMousePos(mx, my, m_panView.cameraStartPos) - m_panView.mouseStartPos);
		}
		else
		{
			// Apply the pan
			m_cameraPos = m_panView.cameraStartPos + (TransformMousePos(mx, my, m_panView.cameraStartPos) - m_panView.mouseStartPos);
			m_panView.panning = false;
		}
	}
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


	// Camera Icon
	glm::vec3 camAngle = GetApp().m_uiView.m_previewView.m_cameraAngle;
	glm::vec3 camPos = GetApp().m_uiView.m_previewView.m_cameraPos;

	m_cameraModel->Render(camPos, camAngle, glm::vec3(2.5));
	/*
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
	*/
}

glm::vec3 CEditView::TransformMousePos(float mx, float my)
{
	return TransformMousePos(mx, my, m_cameraPos);
}


glm::vec3 CEditView::TransformMousePos(float mx, float my, glm::vec3 cameraPos)
{
	// Put the mouse pos into the world
	mx = (mx * 2 - 1) * (m_viewZoom * m_aspectRatio) - cameraPos.x;
	my = (my * 2 - 1) * (m_viewZoom)-cameraPos.z;

	return glm::vec3(mx, 5, my);
}
