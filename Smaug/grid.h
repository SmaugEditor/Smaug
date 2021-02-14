#pragma once
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <bgfx/bgfx.h>

class CGrid
{
public:
	CGrid();
	void Shutdown();
	void Update();
	void Draw();

	void Draw(glm::vec4 screen); // left, right, bottom, top

	glm::vec3 Snap(glm::vec3 in);

private:
	int m_iScale;
	glm::vec4 m_vecScale;

	glm::vec4 m_screen;

	bgfx::VertexBufferHandle m_planeVertexBuf;
	bgfx::IndexBufferHandle m_planeIndexBuf;
	bgfx::UniformHandle m_gridScale;

};

CGrid& Grid();