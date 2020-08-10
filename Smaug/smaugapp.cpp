#include "smaugapp.h"
#include "worldrenderer.h"

void CSmaugApp::initialize(int _argc, char** _argv)
{
	ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;
	GetWorldRenderer().Init();

	m_uiView.Init(ViewID::MAIN_VIEW, 1024, 1024, 0x404040FF);
}

void CSmaugApp::onReset()
{
	bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
	bgfx::setViewRect(0, 0, 0, uint16_t(getWidth()), uint16_t(getHeight()));
}

void CSmaugApp::update(float dt)
{
	m_uiView.Update(dt);
}


CSmaugApp& GetApp()
{
	static CSmaugApp app;
	return app;
}





