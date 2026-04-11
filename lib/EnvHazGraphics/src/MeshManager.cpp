#include "MeshManager.hpp"
#include "BufferManager.hpp"
#include "DataStructs.hpp"
#include "Utils/Alghorithms.hpp"
#include "Utils/HashedStrings.hpp"
#include "Utils/Math_Utils.hpp"
#include "glad/glad.h"
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
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

  bufferManager->InvalidateStaticRange(meshLoc);

  // bufferManager->RemoveRange(meshTransformRanges[mesh]);

  meshes.erase(mesh);
  meshTransforms.erase(mesh);
  meshTransformRanges.erase(mesh);
  meshLocations.erase(mesh);
}

void MeshManager::SetModelShader(ModelID modelID, const ShaderComboID &shader) {

  std::shared_ptr<Model> model = GetModel(modelID);

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
AABB MeshManager::GetModelAABB(const aiScene *scene) {

  AABB finalBox;
  bool first = true;

  for (int i = 0; i < scene->mNumMeshes; i++) {
    const aiMesh *mesh = scene->mMeshes[i];
    if (!IsValid(mesh->mAABB))
      continue;

    AABB currentBox = ConvertAssimpAABB(mesh->mAABB);

    currentBox =
        currentBox.Transform(meshTransforms[computeHash(mesh->mName.data)]);

    if (first) {
      finalBox = currentBox;
      first = false;
    } else {
      finalBox = AABB::Combine(currentBox, finalBox);
    }
  }

  return finalBox;
}
ModelID MeshManager::LoadModel(std::string path) {
  std::vector<MeshID> temps;

  eHazGraphics_Utils::HashedString hashedPath =
      eHazGraphics_Utils::computeHash(path);

  if (loadedModels.contains(hashedPath)) {
    return hashedPath;
  }

  const aiScene *scene = importer.ReadFile(
      path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals |
                /*aiProcess_PreTransformVertices |*/ aiProcess_OptimizeMeshes |
                aiProcess_GenBoundingBoxes | aiProcess_CalcTangentSpace);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {

    SDL_Log("ERROR LOADING THE MODEL: %s", importer.GetErrorString());
  }

  temps = (processNode(scene->mRootNode, scene));

  // Bounding box processing

  AABB modelAABB = GetModelAABB(scene);

  // Model model;
  std::shared_ptr<Model> model = std::make_shared<Model>();
  for (auto mesh : temps) {
    model->AddMesh(mesh);
  }
  model->SetID(hashedPath);

  model->SetAABB(modelAABB);

  importer.FreeScene();

  loadedModels.emplace(hashedPath, model);

  modelPaths.emplace(hashedPath, path);

  return hashedPath;
}

std::vector<MeshID> MeshManager::processNode(aiNode *node,
                                             const aiScene *scene) {
  std::vector<MeshID> meshIDs;
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];

    HashedString t_hsID = computeHash(mesh->mName.data);

    meshes.emplace(t_hsID, processMesh(mesh, scene));

    // TODO: DECIDE HOW TO DO THIS, currently meshTransforms is only used in
    // Renderer.cpp at the InsertStaticMesh part

    glm::mat4 relativeMat =
        eHazGraphics_Utils::convertAssimpMatrixToGLM(GetNodeToRootMat4(node));

    meshTransforms.emplace(t_hsID, relativeMat);
    meshes[t_hsID].setRelativeMatrix(relativeMat);
    meshes[t_hsID].SetID(t_hsID);

    // AddTransformRange(t_hsID, bufferManager->InsertNewDynamicData(
    //                               &relativeMat, sizeof(relativeMat),
    //                              TypeFlags::BUFFER_STATIC_MATRIX_DATA));

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

    if (mesh->HasTangentsAndBitangents()) {

      // 1. Extract raw vec3 data from Assimp
      glm::vec3 n = vertex.Normal;
      glm::vec3 t =
          eHazGraphics_Utils::convertAssimpVec3ToGLM(mesh->mTangents[i]);
      glm::vec3 b =
          eHazGraphics_Utils::convertAssimpVec3ToGLM(mesh->mBitangents[i]);

      // 2. Calculate Handedness (the 'w' component)
      // We check if the bitangent from Assimp matches the direction of (Normal
      // x Tangent). If they point in opposite directions (dot < 0), w is -1.0.
      float handedness = (glm::dot(glm::cross(n, t), b) < 0.0f) ? -1.0f : 1.0f;

      // 3. Optional: Orthonormalize the Tangent
      // This fixes minor inaccuracies in the model's exported data.
      t = glm::normalize(t - n * glm::dot(t, n));

      // 4. Store as vec4
      // Tangent.w holds the handedness for bitangent reconstruction in the
      // shader. Bitangent.w is typically set to 1.0f for alignment/padding.
      vertex.Tangent = glm::vec4(t, handedness);
      vertex.Bitangent = b;
    } else {
      vertex.Tangent = glm::vec4(0.0f);
      vertex.Bitangent = glm::vec3(0.0f);
    }

    if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
    {
      glm::vec2 vec;
      vec.x = mesh->mTextureCoords[0][i].x;
      vec.y = mesh->mTextureCoords[0][i].y;
      vertex.UV = vec;
    } else
      vertex.UV = glm::vec2(0.0f, 0.0f);

    if (mesh->mTextureCoords[1]) {
      glm::vec2 vec;
      vec.x = mesh->mTextureCoords[1][i].x;
      vec.y = mesh->mTextureCoords[1][i].y;
      vertex.UV2 = vec;
    } else {
      vertex.UV2 = glm::vec2(0.0f, 0.0f);
    }

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
