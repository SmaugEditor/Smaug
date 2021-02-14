#include "modelmanager.h"
#include "texturemanager.h"
#include "basicdraw.h"
#include "shadermanager.h"

#include <GLFW/glfw3.h>

#include <assimp/Importer.hpp> 
#include <assimp/scene.h> 
#include <assimp/postprocess.h>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/geometric.hpp>

#include <filesystem>

////////////
// Models //
////////////

static bgfx::UniformHandle      s_textureUniform;
class CModel : public IModel
{
public:
    struct ModelVertex
    {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    ~CModel();
    virtual void Render(glm::vec3 origin, glm::vec3 angle, glm::vec3 scale);

    ModelVertex* m_vertices;
    size_t m_vertCount;

    uint16_t* m_indices;
    size_t m_indicieCount;

    bgfx::VertexBufferHandle        m_vertexBuf;
    bgfx::IndexBufferHandle         m_indexBuf;
    bgfx::TextureHandle             m_texture;

    friend class CModelManager;
    friend class CModelMaker;
};

// Used when a model load fails
class CErrorModel : public IModel
{
public:
    virtual void Render(glm::vec3 origin, glm::vec3 angle, glm::vec3 scale)
    {
        float time = glfwGetTime();
        
        glm::mat4 mtx = glm::identity<glm::mat4>();
        mtx = glm::translate(mtx, origin);
        mtx = glm::scale(mtx, scale);
        mtx *= glm::yawPitchRoll(time * 1.75f, time * 0.5f, 0.0f);
        BasicDraw().Cube(mtx);
        bgfx::submit(ModelManager().CurrentView(), ShaderManager::GetShaderProgram(Shader::ERROR_MODEL_SHADER));// , BGFX_DISCARD_NONE);
    }
};

static IModel* GetErrorModel()
{
    static CErrorModel s_errorModel;
    return &s_errorModel;
}

struct ModelVertLayout
{
    ModelVertLayout()
    {
        layout
            .begin()
            .add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Normal,    3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .end();
    }
    bgfx::VertexLayout layout;
};
static ModelVertLayout s_vertLayout;

///////////////////
// Model Manager //
///////////////////

CModelManager& ModelManager()
{
    static CModelManager s_modelManager;
    return s_modelManager;
}

static void AddMeshToModel(CModel* model, aiMesh* mesh)
{
    // Verts
    size_t vertCount = mesh->mNumVertices;
    model->m_vertCount = vertCount;
    model->m_vertices = new CModel::ModelVertex[vertCount];
        
    assert(vertCount < INT16_MAX);
    for (size_t i = 0; i < vertCount; i++)
    {
        CModel::ModelVertex& mv = model->m_vertices[i];
            
        // Pos
        aiVector3D vec = mesh->mVertices[i];
        mv.pos = { vec.x, vec.y, vec.z };
            
        // Normal
        vec = mesh->mNormals[i];
        mv.normal = { vec.x, vec.y, vec.z };

        // UV
        if (mesh->mTextureCoords[0])
        {
            aiVector3D texVec = mesh->mTextureCoords[0][i];
            mv.uv = { texVec.x, texVec.y };
        }
        else
            mv.uv = { 0,0 };
    }
        
    // Faces
    size_t faceNum = mesh->mNumFaces;
    model->m_indicieCount = faceNum * 3;
    model->m_indices = new uint16_t[model->m_indicieCount];
    size_t index = 0;

    for (size_t i = 0; i < faceNum; i++)
    {
        // must be a tri
        assert(mesh->mFaces[i].mNumIndices == 3);
            
        model->m_indices[index    ] = mesh->mFaces[i].mIndices[0];
        model->m_indices[index + 1] = mesh->mFaces[i].mIndices[1];
        model->m_indices[index + 2] = mesh->mFaces[i].mIndices[2];
        index += 3;
    }

}

CModelManager::CModelManager()
{
    // Move else where?
    s_textureUniform = bgfx::createUniform("s_textureUniform", bgfx::UniformType::Sampler);
}

void CModelManager::Shutdown()
{
    // Move elsewhere?
    bgfx::destroy(s_textureUniform);

    for (auto pair : m_modelMap)
    {
        delete pair.second;
    }
    m_modelMap.clear();
}

IModel* CModelManager::LoadModel(const char* path)
{
    auto modelSearch = m_modelMap.find(path);
    if (modelSearch != m_modelMap.end())
    {
        // Turns out, we already have this loaded.
        // Let's just return it then
        return modelSearch->second;
    }

    // We don't have it loaded... Guess we'll have to grab it then!


    Assimp::Importer importer;

    // And have it read the given file with some example postprocessing
    // Usually - if speed is not the most important aspect for you - you'll
    // probably to request more postprocessing than we do in this example.
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType);
    
    if (!scene || scene->mNumMeshes == 0)
        return GetErrorModel();

    CModel* model = new CModel;
    aiMesh* mesh = scene->mMeshes[0];
    AddMeshToModel(model, mesh);

    model->m_vertexBuf = bgfx::createVertexBuffer(bgfx::makeRef(model->m_vertices, sizeof(CModel::ModelVertex) * model->m_vertCount), s_vertLayout.layout);
    model->m_indexBuf  = bgfx::createIndexBuffer(bgfx::makeRef(model->m_indices, sizeof(uint16_t) * model->m_indicieCount));

    if(scene->mNumMaterials > 0 && mesh->mMaterialIndex != 0)
    {
        aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
        aiString str;
        mat->GetTexture(aiTextureType::aiTextureType_DIFFUSE, /* index */ 0, &str);
        
        // Empty check
        if (*str.C_Str())
        {
            std::filesystem::path p = path;
            p.remove_filename();
            p.append(str.C_Str());
            model->m_texture = TextureManager().LoadTexture((char*)p.u8string().c_str());
        }
        else
            model->m_texture = TextureManager().ErrorTexture();
    }

    // Add the model to the map
    m_modelMap.emplace(std::string(path), model);

    return model;
}



IModel* CModelManager::ErrorModel()
{
    return GetErrorModel();
}

// Model

CModel::~CModel()
{
    delete[] m_vertices;
    delete[] m_indices;
    bgfx::destroy(m_vertexBuf);
    bgfx::destroy(m_indexBuf);
}

void CModel::Render(glm::vec3 origin, glm::vec3 angle, glm::vec3 scale)
{
    glm::mat4 mtx = glm::identity<glm::mat4>();
    mtx = glm::translate(mtx, origin);
    mtx = glm::scale(mtx, scale);
    mtx *= glm::yawPitchRoll(angle.x, angle.y, angle.z);

    
    if (m_texture.idx != bgfx::kInvalidHandle)
        bgfx::setTexture(0, s_textureUniform, m_texture);
    bgfx::setTransform(&mtx[0][0]);
    bgfx::setVertexBuffer(0, m_vertexBuf);
    bgfx::setIndexBuffer(m_indexBuf);

    bgfx::submit(ModelManager().CurrentView(), ShaderManager::GetShaderProgram(Shader::MODEL_SHADER));// , BGFX_DISCARD_NONE);
}
