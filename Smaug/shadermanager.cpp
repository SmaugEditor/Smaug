#include "shadermanager.h"
#include "utils.h"
#include <glm/vec4.hpp>
#include <cstdio>
#include <filesystem>
#include <bigg.hpp>

static struct
{
	const char* fragmentShader;
	const char* vertexShader;
	bgfx::ProgramHandle programHandle;
} 
s_programInfo[] = {
	{nullptr, nullptr}, // Shader::NONE

	{"fs_preview_view.bin", "vs_preview_view.bin"}, // Shader::WORLD_PREVIEW_SHADER
	{"fs_model.bin",        "vs_model.bin"}, // Shader::EDIT_VIEW_SHADER
	{"fs_model.bin",        "vs_model.bin"}, // Shader::CURSOR_SHADER
	{"fs_model.bin",        "vs_model.bin"}, // Shader::MODEL_SHADER
	{"fs_line.bin",         "vs_line.bin" }, // Shader::ERROR_MODEL_SHADER
	{"fs_grid.bin",         "vs_grid.bin" }, // Shader::GRID
	{"fs_line.bin",         "vs_line.bin" }, // Shader::LINE
	
};


bgfx::ProgramHandle LoadShader(const char* fragment, const char* vertex, bgfx::ProgramHandle fallback)
{
	// Static so we don't reinitiate this
	static char const* const shaderPaths[bgfx::RendererType::Count] =
	{
		nullptr,		  // No Renderer
		"shaders/dx9/",   // Direct3D9
		"shaders/dx11/",  // Direct3D11
		"shaders/dx11/",  // Direct3D12
		nullptr,		  // Gnm
		nullptr,		  // Metal
		nullptr,		  // Nvm
		"shaders/essl/",  // OpenGL ES
		"shaders/glsl/",  // OpenGL
		"shaders/spirv/", // Vulkan
		nullptr,		  // WebGPU
	};

	int type = bgfx::getRendererType();
	
	// Out of bounds. Return an invalid handle
	if (type >= bgfx::RendererType::Count || type < 0)
	{
		return BGFX_INVALID_HANDLE;
	}

	const char* shaderPath = shaderPaths[bgfx::getRendererType()];

	// Unsupported platform. Return invalid handle
	if (!shaderPath)
	{
		return BGFX_INVALID_HANDLE;
	}

	char vsPath[128] = "";
	strncat(vsPath, shaderPath, sizeof(vsPath));
	strncat(vsPath, vertex, sizeof(vsPath));

	if (!std::filesystem::exists(vsPath))
	{
		Log::Fault("[ShaderManager::LoadShader] Vertex shader %s not found\n", vsPath);
		return fallback;
	}

	char fsPath[128] = "";
	strncat(fsPath, shaderPath, sizeof(fsPath));
	strncat(fsPath, fragment, sizeof(fsPath));

	if (!std::filesystem::exists(fsPath))
	{
		Log::Fault("[ShaderManager::LoadShader] Fragment shader %s not found\n", fsPath);
		return fallback;
	}

	Log::Print("[ShaderManager::LoadShader] Loading %s and %s\n", vsPath, fsPath);

	return bigg::loadProgram(vsPath, fsPath);
}


void CShaderManager::Init()
{
	for (int i = 1; i < (int)Shader::COUNT; i++)
	{
		
		s_programInfo[i].programHandle = LoadShader(s_programInfo[i].fragmentShader, s_programInfo[i].vertexShader, BGFX_INVALID_HANDLE);
		
		if(s_programInfo[i].programHandle.idx == bgfx::kInvalidHandle)
		{
			Log::TFault("Failed to load program: fragment: %-20s  vertex: %-20s\n", s_programInfo[i].fragmentShader,
				s_programInfo[i].vertexShader);
		}
		
	}


	m_colorUniform = bgfx::createUniform("color", bgfx::UniformType::Vec4);
	m_textureUniform = bgfx::createUniform("u_texture", bgfx::UniformType::Sampler);
}

void CShaderManager::Shutdown()
{
	for (int i = 1; i < (int)Shader::COUNT; i++)
	{
		if (s_programInfo[i].programHandle.idx != bgfx::kInvalidHandle)
		{
			bgfx::destroy(s_programInfo[i].programHandle);
			s_programInfo[i].programHandle = BGFX_INVALID_HANDLE;
		}
	}

	bgfx::destroy(m_colorUniform);
	m_colorUniform = BGFX_INVALID_HANDLE;

	bgfx::destroy(m_textureUniform);
	m_textureUniform = BGFX_INVALID_HANDLE;


}

void CShaderManager::SetColor(glm::vec4 color)
{
	bgfx::setUniform(m_colorUniform, &color);
}

void CShaderManager::SetTexture(texture_t texture)
{
	if (texture != bgfx::kInvalidHandle)
		bgfx::setTexture(0, m_textureUniform, { texture });
	else
		bgfx::setTexture(0, m_textureUniform, { TextureManager().ErrorTexture() });
}

bgfx::ProgramHandle CShaderManager::GetShaderProgram(Shader shader)
{
	// Out of range. Return invalid handle
	if (shader <= Shader::NONE || shader > Shader::COUNT)
	{
		return BGFX_INVALID_HANDLE;
	}

	return s_programInfo[(int)shader].programHandle;
}

CShaderManager& ShaderManager()
{
	static CShaderManager s_ShaderManager;
	return s_ShaderManager;
}
