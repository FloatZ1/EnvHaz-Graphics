#ifndef MESH_MANAGER_HPP
#define MESH_MANAGER_HPP





#include "BitFlags.hpp"
#include "DataStructs.hpp"
#include "ShaderManager.hpp"
#include "glad/glad.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cstddef>
#include <iterator>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>


namespace eHazGraphics
{

class Mesh
{
  private:
    // add variables for used textures and transforms.
    MeshData data{};
    ShaderComboID shaderID{};
    GLuint instances = 1;

    bool GPUresident = false;

  public:
    Mesh() = default;

    Mesh(MeshData data) : data(data) {};

    Mesh(MeshData data, ShaderComboID shaderID) : data(data), shaderID(shaderID) {};

    ShaderComboID GetShaderID() const
    {

        return shaderID;
    }

    void SetInstanceCount(GLuint numInstances)
    {
        instances = numInstances;
    }

    const GLuint &GetInstanceCount() const
    {
        return instances;
    }

    void SetShader(ShaderComboID &shader)
    {
        shaderID = shader;
    }


    std::pair<const Vertex *, const size_t> GetVertexData() const
    {
        return std::pair<const Vertex *, const size_t>{data.vertices.data(), data.vertices.size() * sizeof(Vertex)};
    }
    std::pair<const GLuint *, const size_t> GetIndexData() const
    {
        return std::pair<const GLuint *, const size_t>{data.indecies.data(), data.indecies.size() * sizeof(GLuint)};
    }

    bool isResident() const
    {
        return GPUresident;
    }
};



class Model
{
  public:
    void AddMesh(MeshID ID)
    {
        meshes.push_back(ID);
    }

    const std::vector<MeshID> &GetMeshIDs() const
    {
        return meshes;
    }

    const unsigned int GetMaterialID() const
    {
        return materialID;
    }
    const unsigned int GetInstanceCount() const
    {
        return instanceCount;
    }

    void SetInstanceCount(unsigned int count)
    {
        instanceCount = count;
    }


  private:
    std::vector<MeshID> meshes;
    unsigned int materialID = 0;
    unsigned int instanceCount = 1;
};




class MeshManager
{
  public:
    void Initialize(); // TODO: IMPLEMENT
    Model LoadModel(std::string &path);

    void EraseMesh(MeshID mesh)
    {


        // TODO: erase from mesh maps as well
        meshes.erase(mesh);
    }



    void SetModelShader(Model &model, ShaderComboID &shader)
    {

        for (auto &mesh : model.GetMeshIDs())
        {
            auto it = meshes.find(mesh);
            if (it != meshes.end())
            {
                it->second.SetShader(shader);
            }
            else
            {
                SDL_Log("ERROR, COULD NOT ASSIGN SHADER\n");
                // Optionally log a warning: mesh ID not found
            }
        }
    }


    void SetModelInstanceCount(Model &model, unsigned int count)
    {

        for (auto &mesh : model.GetMeshIDs())
        {

            meshes[mesh].SetInstanceCount(count);
        }

        model.SetInstanceCount(count);
    }



    const Mesh &GetMesh(MeshID id)
    {
        return meshes[id];
    }

    void Destroy(); // TODO: IMPLEMENT

  private:
    std::vector<MeshID> processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    /* vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                          string typeName); */


    unsigned int maxID = 0;

    std::unordered_map<MeshID, Mesh> meshes;
    std::unordered_map<std::string, MeshID> meshPaths;



    Assimp::Importer importer;
};















} // namespace eHazGraphics




#endif
