#include "debugdraw.h"
#include "basicdraw.h"
#include <glfw/glfw3.h>


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
        BasicDraw().Line(m_start, m_end, m_color, m_width);
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
    m_itemsToDraw.push_back(new CTempLine{start,end,color,width,(float)glfwGetTime() + decay});
}

void CDebugDraw::Draw()
{
    float curtime = glfwGetTime();
    
    for (int i = 0; i < m_itemsToDraw.size(); i++)
    {
        ITempItem* t = m_itemsToDraw[i];
        if (t->Dead(curtime))
        {
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

void CDebugDraw::Draw()
{
}

#endif

CDebugDraw& DebugDraw()
{
    static CDebugDraw s_debugdraw;
    return s_debugdraw;
}
