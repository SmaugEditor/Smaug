#include "worldtest.h"
#include "worldeditor.h"
#include "meshtest.h"
#include <glm/geometric.hpp>

void testNode(ray_t ray, CNode* node, testWorld_t& end)
{
    testWorld_t aabbTest;
    aabb_t aabb = node->GetAbsAABB();


    rayAABBTest(ray, aabb, aabbTest);
    if (!aabbTest.hit)
        return;

    // Test mesh
    // Offset the ray to the node
    glm::vec3 origin = node->m_mesh.origin;
    ray.origin -= origin;
    for (auto p : node->m_mesh.parts)
        for (auto f : p->sliced ? p->sliced->collision : p->collision)
        {
            testRayPlane_t rayTest = rayVertLoopTest(ray, f->verts.front(), end.t);
            if (rayTest.hit)
            {
                rayTest.intersect += origin;
                // G++ won't let me just cast rayTest like MSVC does, soo
                *((testRayPlane_t*)&end) = rayTest;
                end.face = f;
                end.part = p;
                end.node = node;
            }
        }

}

testWorld_t testRay(ray_t ray)
{
    testWorld_t end = { false };

    for (auto p : GetWorldEditor().m_nodes)
    {
        testNode(ray, p.second, end);
    }
    return end;
}


// TODO: Add distance based optimizations!
testWorld_t testLine(line_t line)
{
    testWorld_t rpTest = testRay({ line.origin, line.delta });
    if (glm::distance(rpTest.intersect, line.origin) > glm::length(line.delta))
    {
        // Too long!
        return { false };
    }
    return rpTest;
}
