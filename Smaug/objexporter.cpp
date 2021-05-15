#include "objexporter.h"
#include "worldeditor.h"

#include <sstream>
#include <cstring>

static inline void WriteVertex(glm::vec3 vec, std::stringstream& stream)
{
	stream << "v " << vec.x << " " << vec.y << " " << vec.z << "\n";
}

static inline void WriteNorm(glm::vec3 vec, std::stringstream& stream)
{
	stream << "vn " << vec.x << " " << vec.y << " " << vec.z << "\n";
}


char* COBJExporter::Export(CWorldEditor* world)
{
	std::stringstream stream;
	stream << "# Generated by Smaug - ";
#ifdef _DEBUG
	stream << "Debug";
#else
	stream << "Release";
#endif
	stream << " build compiled on " << __DATE__ << "\n";

	stream << "\n# Vertexes\n";
	for (auto p : GetWorldEditor().m_nodes)
	{
		CNode* node = p.second;

		cuttableMesh_t& mesh = node->m_mesh;
		glm::vec3 origin = mesh.origin;
		
		stream << "# Node " << node->NodeID() << "\n";
		
		stream << "# - Mesh Verts\n";

		for (glm::vec3* v : mesh.verts)
			WriteVertex(*v + origin, stream);
		
		stream << "# - Cut Verts\n";
		for (glm::vec3* v : mesh.cutVerts)
			WriteVertex(*v + origin, stream);

		stream << "# - Part Norms\n";
		for (meshPart_t* p : mesh.parts)
			WriteNorm(p->normal, stream);

	}

	int vertOffset = 1;
	stream << "\n# Faces\n";
	int partNormOffset = 1;

	for (auto p : GetWorldEditor().m_nodes)
	{
		CNode* node = p.second;

		cuttableMesh_t& mesh = node->m_mesh;
		stream << "# Node " << node->NodeID() << "\n";
		stream << "# Floor Face\n";
		

		for (auto p : mesh.parts)
		{

			for (auto f : p->tris)
			{
				// If we have < 3 verts, dump this bozo
				if (f->verts.size() < 3)
				{
					continue;
				}

				stream << "f";
				for (auto v : f->verts)
				{
					uint16_t vert = 0;
					auto f = std::find(mesh.verts.begin(), mesh.verts.end(), v->vert);
					if (f != mesh.verts.end())
					{
						vert = f - mesh.verts.begin();// v->vert - vertData;
					}
					else
					{
						f = std::find(mesh.cutVerts.begin(), mesh.cutVerts.end(), v->vert);
						if (f != mesh.cutVerts.end())
						{
							vert = mesh.verts.size() + (f - mesh.cutVerts.begin());// v->vert - vertData;
						}
						else
						{
							Log::Fault("[MeshRenderer] Mesh refering to vert not in list!!\n");
						}
					}

					stream << " " << (vert + vertOffset);
					stream << "//" << partNormOffset;
				}

				stream << "\n";
			}
			partNormOffset++;
		}

		vertOffset += mesh.verts.size() + mesh.cutVerts.size();
	}


	// Output
	std::string str = stream.str();
	// I think this is probably dumb, but oh well
	char* strBuf = new char[str.size() + 1];
	std::memcpy(strBuf, str.c_str(), str.size());
	strBuf[str.size()] = 0;
	
	return strBuf;
}
