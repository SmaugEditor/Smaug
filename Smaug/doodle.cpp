#include "doodle.h"
#include "utils.h"
#include "mesh.h"

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/geometric.hpp>
#include <editorinterface.h>

CDoodle::CDoodle()
{
	
}

CDoodle::~CDoodle()
{
	
}


void CDoodle::Cube(glm::mat4 mtx)
{

}

void CDoodle::Plane(glm::mat4 mtx)
{
	
}

void CDoodle::Plane(glm::vec3 origin, glm::vec3 scale, glm::vec3 angle)
{
	
}

void CDoodle::Line(glm::vec3 start, glm::vec3 end, glm::vec3 color, float width)
{
	EngineInterface()->DrawLine(start, end, width, color);
}

void CDoodle::Face(face_t* face, glm::vec3 color, float width)
{
	glm::vec3 offset = parentMesh(face)->origin;

	vertex_t* cur = face->verts.front(), * end = cur;
	do
	{
		vertex_t* next = cur->edge->vert;
		Line(*cur->vert + offset, *next->vert + offset, color, width);
		cur = next;
	} while (cur != end);
}

CDoodle& Doodle()
{
	static CDoodle s_basicDraw;
	return s_basicDraw;
}

