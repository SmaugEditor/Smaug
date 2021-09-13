#pragma once
#include <bigg.hpp>

class CBasicDraw;
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

	bool m_mouseLocked;

	CBasicDraw* m_basicdraw;
};

CSmaugApp& GetApp();


enum class ECoordSystem
{
	RIGHT_HANDED = 0,
	LEFT_HANDED = 1
};

// Add more stuff to this if you want!
struct RendererProperties_t
{
	ECoordSystem coordSystem;
	bgfx::RendererType::Enum renderType;
};

void SetRendererType(bgfx::RendererType::Enum type);
bgfx::RendererType::Enum RendererType();
const RendererProperties_t& RendererProperties();