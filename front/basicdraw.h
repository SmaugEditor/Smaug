#pragma once
#include "transform.h"
#include <glm/mat4x4.hpp>
#include <bgfx/bgfx.h>

struct face_t;
class CBasicDraw
{
public:
	CBasicDraw();
	~CBasicDraw();

	void Cube(glm::mat4 mtx);
	void Plane(glm::mat4 mtx);
	void Plane(glm::vec3 origin, glm::vec3 scale, glm::vec3 angle);
	void Line(glm::vec3 start, glm::vec3 end, glm::vec3 color = {1,0,1}, float width = 0.5f);
	void Grid(CTransform& transform, int scale);
private:

	bgfx::VertexBufferHandle m_planeVertexBuf;
	bgfx::IndexBufferHandle m_planeIndexBuf;


	bgfx::VertexBufferHandle m_lineVertexBuf;
	bgfx::IndexBufferHandle m_lineIndexBuf;

	bgfx::UniformHandle m_gridScale;
	// Which axis should have grid?
	bgfx::UniformHandle m_gridDirMask;
};


CBasicDraw& BasicDraw();