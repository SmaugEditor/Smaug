#include "editorinterface.h"
#include <worldsave.h>
#include <uiview.h>
#include <imgui.h>

class CEditorInterface : public IEditorInterface
{
public:
	CEditorInterface() {}
	virtual void Startup(ImGuiContext* ctx, ImGuiMemAllocFunc allocfn, ImGuiMemFreeFunc freefn, void* userdata)
	{
		ImGui::SetCurrentContext(ctx);
		ImGui::SetAllocatorFunctions(allocfn, freefn, userdata);

		defaultWorld();
		m_lastUpdateTime = EngineInterface()->Time();
		AppUI().Init(0, 1024, 1024, 0xFF0000FF);
	}

	virtual void Shutdown()
	{
	}

	virtual void Update()
	{
		float curTime = EngineInterface()->Time();
		m_deltaTime = curTime - m_lastUpdateTime;
		m_lastUpdateTime = curTime;

		AppUI().Update(m_deltaTime, 0, 0);
	}
	virtual void Draw()
	{
		AppUI().Draw(m_deltaTime);
	}
	virtual void SetInput(input_t in, int state)
	{
		Input().SetInput(in, state);
	}

	float m_lastUpdateTime = 0;
	float m_deltaTime = 0;

	// Inherited via IEditorInterface
	virtual void EndFrame() override
	{
		Input().EndFrame();
	}
};


static IEngineInterface* s_EngineInterface;

IEngineInterface* EngineInterface()
{
	return s_EngineInterface;
}

IEditorInterface* SupplyEngineInterface(IEngineInterface* ei)
{
	s_EngineInterface = ei;
	SupplyWorldInterface(ei);

	return new CEditorInterface;
}
