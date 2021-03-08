#pragma once
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <bgfx/bgfx.h>

class CGrid
{
public:
	CGrid();
	void Shutdown();
	void Update();
	void Draw();

	// Delete offsetHack when we get transparency!!
	void Draw(glm::vec2 screenSize, glm::vec3 pos, glm::vec3 angles, bool submitTo3D, glm::vec3 offsetHack = {0,0,0}); // left, right, bottom, top

	glm::vec3 Snap(glm::vec3 in);

private:
	int m_iScale;
	glm::vec4 m_vecScale;

	glm::vec2 m_screenSize;
	glm::vec3 m_pos;
	glm::vec3 m_vecGridDirMask;
	glm::vec3 m_angles;

	bgfx::VertexBufferHandle m_planeVertexBuf;
	bgfx::IndexBufferHandle m_planeIndexBuf;
	bgfx::UniformHandle m_gridScale;
	// Which axis should have grid?
	bgfx::UniformHandle m_gridDirMask;

};

CGrid& Grid();