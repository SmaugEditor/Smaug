#include <bigg.hpp>

#include "uiview.h"

class ExampleCubes : public bigg::Application
{
public:

	void initialize(int _argc, char** _argv)
	{
		ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;
		m_uiView.Init(ViewID::MAIN_VIEW, 1024, 1024, 0x404040FF);

	}

	void onReset()
	{
		bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
		bgfx::setViewRect(0, 0, 0, uint16_t(getWidth()), uint16_t(getHeight()));
	}

	void update(float dt)
	{
		m_uiView.Update(dt);
	}

public:
	ExampleCubes()
		: bigg::Application("Cubes", 960, 720) {}
private:
	CUIView m_uiView;


};

ExampleCubes& GetApp()
{
	static ExampleCubes app;
	return app;
}









int main( int argc, char** argv )
{
	return GetApp().run( argc, argv );
}
