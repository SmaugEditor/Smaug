#include "meshrenderer.h"
#include "utils.h"
#include "modelmanager.h"
#include "worldeditor.h"
#include "mesh.h"

#include "GLFW/glfw3.h"
#include <debugdraw.h>

struct rMeshVertex_t
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec4 txdata;
};
static bgfx::VertexLayout MeshVertexLayout()
{
	static bgfx::VertexLayout layout;
	static bool init = true;
	if (init)
	{
		init = false;
		layout.begin()
			.add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Normal,    3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord1, 4, bgfx::AttribType::Float)
			.end();
	}
	
	return layout;
}


void CMeshRenderer::SetOwner(CNode* node)
{
	m_node = node;
	m_mesh = &node->m_mesh;
	m_empty = true;
}

void CMeshRenderer::Destroy()
{
	if(m_vertexBuf.idx != bgfx::kInvalidHandle)
		bgfx::destroy(m_vertexBuf);
	if (m_indexBuf.idx != bgfx::kInvalidHandle)
		bgfx::destroy(m_indexBuf);
}

void CMeshRenderer::Rebuild()
{
	
	m_partRenderData.clear();

	// Sort our parts into batches per texture
	for (auto p : m_mesh->parts)
	{
		if (m_partRenderData.find(p->txData.texture) == m_partRenderData.end())
		{
			// Didn't find it; let's make a new struct for all our nice data
			m_partRenderData.insert({ p->txData.texture, {} });
		}
		m_partRenderData[p->txData.texture].parts.push_back(p);
	}

	const bgfx::Memory* vertexBuf = nullptr;
	const bgfx::Memory* indexBuf = nullptr;
	BuildRenderData(vertexBuf, indexBuf);
	if (!vertexBuf || !indexBuf)
	{
		m_empty = true;
		return;
	}

	m_empty = false;

	if (m_vertexBuf.idx != bgfx::kInvalidHandle)
	{
		bgfx::update(m_vertexBuf, 0, vertexBuf);
		bgfx::update(m_indexBuf, 0, indexBuf);
	}
	else
	{
		m_vertexBuf = bgfx::createDynamicVertexBuffer(vertexBuf, MeshVertexLayout(), BGFX_BUFFER_ALLOW_RESIZE);
		m_indexBuf = bgfx::createDynamicIndexBuffer(indexBuf, BGFX_BUFFER_ALLOW_RESIZE);
	}
}

void CMeshRenderer::Draw(bgfx::ViewId viewID, bgfx::ProgramHandle shader)
{
	CTransform t;
	Draw(t, viewID, shader);
}

void CMeshRenderer::Draw(CTransform trnsfm, bgfx::ViewId viewID, bgfx::ProgramHandle shader)
{
	if (m_empty)
		return;

	trnsfm.SetLocalOrigin(trnsfm.GetLocalOrigin() + m_mesh->origin);
	glm::mat4 mtx = trnsfm.Matrix();
	
	for (auto b : m_partRenderData)
	{
		ShaderManager().SetTexture(b.first);
		bgfx::setTransform(&mtx[0][0]);
		bgfx::setVertexBuffer(0, m_vertexBuf);
		bgfx::setIndexBuffer(m_indexBuf, b.second.indexStart, b.second.indexCount);
		bgfx::submit(viewID, shader);
	}
}

bool CMeshRenderer::Empty()
{
	return m_empty || m_node->IsNewBorn();
}


void CMeshRenderer::BuildRenderData(const bgfx::Memory*& vertBuf, const bgfx::Memory*& indexBuf)
{
	if (!m_partRenderData.size())
		return;

	int indexCount = 0;
	int vertexCount = 0;
	for (auto b : m_partRenderData)
		for(auto p : b.second.parts)
		{
			if (p->sliced)
			{
				indexCount += p->tris.size() * 3; // 3 indexes per tri
				vertexCount += p->verts.size() + p->sliced->cutVerts.size();
			}
			else
			{
				indexCount += p->tris.size() * 3; // 3 indexes per tri
				vertexCount += p->verts.size();
			}
		}

	//SASSERT(indexCount > 0);
	if (indexCount == 0)
		return;
	
	// Allocate our buffers
	// bgfx cleans these up for us
	vertBuf = bgfx::alloc(MeshVertexLayout().getSize(vertexCount));
	indexBuf = bgfx::alloc(indexCount * sizeof(uint16_t)); // Each face is a tri

	uint16_t* idxData = (uint16_t*)indexBuf->data;
	rMeshVertex_t* vtxData = (rMeshVertex_t*)vertBuf->data;

	int vOffset = 0;
	int iOffset = 0;

	for (auto& b : m_partRenderData)
	{
		b.second.indexStart = iOffset;

		for (auto p : b.second.parts)
		{

			glm::vec3 norm = p->normal;
			glm::vec4 txdata = { p->txData.offset.x, p->txData.offset.y, p->txData.scale.x, p->txData.scale.y };
			int vs = vOffset;
			for (auto v : p->verts)
			{
				vtxData[vOffset] = { *v->vert, norm, txdata };
				vOffset++;
			}
			int cv = vOffset;
		
			if(p->sliced)
				for (auto v : p->sliced->cutVerts)
				{
					vtxData[vOffset] = { *v, norm, txdata };
					vOffset++;
				}


			for (auto f : p->tris)
			{
				//SASSERT(f->verts.size() == 3);
				if (f->verts.size() != 3)
					continue;
				/*
				// If our verts are out of order, this will fail. Should we use the HE data to determine this?
				halfEdge_t* he = f->edges[0];
				do
				{

					uint16_t vert = he->vert->vert - vertData;
					if (vert > m_part.verts.size())
						printf("[MeshRenderer] Mesh refering to vert not in list!!\n");

					idxData[offset] = vert;
					offset++;
					he = he->next;
				} while (he != f->edges[0]);
				*/
				for (auto v : f->verts)
				{
					uint16_t vert = 0;
					bool found = false;
					for (int j = 0; j < p->verts.size(); j++)
					{
						if (p->verts[j]->vert == v->vert)
						{
							found = true;
							vert = vs + j;
							break;
						}
					}

					if(!found)
					{

						if (p->sliced)
							for (int j = 0; j < p->sliced->cutVerts.size(); j++)
							{
								if (p->sliced->cutVerts[j] == v->vert)
								{
									found = true;
									vert = cv + j;
									break;
								}
							}
					}

					if(!found)
					{
						Log::Fault("[MeshRenderer] Mesh refering to vert not in list!!\n");
					}

					idxData[iOffset] = vert;
					iOffset++;
				}

			}
		}

		b.second.indexCount = iOffset - b.second.indexStart;
	}

	SASSERT(vOffset <= vertexCount);
	SASSERT(iOffset <= indexCount);

	return;

}
