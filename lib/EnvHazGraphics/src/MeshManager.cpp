#include "MeshManager.hpp"
#include "BufferManager.hpp"
#include "DataStructs.hpp"
#include "Utils/Alghorithms.hpp"
#include "Utils/HashedStrings.hpp"
#include "Utils/Math_Utils.hpp"
#include "glad/glad.h"
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <memory>
#include <vector>

using namespace eHazGraphics_Utils;

namespace eHazGraphics {

void MeshManager::EraseMesh(MeshID mesh) {

  // TODO: maybe find a way to clear the bufferManager buffer, but that could
  // create problems since meshes are different sizes, so preloading is best

  meshes[mesh].SetResidencyStatus(false);
  VertexIndexInfoPair &meshLoc = meshLocations[mesh];

  bufferManager->RemoveStaicRange(meshLoc);

  bufferManager->RemoveRange(meshTransformRanges[mesh]);

  meshes.erase(mesh);
  meshTransforms.erase(mesh);
  meshTransformRanges.erase(mesh);
  meshLocations.erase(mesh);
}

void MeshManager::SetModelShader(std::shared_ptr<Model> model,
                                 ShaderComboID &shader) {

  for (auto &mesh : model->GetMeshIDs()) {
    auto it = meshes.find(mesh);
    if (it != meshes.end()) {
      it->second.SetShader(shader);
    } else {
      SDL_Log("ERROR, COULD NOT ASSIGN SHADER\n");
      // Optionally log a warning: mesh ID not found
    }
  }
}
std::shared_ptr<Model> MeshManager::LoadModel(std::string &path) {
  std::vector<MeshID> temps;

  eHazGraphics_Utils::HashedString hashedPath =
      eHazGraphics_Utils::computeHash(path);

  if (loadedModels.contains(hashedPath)) {
    return loadedModels[hashedPath];
  }

  const aiScene *scene = importer.ReadFile(
      path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals |
                /*aiProcess_PreTransformVertices |*/ aiProcess_OptimizeMeshes);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {

    SDL_Log("ERROR LOADING THE MODEL: %s", importer.GetErrorString());
  }

  temps = (processNode(scene->mRootNode, scene));

  // Model model;
  std::shared_ptr<Model> model = std::make_shared<Model>();
  for (auto mesh : temps) {
    model->AddMesh(mesh);
  }
  model->SetID(hashedPath);

  importer.FreeScene();

  loadedModels.emplace(hashedPath, model);
  return model;
}

std::vector<MeshID> MeshManager::processNode(aiNode *node,
                                             const aiScene *scene) {
  std::vector<MeshID> meshIDs;
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];

    HashedString t_hsID = computeHash(mesh->mName.data);

    meshes.try_emplace(t_hsID, processMesh(mesh, scene));

    // TODO: DECIDE HOW TO DO THIS, currently meshTransforms is only used in
    // Renderer.cpp at the InsertStaticMesh part

    glm::mat4 relativeMat =
        eHazGraphics_Utils::convertAssimpMatrixToGLM(GetNodeToRootMat4(node));

    meshTransforms.emplace(t_hsID, relativeMat);
    meshes[t_hsID].setRelativeMatrix(relativeMat);
    meshes[t_hsID].SetID(t_hsID);

    AddTransformRange(t_hsID, bufferManager->InsertNewDynamicData(
                                  &relativeMat, sizeof(relativeMat),
                                  TypeFlags::BUFFER_STATIC_MATRIX_DATA));

    meshIDs.push_back(t_hsID);
  }

  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    std::vector<MeshID> vec = processNode(node->mChildren[i], scene);

    for (MeshID id : vec) {

      meshIDs.push_back(id);
    }
  }

  return meshIDs;
}
Mesh MeshManager::processMesh(aiMesh *mesh, const aiScene *scene) {
  std::vector<Vertex> vertices;
  std::vector<GLuint> indices;

  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    Vertex vertex;
    glm::vec3 vector;
    vector.x = mesh->mVertices[i].x;
    vector.y = mesh->mVertices[i].y;
    vector.z = mesh->mVertices[i].z;
    vertex.Position = vector;

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
    } else
      vertex.UV = glm::vec2(0.0f, 0.0f);

    if (mesh->HasBones()) {
      // TODO: IMPLEMENT BONE GET
    } else {
      vertex.boneIDs = glm::ivec4(0, 0, 0, 0);
      vertex.boneWeights = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    }

    vertices.push_back(vertex);
  }

  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; j++)
      indices.push_back(face.mIndices[j]);
  }

  // material stuff here
  //
  //
  return Mesh({vertices, indices}, ShaderComboID());
}

void MeshManager::Initialize(BufferManager *bufferManager) {
  this->bufferManager = bufferManager;
}

void MeshManager::Destroy() {}

} // namespace eHazGraphics
