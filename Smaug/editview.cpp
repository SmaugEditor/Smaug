#include "editview.h"

#include "smaugapp.h"

#include <bigg.hpp>
#include <bx/string.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp> 
#include "worldeditor.h"
#include "worldrenderer.h"
#include "utils.h"

/*
 * Snagged from Bigg's examples
 *
 * Simple rotating cubes example, based off bgfx's example-01-cubes.
 * Does not mimic the original example completely because we're in a
 * different coordinate system.
 *
 * Copyright 2011-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

static struct PosColorVertex
{
	float x;
	float y;
	float z;
	uint32_t abgr;
	static void init()
	{
		ms_layout
			.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
			.end();
	}
	static bgfx::VertexLayout ms_layout;
};
bgfx::VertexLayout PosColorVertex::ms_layout;

static PosColorVertex s_cubeVertices[] =
{
	{-1.0f,  1.0f,  1.0f, 0xff000000 },
	{ 1.0f,  1.0f,  1.0f, 0xff0000ff },
	{-1.0f, -1.0f,  1.0f, 0xff00ff00 },
	{ 1.0f, -1.0f,  1.0f, 0xff00ffff },
	{-1.0f,  1.0f, -1.0f, 0xffff0000 },
	{ 1.0f,  1.0f, -1.0f, 0xffff00ff },
	{-1.0f, -1.0f, -1.0f, 0xffffff00 },
	{ 1.0f, -1.0f, -1.0f, 0xffffffff },
};
static const uint16_t s_cubeTriList[] = { 2, 1, 0, 2, 3, 1, 5, 6, 4, 7, 6, 5, 4, 2, 0, 6, 2, 4, 3, 5, 1, 3, 7, 5, 1, 4, 0, 1, 5, 4, 6, 3, 2, 7, 3, 6 };

#define SCROLL_SPEED 5

void CEditView::Init(bgfx::ViewId viewId, int width, int height, uint32_t clearColor)
{
	CBaseView::Init(viewId, width, height, clearColor);

	PosColorVertex::init();

	char vsName[32];
	char fsName[32];

	const char* shaderPath = "???";

	switch (bgfx::getRendererType())
	{
	case bgfx::RendererType::Noop:
	case bgfx::RendererType::Direct3D9:  shaderPath = "shaders/dx9/";   break;
	case bgfx::RendererType::Direct3D11:
	case bgfx::RendererType::Direct3D12: shaderPath = "shaders/dx11/";  break;
	case bgfx::RendererType::Gnm:                                       break;
	case bgfx::RendererType::Metal:      shaderPath = "shaders/metal/"; break;
	case bgfx::RendererType::OpenGL:     shaderPath = "shaders/glsl/";  break;
	case bgfx::RendererType::OpenGLES:   shaderPath = "shaders/essl/";  break;
	case bgfx::RendererType::Vulkan:     shaderPath = "shaders/spirv/"; break;
	case bgfx::RendererType::Count:                                     break;
	}

	bx::strCopy(vsName, BX_COUNTOF(vsName), shaderPath);
	bx::strCat(vsName, BX_COUNTOF(vsName), "vs_cubes.bin");

	bx::strCopy(fsName, BX_COUNTOF(fsName), shaderPath);
	bx::strCat(fsName, BX_COUNTOF(fsName), "fs_cubes.bin");

	m_hShaderProgram = bigg::loadProgram(vsName, fsName);
	m_hVertexBuf = bgfx::createVertexBuffer(bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices)), PosColorVertex::ms_layout);
	m_hIndexBuf = bgfx::createIndexBuffer(bgfx::makeRef(s_cubeTriList, sizeof(s_cubeTriList)));
	

	// Our Z is upside-down for some reason. Fix it?

	CTriNode* tri = GetWorldEditor().CreateTri();
	tri->m_origin = glm::vec3(0, 0, -20);

	for (int i = 0; i < tri->m_sideCount; i++)
		tri->m_sides[i].vertex1->origin *= 10;

	// Zoom and pan
	m_viewportHeight = m_viewportWidth = 80;
	m_cameraPos = glm::vec3(0, 10, 0);

	m_cursorSpin = 1;
	m_actionData.actionMode = ActionMode::NONE;
}

float t = 0;

void CEditView::Update(float dt, float mx, float my)
{
	float scrollDelta = ImGui::GetIO().MouseWheel * SCROLL_SPEED;
	m_viewportWidth  -= scrollDelta;
	m_viewportHeight -= scrollDelta;
	// Offset the cam with the scrollDelta!

	// Put the mouse pos into the world
	mx = (mx * 2 - 1) * m_viewportWidth - m_cameraPos.x;
	my = (my * 2 - 1) * m_viewportHeight - m_cameraPos.z;

	glm::mat4 mtx;

	m_mousePos = glm::vec3(mx, 5, my);

	m_cursorSpin = 1;

	CSmaugApp& app = GetApp();

	if (m_actionData.actionMode == ActionMode::NONE)
	{
		// Should we pan the view?
		if (app.isMouseButtonDown(GLFW_MOUSE_BUTTON_3))
		{
			m_actionData.mouseStartPos = m_mousePos;
			m_actionData.actionMode = ActionMode::PAN_VIEW;
			return;
		}

		m_actionData.selectedSide = nullptr;
		m_actionData.selectedNode = nullptr;
		bool hasFoundItem = false;
		m_actionData.cursorPos = m_mousePos;

		// Find the selected item
		for (int i = 0; i < GetWorldEditor().m_nodes.size(); i++)
		{
			CNode* node = GetWorldEditor().m_nodes[i];

			for (int j = 0; j < node->m_sideCount; j++)
			{
				if (IsPointOnLine(node->m_sides[j].vertex1->origin + node->m_origin, node->m_sides[j].vertex2->origin + node->m_origin, m_mousePos, 2))
				{
					// Spin the cursor backwards if we're selecting something
					m_cursorSpin = -1;

					m_actionData.selectedSide = &node->m_sides[j];
					m_actionData.selectedNode = node;

					m_actionData.cursorPos = (m_actionData.selectedSide->vertex1->origin + m_actionData.selectedSide->vertex2->origin) / 2.0f + node->m_origin;
					
					hasFoundItem = true;

					break;
				}
			}
			if (hasFoundItem)
				break;
		}


		// Did we find a side?
		if (m_actionData.selectedSide)
		{
			// Should we extrude the selected side?
			if (app.isMouseButtonDown(GLFW_MOUSE_BUTTON_1) && app.isKeyDown(GLFW_KEY_LEFT_CONTROL))
			{
				m_actionData.mouseStartPos = m_mousePos;
				m_actionData.actionMode = ActionMode::EXTRUDE_SIDE;
			}
			// Should we extend the selected side?
			else if (app.isMouseButtonDown(GLFW_MOUSE_BUTTON_1))
			{
				m_actionData.mouseStartPos = m_mousePos;
				m_actionData.actionMode = ActionMode::DRAG_SIDE;
			}

		}
		

	}
	else if(m_actionData.actionMode == ActionMode::PAN_VIEW)
	{
		if (app.isMouseButtonDown(GLFW_MOUSE_BUTTON_3))
		{
		}
		else
		{
			m_cameraPos += m_mousePos - m_actionData.mouseStartPos;
			m_actionData.mouseStartPos = m_mousePos;
			m_actionData.actionMode = ActionMode::NONE;
		}
	}
	else if (m_actionData.actionMode == ActionMode::DRAG_SIDE)
	{
		if (app.isMouseButtonDown(GLFW_MOUSE_BUTTON_1))
		{
		}
		else
		{
			glm::vec3 mouseDelta = m_mousePos - m_actionData.mouseStartPos;

			m_actionData.selectedSide->vertex1->origin += mouseDelta;
			m_actionData.selectedSide->vertex2->origin += mouseDelta;
			m_actionData.selectedNode->Update();

			printf("Stretched Node\n");
			
			m_actionData.selectedNode = nullptr;
			m_actionData.selectedSide = nullptr;
			m_actionData.actionMode = ActionMode::NONE;
		}
	}
	else if (m_actionData.actionMode == ActionMode::EXTRUDE_SIDE)
	{
		if (app.isMouseButtonDown(GLFW_MOUSE_BUTTON_1) && app.isKeyDown(GLFW_KEY_LEFT_CONTROL))
		{
		}
		else
		{
			glm::vec3 mouseDelta = m_mousePos - m_actionData.mouseStartPos;

			CQuadNode* quad = GetWorldEditor().CreateQuad();
			quad->m_origin = m_actionData.selectedNode->m_origin;

			// If we don't flip vertex 1 and 2 here, the tri gets messed up and wont render.

			// 2 to 1 and 1 to 2
			// 
			// 2-----1
			// |     |
			// 1 --- 2

			quad->m_sides[0].vertex1->origin = m_actionData.selectedSide->vertex2->origin;
			quad->m_sides[0].vertex2->origin = m_actionData.selectedSide->vertex1->origin;

			quad->m_sides[2].vertex1->origin = m_actionData.selectedSide->vertex1->origin + mouseDelta;
			quad->m_sides[2].vertex2->origin = m_actionData.selectedSide->vertex2->origin + mouseDelta;

			quad->Update();

			printf("Created New Node\n");

			m_actionData.selectedNode = nullptr;
			m_actionData.selectedSide = nullptr;
			m_actionData.actionMode = ActionMode::NONE;

		}
	}



}

void CEditView::Draw(float dt)
{
	t += dt * m_cursorSpin;
	CBaseView::Draw(dt);


	// Camera
	glm::mat4 view = glm::mat4(1.0f);
	view = glm::rotate(view, glm::radians(90.0f), glm::vec3(1, 0, 0)); // Look down
	view = glm::translate(view, m_cameraPos); // Set our pos to be 10 units up
	glm::mat4 proj = glm::ortho(-m_viewportWidth, m_viewportWidth, -m_viewportHeight, m_viewportHeight, -900.0f, 800.0f);
	bgfx::setViewTransform(m_viewId, &view[0][0], &proj[0][0]);


	GetWorldRenderer().Draw(m_viewId, m_hShaderProgram);


	for (int i = 0; i < GetWorldEditor().m_nodes.size(); i++)
	{
		glm::mat4 mtx = glm::identity<glm::mat4>();
		mtx = glm::translate(mtx, GetWorldEditor().m_nodes[i]->m_origin);
		mtx = glm::scale(mtx, glm::vec3(2.5f, 2.5f, 2.5f));
		mtx *= glm::yawPitchRoll(1.37f * t, t, 0.0f);
		bgfx::setTransform(&mtx[0][0]);
		bgfx::setVertexBuffer(0, m_hVertexBuf);
		bgfx::setIndexBuffer(m_hIndexBuf);
		bgfx::setState(BGFX_STATE_DEFAULT);
		bgfx::submit(m_viewId, m_hShaderProgram);
	}


	// Cursor
	glm::mat4 mtx = glm::identity<glm::mat4>();
	mtx = glm::translate(mtx, m_actionData.cursorPos);
	mtx = glm::scale(mtx, glm::vec3(2.5f, 2.5f, 2.5f));
	mtx *= glm::yawPitchRoll(1.37f * t, t, 0.0f);
	bgfx::setTransform(&mtx[0][0]);
	bgfx::setVertexBuffer(0, m_hVertexBuf);
	bgfx::setIndexBuffer(m_hIndexBuf);
	bgfx::setState(BGFX_STATE_DEFAULT);
	bgfx::submit(m_viewId, m_hShaderProgram);

}