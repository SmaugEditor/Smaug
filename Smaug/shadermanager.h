#pragma once
#include "bgfx/bgfx.h"

// Probably a WAAAAAY better way to do this.

enum class Shader
{
	NONE,
	
	WORLD_PREVIEW_SHADER,
	EDIT_VIEW_SHADER,
	CURSOR_SHADER,

	COUNT,
};

namespace ShaderManager
{
	void Init();
	void Shutdown();

	bgfx::ProgramHandle GetShaderProgram(Shader shader);
};

