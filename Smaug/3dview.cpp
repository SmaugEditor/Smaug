#include "3dview.h"

#include "worldrenderer.h"
#include "shadermanager.h"
#include "smaugapp.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <GLFW/glfw3.h>

#define PREVIEW_MOUSE_SENSETIVITY 2
#define PREVIEW_MOVE_SPEED 10

void C3DView::Init(bgfx::ViewId viewId, int width, int height, uint32_t clearColor)
{
	CBaseView::Init(viewId, width, height, clearColor);
	m_cameraAngle = glm::vec3(0, 0, 0);
	m_controllingCamera = false;
}

void C3DView::Draw(float dt)
{
	CBaseView::Draw(dt);

	glm::mat4 view = glm::identity<glm::mat4>();
	//view = glm::orientate4(m_cameraAngle); not sure why but this doesn't work...
	view *= glm::eulerAngleX(m_cameraAngle.x);
	view *= glm::eulerAngleY(m_cameraAngle.y);
	view *= glm::eulerAngleZ(m_cameraAngle.z);
	view = glm::translate(view, m_cameraPos);

	glm::mat4 proj = glm::perspective(glm::radians(60.0f), float(m_width) / m_height, 0.1f, 800.0f);
	bgfx::setViewTransform(m_viewId, &view[0][0], &proj[0][0]);

	GetWorldRenderer().Draw3D(m_viewId, Shader::WORLD_PREVIEW_SHADER);
	GetWorldRenderer().Draw2D(m_viewId, Shader::WORLD_PREVIEW_SHADER);

}

void C3DView::Update(float dt, float mx, float my)
{
	ImGuiIO& io = ImGui::GetIO();
	
	if (io.KeysDown[GLFW_KEY_Z] && io.KeysDownDuration[GLFW_KEY_Z] == 0)
	{
		m_controllingCamera = !m_controllingCamera;
		GetApp().SetMouseLock(m_controllingCamera);
	}

	if (m_controllingCamera)
	{
		glm::vec3 angleDelta = glm::vec3(io.MouseDelta.y, io.MouseDelta.x, 0);

		angleDelta *= dt * PREVIEW_MOUSE_SENSETIVITY;
		m_cameraAngle += angleDelta;

		// Lock the view to prevent having upsidedown eyes
		if (m_cameraAngle.x >= glm::radians(90.0f))
		{
			m_cameraAngle.x = glm::radians(90.0f);
		}
		else if (m_cameraAngle.x <= glm::radians(-90.0f))
		{
			m_cameraAngle.x = glm::radians(-90.0f);
		}
	}
	
	// We can always control the position
	glm::vec3 moveDelta(0,0,0);
	moveDelta.x = (int)io.KeysDown[GLFW_KEY_D] - (int)io.KeysDown[GLFW_KEY_A];
	moveDelta.z = (int)io.KeysDown[GLFW_KEY_W] - (int)io.KeysDown[GLFW_KEY_S];
	moveDelta.y += (int)io.KeysDown[GLFW_KEY_SPACE] - (int)io.KeysDown[GLFW_KEY_LEFT_CONTROL]; // Why is this upside down?
	moveDelta *= PREVIEW_MOVE_SPEED * dt;
	
	// TODO: Make the move based on where we're looking!

	m_cameraPos += moveDelta;



#ifdef _DEBUG
	// Nice little debug view for testing
	if (ImGui::Begin("Camera Debug"))
	{
		ImGui::InputFloat3("Angle", (float*)&m_cameraAngle);
		ImGui::InputFloat3("Position", (float*)&m_cameraPos);
		ImGui::InputFloat3("MoveDelta", (float*)&moveDelta);
		ImGui::End();
	}
#endif

}
