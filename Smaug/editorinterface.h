#pragma once
#include "worldinterface.h"
#include "transform.h"
#include "input.h"
#include <imgui.h>

class IEngineInterface;
class IEditorInterface;

IEditorInterface* SupplyEngineInterface(IEngineInterface* ei);

class IEditorInterface
{
public:

	// Startup and shutdown Smaug
	virtual void Startup(ImGuiContext* ctx, ImGuiMemAllocFunc allocfn, ImGuiMemFreeFunc freefn, void* userdata) = 0;
	virtual void Shutdown() = 0;

	// Start the frame with an update, then draw, then end it
	virtual void Update() = 0;
	virtual void Draw() = 0;
	virtual void EndFrame() = 0;

	// Send in key and mouse inputs
	// Do this before you call update!
	// Should be updated regardless of whatever Dear ImGui's up to
	virtual void SetInput(input_t in, int state) = 0;
};


class IModel
{
public:
	virtual ~IModel() {};
	virtual void Draw(glm::vec3 origin, glm::vec3 angle, glm::vec3 scale) = 0;
	virtual void Draw(CTransform* transform) = 0;

};

typedef void* renderTarget_t;

// To be by the editor implementer
// Covers world interface as well
class IEngineInterface : public IWorldInterface
{
public:
	
	// Current time in seconds
	virtual float Time() = 0;

	// Read content from path
	// The editor will free the memory once it is done with it
	virtual char* ReadFile(const char* path, size_t& fileLen) = 0;
	
	// Save contents of buf to path for size len
	virtual void SaveFile(const char* path, char* buf, size_t len) = 0;
	
	// Will dump into pathOut the path it got from the prompt
	// Return null on cancel
	// If pathOut is null or destLen is 0, do not copy into pathOut
	virtual char* ReadFilePrompt(size_t& fileLen, char* pathOut, size_t destLen) = 0;
	
	// Saves file to user requested location via file prompt
	// Returns false on cancel
	virtual bool SaveFilePrompt(char* buf, size_t len) = 0;

	// Draw a line to the current view
	// If your engine does not currently support line drawing, I suggest using two intersecting quads
	// Works quite well!
	virtual void DrawLine(glm::vec3 start, glm::vec3 end, float width, glm::vec3 color) = 0;

	// Suggest that this is cached to some degree 
	virtual IModel* LoadModel(const char* path) = 0;

	// Lock the mouse up so we can do 3D fly camera stuff
	virtual void LockMouse(bool locked) = 0;

	// Create a rt for us to draw our different views to
	virtual renderTarget_t CreateRenderTarget(uint32_t width, uint32_t height) = 0;
	virtual texture_t TextureFromRenderTarget(renderTarget_t rt) = 0;

	// color here is RGBA one byte per component
	virtual void ClearColor(renderTarget_t rt, uint32_t color) = 0;

	// We need to be able to render to a texture
	// These two mark when we begin and end rendering to a texture
	virtual void BeginView(renderTarget_t rt) = 0;
	virtual void EndView(renderTarget_t rt) = 0;

	// Set the view and perspective of what we're currently working on
	virtual void SetViewMatrix(glm::mat4& view, glm::mat4& proj) = 0;

	// We let you take care of this, not because I'm lazy, but because I'm sure there's odd engines with weird ways of rendering
	// 3D is for our fly cams and 2D is for our top downs
	virtual void DrawWorld3D() = 0;
	virtual void DrawWorld2D() = 0;

};

IEngineInterface* EngineInterface();

