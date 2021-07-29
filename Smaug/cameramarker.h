#pragma once
#include <glm/glm.hpp>
#include <editorinterface.h>

// Down the line, it'd be nice if when we have multiple 3d views we can draw all their markers from here
class CCameraMarker
{
public:
	CCameraMarker();
	
	void Draw();
	void SetTransform(glm::vec3 pos, glm::vec3 ang);
private:
	glm::vec3 m_pos;
	glm::vec3 m_ang;
	IModel* m_model;
};

CCameraMarker& CameraMarker();
