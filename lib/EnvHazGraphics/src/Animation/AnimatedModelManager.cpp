#include "Animation/AnimatedModelManager.hpp"
#include "Animation/Animation.hpp"
#include "Animation/Animator.hpp"
#include "DataStructs.hpp"
#include "MeshManager.hpp"
#include "Utils/Alghorithms.hpp"
#include "Utils/HashedStrings.hpp"
#include "Utils/Math_Utils.hpp"
#include "glm/fwd.hpp"
#include "glm/matrix.hpp"

#include <algorithm>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
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
      joint.mOffsetMatrix = convertAssimpMatrixToGLM(curBone->mOffsetMatrix);

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

    childJoint.localBindTransform =
        convertAssimpMatrixToGLM(node->mTransformation);
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

void AnimatedModelManager::ComputeGlobalBindTransforms(
    aiNode *node, const glm::mat4 &parentGlobal) {
  glm::mat4 local = convertAssimpMatrixToGLM(node->mTransformation);
  glm::mat4 global = parentGlobal * local;

  auto it = m_BoneMap.find(node->mName.C_Str());
  if (it != m_BoneMap.end()) {
    int idx = it->second;
    Joint &j = processingSkeleton.m_Joints[idx];
    j.m_GlobalTransform = global;
    j.localBindTransform = local; // store global bind
    // Recompute inverse bind from the node global transform — often fixes
    // FBX/Mixamo root-scale problems
    j.mOffsetMatrix = glm::inverse(global);
  }

  for (unsigned int i = 0; i < node->mNumChildren; ++i)
    ComputeGlobalBindTransforms(node->mChildren[i], global);
}

AnimatedModel AnimatedModelManager::LoadAnimatedModel(std::string path) {

  processingSkeleton.m_Joints.clear();
  processingSkeleton.finalMatrices.clear();

  std::vector<MeshID> r_meshes;

  eHazGraphics_Utils::HashedString hashedPath =
      eHazGraphics_Utils::computeHash(path);

  if (loadedModels.contains(hashedPath)) {
    return loadedModels[hashedPath];
  }

  scene = importer.ReadFile(
      path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals |
                aiProcess_CalcTangentSpace | aiProcess_PopulateArmatureData);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {

    SDL_Log("ERROR LOADING THE ANIMATED MODEL: %s", importer.GetErrorString());
  }

  BuildBaseSkeleton();

  r_meshes = processNode(scene->mRootNode);

  SetParentHierarchy(scene->mRootNode);

  ComputeGlobalBindTransforms(scene->mRootNode, glm::mat4(1.0f));
  processingSkeleton.m_RootTransform =
      convertAssimpMatrixToGLM(scene->mRootNode->mTransformation);
  processingSkeleton.m_InverseRoot =
      glm::inverse(processingSkeleton.m_RootTransform);
  std::cout << determinant(processingSkeleton.m_Joints[0].mOffsetMatrix)
            << std::endl;
  skeletons.push_back(std::make_shared<Skeleton>(processingSkeleton));

  AnimatedModel model;
  model.SetSkeleton(skeletons[skeletons.size() - 1]);

  animators.push_back(std::make_shared<Animator>());
  model.SetAnimatorID(animators.size() - 1);
  animators[animators.size() - 1]->SetSkeleton(model.GetSkeleton());

  /* for (int i = 0; i < processingSkeleton.m_Joints.size(); ++i) {
     // Recompute global bind using parent relationships
     glm::mat4 globalBind = processingSkeleton.m_Joints[i].localBindTransform;
     int parent = processingSkeleton.m_Joints[i].m_ParentJoint;
     if (parent != -1) {
       globalBind =
           processingSkeleton.m_Joints[parent].m_GlobalTransform * globalBind;
     }

     // Multiply by inverse bind (offset matrix)
     glm::mat4 test = globalBind * processingSkeleton.m_Joints[i].mOffsetMatrix;

     // If bind was correct, test ≈ identity
     if (glm::length(glm::vec3(test[0][0] - 1.0f, test[1][1] - 1.0f,
                               test[2][2] - 1.0f)) > 0.01f) {
       std::cout << "Joint " << processingSkeleton.m_Joints[i].m_Name
                 << " has incorrect bind pose!\n";
     }
   } */
  for (int i = 0; i < processingSkeleton.m_Joints.size(); ++i) {

    // **CRITICAL FIX:** Do NOT recompute the global bind from parent indices.
    // Instead, use the correct global bind transform already calculated
    // and stored by ComputeGlobalBindTransforms (which used the full Assimp
    // node path).
    glm::mat4 globalBind = processingSkeleton.m_Joints[i].m_GlobalTransform;

    // The inverse bind matrix (offset matrix) was set to inverse(globalBind)
    // inside ComputeGlobalBindTransforms, so this check should now work for ALL
    // joints.
    glm::mat4 test = globalBind * processingSkeleton.m_Joints[i].mOffsetMatrix;

    // If bind was correct, test ≈ identity
    if (glm::length(glm::vec3(test[0][0] - 1.0f, test[1][1] - 1.0f,
                              test[2][2] - 1.0f)) > 0.01f) {
      std::cout << "Joint " << processingSkeleton.m_Joints[i].m_Name
                << " has incorrect bind pose (multi-root fix applied)!\n";
    }
  }
  // defjidsmgokdlmg;
  //
  //
  //
  //
  //
  //

  for (auto &meshPair : meshes) {
    Mesh &mesh = meshPair.second;
    for (size_t vi = 0; vi < mesh.GetMeshData().vertices.size(); ++vi) {
      auto &v = mesh.GetMeshData().vertices[vi];
      for (int k = 0; k < 4; ++k) {
        int id = v.boneIDs[k];
        if (id < 0 || id >= (int)processingSkeleton.m_Joints.size()) {
          std::cout << "Vertex " << vi << " bad bone id " << id << "\n";
        }
      }
      float sum =
          v.boneWeights.x + v.boneWeights.y + v.boneWeights.z + v.boneWeights.w;
      if (std::abs(sum - 1.0f) > 0.01f) {
        std::cout << "Vertex " << vi << " weight sum = " << sum << "\n";
      }
    }
  }

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

    float sum = finalData.boneWeights[0] + finalData.boneWeights[1] +
                finalData.boneWeights[2] + finalData.boneWeights[3];
    if (sum > 1e-6f) {

      for (int i = 0; i < 4; i++)

        finalData.boneWeights[i] /= sum;
    } else {

      for (int i = 0; i < 4; i++) {
        finalData.boneIDs[i] = (0);
        finalData.boneWeights[i] = (0.0f);
      }
    }
  }

  int jointCount = processingSkeleton.m_Joints.size();
  for (int k = 0; k < 4; k++) {
    if (finalData.boneIDs[k] < 0 || finalData.boneIDs[k] >= jointCount) {
      std::cout << "Vertex " << vertexID << " invalid bone id "
                << finalData.boneIDs[k] << "\n";
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

    VertexBoneData boneData = GetVertexBoneData(i, mesh);
    float sum = boneData.boneWeights[0] + boneData.boneWeights[1] +
                boneData.boneWeights[2] + boneData.boneWeights[3];
    if (sum < 1e-6f) {
      // Try to assign the vertex to a reasonable fallback bone:
      if (mesh->mNumBones > 0) {
        // Use the first bone of this mesh (most exporters attach the mesh to
        // one bone)
        std::string firstBoneName = mesh->mBones[0]->mName.data;
        if (m_BoneMap.contains(firstBoneName)) {
          int fallbackID = m_BoneMap[firstBoneName];
          boneData.boneIDs[0] = fallbackID;
          boneData.boneWeights[0] = 1.0f;
          boneData.boneIDs[1] = boneData.boneIDs[2] = boneData.boneIDs[3] = 0;
          boneData.boneWeights[1] = boneData.boneWeights[2] =
              boneData.boneWeights[3] = 0.0f;
        } else {
          // As a last resort, assign to bone 0
          //
          if (i > 0) {

            for (int l = 0; l < 4; l++) {

              boneData.boneIDs[l] = vertices[i - 1].boneIDs[l];
              boneData.boneWeights[l] = vertices[i - 1].boneWeights[l];
            }

          } else {

            boneData.boneIDs[0] = 0;
            boneData.boneWeights[0] = 0.0f;
            boneData.boneIDs[1] = boneData.boneIDs[2] = boneData.boneIDs[3] = 0;
            boneData.boneWeights[1] = boneData.boneWeights[2] =
                boneData.boneWeights[3] = 0.0f;
          }
        }
      } else {
        // Mesh had no bones at all, keep as rigid (no skinning). We still set
        // default bone 0 so shader can't index out of range.
        boneData.boneIDs[0] = 0;
        boneData.boneWeights[0] = 0.0f;
        boneData.boneIDs[1] = boneData.boneIDs[2] = boneData.boneIDs[3] = 0;
        boneData.boneWeights[1] = boneData.boneWeights[2] =
            boneData.boneWeights[3] = 0.0f;
      }
    }

    glm::ivec4 boneIDs = {boneData.boneIDs[0], boneData.boneIDs[1],
                          boneData.boneIDs[2], boneData.boneIDs[3]};

    glm::vec4 boneWeights = {boneData.boneWeights[0], boneData.boneWeights[1],
                             boneData.boneWeights[2], boneData.boneWeights[3]};

    vertex.boneIDs = boneIDs;
    vertex.boneWeights = boneWeights;

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
  for (auto &animator : animators) {
    animator->Update(deltaTime);
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

    auto &animator = animators[model->GetAnimatorID()];
    UploadBonesToGPU(
        animator->GetGPULocation(),
        animator->GetFinalMatrices()); // your existing submission logic
  }
}

} // namespace eHazGraphics
