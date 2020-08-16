#include "smaugapp.h"
#include "worldrenderer.h"
#include "shadermanager.h"

void CSmaugApp::initialize(int _argc, char** _argv)
{
	ShaderManager::Init();

	ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;
	GetWorldRenderer().Init();

	m_uiView.Init(ViewID::MAIN_VIEW, 1024, 1024, 0x404040FF);
}

int CSmaugApp::shutdown()
{
	ShaderManager::Shutdown();
	return 0;
}

void CSmaugApp::onReset()
{
	bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
	bgfx::setViewRect(0, 0, 0, uint16_t(getWidth()), uint16_t(getHeight()));
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
