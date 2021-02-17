#pragma once

#include <bgfx/bgfx.h>
#include <map>
#include <string>
#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

// Is this class over engineered?
class CModelTransform
{
public:
	CModelTransform();

	void SetParent(CModelTransform* parent);

	// Local to parent
	void SetLocalAngles(glm::vec3 ang) { m_angles = ang; }
	void SetLocalOrigin(glm::vec3 pos) { m_position = pos; }
	void SetLocalScale(glm::vec3 scale) { m_scale = scale; }

	glm::vec3 GetLocalAngles() { return m_angles; }
	glm::vec3 GetLocalOrigin() { return m_position; }
	glm::vec3 GetLocalScale()  { return m_scale; }

	// Absolute position in the world
	void SetAbsAngles(glm::vec3 ang);
	void SetAbsOrigin(glm::vec3 pos);
	void SetAbsScale(glm::vec3 scale);

	glm::vec3 GetAbsAngles();
	glm::vec3 GetAbsOrigin();
	glm::vec3 GetAbsScale();

	glm::mat4 Matrix();

private:

	glm::vec3 m_angles;
	glm::vec3 m_position;
	glm::vec3 m_scale;

	CModelTransform* m_pParent;
};


class IModel
{
public:
	virtual ~IModel() {};
	virtual void Render(glm::vec3 origin, glm::vec3 angle, glm::vec3 scale) = 0;
	virtual void Render(CModelTransform* transform) = 0;

};

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