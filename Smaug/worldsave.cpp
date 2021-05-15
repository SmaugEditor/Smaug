#include "worldsave.h"
#include "worldeditor.h"
#include "svarex.h"
#include "actionmanager.h"
#include <KeyValue.h>

static const int SAVE_FILE_VERSION = 0;


char* saveWorld()
{
	char buf[256];
	
	KeyValueRoot file;
	snprintf(buf, sizeof(buf), "%d", SAVE_FILE_VERSION);
	file.Add("smf", buf);

	for (auto n : GetWorldEditor().m_nodes)
	{
		KeyValue* node = file.AddNode("node");
		snprintf(buf, sizeof(buf), "%d", n.first);
		node->Add("id", buf);

		glm::vec3 origin = n.second->m_mesh.origin;
		snprintf(buf, sizeof(buf), "%a %a %a", origin.x, origin.y, origin.z);
		node->Add("origin", buf);

		auto vertList = n.second->m_mesh.verts;

		KeyValue* verts = node->AddNode("verts");
		for (auto v : vertList)
		{
			snprintf(buf, sizeof(buf), "%a %a %a", v->x, v->y, v->z);
			verts->Add("vert", buf);
		}

		KeyValue* parts = node->AddNode("parts");
		for (auto p : n.second->m_mesh.parts)
		{
			if (p->verts.size() == 0)
				continue;

			vertex_t* v = p->verts.front(), *vs = v;
			do
			{
				int idx = 0;
				auto find = std::find(vertList.begin(), vertList.end(), v->vert);
				if (find == vertList.end())
				{
					Log::Fault("[WorldSave] Failed to find mesh part vertex!\n");
				}
				else
				{
					idx = find - vertList.begin();
				}

				// For our first element, we don't want any spaces
				if (v == vs)
				{
					snprintf(buf, sizeof(buf), "%d", idx);
				}
				else
				{
					char idxBuf[32];
					snprintf(idxBuf, sizeof(idxBuf), " %d", idx);
					strcat(buf, idxBuf);
				}

				v = v->edge->vert;
			} while (v != vs);
			
			parts->Add("part", buf);
		}

		
	}

	return file.ToString();
}


// This is lame!
void loadWorld(char* input)
{
	GetWorldEditor().Clear();
	GetActionManager().Clear();

	KeyValueRoot kvFile(input);

	for (KeyValue* kvNode = kvFile.children; kvNode; kvNode = kvNode->next)
	{

		if (strncmp(kvNode->key.string, "node", kvNode->key.length) == 0)
		{
			
			std::vector<glm::vec3> verts;
			std::vector<std::vector<int>> parts;
			int id = 0;
			glm::vec3 origin{0,0,0};

			// Suck data for each node
			for (KeyValue* kv = kvNode->children; kv; kv = kv->next)
			{

				if (kv->hasChildren)
				{
					if (strncmp(kv->key.string, "verts", kv->key.length) == 0)
					{
						for (KeyValue* vert = kv->children; vert; vert = vert->next)
						{
							if (strncmp(vert->key.string, "vert", vert->key.length) == 0)
							{
								glm::vec3 vec;
								int success = sscanf(vert->value.string, "%a %a %a", &vec.x, &vec.y, &vec.z);
								if (success == 3)
									verts.push_back(vec);
							}
						}
					}

					if (strncmp(kv->key.string, "parts", kv->key.length) == 0)
					{
						for (KeyValue* part = kv->children; part; part = part->next)
						{

							if (strncmp(part->key.string, "part", part->key.length) == 0)
							{
								std::vector<int> face;
								char* szIdx = part->value.string;
								while (szIdx && *szIdx)
								{
									int idx = strtol(szIdx, &szIdx, 10);
									face.push_back(idx);
								}
								if (face.size() > 3)
									parts.push_back(face);
								else
									Log::Fault("[LoadWorld] Misformed mesh part!\n");
							}
						}
					}
				}
				else
				{
					if (strncmp(kv->key.string, "id", kv->key.length) == 0)
					{
						id = strtol(kv->value.string, nullptr, 10);
					}

					if (strncmp(kv->key.string, "origin", kv->key.length) == 0)
					{
						glm::vec3 vec;
						int success = sscanf(kv->value.string, "%a %a %a", &vec.x, &vec.y, &vec.z);
						if (success == 3)
							origin = vec;
					}
				}

			}

			// Create the node out of the data we snatched
			if (parts.size() != 0 && verts.size() != 0)
			{
				CNode* node = new CNode();

				node->m_mesh.origin = origin;

				auto& vertList = node->m_mesh.verts;
				for (auto v : verts)
					vertList.push_back(new glm::vec3{v});
				for (auto p : parts)
				{
					std::vector<glm::vec3*> faceVerts;
					for (auto v : p)
					{
						if (v >= vertList.size())
						{
							Log::Fault("[LoadWorld] Malformed part!\n");
						}
						faceVerts.push_back(vertList[v]);
					}
					addMeshFace(node->m_mesh, faceVerts.data(), faceVerts.size());
				}

				node->Init();

				GetWorldEditor().AssignID(node, id);
			}
			else
			{
				Log::Fault("[LoadWorld] Empty Node!\n");
			}
		}
	}

	for (auto n : GetWorldEditor().m_nodes)
		n.second->UpdateThisOnly();

}