#pragma once
#include <glm/mat4x4.hpp>

struct face_t;
class CDoodle
{
public:
	CDoodle();
	~CDoodle();

	void Cube(glm::mat4 mtx);
	void Plane(glm::mat4 mtx);
	void Plane(glm::vec3 origin, glm::vec3 scale, glm::vec3 angle);
	void Line(glm::vec3 start, glm::vec3 end, glm::vec3 color = {1,0,1}, float width = 0.5f);
	void Face(face_t* face, glm::vec3 color = { 1,0,1 }, float width = 0.5f);
};


CDoodle& Doodle();