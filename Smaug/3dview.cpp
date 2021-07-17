#include "3dview.h"

#include "worldrenderer.h"
#include "shadermanager.h"
#include "smaugapp.h"
#include "utils.h"
#include "cursor.h"
#include "meshrenderer.h"
#include "raytest.h"

#include "svar.h"
#include "svarex.h"


#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/geometric.hpp>
#include <GLFW/glfw3.h>
#include <grid.h>
#include <debugdraw.h>


BEGIN_SVAR_TABLE(C3DViewSettings)
	DEFINE_TABLE_SVAR(viewFOV,           60.0f)
	DEFINE_TABLE_SVAR(moveSpeed,         10.0f)
	DEFINE_TABLE_SVAR(mouseSensitivity,  2.0f)
	DEFINE_TABLE_SVAR(panningMultiplier, 20.0f)
	DEFINE_TABLE_SVAR(scrollSpeed,       2.0f)
	DEFINE_TABLE_SVAR_INPUT(forward,      GLFW_KEY_W, false)
	DEFINE_TABLE_SVAR_INPUT(backward,     GLFW_KEY_S, false)
	DEFINE_TABLE_SVAR_INPUT(left,         GLFW_KEY_A, false)
	DEFINE_TABLE_SVAR_INPUT(right,        GLFW_KEY_D, false)
	DEFINE_TABLE_SVAR_INPUT(up,           GLFW_KEY_SPACE, false)
	DEFINE_TABLE_SVAR_INPUT(down,         GLFW_KEY_LEFT_CONTROL, false)
	DEFINE_TABLE_SVAR_INPUT(enable,       GLFW_KEY_Z, false)
	DEFINE_TABLE_SVAR_INPUT(panView,	  GLFW_MOUSE_BUTTON_3, true)

	DEFINE_TABLE_SVAR_INPUT(wireframe,	  GLFW_KEY_F1, false)
END_SVAR_TABLE()

static C3DViewSettings s_3dViewSettings;
DEFINE_SETTINGS_MENU("3D View", s_3dViewSettings);


void C3DView::Init(bgfx::ViewId viewId, int width, int height, uint32_t clearColor)
{
	CBaseView::Init(viewId, width, height, clearColor);
	m_cameraAngle = glm::vec3(0.25, 0, 0);
	m_cameraPos = glm::vec3(0, 15, -15);
	m_controllingCamera = false;

	m_gridCenter = ModelManager().LoadModel("assets/top.obj");
}

void C3DView::Draw(float dt)
{
	CBaseView::Draw(dt);

	glm::vec3 forwardDir;
	Directions(m_cameraAngle, &forwardDir);


	m_view = glm::lookAt(m_cameraPos, m_cameraPos + forwardDir, { 0,1,0 });
	m_proj = glm::perspective(glm::radians(s_3dViewSettings.viewFOV.GetValue()), m_aspectRatio, 0.1f, 800.0f);
	bgfx::setViewTransform(m_viewId, &m_view[0][0], &m_proj[0][0]);
	GetWorldRenderer().Draw3D(m_viewId, Shader::WORLD_PREVIEW_SHADER);


	
	// Draw the Cursor
	//float scale = glm::distance(GetCursor().GetPosition(), m_cameraPos) / 20;
	//scale = clamp(scale, 0.25f, 1.5f);
	GetCursor().Draw(1);

	// Draw center of the edit view
	CModelTransform r;

	r.SetAbsScale(clamp(CEditView::m_viewZoom / 80.0f, 0.25f, 1.0f));
	m_gridCenter->Render(&r);


#ifdef _DEBUG
	DebugDraw().Draw();
#endif

}

void C3DView::Update(float dt, float mx, float my)
{
	ImGuiIO& io = ImGui::GetIO();
	
	if (Input().IsDown(s_3dViewSettings.enable.GetValue()) && io.KeysDownDuration[s_3dViewSettings.enable.GetValue().code] == 0)
	{
		m_controllingCamera = !m_controllingCamera;
		GetApp().SetMouseLock(m_controllingCamera);
	}

	if (m_controllingCamera)
	{
		glm::vec3 angleDelta = glm::vec3(io.MouseDelta.y, io.MouseDelta.x, 0);

		angleDelta *= s_3dViewSettings.mouseSensitivity.GetValue() / 1000.0f;
		m_cameraAngle += angleDelta;

		// Lock the view to prevent having upsidedown eyes
		m_cameraAngle.x = clamp(m_cameraAngle.x, glm::radians(-89.9f), glm::radians(89.9f));
	}
	else
	{
		// If we're not flying, our mouse is our cursor

		glm::vec3 o = glm::vec3{ 1 - mx, 1 - my, 0 };
		o = glm::unProject(o, m_view, m_proj, glm::vec4{ 0, 0, 1, 1 });
		testWorld_t t = testRay({ o, glm::normalize(o - m_cameraPos) });
		if (t.hit)
		{
			DebugDraw().HEFace(t.face, COLOR_GREEN, 0.75f, 0.1f);
			GetCursor().SetPositionForce(t.intersect);
		}
	}
	
	// We can always control the position

	// Our movement directions
	glm::vec3 forwardDir, rightDir, upDir;
	glm::vec3 moveDelta(0, 0, 0);
	Directions(m_cameraAngle, &forwardDir, &rightDir, &upDir);

	// View panning
	if (!m_panView.panning)
	{
		// Should we pan the view?
		if (Input().IsDown(s_3dViewSettings.panView))
		{
			m_panView.mouseStartPos = { mx, my };
			m_panView.cameraStartPos = m_cameraPos;
			m_panView.panning = true;
			return;
		}
	}
	else
	{
		moveDelta -= (mx - m_panView.mouseStartPos.x) * rightDir;
		moveDelta += (my - m_panView.mouseStartPos.y) * upDir;
		moveDelta *= s_3dViewSettings.panningMultiplier;
		

		// Preview the pan
		m_cameraPos = m_panView.cameraStartPos + moveDelta;

		if (!Input().IsDown(s_3dViewSettings.panView))
		{
			// Apply the pan
			m_panView.panning = false;
			return;
		}
	}

	// Yes, we want this if twice. Otherwise, there would be a nested mess in here.

	if (!m_panView.panning)
	{
		
		float forward = Input().Axis(s_3dViewSettings.forward, s_3dViewSettings.backward);
		float right = Input().Axis(s_3dViewSettings.right, s_3dViewSettings.left);
		float up = Input().Axis(s_3dViewSettings.up, s_3dViewSettings.down);


		float magnitude = sqrt(forward * forward + right * right);

		// If our magnitude is 0, we're not moving forward or right
		if (magnitude != 0)
		{
			// We need to make sure we maintain a magnitude of 1
			moveDelta += forwardDir * forward / magnitude;
			moveDelta += rightDir * right / magnitude;
		}

		moveDelta.y += up;

		moveDelta *= s_3dViewSettings.moveSpeed * dt;


		float scrollDelta = io.MouseWheel * s_3dViewSettings.scrollSpeed;
		if (scrollDelta != 0)
		{
			moveDelta += scrollDelta * forwardDir;
			/*
			if (m_controllingCamera)
			{
				moveDelta += scrollDelta * forwardDir;
			}
			else
			{
				// Run TransformMousePos backwards to keep our mouse pos stable
				moveDelta += ((mx * 2 - 1) * (scrollDelta * m_aspectRatio) - mx) * rightDir;
				moveDelta -= ((my * 2 - 1) * (scrollDelta)-my) * upDir;
				moveDelta += scrollDelta * forwardDir;
			}
			*/
		}

		m_cameraPos += moveDelta;
	}

#if defined( _DEBUG )
	
#if 0
	// Nice little debug view for testing
	ImGui::SetNextWindowFocus();
	if (ImGui::Begin("Camera Debug", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration))
	{
		ImGui::InputFloat3("Angle", (float*)&m_cameraAngle);
		ImGui::InputFloat3("Position", (float*)&m_cameraPos);
		ImGui::InputFloat3("MoveDelta", (float*)&moveDelta);
		ImVec2 size = ImGui::GetItemRectSize();
		ImGui::SetWindowSize({ size.x, size.y * 30 });
	}
	ImGui::End();
#endif

	static bool wireframe = false;
	wireframe |= Input().IsDown(s_3dViewSettings.wireframe);
	if (wireframe)
	{
		for (auto n : GetWorldEditor().m_nodes)
		{
			for (auto p : n.second->m_mesh.parts)
				for (auto f : /*p->sliced ? p->sliced->faces : p->collision*/ p->tris)
					DebugDraw().HEFace(f, p->sliced ? COLOR_GREEN : COLOR_RED, 0.25f, 0.01f);
		}
	}
#endif

}

