#pragma once

#include "editorinterface.h"

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

#include <map>
#include <string>
#include <vector>


class CModelManager
{
public:
	CModelManager();
	IModel* LoadModel(const char* path);
	void Shutdown();
	IModel* ErrorModel();
	void SetView(uint16_t view) { m_currentView = view; }
	uint16_t CurrentView() { return m_currentView; }

private:
	// We map paths to textures as we load them so that we don't reload them later
	std::map<std::string, IModel*> m_modelMap;
	uint16_t m_currentView;
};

CModelManager& ModelManager();