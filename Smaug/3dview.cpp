#include "3dview.h"

#include "worldrenderer.h"
#include "shadermanager.h"
#include "smaugapp.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/geometric.hpp>
#include <GLFW/glfw3.h>
#include "utils.h"

#define PREVIEW_MOUSE_SENSETIVITY 2
#define PREVIEW_MOVE_SPEED 10



void C3DView::Init(bgfx::ViewId viewId, int width, int height, uint32_t clearColor)
{
	CBaseView::Init(viewId, width, height, clearColor);
	m_cameraAngle = glm::vec3(0, 0, 0);
	m_cameraPos = glm::vec3(0, 0, 0);
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
		glm::vec3 angleDelta = glm::vec3(0, io.MouseDelta.y, io.MouseDelta.x);

		angleDelta *= dt * PREVIEW_MOUSE_SENSETIVITY;
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
	glm::vec3 forwardDir;
	glm::vec3 rightDir;

	Directions(m_cameraAngle, &forwardDir, &rightDir);

	float forward = io.KeysDown[GLFW_KEY_W] - (int)io.KeysDown[GLFW_KEY_S];
	float right = io.KeysDown[GLFW_KEY_D] - (int)io.KeysDown[GLFW_KEY_A];
	float up = io.KeysDown[GLFW_KEY_SPACE] - (int)io.KeysDown[GLFW_KEY_LEFT_CONTROL];

	glm::vec3 moveDelta(0, 0, 0);

	float magnitude = sqrt(forward * forward + right * right);

	// If our magnitude is 0, we're not moving forward or right
	if (magnitude != 0)
	{
		// We need to make sure we maintain a magnitude of 1
		moveDelta += forwardDir * forward / magnitude;
		moveDelta += rightDir * right / magnitude;
	}

	moveDelta.y += up;

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

