#include "cameramarker.h"

CCameraMarker::CCameraMarker() : m_pos(0, 0, 0), m_ang(0, 0, 0)
{
	m_model = EngineInterface()->LoadModel("assets/camera");
}

void CCameraMarker::Draw()
{
	m_model->Draw(m_pos, m_ang, glm::vec3(2.5));
}

void CCameraMarker::SetTransform(glm::vec3 pos, glm::vec3 ang)
{
	m_pos = pos;
	m_ang = ang;
}

CCameraMarker& CameraMarker()
{
	static CCameraMarker s_cameraMarker;
	return s_cameraMarker;
}
