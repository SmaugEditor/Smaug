#include "utils.h"

bool IsPointOnLine(glm::vec3 point1, glm::vec3 point2, glm::vec3 mouse, float range)
{
	// Top point is always point1
	if (point2.z > point1.z)
	{
		glm::vec3 temp = point1;
		point1 = point2;
		point2 = temp;
	}
	// If the Zs are equal, then the left point is always point1
	else if (point2.z == point1.z && point2.x < point1.x)
	{
		glm::vec3 temp = point1;
		point1 = point2;
		point2 = temp;
	}

	// Is the line is straight up?
	if (point1.x == point2.x)
	{
		return abs(mouse.x - point1.x) < range // Is within the width bounds?
			&& mouse.z <= point1.z  // Is not higher than
			&& mouse.z >= point2.z; // Is not lower than
	}

	// Is the line straight sideways?
	if (point1.z == point2.z)
	{
		return abs(mouse.z - point1.z) < range // Is within the height bounds?
			&& mouse.x >= point1.x  // Is not left of
			&& mouse.x <= point2.x; // Is not right of
	}

	float slope = (point1.z - point2.z) / (point1.x - point2.x);
	float perp = 1 / slope;

	glm::vec3 topLeft = glm::vec3(point1.x - range, 0, point1.z - range * perp);

	glm::vec3 bottomRight = glm::vec3(point2.x + range, 0, point2.z + range * perp);

	// Right 
	if ((mouse.x - bottomRight.x) * slope + bottomRight.z > mouse.z)
		return false;

	// Left
	if ((mouse.x - topLeft.x) * slope + topLeft.z < mouse.z)
		return false;

	// Top
	if ((mouse.x - topLeft.x) * perp + topLeft.z < mouse.z)
		return false;

	// Bottom
	if ((mouse.x - bottomRight.x) * perp + bottomRight.z > mouse.z)
		return false;

	return true;



}
