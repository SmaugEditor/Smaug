#include "raytest.h"
#include "worldeditor.h"
#include <glm/geometric.hpp>


struct tri_t
{
    union
    {
        glm::vec3 verts[3];
        struct
        {
            glm::vec3 a, b, c;
        };
    };
    glm::vec3& operator[](int i) { return verts[i]; }
};



// Adapted from 3D math primer for graphics and game development / Fletcher Dunn, Ian Parberry. -- 2nd ed. pg 724
test_t rayTriangleTest(line_t line, tri_t tri, float closestT)
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

    test_t test;
    test.hit = true;
    test.t = t;
    test.normal = normal;
    test.approach = approach;
    test.hitPos = p;

    return test;
}

void rayTriangleTest(line_t line, tri_t tri, test_t& lastTest)
{
    test_t rayTest = rayTriangleTest(line, tri, lastTest.t);
    if (rayTest.hit)
        lastTest = rayTest;
}

// In clockwise order from topleft
struct quad_t { glm::vec3 topLeft, topRight, bottomRight, bottomLeft; };
void rayQuadTest(line_t line, quad_t quad, test_t& lastTest)
{
    rayTriangleTest(line, { quad.topLeft, quad.topRight, quad.bottomRight }, lastTest);
    rayTriangleTest(line, { quad.bottomRight, quad.bottomLeft, quad.topLeft }, lastTest);
}


void testNode(line_t line, CNode* node, test_t& end)
{

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

test_t testLine(line_t line)
{
    test_t end = { false };

    size_t nodeCount = GetWorldEditor().m_nodes.size();
    for (size_t i = 0; i < nodeCount; i++)
    {
        testNode(line, GetWorldEditor().m_nodes[i], end );
    }
    return end;
}
