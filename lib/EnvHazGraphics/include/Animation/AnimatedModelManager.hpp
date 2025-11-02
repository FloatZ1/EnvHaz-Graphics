
#ifndef ENVHAZ_ANIMATED_MODEL_MANAGER_HPP
#define ENVHAZ_ANIMATED_MODEL_MANAGER_HPP

#include "Animation/Animator.hpp"
#include "BufferManager.hpp"
#include "DataStructs.hpp"
#include "MeshManager.hpp"
#include "Utils/HashedStrings.hpp"

#include "Animation.hpp"

#include "glm/ext/matrix_transform.hpp"
#include "tinygltf/tiny_gltf.h"
#include <glm/gtc/quaternion.hpp>
#include <map>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace eHazGraphics {

struct Joint {

  int m_GLobalGltfNodeIndex;
  std::string m_Name;

  glm::mat4 m_UndeformedNodeMatrix = glm::mat4(1.0f); // bind
  glm::mat4 m_InverseBindMatrix;

  glm::vec3 m_DeformedNodeTranslation = glm::vec3(0.0f);
  glm::quat m_DeformedNodeRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
  glm::vec3 m_DeformedNodeScale = glm::vec3(1.0f);

  glm::mat4 GetDeformedBindMatrix() {

    return glm::translate(glm::mat4(1.0f), m_DeformedNodeTranslation) *
           glm::mat4(m_DeformedNodeRotation) *
           glm::scale(glm::mat4(1.0f), m_DeformedNodeScale) *
           m_UndeformedNodeMatrix;
  }

  int m_ParentJoint;
  std::vector<int> m_Children;
};

struct Skeleton {

  void Update(float deltaTime);
  void UpdateJoint(int jointIndex);

  Animator animator;

  BufferRange GPUlocation;
  bool m_IsAnimated = true;
  std::vector<Joint> m_Joints;
  std::string m_Name;
  std::map<int, int> m_GlobalGltfNodeToRootIndex;
  std::vector<glm::mat4> finalMatrices;
};

class AnimatedModel {

public:
  void AddMesh(MeshID ID) { meshes.push_back(ID); }
  const std::vector<MeshID> &GetMeshIDs() const { return meshes; }
  const unsigned int GetMaterialID() const { return materialID; }
  void SetPositionMat4(glm::mat4 Postion) { position = Postion; }
  const std::vector<InstanceData> &GetInstances() const { return instanceData; }
  const std::vector<BufferRange> &GetInstanceRanges() const {
    return instanceRanges;
  }
  void SetInstances(const std::vector<InstanceData> &instances,
                    const std::vector<BufferRange> &InstanceRanges) {
    instanceCount = instances.size();
    instanceRanges = InstanceRanges;
    instanceData = instances;
  }

  /* void SetShaderID(ShaderComboID id)
   {
       shader = id;
   }
   ShaderComboID GetShaderID() const
   {
       return shader;
   } */

  void SetSkeletonID(int id) { skeletonID = id; }

  void SetAnimatorID(int id) { animatorID = id; }

  int GetAnimatorID() const { return animatorID; }

  int GetSkeletonID() const { return skeletonID; }

  glm::mat4 GetPositionMat4() { return position; }

private:
  std::vector<MeshID> meshes;
  // std::map<std::string, std::unique_ptr<Joint>> joints;

  // array type shit for the bones

  int skeletonID;

  int animatorID;

  ShaderComboID shader;

  glm::mat4 position;
  unsigned int materialID = 0;

  std::vector<InstanceData> instanceData;
  std::vector<BufferRange> instanceRanges;

  unsigned int instanceCount = 1;
};

class AnimatedModelManager {
public:
  void Initialize(BufferManager *bufferManager) {
    this->bufferManager = bufferManager;
  }

  void AddMeshLocation(const MeshID &mesh, VertexIndexInfoPair &location) {

    meshLocations.try_emplace(mesh, location);
  }

  const VertexIndexInfoPair &GetMeshLocation(const MeshID &mesh) {

    return meshLocations[mesh];
  }

  void SaveMeshLocation(const MeshID &mesh, const VertexIndexInfoPair &range) {
    meshLocations.try_emplace(mesh, range);
  }

  const Mesh &GetMesh(MeshID id) { return meshes[id]; }
  void SetModelShader(AnimatedModel &model, ShaderComboID &shader) {

    for (auto &mesh : model.GetMeshIDs()) {
      auto it = meshes.find(mesh);
      if (it != meshes.end()) {
        it->second.SetShader(shader);
      } else {
        SDL_Log("ERROR, COULD NOT ASSIGN SHADER\n");
        // Optionally log a warning: mesh ID not found
      }
    }
  }

  AnimatedModel LoadAnimatedModel(std::string path);

  void LoadAnimation(int skeletonID, std::string &path, int &r_AnimationID);

  void LoadAnimationForSkeleton(int skeletonID, Animation &animation) {
    Skeleton &skeleton = skeletons[skeletonID];
    skeleton.animator.AddAnimation(animation);
    // skeleton.animator.PlayAnimation(0);
  }

  void Update(float deltaTime);

  void SetMeshResidency(MeshID mesh, bool status) {
    meshes[mesh].SetResidencyStatus(status);
  }

  Skeleton &GetSkeleton(int ID) { return skeletons[ID]; }

  void AddSubmittedModel(AnimatedModel *model) {
    // TODO: make all model calls be shared pointers , and in static models
    submittedAnimatedModels.push_back(model);
  }

  Animation GetAnimation(unsigned int animationID) {

    return animations[animationID];
  }

  void Destroy();

private:
  std::vector<MeshID> ProcessMeshes(tinygltf::Model &model, Skeleton &skeleton);

  void UploadBonesToGPU(BufferRange &range,
                        std::vector<glm::mat4> finalMatrices);

  unsigned int maxID = 0;

  BufferManager *bufferManager;

  std::unordered_map<MeshID, Mesh> meshes;
  std::unordered_map<eHazGraphics_Utils::HashedString, AnimatedModel>
      loadedModels;
  std::unordered_map<MeshID, VertexIndexInfoPair> meshLocations;
  std::vector<AnimatedModel *> submittedAnimatedModels;
  std::vector<Skeleton> skeletons; // in da closet
  std::vector<Animation> animations;
};

} // namespace eHazGraphics

#endif
