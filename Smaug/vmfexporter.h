#pragma once
#include "baseexporter.h"
#include "glm/vec3.hpp"

class KeyValue;
class CNode;

typedef glm::vec3 vmfPlane_t[3];
struct vmfBrush_t
{	
	glm::vec3 top[4];
	glm::vec3 bottom[4];

	glm::vec3& bottomFrontLeft() { return bottom[0]; };
	glm::vec3& bottomFrontRight() { return bottom[1]; };
	glm::vec3& bottomBackLeft() { return bottom[2]; };
	glm::vec3& bottomBackRight() { return bottom[3]; };

	glm::vec3& topFrontLeft() { return top[0]; };
	glm::vec3& topFrontRight() { return top[1]; };
	glm::vec3& topBackLeft() { return top[2]; };
	glm::vec3& topBackRight() { return top[3]; };
};

class CVMFExporter : public CBaseExporter
{
private:

public:
	virtual char* Export(CWorldEditor* world) override;
private:
	void AddNode(KeyValue* parentKv, CNode* node);

	

	void AddBrush(KeyValue* parentKv, vmfBrush_t brush);
	void AddSide(KeyValue* parentKv, vmfPlane_t plane);
	void AddSide(KeyValue* parentKv, glm::vec3 a, glm::vec3 b, glm::vec3 c);
	void GetId(char* str);
	size_t m_currentId;
};

