#include "MeshManager.hpp"
#include "DataStructs.hpp"
#include "glad/glad.h"
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <vector>



namespace eHazGraphics
{





Model MeshManager::LoadModel(std::string &path)
{
    std::vector<MeshID> temps;


    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {


        SDL_Log("ERROR LOADING THE MODEL: %s", importer.GetErrorString());
    }


    temps = (processNode(scene->mRootNode, scene));

    Model model;

    for (auto mesh : temps)
    {
        model.AddMesh(mesh);
    }

    importer.FreeScene();
    return model;
}

std::vector<MeshID> MeshManager::processNode(aiNode *node, const aiScene *scene)
{
    std::vector<MeshID> meshIDs;
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        maxID += 1;
        // meshes[maxID] = processMesh(mesh, scene);
        meshes.try_emplace(maxID, processMesh(mesh, scene));
        meshIDs.push_back(maxID);
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        std::vector<MeshID> vec = processNode(node->mChildren[i], scene);

        for (MeshID id : vec)
        {

            meshIDs.push_back(id);
        }
    }

    return meshIDs;
}
Mesh MeshManager::processMesh(aiMesh *mesh, const aiScene *scene)
{
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;


    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Postion = vector;

        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.Normal = vector;

        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.UV = vec;
        }
        else
            vertex.UV = glm::vec2(0.0f, 0.0f);



        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // material stuff here
    //
    //
    return Mesh({vertices, indices}, ShaderComboID());
}


void MeshManager::Initialize()
{
}

void MeshManager::Destroy()
{
}



} // namespace eHazGraphics
