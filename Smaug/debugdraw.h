#pragma once

#include <glm/vec3.hpp>
#include <vector>

class CDebugDraw
{
public:
	class ITempItem;

	void Line(glm::vec3 start, glm::vec3 end, glm::vec3 color = {1.0f,0.0f,1.0f}, float width = 1.0f, float decay = 5.0f);
//	void Cube();
	void Draw();
private:
	std::vector<ITempItem*> m_itemsToDraw;
};


CDebugDraw& DebugDraw();