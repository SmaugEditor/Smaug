#include "smaugapp.h"
#include "worldrenderer.h"
#include "shadermanager.h"

void CSmaugApp::initialize(int _argc, char** _argv)
{
	ShaderManager::Init();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigWindowsMoveFromTitleBarOnly = true;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigDockingWithShift = true;

	GetWorldRenderer().Init();

	m_uiView.Init(ViewID::MAIN_VIEW, 1024, 1024, 0x404040FF);

	m_mouseLocked = false;
	mFpsLock = 120;
}

int CSmaugApp::shutdown()
{
	ShaderManager::Shutdown();
	return 0;
}

void CSmaugApp::onReset()
{
	uint32_t width = getWidth();
	uint32_t height = getHeight();

	m_uiView.m_width = width;
	m_uiView.m_height = height;

	bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
	bgfx::setViewRect(0, 0, 0, uint16_t(width), uint16_t(height));
}

void CSmaugApp::SetMouseLock(bool state)
{
	m_mouseLocked = state;
	if (state)
	{
		glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	else
	{
		glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void CSmaugApp::ToggleMouseLock()
{
	SetMouseLock(!m_mouseLocked);
}


void CSmaugApp::update(float dt)
{
	m_uiView.Update(dt,0,0);
	m_uiView.Draw(dt);
}


CSmaugApp& GetApp()
{
	static CSmaugApp app;
	return app;
}
