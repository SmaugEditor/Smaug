#include "meshrenderer.h"
#include "utils.h"

static bgfx::VertexLayout MeshVertexLayout()
{
	static bgfx::VertexLayout layout;
	static bool init = true;
	if (init)
	{
		init = false;
		layout.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.end();
	}
	
	return layout;
}


CMeshRenderer::CMeshRenderer(cutttableShape_t& shape) : m_shape(shape)
{
	const bgfx::Memory* vertBuf;
	const bgfx::Memory* indexBuf;

	BuildRenderData(vertBuf, indexBuf);

	m_vertexBuf = bgfx::createDynamicVertexBuffer(vertBuf, MeshVertexLayout(), BGFX_BUFFER_ALLOW_RESIZE);
	m_indexBuf = bgfx::createDynamicIndexBuffer(indexBuf, BGFX_BUFFER_ALLOW_RESIZE);
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

	bgfx::update(m_vertexBuf, 0, vertBuf);
	bgfx::update(m_indexBuf, 0, indexBuf);
}

void CMeshRenderer::Render(CModelTransform& trnsfm)
{
	glm::mat4 mtx = trnsfm.Matrix();

	bgfx::setTransform(&mtx[0][0]);
	bgfx::setVertexBuffer(0, m_vertexBuf);
	bgfx::setIndexBuffer(m_indexBuf, 0, m_indexCount);
	// This does not bgfx::submit!!
}


void CMeshRenderer::BuildRenderData(const bgfx::Memory*& vertBuf, const bgfx::Memory*& indexBuf)
{
	triangluateShapeFaces(m_shape);
	// Allocate our buffers
	// bgfx cleans these up for us
	vertBuf = bgfx::alloc(m_shape.verts.size() * sizeof(glm::vec3));
	indexBuf = bgfx::alloc(m_shape.faces.size()  * sizeof(uint16_t) * 3 ); // Each face is a tri

	glm::vec3* vertData = m_shape.verts.data();
	uint16_t* idxData = (uint16_t*)indexBuf->data;

	memcpy(vertBuf->data, vertData, m_shape.verts.size() * sizeof(glm::vec3));

	int offset = 0;
	for (auto f : m_shape.faces)
	{
		/*
		// If our verts are out of order, this will fail. Should we use the HE data to determine this?
		halfEdge_t* he = f->edges[0];
		do
		{

			uint16_t vert = he->vert->vert - vertData;
			if (vert > m_shape.verts.size())
				printf("[MeshRenderer] Mesh refering to vert not in list!!\n");

			idxData[offset] = vert;
			offset++;
			he = he->next;
		} while (he != f->edges[0]);
		*/
		for (auto v : f->verts)
		{
			uint16_t vert = v->vert - vertData;
			if (vert > m_shape.verts.size())
				printf("[MeshRenderer] Mesh refering to vert not in list!!\n");

			idxData[offset] = vert;
			offset++;
		}

	}
	m_indexCount = offset;

}
