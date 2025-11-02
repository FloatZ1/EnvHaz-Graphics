#include "Animation/AnimatedModelManager.hpp"
#include "Animation/Animation.hpp"
#include "DataStructs.hpp"
#include "MeshManager.hpp"
#include "Utils/HashedStrings.hpp"
#include "Utils/Math_Utils.hpp"
#include "glm/fwd.hpp"

#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace eHazGraphics_Utils;
namespace eHazGraphics {

/*
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

void AnimatedModelManager::BuildBaseSkeleton() {

  for (int i = 0; i < scene->mNumMeshes; i++) {
    aiMesh *curMesh = scene->mMeshes[i];

    for (int j = 0; j < curMesh->mNumBones; j++) {
      aiBone *curBone = curMesh->mBones[j];
      std::string boneName = curBone->mName.data;

      // CRITICAL CHECK: If the bone is already processed, skip it.
      if (m_BoneMap.contains(boneName)) {
        continue;
      }

      Joint joint;
      joint.m_Name = boneName;
      // ... set mOffsetMatrix ...

      // Add the new, unique joint
      processingSkeleton.m_Joints.push_back(joint);
      int jointIndex = processingSkeleton.m_Joints.size() - 1;

      // Map the name to the new, unique index
      m_BoneMap[boneName] = jointIndex;
    }
  }
}

void AnimatedModelManager::SetParentHierarchy(aiNode *node) {
  if (m_BoneMap.contains(node->mName.data)) {
    int childJointIndex = m_BoneMap[node->mName.data];
    Joint &childJoint = processingSkeleton.m_Joints[childJointIndex];

    if (node->mParent && m_BoneMap.contains(node->mParent->mName.data)) {
      int parentJointIndex = m_BoneMap[node->mParent->mName.data];
      childJoint.m_ParentJoint = parentJointIndex;
    } else {
      childJoint.m_ParentJoint = -1;
    }
  }

  for (unsigned int i = 0; i < node->mNumChildren; i++) {

    SetParentHierarchy(node->mChildren[i]);
  }
}

/*
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */
AnimatedModel AnimatedModelManager::LoadAnimatedModel(std::string path) {

  std::vector<MeshID> r_meshes;

  eHazGraphics_Utils::HashedString hashedPath =
      eHazGraphics_Utils::computeHash(path);

  if (loadedModels.contains(hashedPath)) {
    return loadedModels[hashedPath];
  }

  scene = importer.ReadFile(
      path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals |
                /*aiProcess_PreTransformVertices |*/ aiProcess_OptimizeMeshes);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {

    SDL_Log("ERROR LOADING THE ANIMATED MODEL: %s", importer.GetErrorString());
  }

  BuildBaseSkeleton();

  r_meshes = processNode(scene->mRootNode);

  SetParentHierarchy(scene->mRootNode);

  skeletons.push_back(processingSkeleton);

  AnimatedModel model;
  model.SetSkeletonID(skeletons.size() - 1);

  for (auto &mesh : r_meshes) {

    model.AddMesh(mesh);
  }

  return model;
}

std::vector<MeshID> AnimatedModelManager::processNode(aiNode *node) {
  std::vector<MeshID> meshIDs;
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    maxID += 1;
    // meshes[maxID] = processMesh(mesh, scene);

    meshes.try_emplace(maxID, processMesh(mesh));

    meshIDs.push_back(maxID);
  }

  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    std::vector<MeshID> vec = processNode(node->mChildren[i]);

    for (MeshID id : vec) {

      meshIDs.push_back(id);
    }
  }

  return meshIDs;
}

void AnimatedModelManager::PopulateMeshBoneData(aiMesh *mesh) {
  m_CurrentMeshBoneData.clear();

  for (unsigned int i = 0; i < mesh->mNumBones; i++) {
    aiBone *curBone = mesh->mBones[i];
    int ehazBoneID = m_BoneMap[curBone->mName.data]; // Fast map lookup

    // Correctly iterate over all weights the bone contributes
    for (unsigned int j = 0; j < curBone->mNumWeights; j++) {
      aiVertexWeight weight = curBone->mWeights[j];

      // Map the VertexID to its list of (BoneID, Weight) pairs
      m_CurrentMeshBoneData[weight.mVertexId].push_back(
          {ehazBoneID, weight.mWeight});
    }
  }
  // Optimization: Sort the lists by weight descending here if you only want the
  // top 4 Otherwise, the original order (usually fine) is kept.
}
AnimatedModelManager::VertexBoneData
AnimatedModelManager::GetVertexBoneData(int vertexID, aiMesh *mesh) {
  // We ignore aiMesh* because all necessary weight/ID data for the current mesh
  // should have been pre-processed into m_CurrentMeshBoneData.

  VertexBoneData finalData = {}; // Initialize to 0s
  unsigned int u_vertexID = (unsigned int)vertexID;

  // 1. Perform a fast lookup on the pre-populated map
  if (m_CurrentMeshBoneData.contains(u_vertexID)) {

    const auto &assignments = m_CurrentMeshBoneData.at(u_vertexID);

    // 2. Assign the top influences (limited to 4)
    for (size_t i = 0; i < std::min((size_t)4, assignments.size()); ++i) {

      // assignments[i].first  = The Bone Index (ehazBoneID)
      // assignments[i].second = The Bone Weight
      finalData.boneIDs[i] = assignments[i].first;
      finalData.boneWeights[i] = assignments[i].second;
    }
  }

  return finalData;
}
Mesh AnimatedModelManager::processMesh(aiMesh *mesh) {
  std::vector<Vertex> vertices;
  std::vector<GLuint> indices;

  PopulateMeshBoneData(mesh);

  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
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
    } else
      vertex.UV = glm::vec2(0.0f, 0.0f);

    if (mesh->HasBones()) {
      // TODO: IMPLEMENT BONE GET

      VertexBoneData boneData = GetVertexBoneData(i, mesh);

      glm::ivec4 boneIDs = {boneData.boneIDs[0], boneData.boneIDs[1],
                            boneData.boneIDs[2], boneData.boneIDs[3]};

      glm::vec4 boneWeights = {boneData.boneWeights[0], boneData.boneWeights[1],
                               boneData.boneWeights[2],
                               boneData.boneWeights[3]};

      vertex.boneIDs = boneIDs;
      vertex.boneWeights = boneWeights;

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

  return Mesh({vertices, indices}, ShaderComboID());
}

/*
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

void AnimatedModelManager::LoadAnimation(int skeletonID, std::string &path,
                                         int &r_AnimationID) {}

void AnimatedModelManager::UploadBonesToGPU(
    BufferRange &range, std::vector<glm::mat4> finalMatrices) {

  if (range.OwningBuffer != 2) {
    range = bufferManager->InsertNewDynamicData(
        finalMatrices.data(), finalMatrices.size() * sizeof(glm::mat4),
        TypeFlags::BUFFER_ANIMATION_DATA);
  } else {
    bufferManager->UpdateData(range, finalMatrices.data(),
                              finalMatrices.size() * sizeof(glm::mat4));
  }
}

void AnimatedModelManager::Update(float deltaTime) {
  for (Skeleton &skeleton : skeletons) {
    skeleton.Update(deltaTime);
  }

  // Submit to GPU or instance buffer
  for (AnimatedModel *model : submittedAnimatedModels) {

    auto Instances = model->GetInstances();
    auto InstanceRanges = model->GetInstanceRanges();
    assert(Instances.size() == InstanceRanges.size());

    for (unsigned int i = 0; i < Instances.size(); i++) {

      bufferManager->UpdateData(InstanceRanges[i], &Instances[i],
                                sizeof(InstanceData));
    }

    Skeleton &skeleton = skeletons[model->GetSkeletonID()];
    UploadBonesToGPU(skeleton.GPUlocation,
                     skeleton.finalMatrices); // your existing submission logic
  }
}

} // namespace eHazGraphics
