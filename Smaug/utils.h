#pragma once

#include <glm/vec3.hpp>
#include <bgfx/bgfx.h>

// This function does not take Y into account
bool IsPointOnLine(glm::vec3 point1, glm::vec3 point2, glm::vec3 mouse, float range);

bgfx::ProgramHandle LoadShader(const char* fragment, const char* vertex);