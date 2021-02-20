#include "raytest.h"
#include "worldeditor.h"
#include <glm/geometric.hpp>
#include "basicdraw.h"
#include <modelmanager.h>

using namespace glm;

struct tri_t
{
    union
    {
        struct
        {
            glm::vec3 a, b, c;
        };
        glm::vec3 verts[3];
    };
    glm::vec3& operator[](int i) { return verts[i]; }
};


bool testPointInAABB(glm::vec3 point, aabb_t aabb)
{
    return point.x <= aabb.max.x && point.y <= aabb.max.y && point.z <= aabb.max.z
        && point.x >= aabb.min.x && point.y >= aabb.min.y && point.z >= aabb.min.z;
}

bool testPointInAABB(glm::vec3 point, aabb_t aabb, float aabbBloat)
{
    aabb.min -= aabbBloat;
    aabb.max += aabbBloat;
    return testPointInAABB(point, aabb);
}

///////////////
// Ray Tests //
///////////////
// Adapted from 3D math primer for graphics and game development / Fletcher Dunn, Ian Parberry. -- 2nd ed. pg 724
// Lot of this code is a bit duplicated. Clean it up?

testLine_t rayPlaneTest(line_t line, tri_t tri, float closestT)
{
    glm::vec3 edge1 = tri.b - tri.a;
    glm::vec3 edge2 = tri.c - tri.b;

    // Unnormalized normal of the face
    glm::vec3 normal = glm::cross(edge1, edge2);

    // Angle of approach on the face
    float approach = glm::dot(line.delta, normal);

    // If parallel to or not facing the plane, dump it
    // 
    // the ! < dumps malformed triangles with NANs!
    // does not serve the same purpose as >=
    if (!(approach < 0.0f))
    {
        return { false };
    }

    // All points on the plane have the same dot, defined as d
    float d = glm::dot(tri.a, normal);

    // Parametric point of intersection with the plane of the tri and the line
    float t = (d - glm::dot(line.origin, normal)) / approach;

    // (p0 + tD) * n = d
    // tD*n = d - p0*d


    // Ignore backside and NANs
    if (!(t >= 0.0f))
        return { false };

    // If something else was closer, skip our current and NANs
    if (!(t <= closestT))
    {
        return { false };
    }

    // 3D point of intersection
    glm::vec3 p = line.origin + line.delta * t;

    testLine_t test;
    test.hit = true;
    test.t = t;
    test.normal = normal;
    test.approach = approach;
    test.hitPos = p;

    return test;
}

void rayPlaneTest(line_t line, tri_t tri, testLine_t& lastTest)
{
    testLine_t rayTest = rayPlaneTest(line, tri, lastTest.t);
    if (rayTest.hit)
        lastTest = rayTest;
}



testLine_t rayPlaneTestNoCull(line_t line, tri_t tri, float closestT)
{
    glm::vec3 edge1 = tri.b - tri.a;
    glm::vec3 edge2 = tri.c - tri.b;

    // Unnormalized normal of the face
    glm::vec3 normal = glm::cross(edge1, edge2);

    // Angle of approach on the face
    float approach = glm::dot(line.delta, normal);

    // If parallel to or not facing the plane, dump it
    // 
    // the ! < dumps malformed triangles with NANs!
    // does not serve the same purpose as >=
    if (!(approach < 0.0f))
    {
        return { false };
    }

    // All points on the plane have the same dot, defined as d
    float d = glm::dot(tri.a, normal);

    // Parametric point of intersection with the plane of the tri and the line
    float t = (d - glm::dot(line.origin, normal)) / approach;

    // (p0 + tD) * n = d
    // tD*n = d - p0*d


    // If something else was closer, skip our current and NANs
    if (!(t <= closestT))
    {
        return { false };
    }

    // 3D point of intersection
    glm::vec3 p = line.origin + line.delta * t;

    testLine_t test;
    test.hit = true;
    test.t = t;
    test.normal = normal;
    test.approach = approach;
    test.hitPos = p;

    return test;
}

void rayPlaneTestNoCull(line_t line, tri_t tri, testLine_t& lastTest)
{
    testLine_t rayTest = rayPlaneTest(line, tri, lastTest.t);
    if (rayTest.hit)
        lastTest = rayTest;
}



testLine_t rayTriangleTest(line_t line, tri_t tri, float closestT)
{
    glm::vec3 edge1 = tri.b - tri.a;
    glm::vec3 edge2 = tri.c - tri.b;
    
    // Unnormalized normal of the face
    glm::vec3 normal = glm::cross(edge1, edge2);
    
    // Angle of approach on the face
    float approach = glm::dot(line.delta, normal);

    // If parallel to or not facing the plane, dump it
    // 
    // the ! < dumps malformed triangles with NANs!
    // does not serve the same purpose as >=
    if (!(approach < 0.0f))
    {
        return { false };
    }

    // All points on the plane have the same dot, defined as d
    float d = glm::dot(tri.a, normal);

    // Parametric point of intersection with the plane of the tri and the line
    float t = (d - glm::dot(line.origin, normal)) / approach;

    // (p0 + tD) * n = d
    // tD*n = d - p0*d


    // Ignore backside and NANs
    if (!(t >= 0.0f))
        return { false };

    // If something else was closer, skip our current and NANs
    if (!(t <= closestT))
    {
        return { false };
    }

    // 3D point of intersection
    glm::vec3 p = line.origin + line.delta * t;

    // Find the dominant axis
    int uAxis, vAxis;
    if (fabs(normal.x) > fabs(normal.y))
    {
        if (fabs(normal.x) > fabs(normal.z))
        {
            uAxis = 1;
            vAxis = 2;
        }
        else
        {
            uAxis = 0;
            vAxis = 1;
        }
    }
    else
    {
        if (fabs(normal.y) > fabs(normal.z))
        {
            uAxis = 0;
            vAxis = 2;
        }
        else
        {
            uAxis = 0;
            vAxis = 1;
        }
    }

    // Project plane onto axis
    glm::vec3 u = { p[uAxis] - tri.a[uAxis], tri.b[uAxis] - tri.a[uAxis], tri.c[uAxis] - tri.a[uAxis] };
    glm::vec3 v = { p[vAxis] - tri.a[vAxis], tri.b[vAxis] - tri.a[vAxis], tri.c[vAxis] - tri.a[vAxis] };

    // Denominator
    float temp = u[1] * v[2] - v[1] * u[2];
    if (!(temp != 0))
        return { false };
    temp = 1.0f / temp;

    // Barycentric coords with NAN checks
    float alpha = (u[0] * v[2] - v[0] * u[2]) * temp;
    float beta  = (u[1] * v[0] - v[1] * u[0]) * temp;
    float gamma = 1.0f - alpha - beta;
    if (!(alpha >= 0.0f) || !(beta >= 0.0f) || !(gamma >= 0.0f))
        return { false };

    testLine_t test;
    test.hit = true;
    test.t = t;
    test.normal = normal;
    test.approach = approach;
    test.hitPos = p;

    return test;
}

void rayTriangleTest(line_t line, tri_t tri, testLine_t& lastTest)
{
    testLine_t rayTest = rayTriangleTest(line, tri, lastTest.t);
    if (rayTest.hit)
        lastTest = rayTest;
}


// In clockwise order from topleft
struct quad_t { glm::vec3 topLeft, topRight, bottomRight, bottomLeft; };
void rayQuadTest(line_t line, quad_t quad, testLine_t& lastTest)
{
    rayTriangleTest(line, { quad.topLeft, quad.topRight, quad.bottomRight }, lastTest);
    rayTriangleTest(line, { quad.bottomRight, quad.bottomLeft, quad.topLeft }, lastTest);
}



// This is terrible
void rayAABBTest(line_t line, aabb_t aabb, testLine_t& lastTest)
{
    vec3 min = aabb.min;
    vec3 max = aabb.max;
    
    // 6 sides
    // clockwise-side-out order
    // max first and min second
    // made from our aabb

    testLine_t test;
    tri_t plane;

    plane = { max,                       vec3(max.x, max.y, min.z), vec3(min.x, max.y, min.z) }; // Top
    test = rayPlaneTestNoCull(line, plane, lastTest.t);
    if (test.hit && test.hitPos.x >= plane[2].x && test.hitPos.z >= plane[2].z && test.hitPos.x <= plane[0].x && test.hitPos.z <= plane[0].z)
        lastTest = test;

    plane = { vec3(max.x, min.y, max.z), vec3(min.x, min.y, max.z), min                       };  // Bottom
    test = rayPlaneTestNoCull(line, plane, lastTest.t);
    if (test.hit && test.hitPos.x >= plane[2].x && test.hitPos.z >= plane[2].z && test.hitPos.x <= plane[0].x && test.hitPos.z <= plane[0].z)
        lastTest = test;

    plane = { max,                       vec3(max.x, min.y, max.z), vec3(max.x, min.y, min.z) }; // Right
    test = rayPlaneTestNoCull(line, plane, lastTest.t);
    if (test.hit && test.hitPos.z >= plane[2].z && test.hitPos.y >= plane[2].y && test.hitPos.z <= plane[0].z && test.hitPos.y <= plane[0].y)
        lastTest = test;

    plane = { vec3(min.x, max.y, max.z), vec3(min.x, max.y, min.z), min                       }; // Left
    test = rayPlaneTestNoCull(line, plane, lastTest.t);
    if (test.hit && test.hitPos.z >= plane[2].z && test.hitPos.y >= plane[2].y && test.hitPos.z <= plane[0].z && test.hitPos.y <= plane[0].y)
        lastTest = test;

    plane = { max,                       vec3(min.x, max.y, max.z), vec3(min.x, min.y, max.z) }; // Front
    test = rayPlaneTestNoCull(line, plane, lastTest.t);
    if (test.hit && test.hitPos.x >= plane[2].x && test.hitPos.y >= plane[2].y && test.hitPos.x <= plane[0].x && test.hitPos.y <= plane[0].y)
        lastTest = test;

    plane = { vec3(max.x, max.y, min.z), vec3(max.x, min.y, min.z), min                       }; // Back
    test = rayPlaneTestNoCull(line, plane, lastTest.t);
    if (test.hit && test.hitPos.x >= plane[2].x && test.hitPos.y >= plane[2].y && test.hitPos.x <= plane[0].x && test.hitPos.y <= plane[0].y)
        lastTest = test;

}

void testNode(line_t line, CNode* node, testLine_t& end)
{
    testLine_t aabbTest;
    aabb_t aabb = node->GetAbsAABB();


    rayAABBTest(line, aabb, aabbTest);
    if (!aabbTest.hit)
        return;

    // Test walls
    for (int i = 0; i < node->m_sideCount; i++)
    {
        nodeSide_t& side = node->m_sides[i];
        for (int j = 0; j < side.walls.size(); j++)
        {
            nodeWall_t& wall = side.walls[j];
            rayQuadTest(line, { node->m_origin + wall.topPoints[0], node->m_origin + wall.topPoints[1], node->m_origin + wall.bottomPoints[1], node->m_origin + wall.bottomPoints[0] }, end);
        }
    }

    // Test floor
    if(node->m_nodeType == NodeType::QUAD)
        rayQuadTest(line, { node->m_origin + node->m_vertexes[0].origin, node->m_origin + node->m_vertexes[1].origin, node->m_origin + node->m_vertexes[2].origin, node->m_origin + node->m_vertexes[3].origin }, end);
    else if (node->m_nodeType == NodeType::TRI)
        rayTriangleTest(line, { node->m_origin + node->m_vertexes[0].origin, node->m_origin + node->m_vertexes[1].origin, node->m_origin + node->m_vertexes[2].origin }, end);
}

testLine_t testLine(line_t line)
{
    testLine_t end = { false };

    size_t nodeCount = GetWorldEditor().m_nodes.size();
    for (size_t i = 0; i < nodeCount; i++)
    {
        testNode(line, GetWorldEditor().m_nodes[i], end );
    }
    return end;
}

