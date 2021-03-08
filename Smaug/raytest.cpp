#include "raytest.h"
#include "worldeditor.h"
#include <glm/geometric.hpp>
#include "basicdraw.h"
#include <modelmanager.h>

using namespace glm;



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

testRayPlane_t rayPlaneTest(ray_t ray, tri_t tri, float closestT)
{
    glm::vec3 edge1 = tri.b - tri.a;
    glm::vec3 edge2 = tri.c - tri.b;

    // Unnormalized normal of the face
    glm::vec3 normal = glm::cross(edge1, edge2);

    // Angle of approach on the face
    float approach = glm::dot(ray.dir, normal);

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

    // Parametric point of intersection with the plane of the tri and the ray
    float t = (d - glm::dot(ray.origin, normal)) / approach;

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
    glm::vec3 p = ray.origin + ray.dir * t;

    testRayPlane_t test;
    test.hit = true;
    test.t = t;
    test.normal = normal;
    test.approach = approach;
    test.intersect = p;

    return test;
}

void rayPlaneTest(ray_t ray, tri_t tri, testRayPlane_t& lastTest)
{
    testRayPlane_t rayTest = rayPlaneTest(ray, tri, lastTest.t);
    if (rayTest.hit)
        lastTest = rayTest;
}



testRayPlane_t rayPlaneTestNoCull(ray_t ray, tri_t tri, float closestT)
{
    glm::vec3 edge1 = tri.b - tri.a;
    glm::vec3 edge2 = tri.c - tri.b;

    // Unnormalized normal of the face
    glm::vec3 normal = glm::cross(edge1, edge2);

    // Angle of approach on the face
    float approach = glm::dot(ray.dir, normal);

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

    // Parametric point of intersection with the plane of the tri and the ray
    float t = (d - glm::dot(ray.origin, normal)) / approach;

    // (p0 + tD) * n = d
    // tD*n = d - p0*d


    // If something else was closer, skip our current and NANs
    if (!(t <= closestT))
    {
        return { false };
    }

    // 3D point of intersection
    glm::vec3 p = ray.origin + ray.dir * t;

    testRayPlane_t test;
    test.hit = true;
    test.t = t;
    test.normal = normal;
    test.approach = approach;
    test.intersect = p;

    return test;
}

void rayPlaneTestNoCull(ray_t ray, tri_t tri, testRayPlane_t& lastTest)
{
    testRayPlane_t rayTest = rayPlaneTestNoCull(ray, tri, lastTest.t);
    if (rayTest.hit)
        lastTest = rayTest;
}


template<bool cull = true>
testRayPlane_t rayTriangleTest(ray_t ray, tri_t tri, float closestT)
{
    glm::vec3 edge1 = tri.b - tri.a;
    glm::vec3 edge2 = tri.c - tri.b;
    
    // Unnormalized normal of the face
    glm::vec3 normal = glm::cross(edge1, edge2);
    
    // Angle of approach on the face
    float approach = glm::dot(ray.dir, normal);

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

    // Parametric point of intersection with the plane of the tri and the ray
    float t = (d - glm::dot(ray.origin, normal)) / approach;

    // (p0 + tD) * n = d
    // tD*n = d - p0*d

    if constexpr (cull)
    {
        // Ignore backside and NANs
        if (!(t >= 0.0f))
            return { false };
    }

    // If something else was closer, skip our current and NANs
    if (!(t <= closestT))
    {
        return { false };
    }

    // 3D point of intersection
    glm::vec3 p = ray.origin + ray.dir * t;

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

    testRayPlane_t test;
    test.hit = true;
    test.t = t;
    test.normal = normal;
    test.approach = approach;
    test.intersect = p;

    return test;
}

void rayTriangleTest(ray_t ray, tri_t tri, testRayPlane_t& lastTest)
{
    testRayPlane_t rayTest = rayTriangleTest(ray, tri, lastTest.t);
    if (rayTest.hit)
        lastTest = rayTest;
}


// In clockwise order from topleft
struct quad_t { glm::vec3 topLeft, topRight, bottomRight, bottomLeft; };
void rayQuadTest(ray_t ray, quad_t quad, testRayPlane_t& lastTest)
{
    rayTriangleTest(ray, { quad.topLeft, quad.topRight, quad.bottomRight }, lastTest);
    rayTriangleTest(ray, { quad.bottomRight, quad.bottomLeft, quad.topLeft }, lastTest);
}



// This is terrible
void rayAABBTest(ray_t ray, aabb_t aabb, testRayPlane_t& lastTest)
{
    vec3 min = aabb.min;
    vec3 max = aabb.max;
    
    // 6 sides
    // clockwise-side-out order
    // max first and min second
    // made from our aabb

    testRayPlane_t test;
    tri_t plane;

    plane = { max,                       vec3(max.x, max.y, min.z), vec3(min.x, max.y, min.z) }; // Top
    test = rayPlaneTestNoCull(ray, plane, lastTest.t);
    if (test.hit && test.intersect.x >= plane[2].x && test.intersect.z >= plane[2].z && test.intersect.x <= plane[0].x && test.intersect.z <= plane[0].z)
        lastTest = test;

    plane = { vec3(max.x, min.y, max.z), vec3(min.x, min.y, max.z), min                       };  // Bottom
    test = rayPlaneTestNoCull(ray, plane, lastTest.t);
    if (test.hit && test.intersect.x >= plane[2].x && test.intersect.z >= plane[2].z && test.intersect.x <= plane[0].x && test.intersect.z <= plane[0].z)
        lastTest = test;

    plane = { max,                       vec3(max.x, min.y, max.z), vec3(max.x, min.y, min.z) }; // Right
    test = rayPlaneTestNoCull(ray, plane, lastTest.t);
    if (test.hit && test.intersect.z >= plane[2].z && test.intersect.y >= plane[2].y && test.intersect.z <= plane[0].z && test.intersect.y <= plane[0].y)
        lastTest = test;

    plane = { vec3(min.x, max.y, max.z), vec3(min.x, max.y, min.z), min                       }; // Left
    test = rayPlaneTestNoCull(ray, plane, lastTest.t);
    if (test.hit && test.intersect.z >= plane[2].z && test.intersect.y >= plane[2].y && test.intersect.z <= plane[0].z && test.intersect.y <= plane[0].y)
        lastTest = test;

    plane = { max,                       vec3(min.x, max.y, max.z), vec3(min.x, min.y, max.z) }; // Front
    test = rayPlaneTestNoCull(ray, plane, lastTest.t);
    if (test.hit && test.intersect.x >= plane[2].x && test.intersect.y >= plane[2].y && test.intersect.x <= plane[0].x && test.intersect.y <= plane[0].y)
        lastTest = test;

    plane = { vec3(max.x, max.y, min.z), vec3(max.x, min.y, min.z), min                       }; // Back
    test = rayPlaneTestNoCull(ray, plane, lastTest.t);
    if (test.hit && test.intersect.x >= plane[2].x && test.intersect.y >= plane[2].y && test.intersect.x <= plane[0].x && test.intersect.y <= plane[0].y)
        lastTest = test;

}

void testNode(ray_t ray, CNode* node, testRayPlane_t& end)
{
    testRayPlane_t aabbTest;
    aabb_t aabb = node->GetAbsAABB();


    rayAABBTest(ray, aabb, aabbTest);
    if (!aabbTest.hit)
        return;

    // Test mesh
    vec3 origin = node->m_mesh.origin;
    for (auto p : node->m_mesh.parts)
        for (auto f : p->faces)
        {
            if (f->verts.size() == 3)
            {
                // Technically, we shouldn't be accessing the data like this
                // Buut triangulation should have us clear? For now?
                // CHECK ME!
                rayTriangleTest(ray, { origin + *f->verts[0]->vert, origin + *f->verts[1]->vert, origin + *f->verts[2]->vert }, end);

            }
        }

}

testRayPlane_t testRay(ray_t ray)
{
    testRayPlane_t end = { false };

    size_t nodeCount = GetWorldEditor().m_nodes.size();
    for (size_t i = 0; i < nodeCount; i++)
    {
        testNode(ray, GetWorldEditor().m_nodes[i], end );
    }
    return end;
}

// TODO: Add distance based optimizations!
testRayPlane_t testLine(line_t line)
{
    testRayPlane_t rpTest = testRay({ line.origin, line.delta });
    if (glm::distance(rpTest.intersect, line.origin) > glm::length(line.delta))
    {
        // Too long!
        return { false };
    }
    return rpTest;
}



testLineLine_t testLineLine(line_t a, line_t b, float tolerance)
{

    vec3 cross = glm::cross(a.delta, a.delta);
    if (cross.x == 0.0f && cross.y == 0.0f && cross.z == 0.0f)
        return {false};

    vec3 aDir = glm::normalize(a.delta);
    vec3 bDir = glm::normalize(b.delta);

    // Defined for reuse in t1 & t2
    float crossPow = pow(glm::length(cross), 2);
    vec3 dirCross = glm::cross(aDir, bDir);
    vec3 originDelta = b.origin - a.origin;
    
    float t1 = glm::dot(glm::cross(originDelta, bDir), dirCross) / crossPow;
    float t2 = glm::dot(glm::cross(originDelta, aDir), dirCross) / crossPow;

    vec3 p1 = a.origin + t1 * aDir;
    vec3 p2 = b.origin + t2 * bDir;

    // Are the points valid?
    if (glm::distance(p1, p2) > tolerance)
        return { false };

    // Unnecessary floating point error introduction? Maybe...
    vec3 point = (p1 + p2) / 2.0f;

    // Check if we're too long
    if (glm::distance(a.origin, point) > glm::distance(a.origin, a.delta)
        || glm::distance(b.origin, point) > glm::distance(b.origin, b.delta))
        return { false };

    return { true, point };
}


void findDominantAxis(glm::vec3 normal, int& uAxis, int& vAxis)
{
    // Find the dominant axis
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
}

bool testPointInTri(float pU, float pV, glm::vec3 domU, glm::vec3 domV)
{

    // Project plane onto axis
    glm::vec3 u = { pU - domU.x, domU.y - domU.x, domU.z - domU.x };
    glm::vec3 v = { pV - domV.x, domV.y - domV.x, domV.z - domV.x };

    // Denominator
    float temp = u[1] * v[2] - v[1] * u[2];
    if (!(temp != 0))
        return false;
    temp = 1.0f / temp;

    // Barycentric coords with NAN checks
    float alpha = (u[0] * v[2] - v[0] * u[2]) * temp;
    float beta = (u[1] * v[0] - v[1] * u[0]) * temp;
    float gamma = 1.0f - alpha - beta;
    if (!(alpha >= 0.0f) || !(beta >= 0.0f) || !(gamma >= 0.0f))
        return false;
    return false;
}


bool testPointInTriNoEdges(float pU, float pV, glm::vec3 domU, glm::vec3 domV)
{

    // Project plane onto axis
    glm::vec3 u = { pU - domU.x, domU.y - domU.x, domU.z - domU.x };
    glm::vec3 v = { pV - domV.x, domV.y - domV.x, domV.z - domV.x };

    // Denominator
    float temp = u[1] * v[2] - v[1] * u[2];
    if (!(temp != 0))
        return false;
    temp = 1.0f / temp;

    // Barycentric coords with NAN checks
    float alpha = (u[0] * v[2] - v[0] * u[2]) * temp;
    float beta = (u[1] * v[0] - v[1] * u[0]) * temp;
    float gamma = 1.0f - alpha - beta;
    if (!(alpha >= 0.0f) || !(beta >= 0.0f) || !(gamma >= 0.0f))
        return false;
    return false;
}



bool testPointInTri(glm::vec3 p, glm::vec3 tri0, glm::vec3 tri1, glm::vec3 tri2)
{

    glm::vec3 edge1 = tri1 - tri0;
    glm::vec3 edge2 = tri2 - tri1;

    // Unnormalized normal of the face
    glm::vec3 normal = glm::cross(edge1, edge2);

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
    glm::vec3 u = { p[uAxis] - tri0[uAxis], tri1[uAxis] - tri0[uAxis], tri2[uAxis] - tri0[uAxis] };
    glm::vec3 v = { p[vAxis] - tri0[vAxis], tri1[vAxis] - tri0[vAxis], tri2[vAxis] - tri0[vAxis] };

    // Denominator
    float temp = u[1] * v[2] - v[1] * u[2];
    if (!(temp != 0))
        return false;
    temp = 1.0f / temp;

    // Barycentric coords with NAN checks
    float alpha = (u[0] * v[2] - v[0] * u[2]) * temp;
    float beta = (u[1] * v[0] - v[1] * u[0]) * temp;
    float gamma = 1.0f - alpha - beta;
    if (!(alpha >= 0.0f) || !(beta >= 0.0f) || !(gamma >= 0.0f))
        return false;

    return true;
}


bool testPointInTriNoEdges(glm::vec3 p, glm::vec3 tri0, glm::vec3 tri1, glm::vec3 tri2)
{

    glm::vec3 edge1 = tri1 - tri0;
    glm::vec3 edge2 = tri2 - tri1;

    // Unnormalized normal of the face
    glm::vec3 normal = glm::cross(edge1, edge2);

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
    glm::vec3 u = { p[uAxis] - tri0[uAxis], tri1[uAxis] - tri0[uAxis], tri2[uAxis] - tri0[uAxis] };
    glm::vec3 v = { p[vAxis] - tri0[vAxis], tri1[vAxis] - tri0[vAxis], tri2[vAxis] - tri0[vAxis] };

    // Denominator
    float temp = u[1] * v[2] - v[1] * u[2];
    if (!(temp != 0))
        return false;
    temp = 1.0f / temp;

    // Barycentric coords with NAN checks
    float alpha = (u[0] * v[2] - v[0] * u[2]) * temp;
    float beta = (u[1] * v[0] - v[1] * u[0]) * temp;
    float gamma = 1.0f - alpha - beta;
    if (!(alpha > 0.0f) || !(beta > 0.0f) || !(gamma > 0.0f))
        return false;

    return true;
}

testRayPlane_t pointOnPart(meshPart_t* part, glm::vec3 p)
{
    // To Abs
    p -= part->mesh->origin;
    //glm::vec3 origin = part->mesh->origin;
    for (auto f : part->faces)
    {
        if (f->verts.size() == 3)
        {
            glm::vec3 norm = faceNormal(f);

            testRayPlane_t t = rayTriangleTest<false>({ p, -norm }, { *f->verts[0]->vert, *f->verts[1]->vert, *f->verts[2]->vert }, FLT_MAX);

            if (t.hit)
            {

                return t;
            }
        }
    }
    return { false };
}