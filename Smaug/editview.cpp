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
#include "grid.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp> 
#include <numbers>
#include <debugdraw.h>

BEGIN_SVAR_TABLE(CEditViewSettings)
	DEFINE_TABLE_SVAR(scrollSpeed,              5.0f)
	DEFINE_TABLE_SVAR(proportionalScroll,       true)
	DEFINE_TABLE_SVAR(proportionalScrollScale,  100.0f)
	DEFINE_TABLE_SVAR_INPUT(panView, GLFW_MOUSE_BUTTON_3, true)
END_SVAR_TABLE()

static CEditViewSettings s_editViewSettings;
DEFINE_SETTINGS_MENU("Edit View", s_editViewSettings);

glm::vec3 CEditView::m_cameraPos = glm::vec3(0, 0, 0);
float CEditView::m_viewZoom = 80;

IModel* CEditView::m_cameraModel = nullptr;
IModel* CEditView::m_tickModel = nullptr;
void CEditView::Init(bgfx::ViewId viewId, int width, int height, uint32_t clearColor)
{
	CBaseView::Init(viewId, width, height, clearColor);


	m_shaderProgram = ShaderManager().GetShaderProgram(Shader::EDIT_VIEW_SHADER);
	if(!m_cameraModel)
		m_cameraModel = ModelManager().LoadModel("assets/camera.obj");
	if(!m_tickModel)
		m_tickModel = ModelManager().LoadModel("assets/tick.obj");

	// Zoom and pan
	m_viewZoom = 80;
	
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
		float nx = -(mx * 2 - 1) * (m_viewZoom * m_aspectRatio);
		float ny = (my * 2 - 1) * (m_viewZoom);

		glm::vec3 forward, right, up;
		Directions(m_editPlaneAngle, &forward, &right, &up);
		m_cameraPos = nx * right + ny * forward + startMouseStartPos;
	}

	if (m_viewZoom <= 0)
		m_viewZoom = 0.01;


	// View panning
	if (!m_panView.panning)
	{
		// Should we pan the view?
		if (Input().IsDown(s_editViewSettings.panView))
		{
			m_panView.mouseStartPos = TransformMousePos(mx,my);
			m_panView.cameraStartPos = m_cameraPos;
			m_panView.panning = true;
		}
	}
	else
	{
		if (Input().IsDown(s_editViewSettings.panView))
		{
			// Preview the pan
			m_cameraPos = m_panView.cameraStartPos - (TransformMousePos(mx, my, m_panView.cameraStartPos) - m_panView.mouseStartPos);
		}
		else
		{
			// Apply the pan
			m_cameraPos = m_panView.cameraStartPos - (TransformMousePos(mx, my, m_panView.cameraStartPos) - m_panView.mouseStartPos);
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


	glm::vec3 forward, right, up;
	Directions(m_editPlaneAngle, &forward, &right, &up);
	// HACK: Not sure why this is how it is. Will find out soon. On a deadline
	if (glm::length(m_editPlaneAngle) == 0)
	{
		view = glm::rotate(view, m_editPlaneAngle.x + glm::radians(90.0f), glm::vec3(1, 0, 0)); // Look down
		view = glm::translate(view, m_cameraPos); // Set our pos to be 10 units up
		view = glm::rotate(view, glm::radians(180.0f), glm::vec3(0, 1, 0)); // Flip 180
	}
	else
	{
		view = glm::lookAt(m_cameraPos, m_cameraPos - up, glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::mat4 proj = glm::ortho(-(float)width, (float)width, -(float)height, (float)height, -900.0f, 800.0f);
	bgfx::setViewTransform(m_viewId, &view[0][0], &proj[0][0]);

	

	GetWorldRenderer().Draw2D(m_viewId, Shader::WORLD_PREVIEW_SHADER);
	
	float t = glfwGetTime();

	for (auto p : GetWorldEditor().m_nodes)
	{
		CModelTransform mt;
		mt.SetAbsOrigin(p.second->Origin());
		mt.SetAbsScale(min(m_viewZoom / 16.0f, 4.0f));
		m_tickModel->Render(&mt);
	}


	// Cursor
	GetCursor().Draw(m_viewZoom / 16.0f);


	// Camera Icon
	glm::vec3 camAngle = GetApp().m_uiView.m_previewView.m_cameraAngle;
	glm::vec3 camPos = GetApp().m_uiView.m_previewView.m_cameraPos;

	m_cameraModel->Render(camPos, camAngle, glm::vec3(2.5));
	
	// Grid gets drawn last. Figure out transparency!
	Grid().Draw({ width, height }, m_cameraPos, m_editPlaneAngle, m_focused, up * -200.0f);


	if (m_focused)
		GetCursor().SetWorkingAxis(up);


#ifdef _DEBUG
	DebugDraw().Draw();
#endif
}

glm::vec3 CEditView::TransformMousePos(float mx, float my)
{
	return TransformMousePos(mx, my, m_cameraPos);
}


glm::vec3 CEditView::TransformMousePos(float mx, float my, glm::vec3 cameraPos)
{
	glm::vec3 forward, right, up;
	Directions(m_editPlaneAngle, &forward, &right, &up);

	float inv = RendererProperties().coordSystem == ECoordSystem::LEFT_HANDED ? -1.f : 1.f;

	// Put the mouse pos into the world
	mx =  (mx * 2 - 1) * (m_viewZoom * m_aspectRatio);
	my = inv * (my * 2 - 1) * (m_viewZoom);


	//printf("%f, %f, { %f, %f, %f } - { %f, %f, %f }\n", mx, my, forward.x, forward.y, forward.z, right.x, right.y, right.z);
	return mx * right + my * forward + cameraPos;
}
