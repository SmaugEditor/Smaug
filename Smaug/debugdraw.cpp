#include "debugdraw.h"
#include <editorinterface.h>


class CDebugDraw::ITempItem
{
public:
    virtual void Draw() = 0;
    virtual bool Dead(float curTime) = 0;
};

#ifdef _DEBUG

class CTempLine : public CDebugDraw::ITempItem
{
public:
    CTempLine(glm::vec3 start, glm::vec3 end, glm::vec3 color, float width, float deathTime) :
        m_start(start), m_end(end), m_color(color), m_width(width), m_deathTime(deathTime) {}
    
    virtual void Draw()
    {
        EngineInterface()->DrawLine(m_start, m_end, m_width, m_color);
    }

    virtual bool Dead(float curTime)
    {
        return curTime >= m_deathTime;
    }

    glm::vec3 m_start;
    glm::vec3 m_end;
    glm::vec3 m_color;
    float m_width;
    float m_deathTime;

};


void CDebugDraw::Line(glm::vec3 start, glm::vec3 end, glm::vec3 color, float width, float decay)
{
    m_itemsToDraw.push_back(new CTempLine{start,end,color,width,EngineInterface()->Time() + decay});
}



void CDebugDraw::HELoop(vertex_t* start, vertex_t* end, glm::vec3 color, float width, float decay)
{
    vertex_t* cur = start;
    do
    {
        vertex_t* next = cur->edge->vert;
        Line(*cur->vert, *next->vert, color, width, decay);
        cur = next;
    } while (cur != end);
}

void CDebugDraw::HEFace(face_t* face, glm::vec3 color, float width, float decay)
{
    glm::vec3 offset = parentMesh(face)->origin;

    vertex_t* cur = face->verts.front(), *end = cur;
    do
    {
        vertex_t* next = cur->edge->vert;
        Line(*cur->vert + offset, *next->vert + offset, color, width, decay);
        cur = next;
    } while (cur != end);

}

void CDebugDraw::HEPart(meshPart_t* part, glm::vec3 color, float width, float decay)
{
    for (auto f : part->tris)
        HEFace(f, color, width, decay);
}


void CDebugDraw::AABB(glm::vec3 origin, aabb_t aabb, glm::vec3 color, float width, float decay)
{
    aabb.min += origin;
    aabb.max += origin;

    AABB(aabb, color, width, decay);
}

void CDebugDraw::ClearAll()
{
    for (auto i : m_itemsToDraw)
        delete i;

    m_itemsToDraw.clear();
}

void CDebugDraw::AABB(aabb_t aabb, glm::vec3 color, float width, float decay)
{
    Line(aabb.min, { aabb.max.x, aabb.min.y, aabb.min.z }, color, width, decay);
    Line(aabb.min, { aabb.min.x, aabb.max.y, aabb.min.z }, color, width, decay);
    Line(aabb.min, { aabb.min.x, aabb.min.y, aabb.max.z }, color, width, decay);

    Line(aabb.max, { aabb.min.x, aabb.max.y, aabb.max.z }, color, width, decay);
    Line(aabb.max, { aabb.max.x, aabb.min.y, aabb.max.z }, color, width, decay);
    Line(aabb.max, { aabb.max.x, aabb.max.y, aabb.min.z }, color, width, decay);

    Line({ aabb.min.x, aabb.max.y, aabb.max.z }, { aabb.min.x, aabb.max.y, aabb.min.z }, color, width, decay);
    Line({ aabb.min.x, aabb.max.y, aabb.max.z }, { aabb.min.x, aabb.min.y, aabb.max.z }, color, width, decay);
    
    Line({ aabb.max.x, aabb.min.y, aabb.max.z }, { aabb.min.x, aabb.min.y, aabb.max.z }, color, width, decay);
    Line({ aabb.max.x, aabb.min.y, aabb.max.z }, { aabb.max.x, aabb.min.y, aabb.min.z }, color, width, decay);

    Line({ aabb.max.x, aabb.max.y, aabb.min.z }, { aabb.min.x, aabb.max.y, aabb.min.z }, color, width, decay);
    Line({ aabb.max.x, aabb.max.y, aabb.min.z }, { aabb.max.x, aabb.min.y, aabb.min.z }, color, width, decay);


}

void CDebugDraw::Draw()
{
    float curtime = EngineInterface()->Time();
    
    for (int i = 0; i < m_itemsToDraw.size(); i++)
    {
        ITempItem* t = m_itemsToDraw[i];
        if (t->Dead(curtime))
        {
            delete t;
            m_itemsToDraw.erase(m_itemsToDraw.begin() + i);
            i--;
        }
        else
        {
            t->Draw();
        }
    }
}


#else
// We don't want debug stuff to pop up while in release!

void CDebugDraw::Line(glm::vec3 start, glm::vec3 end, glm::vec3 color, float width, float decay)
{
}

void CDebugDraw::ClearAll()
{
}

void CDebugDraw::Draw()
{
}

void CDebugDraw::HELoop(vertex_t* start, vertex_t* end, glm::vec3 color, float width, float decay)
{
}

void CDebugDraw::HEFace(face_t* face, glm::vec3 color, float width, float decay)
{
}

void CDebugDraw::HEPart(meshPart_t* part, glm::vec3 color, float width, float decay)
{
}
void CDebugDraw::AABB(glm::vec3 origin, aabb_t aabb, glm::vec3 color, float width, float decay)
{
}
void CDebugDraw::AABB(aabb_t aabb, glm::vec3 color, float width, float decay)
{
}
#endif

CDebugDraw& DebugDraw()
{
    static CDebugDraw s_debugdraw;
    return s_debugdraw;
}
