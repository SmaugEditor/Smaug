#include "editview.h"

#include "settingsmenu.h"
#include "worldeditor.h"
#include "utils.h"
#include "editoractions.h"
#include "cursor.h"
#include "svarex.h"
#include "grid.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <debugdraw.h>
#include <transform.h>
#include <editorinterface.h>
#include <imgui.h>
#include <selectionmanager.h>

BEGIN_SVAR_TABLE(CEditViewSettings)
	DEFINE_TABLE_SVAR(scrollSpeed,              5.0f)
	DEFINE_TABLE_SVAR(proportionalScroll,       true)
	DEFINE_TABLE_SVAR(proportionalScrollScale,  100.0f)
	DEFINE_TABLE_SVAR_INPUT(panView, MOUSE_3,   true)
END_SVAR_TABLE()

static CEditViewSettings s_editViewSettings;
DEFINE_SETTINGS_MENU("Edit View", s_editViewSettings);

glm::vec3 CEditView::m_cameraPos = glm::vec3(0, 0, 0);
float CEditView::m_viewZoom = 80;

IModel* CEditView::m_tickModel = nullptr;
void CEditView::Init(uint16_t viewId, int width, int height, uint32_t clearColor)
{
	CBaseView::Init(viewId, width, height, clearColor);


	if(!m_tickModel)
		m_tickModel = EngineInterface()->LoadModel("assets/tick");

	// Zoom and pan
	m_viewZoom = 80;
	
	m_panView.panning = false;

}


void CEditView::Update(float dt, float mx, float my)
{
	
	ImGuiIO& io = ImGui::GetIO();

	glm::vec3 mousePos = TransformMousePos(mx, my);

	// View zooming
	float scrollDelta = io.MouseWheel * s_editViewSettings.scrollSpeed.GetValue();
	if (scrollDelta != 0)
	{
		// Scroll and solve mouse to stay in the same pos

		if(s_editViewSettings.proportionalScroll.GetValue())
			m_viewZoom -= m_viewZoom / s_editViewSettings.proportionalScrollScale.GetValue() * scrollDelta;
		else
			m_viewZoom -= scrollDelta;

		// Run TransformMousePos backwards to keep our mouse pos stable
		float nx = -(mx * 2 - 1) * (m_viewZoom * m_aspectRatio);
		float ny = (my * 2 - 1) * (m_viewZoom);

		glm::vec3 forward, right, up;
		Directions(m_editPlaneAngle, &forward, &right, &up);
		m_cameraPos = nx * right + ny * forward + mousePos;
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

		// If we're not panning, we can do selecting!
		SelectionManager().Update2D(mousePos);
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
	EngineInterface()->BeginView(m_renderTarget);
	EngineInterface()->ClearColor(m_renderTarget, m_clearColor);


	float width = m_viewZoom * m_aspectRatio;
	float height = m_viewZoom;

	// Camera
	glm::mat4 view = glm::mat4(1.0f);


	glm::vec3 forward, right, up;
	Directions(m_editPlaneAngle, &forward, &right, &up);

	// HACK: Not sure why this is how it is. Will find out soon. On a deadline
	if (glm::length(m_editPlaneAngle) == 0)
	{
		view = glm::lookAt(m_cameraPos, m_cameraPos - up, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else
	{
		view = glm::lookAt(m_cameraPos, m_cameraPos - up, glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::mat4 proj = glm::ortho(-(float)width, (float)width, -(float)height, (float)height, -900.0f, 800.0f);
	EngineInterface()->SetViewMatrix(view, proj);

	EngineInterface()->DrawWorld2D();


	for (auto p : GetWorldEditor().m_nodes)
	{
		CTransform mt;
		mt.SetAbsOrigin(p.second->Origin());
		mt.SetAbsScale(min(m_viewZoom / 16.0f, 4.0f));
		m_tickModel->Draw(&mt);
	}


	// Cursor
	GetCursor().Draw(m_viewZoom / 16.0f);


	// Grid gets drawn last. Figure out transparency!
	Grid().Draw({ width, height }, m_cameraPos, m_editPlaneAngle, m_focused, up * -200.0f);


	if (m_focused)
		GetCursor().SetWorkingAxis(up);


#ifdef _DEBUG
	DebugDraw().Draw();
#endif
	SelectionManager().Draw();

	EngineInterface()->EndView(m_renderTarget);
}

glm::vec3 CEditView::TransformMousePos(float mx, float my)
{
	return TransformMousePos(mx, my, m_cameraPos);
}


glm::vec3 CEditView::TransformMousePos(float mx, float my, glm::vec3 cameraPos)
{
	glm::vec3 forward, right, up;
	Directions(m_editPlaneAngle, &forward, &right, &up);


	// Put the mouse pos into the world
	mx = (mx * 2 - 1) * (m_viewZoom * m_aspectRatio);
	my = -(my * 2 - 1) * (m_viewZoom);


	//printf("%f, %f, { %f, %f, %f } - { %f, %f, %f }\n", mx, my, forward.x, forward.y, forward.z, right.x, right.y, right.z);
	return mx * right + my * forward + cameraPos;
}
