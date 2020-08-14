#include "shadermanager.h"
#include "utils.h"

static struct
{
	const char* fragmentShader;
	const char* vertexShader;
	bgfx::ProgramHandle programHandle;
} 
s_programInfo[] = {
	{nullptr, nullptr}, // Shader::NONE

	{"fs_cubes.bin", "vs_cubes.bin"}, // Shader::WORLD_PREVIEW_SHADER
	{"fs_cubes.bin", "vs_cubes.bin"}, // Shader::EDIT_VIEW_SHADER
	{"fs_cubes.bin", "vs_cubes.bin"}, // Shader::CURSOR_SHADER
	
};


void ShaderManager::Init()
{
	for (int i = 1; i < (int)Shader::COUNT; i++)
	{
		s_programInfo[i].programHandle = LoadShader(s_programInfo[i].fragmentShader, s_programInfo[i].vertexShader);
	}
}

void ShaderManager::Shutdown()
{
	for (int i = 1; i < (int)Shader::COUNT; i++)
	{
		bgfx::destroy(s_programInfo[i].programHandle);
		s_programInfo[i].programHandle = BGFX_INVALID_HANDLE;
	}
	
}

bgfx::ProgramHandle ShaderManager::GetShaderProgram(Shader shader)
{
	// Out of range. Return invalid handle
	if (shader <= Shader::NONE || shader > Shader::COUNT)
	{
		return BGFX_INVALID_HANDLE;
	}

	return s_programInfo[(int)shader].programHandle;
}
