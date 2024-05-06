#include "smaugapp.h"
#include "worldrenderer.h"
#include "shadermanager.h"
#include "interfaceimpl.h"
#include "basicdraw.h"


IEditorInterface* s_editorInterface;


void smaugKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	ImGuiIO& io = ImGui::GetIO();
	if (key >= 0 && key < IM_ARRAYSIZE(io.KeysDown))
		io.KeysDown[key] = action != GLFW_RELEASE;

	io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
	io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
	io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
	io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];

	s_editorInterface->SetInput({key, false}, action);
}

void smaugMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	ImGuiIO& io = ImGui::GetIO();
	if (button >= 0 && button < IM_ARRAYSIZE(io.MouseDown))
		io.MouseDown[button] = action != GLFW_RELEASE;

	s_editorInterface->SetInput({ button, true }, action);
}

void CSmaugApp::initialize(int _argc, char** _argv)
{
	// Must be made before the shadermanager inits for the uniforms to be created! 
	m_basicdraw = new CBasicDraw();
	ShaderManager().Init();


	ImGuiIO& io = ImGui::GetIO();
	io.ConfigWindowsMoveFromTitleBarOnly = true;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//io.ConfigDockingWithShift = true;

	s_editorInterface = SupplyEngineInterface(new CEngineInterface);

	GetWorldRenderer().Init();

	ImGuiMemAllocFunc allocfn;
	ImGuiMemFreeFunc freefn;
	void* userdata;
	ImGui::GetAllocatorFunctions(&allocfn, &freefn, &userdata);
	s_editorInterface->Startup(ImGui::GetCurrentContext(), allocfn, freefn, userdata);


	m_mouseLocked = false;
	mFpsLock = 120;

	// Bigg's gonna redirect our input to imgui, and we can't have that
	// Let's take it over ourselves
	glfwSetKeyCallback(mWindow, smaugKeyCallback);
	glfwSetMouseButtonCallback(mWindow, smaugMouseButtonCallback);
}

int CSmaugApp::shutdown()
{
	s_editorInterface->Shutdown();

	delete m_basicdraw;
	ShaderManager().Shutdown();
	return 0;
}

void CSmaugApp::onReset()
{
	uint32_t width = getWidth();
	uint32_t height = getHeight();

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
	bgfx::setViewRect(0, 0, 0, getWidth(), getHeight());
	bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
	bgfx::touch(0);

	s_editorInterface->Update();
	s_editorInterface->Draw();
	s_editorInterface->EndFrame();
}


CSmaugApp& GetApp()
{
	static CSmaugApp app;
	return app;
}

// Keep this private
RendererProperties_t gRenderProps;

void SetRendererType(bgfx::RendererType::Enum type)
{
	auto old = gRenderProps.renderType;
	gRenderProps.renderType = type;

	// Init the rest of the fields 
	if (old != type)
	{
		using RT = bgfx::RendererType::Enum;
		switch (type)
		{
		case RT::Direct3D11:
		case RT::Direct3D12:
		case RT::Direct3D9:
		case RT::Vulkan:
		case RT::Metal:
			gRenderProps.coordSystem = ECoordSystem::LEFT_HANDED;
			break;
		default:
			gRenderProps.coordSystem = ECoordSystem::RIGHT_HANDED;
		}
	}

}

bgfx::RendererType::Enum RendererType()
{
	return gRenderProps.renderType;
}

const RendererProperties_t& RendererProperties()
{
	return gRenderProps;
}