#pragma once

#include <glm/vec3.hpp>
#include <bgfx/bgfx.h>

// This function does not take Y into account
bool IsPointOnLine2D(glm::vec3 point1, glm::vec3 point2, glm::vec3 mouse, float range);

bool IsPointNearPoint2D(glm::vec3 point1, glm::vec3 point2, float range);

bgfx::ProgramHandle LoadShader(const char* fragment, const char* vertex);


void Directions(glm::vec3 angles, glm::vec3* forward = nullptr, glm::vec3* right = nullptr, glm::vec3* up = nullptr);
