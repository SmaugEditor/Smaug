#include "meshrenderer.h"
#include "utils.h"
#include "GLFW/glfw3.h"

#include "modelmanager.h"

struct rMeshVertex_t
{
	glm::vec3 pos;
	glm::vec3 normal;
};
static bgfx::VertexLayout MeshVertexLayout()
{
	static bgfx::VertexLayout layout;
	static bool init = true;
	if (init)
	{
		init = false;
		layout.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Normal,   3, bgfx::AttribType::Float)
			.end();
	}
	
	return layout;
}


CMeshRenderer::CMeshRenderer(cuttableMesh_t& mesh) : m_mesh(mesh), m_indexCount(0)
{
}

CMeshRenderer::~CMeshRenderer()
{
	bgfx::destroy(m_vertexBuf);
	bgfx::destroy(m_indexBuf);
}

void CMeshRenderer::RebuildRenderData()
{
	const bgfx::Memory* vertBuf;
	const bgfx::Memory* indexBuf;

	BuildRenderData(vertBuf, indexBuf);

	if (m_vertexBuf.idx != bgfx::kInvalidHandle)
	{
		bgfx::update(m_vertexBuf, 0, vertBuf);
		bgfx::update(m_indexBuf, 0, indexBuf);
	}
	else
	{
		m_vertexBuf = bgfx::createDynamicVertexBuffer(vertBuf, MeshVertexLayout(), BGFX_BUFFER_ALLOW_RESIZE);
		m_indexBuf = bgfx::createDynamicIndexBuffer(indexBuf, BGFX_BUFFER_ALLOW_RESIZE);
	}
}

void CMeshRenderer::Render()
{
	CModelTransform t;
	Render(t);
}

void CMeshRenderer::Render(CModelTransform trnsfm)
{
	trnsfm.SetLocalOrigin(trnsfm.GetLocalOrigin() + m_mesh.origin);
	glm::mat4 mtx = trnsfm.Matrix();
	
	bgfx::setTransform(&mtx[0][0]);
	bgfx::setVertexBuffer(0, m_vertexBuf);
	bgfx::setIndexBuffer(m_indexBuf, 0, m_indexCount);
	// This does not bgfx::submit!!
}


void CMeshRenderer::BuildRenderData(const bgfx::Memory*& vertBuf, const bgfx::Memory*& indexBuf)
{
	// We can only render tris!
	if (m_mesh.verts.size() < 3)
		return;

	int indexCount = 0;
	int vertexCount = 0;
	for (auto p : m_mesh.parts)
	{
		if (p->isCut)
		{
			indexCount += p->tris.size() * 3; // 3 indexes per tri
			vertexCount += p->verts.size() + p->cutVerts.size();
		}
		else
		{
			indexCount += p->tris.size() * 3; // 3 indexes per tri
			vertexCount += p->verts.size();

		}
		

	}

	SASSERT(indexCount > 0);

	// Allocate our buffers
	// bgfx cleans these up for us
	vertBuf = bgfx::alloc(MeshVertexLayout().getSize(vertexCount));
	indexBuf = bgfx::alloc(indexCount * sizeof(uint16_t)); // Each face is a tri

	uint16_t* idxData = (uint16_t*)indexBuf->data;
	rMeshVertex_t* vtxData = (rMeshVertex_t*)vertBuf->data;

	int vOffset = 0;
	int iOffset = 0;


	for (auto p : m_mesh.parts)
	{

		glm::vec3 norm = glm::normalize(faceNormal(p));
		int vs = vOffset;
		for (auto v : p->verts)
		{
			vtxData[vOffset] = { *v->vert, norm };
			vOffset++;
		}
		int cv = vOffset;
		for (auto v : p->cutVerts)
		{
			vtxData[vOffset] = { *v, norm };
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
					for (int j = 0; j < p->cutVerts.size(); j++)
					{
						if (p->cutVerts[j] == v->vert)
						{
							found = true;
							vert = cv + j;
							break;
						}
					}
				}

				if(!found)
				{
					printf("[MeshRenderer] Mesh refering to vert not in list!!\n");
				}

				idxData[iOffset] = vert;
				iOffset++;
			}

		}
	}

	SASSERT(vOffset <= vertexCount);
	SASSERT(iOffset <= indexCount);

	m_indexCount = iOffset;

}
