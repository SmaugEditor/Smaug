#pragma once
#include <bigg.hpp>
#include "uiview.h"


class CSmaugApp : public bigg::Application
{
public:

	CSmaugApp()
		: bigg::Application("Smaug", 960, 720) {}

	void initialize(int _argc, char** _argv);
	int  shutdown();
	void update(float dt);
	void onReset();

	void SetMouseLock(bool state);
	void ToggleMouseLock();
	
	GLFWwindow* GetWindow() { return mWindow; };


	CUIView m_uiView;
	bool m_mouseLocked;
};

CSmaugApp& GetApp();