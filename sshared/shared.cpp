#include "shared.h"
#include <glm/geometric.hpp>


void Directions(glm::vec3 angles, glm::vec3* forward, glm::vec3* right, glm::vec3* up)
{
	const float pitch = angles.x;
	const float yaw = angles.y;
	const float roll = -angles.z;

	glm::vec3 _forward;
	_forward.x = sin(yaw) * cos(pitch);
	_forward.y = -sin(pitch);
	_forward.z = cos(yaw) * cos(pitch);
	_forward = glm::normalize(_forward);

	if (forward)
		*forward = _forward;


	glm::vec3 _right;
	_right.x = cos(roll) * cos(yaw);
	_right.y = sin(roll);
	_right.z = -cos(roll) * sin(yaw);
	_right = glm::normalize(_right);


	if (right)
		*right = _right;

	if (up)
		*up = -glm::normalize(glm::cross(_right, _forward));
	/*
		if (up)
		{
			glm::vec3 _up;
			_up.x = cos(-(roll + yaw)) * sin(pitch);
			_up.y = cos(-roll) * cos(pitch);
			_up.z = sin(-roll) * cos(yaw) * ((sin(pitch) + 1)/2);
			_up = glm::normalize(_up);

			*up = _up;
		}
		*/
}

glm::vec3 Angles(glm::vec3 forward, glm::vec3* up)
{
	glm::vec3 right = glm::cross(forward, up ? *up : glm::vec3(0, 1, 0));

	forward = glm::normalize(forward);
	float pitch = asin(-forward.y);
	float roll = asin(right.y);

	float cosPitch = cos(pitch);
	float cosRoll = cos(roll);
	float yaw = cosPitch != 0.0f ? acos(forward.z / cos(pitch)) : (cosRoll != 0.0f ? acos(-right.x / cosRoll) : 0.0f);

	if (forward.x < 0) yaw *= -1;



	return { pitch, yaw, roll };
}
