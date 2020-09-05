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

	// TODO: Check why this feels so funny
	glm::mat4 view = glm::identity<glm::mat4>();// glm::lookAt(glm::vec3(10.0f, 85.0f, 40.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	view *= glm::orientate4(m_cameraAngle);
	view = glm::translate(view, m_cameraPos);

	glm::mat4 proj = glm::perspective(glm::radians(60.0f), float(m_width) / m_height, 0.1f, 800.0f);
	bgfx::setViewTransform(m_viewId, &view[0][0], &proj[0][0]);

	GetWorldRenderer().Draw3D(m_viewId, Shader::WORLD_PREVIEW_SHADER);
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
		glm::vec3 angleDelta = glm::vec3(io.MouseDelta.y, 0, io.MouseDelta.x);
		angleDelta *= dt * PREVIEW_MOUSE_SENSETIVITY;
		m_cameraAngle += angleDelta;
	
		// TODO: Clamp this angle!
	}
	
	// We can always control the position
	glm::vec3 moveDelta(0,0,0);
	moveDelta.x = (int)io.KeysDown[GLFW_KEY_D] - (int)io.KeysDown[GLFW_KEY_A];
	moveDelta.z = (int)io.KeysDown[GLFW_KEY_W] - (int)io.KeysDown[GLFW_KEY_S];
	moveDelta.y += (int)io.KeysDown[GLFW_KEY_SPACE] - (int)io.KeysDown[GLFW_KEY_LEFT_CONTROL];
	printf("moveDelta %f %f %f\n", moveDelta.x, moveDelta.y, moveDelta.z);
	moveDelta *= PREVIEW_MOVE_SPEED * dt;
	
	// TODO: Make the move based on where we're looking!

	m_cameraPos += moveDelta;


}
