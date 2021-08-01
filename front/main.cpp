#include "smaugapp.h"
#include "utils.h"

#ifdef _WIN32
static constexpr auto DEFAULT_RENDER_TYPE = bgfx::RendererType::Direct3D11;
#else
static constexpr auto DEFAULT_RENDER_TYPE = bgfx::RendererType::OpenGL;
#endif 

int main( int argc, char** argv )
{
	CommandLine::Set(argc, argv);
	
	auto renderType = DEFAULT_RENDER_TYPE;
	if(CommandLine::HasAny("-gl", "-gles"))
		renderType = bgfx::RendererType::OpenGL;
	else if(CommandLine::HasAny("-vulkan", "-vk"))
		renderType = bgfx::RendererType::Vulkan;
	else if(CommandLine::HasAny("-dx9", "-d3d9", "-directx9"))
		renderType = bgfx::RendererType::Direct3D9;
	else if(CommandLine::HasAny("-dx11", "-d3d11", "-directx11"))
		renderType = bgfx::RendererType::Direct3D11;
	SetRendererType(renderType);
	
	return GetApp().run( argc, argv, renderType );
}
