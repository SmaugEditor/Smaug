#include "3dview.h"

#include "worldrenderer.h"
#include "shadermanager.h"
#include "smaugapp.h"
#include "utils.h"
#include "cursor.h"

#include "svar.h"
#include "svarex.h"


#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/geometric.hpp>
#include <GLFW/glfw3.h>


BEGIN_SVAR_TABLE(C3DViewSettings)
	DEFINE_TABLE_SVAR(viewFOV,           60.0f)
	DEFINE_TABLE_SVAR(moveSpeed,         10.0f)
	DEFINE_TABLE_SVAR(mouseSensitivity,  2.0f)
	DEFINE_TABLE_SVAR(panningMultiplier, 20.0f)
	DEFINE_TABLE_SVAR(scrollSpeed,       60.0f)
	DEFINE_TABLE_SVAR_KEY(forward,      GLFW_KEY_W)
	DEFINE_TABLE_SVAR_KEY(backward,     GLFW_KEY_S)
	DEFINE_TABLE_SVAR_KEY(left,         GLFW_KEY_A)
	DEFINE_TABLE_SVAR_KEY(right,        GLFW_KEY_D)
	DEFINE_TABLE_SVAR_KEY(up,           GLFW_KEY_SPACE)
	DEFINE_TABLE_SVAR_KEY(down,         GLFW_KEY_LEFT_CONTROL)
	DEFINE_TABLE_SVAR_KEY(enable,       GLFW_KEY_Z)
END_SVAR_TABLE()

static C3DViewSettings s_3dViewSettings;
DEFINE_SETTINGS_MENU("3D View", s_3dViewSettings);


void C3DView::Init(bgfx::ViewId viewId, int width, int height, uint32_t clearColor)
{
	CBaseView::Init(viewId, width, height, clearColor);
	m_cameraAngle = glm::vec3(0, 0.25, -1.57);
	m_cameraPos = glm::vec3(0, 15, 15);
	m_controllingCamera = false;

}

void C3DView::Draw(float dt)
{
	CBaseView::Draw(dt);

	glm::mat4 view = glm::identity<glm::mat4>();
	//view = glm::orientate4(m_cameraAngle); not sure why but this doesn't work...
	//view *= glm::eulerAngleX(-m_cameraAngle.x);
	//view *= glm::eulerAngleY(-m_cameraAngle.y);
	//view *= glm::eulerAngleZ(-m_cameraAngle.z);
	glm::vec3 forwardDir;

	Directions(m_cameraAngle, &forwardDir);


	view = glm::lookAt(m_cameraPos, m_cameraPos + forwardDir, glm::vec3(0.0f, 1.0f, 0.0f));
	//view = glm::translate(view, m_cameraPos * -1.0f);

	glm::mat4 proj = glm::perspective(glm::radians(s_3dViewSettings.viewFOV.GetValue()), m_aspectRatio, 0.1f, 800.0f);
	bgfx::setViewTransform(m_viewId, &view[0][0], &proj[0][0]);

	GetWorldRenderer().Draw3D(m_viewId, Shader::WORLD_PREVIEW_SHADER);
	GetWorldRenderer().Draw2D(m_viewId, Shader::WORLD_PREVIEW_SHADER);

	//bgfx::submit(m_viewId, ShaderManager::GetShaderProgram(Shader::CURSOR_SHADER));
	// Draw the Cursor
	GetCursor().Draw();

}

void C3DView::Update(float dt, float mx, float my)
{
	ImGuiIO& io = ImGui::GetIO();
	
	if (io.KeysDown[s_3dViewSettings.enable.GetValue().key] && io.KeysDownDuration[s_3dViewSettings.enable.GetValue().key] == 0)
	{
		m_controllingCamera = !m_controllingCamera;
		GetApp().SetMouseLock(m_controllingCamera);
	}

	if (m_controllingCamera)
	{
		glm::vec3 angleDelta = glm::vec3(0, io.MouseDelta.y, io.MouseDelta.x);

		angleDelta *= s_3dViewSettings.mouseSensitivity.GetValue() / 1000.0f;
		m_cameraAngle += angleDelta;

		// Lock the view to prevent having upsidedown eyes
		if (m_cameraAngle.y >= glm::radians(89.9f))
		{
			m_cameraAngle.y = glm::radians(89.9f);
		}
		else if (m_cameraAngle.y <= glm::radians(-89.9f))
		{
			m_cameraAngle.y = glm::radians(-89.9f);
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
		if (io.MouseDown[GLFW_MOUSE_BUTTON_3])
		{
			m_panView.mouseStartPos = { mx, my };
			m_panView.cameraStartPos = m_cameraPos;
			m_panView.panning = true;
		}
	}
	else
	{
		moveDelta -= (mx - m_panView.mouseStartPos.x) * rightDir;
		moveDelta += (my - m_panView.mouseStartPos.y) * upDir;
		moveDelta *= s_3dViewSettings.panningMultiplier.GetValue();

		// Preview the pan
		m_cameraPos = m_panView.cameraStartPos + moveDelta;

		if (!io.MouseDown[GLFW_MOUSE_BUTTON_3])
		{
			// Apply the pan
			m_panView.panning = false;
		}
	}

	// Yes, we want this if twice. Otherwise, there would be a nested mess in here.

	if (!m_panView.panning)
	{
		float scrollDelta = io.MouseWheel * s_3dViewSettings.scrollSpeed.GetValue();
		if (scrollDelta != 0)
		{

			// Run TransformMousePos backwards to keep our mouse pos stable
			moveDelta += ((mx * 2 - 1) * (scrollDelta * m_aspectRatio) - mx) * rightDir;
			moveDelta -= ((my * 2 - 1) * (scrollDelta)-my) * upDir;
			moveDelta += scrollDelta * forwardDir;
		}




		float forward = io.KeysDown[s_3dViewSettings.forward.GetValue().key] - (int)io.KeysDown[s_3dViewSettings.backward.GetValue().key];
		float right = io.KeysDown[s_3dViewSettings.right.GetValue().key] - (int)io.KeysDown[s_3dViewSettings.left.GetValue().key];
		float up = io.KeysDown[s_3dViewSettings.up.GetValue().key] - (int)io.KeysDown[s_3dViewSettings.down.GetValue().key];


		float magnitude = sqrt(forward * forward + right * right);

		// If our magnitude is 0, we're not moving forward or right
		if (magnitude != 0)
		{
			// We need to make sure we maintain a magnitude of 1
			moveDelta += forwardDir * forward / magnitude;
			moveDelta += rightDir * right / magnitude;
		}

		moveDelta.y += up;

		moveDelta *= s_3dViewSettings.moveSpeed.GetValue() * dt;

		m_cameraPos += moveDelta;
	}

#ifdef _DEBUG
	// Nice little debug view for testing
	if (ImGui::Begin("Camera Debug"))
	{
		ImGui::InputFloat3("Angle", (float*)&m_cameraAngle);
		ImGui::InputFloat3("Position", (float*)&m_cameraPos);
		ImGui::InputFloat3("MoveDelta", (float*)&moveDelta);
	}
	ImGui::End();

#endif

}

