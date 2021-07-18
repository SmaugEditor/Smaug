#include "modelmanager.h"
#include "texturemanager.h"
#include "basicdraw.h"
#include "shadermanager.h"
#include "log.h"

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
    virtual void Render(CModelTransform* transform);

    ModelVertex* m_vertices;
    size_t m_vertCount;

    uint16_t* m_indices;
    size_t m_indicieCount;

    bgfx::VertexBufferHandle        m_vertexBuf;
    bgfx::IndexBufferHandle         m_indexBuf;
    texture_t                       m_texture;

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
        //bgfx::submit(ModelManager().CurrentView(), ShaderManager().GetShaderProgram(Shader::ERROR_MODEL_SHADER));// , BGFX_DISCARD_NONE);
    }
    virtual void Render(CModelTransform* transform)
    {
        float time = glfwGetTime();

        glm::mat4 mtx = glm::identity<glm::mat4>();
        mtx = glm::translate(mtx, transform ? transform->GetAbsOrigin() : glm::vec3(0));
        mtx = glm::scale(mtx, transform ? transform->GetAbsScale() : glm::vec3(2.5));
        mtx *= glm::yawPitchRoll(time * 1.75f, time * 0.5f, 0.0f);
        BasicDraw().Cube(mtx);
       // bgfx::submit(ModelManager().CurrentView(), ShaderManager().GetShaderProgram(Shader::ERROR_MODEL_SHADER));// , BGFX_DISCARD_NONE);
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
        if (mesh->mNormals)
            vec = mesh->mNormals[i];
        else
            vec = { 0,1,0 };
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
}

void CModelManager::Shutdown()
{
 
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
    {
        // Oh, no. We failed to load the image...
        Log::TWarn("Failed to load model %s\n", path);
        return GetErrorModel();
    }

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
    else
        model->m_texture = TextureManager().ErrorTexture();


    // Add the model to the map
    m_modelMap.emplace(std::string(path), model);



    Log::TPrint("Loaded model %s\n", path);
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
    mtx *= glm::yawPitchRoll(angle.y, angle.x, angle.z);


    ShaderManager().SetTexture(m_texture);
    bgfx::setTransform(&mtx[0][0]);
    bgfx::setVertexBuffer(0, m_vertexBuf);
    bgfx::setIndexBuffer(m_indexBuf);

    bgfx::submit(ModelManager().CurrentView(), ShaderManager().GetShaderProgram(Shader::MODEL_SHADER));// , BGFX_DISCARD_NONE);
}

void CModel::Render(CModelTransform* transform)
{
    glm::mat4 mtx = transform ? transform->Matrix() : glm::identity<glm::mat4>();

    ShaderManager().SetTexture(m_texture);
    bgfx::setTransform(&mtx[0][0]);
    bgfx::setVertexBuffer(0, m_vertexBuf);
    bgfx::setIndexBuffer(m_indexBuf);

    bgfx::submit(ModelManager().CurrentView(), ShaderManager().GetShaderProgram(Shader::MODEL_SHADER));// , BGFX_DISCARD_NONE);
}

CModelTransform::CModelTransform()
{
    m_position = { 0.0f,0.0f,0.0f };
    m_angles   = { 0.0f,0.0f,0.0f };
    m_scale    = { 1.0f,1.0f,1.0f };
    m_pParent  = nullptr;

}

void CModelTransform::SetParent(CModelTransform* parent)
{
    if (parent != this)
        m_pParent = parent;
}

void CModelTransform::SetAbsAngles(glm::vec3 ang)
{
    for (CModelTransform* p = m_pParent; p; p = p->m_pParent)
    {
        ang -= p->m_angles;
    }

    m_angles = ang;
}

void CModelTransform::SetAbsOrigin(glm::vec3 pos)
{
    for (CModelTransform* p = m_pParent; p; p = p->m_pParent)
    {
        pos -= p->m_position;
    }

    m_position = pos;
}

void CModelTransform::SetAbsScale(glm::vec3 scale)
{
    for (CModelTransform* p = m_pParent; p; p = p->m_pParent)
    {
        scale /= p->m_scale;
    }

    m_scale = scale;
}

glm::vec3 CModelTransform::GetAbsAngles()
{
    glm::vec3 ang = m_angles;
    for (CModelTransform* p = m_pParent; p; p = p->m_pParent)
    {
        ang += p->m_angles;
    }

    return ang;
}

glm::vec3 CModelTransform::GetAbsOrigin()
{
    glm::vec3 pos = m_position;
    for (CModelTransform* p = m_pParent; p; p = p->m_pParent)
    {
        pos += p->m_position;
    }

    return pos;
}

glm::vec3 CModelTransform::GetAbsScale()
{
    glm::vec3 scale = m_scale;
    for (CModelTransform* p = m_pParent; p; p = p->m_pParent)
    {
        scale *= p->m_scale;
    }

    return scale;
}

glm::mat4 CModelTransform::Matrix()
{
    glm::mat4 mtx = glm::identity<glm::mat4>();
    if (m_pParent)
        mtx = m_pParent->Matrix();
    mtx = glm::translate(mtx, m_position);
    mtx *= glm::yawPitchRoll(m_angles.y, m_angles.x, m_angles.z);
    mtx = glm::scale(mtx, m_scale);

    return mtx;
}
