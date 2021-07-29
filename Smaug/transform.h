#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

// Is this class over engineered?
class CTransform
{
public:
	CTransform();
	CTransform(glm::vec3 pos, glm::vec3 rotation = { 0,0,0 }, glm::vec3 scale = { 1,1,1 }, CTransform* parent = nullptr) { m_pParent = parent; SetAbsOrigin(pos); SetAbsAngles(rotation); SetAbsScale(scale); }

	void SetParent(CTransform* parent);

	// Local to parent
	void SetLocalAngles(glm::vec3 ang) { m_angles = ang; }
	void SetLocalAngles(float pitch, float yaw, float roll) { m_angles = { pitch, yaw, roll }; }
	void SetLocalPitch(float pitch) { m_angles.x = pitch; }
	void SetLocalYaw(float yaw)     { m_angles.y = yaw; }
	void SetLocalRoll(float roll)   { m_angles.z = roll; }
	void SetLocalOrigin(glm::vec3 pos) { m_position = pos; }
	void SetLocalOrigin(float x, float y, float z) { m_position = { x, y, z }; }
	void SetLocalScale(glm::vec3 scale) { m_scale = scale; }
	void SetLocalScale(float scale) { m_scale = { scale, scale, scale }; }
	void SetLocalScale(float x, float y, float z) { m_scale = { x, y, z }; }

	glm::vec3 GetLocalAngles() { return m_angles; }
	glm::vec3 GetLocalOrigin() { return m_position; }
	glm::vec3 GetLocalScale()  { return m_scale; }

	// Absolute position in the world
	void SetAbsAngles(glm::vec3 ang);
	void SetAbsAngles(float pitch, float yaw, float roll) { SetAbsAngles({ pitch, yaw, roll }); }
	void SetAbsOrigin(glm::vec3 pos);
	void SetAbsOrigin(float x, float y, float z) { SetAbsOrigin({ x, y, z }); }
	void SetAbsScale(glm::vec3 scale);
	void SetAbsScale(float scale) { SetAbsScale({ scale, scale, scale }); }
	void SetAbsScale(float x, float y, float z) { SetAbsScale({ x, y, z }); }

	glm::vec3 GetAbsAngles();
	glm::vec3 GetAbsOrigin();
	glm::vec3 GetAbsScale();

	glm::mat4 Matrix();

private:

	glm::vec3 m_angles;
	glm::vec3 m_position;
	glm::vec3 m_scale;

	CTransform* m_pParent;
};
