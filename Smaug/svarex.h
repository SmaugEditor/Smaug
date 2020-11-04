#pragma once
#include "svar.h"

#include "glm/vec3.hpp"

// Extra types for things like glm::vec3


// Check how long floats can be at some point
BEGIN_SVAR_TYPE_IMPLEMENT(glm::vec3)
	IMPL_FORMAT_TOSTRING_EX(128, "%f %f %f", m_data.x, m_data.y, m_data.z)
	virtual void FromString(char* str)
	{
		char* endptr;
		m_data.x = strtof(str, &endptr);
		m_data.y = strtof(endptr, &endptr);
		m_data.z = strtof(endptr, nullptr);
	}
END_SVAR_TYPE_IMPLEMENT()
