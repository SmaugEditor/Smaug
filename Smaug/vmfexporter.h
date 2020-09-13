#pragma once
#include "baseexporter.h"
#include "glm/vec3.hpp"

class KeyValue;
class CNode;

typedef glm::vec3 vmfPlane_t[3];
struct vmfBrush_t
{
	union
	{

		struct
		{
			glm::vec3 topFrontLeft;
			glm::vec3 topFrontRight;

			glm::vec3 topBackLeft;
			glm::vec3 topBackRight;
		};

		glm::vec3 top[4];
	};

	union
	{

		struct
		{
			glm::vec3 bottomFrontLeft;
			glm::vec3 bottomFrontRight;

			glm::vec3 bottomBackLeft;
			glm::vec3 bottomBackRight;
		};

		glm::vec3 bottom[4];
	};
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
	void GetId(char* str);
	size_t m_currentId;
};

