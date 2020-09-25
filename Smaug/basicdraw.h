#pragma once
#include <glm/mat4x4.hpp>
#include <bgfx/bgfx.h>

// These do not call submit!
class CBasicDraw
{
public:
	CBasicDraw();

	void Cube(glm::mat4 mtx);
	void Plane(glm::mat4 mtx);
private:

	bgfx::VertexBufferHandle m_cubeVertexBuf;
	bgfx::IndexBufferHandle m_cubeIndexBuf;

	bgfx::VertexBufferHandle m_planeVertexBuf;
	bgfx::IndexBufferHandle m_planeIndexBuf;
};


CBasicDraw& BasicDraw();