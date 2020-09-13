#include "vmfexporter.h"
#include <KeyValue.h>
#include "worldeditor.h"



char* CVMFExporter::Export(CWorldEditor* world)
{
    m_currentId = 1;

    KeyValueRoot kv;
    KeyValue* worldNode = kv.AddNode("world");
    
    worldNode->Add("id", "1");
    worldNode->Add("mapversion", "1");
    worldNode->Add("classname", "worldspawn");

    const size_t nodeCount = world->m_nodes.size();
    for (size_t i = 0; i < nodeCount; i++)
    {
        CNode* node = world->m_nodes[i];
        
        vmfBrush_t floor;
        
        floor.topFrontLeft = node->m_vertexes[0].origin + node->m_origin;
        floor.topBackLeft = node->m_vertexes[1].origin + node->m_origin;
        floor.topBackRight = node->m_vertexes[2].origin + node->m_origin;
        floor.topFrontRight = node->m_vertexes[3].origin + node->m_origin;

        memcpy(floor.bottom, floor.top, sizeof(glm::vec3) * 4);

        for (int j = 0; j < node->m_sideCount; j++)
        {
            floor.bottom[j].y -= 10;
        }
        AddBrush(worldNode, floor);
    }

    return kv.ToString();
}

void CVMFExporter::AddNode(KeyValue* parentKv, CNode* node)
{
}

void CVMFExporter::AddBrush(KeyValue* parentKv, vmfBrush_t brush)
{
    KeyValue* kv = parentKv->AddNode("solid");
    
    char id[8];
    GetId(id);
    kv->Add("id", id);

    // Top
    AddSide(kv, vmfPlane_t{ brush.topFrontLeft, brush.topBackLeft, brush.topBackRight });
    // Bottom
    AddSide(kv, vmfPlane_t{ brush.bottomFrontRight, brush.bottomBackRight, brush.bottomBackLeft });
    // Front
    AddSide(kv, vmfPlane_t{ brush.bottomFrontLeft, brush.topFrontLeft, brush.topFrontRight });
    // Back
    AddSide(kv, vmfPlane_t{ brush.bottomBackRight, brush.topBackRight, brush.topBackLeft });
    // Left
    AddSide(kv, vmfPlane_t{ brush.bottomBackLeft, brush.topBackLeft, brush.topFrontLeft });
    // Right
    AddSide(kv, vmfPlane_t{ brush.bottomFrontRight, brush.topFrontRight, brush.topBackRight });
}

void CVMFExporter::AddSide(KeyValue* parentKv, vmfPlane_t plane)
{
    /*
    side
    {
          "id" "6"
          "plane" "(512 -512 -512) (-512 -512 -512) (-512 -512 512)"
          "material" "BRICK/BRICKFLOOR001A"
          "uaxis" "[1 0 0 0] 0.25"
          "vaxis" "[0 0 -1 0] 0.25"
          "rotation" "0"
          "lightmapscale" "16"
          "smoothing_groups" "0"
          dispinfo{}
    }
    */
    KeyValue* kv = parentKv->AddNode("side");

    // Id
    char id[8];
    GetId(id);
    kv->Add("id", id);

    // Plane
    char planeBuf[128];
    // Bit mega eh?
    // -X Z Y is on purpose as we're in a different coordinate space
    snprintf(planeBuf, 128, "(%f %f %f) (%f %f %f) (%f %f %f)", plane[0].x, plane[0].z, plane[0].y, plane[1].x, plane[1].z, plane[1].y, plane[2].x, plane[2].z, plane[2].y );
    kv->Add("plane", planeBuf);

}

void CVMFExporter::GetId(char* str)
{
    itoa(m_currentId, str, 10);
    m_currentId++;
}
