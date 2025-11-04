
#ifndef ENVHAZ_ANIMATED_MODEL_MANAGER_HPP
#define ENVHAZ_ANIMATED_MODEL_MANAGER_HPP

#include "Animation/Animation.hpp"
#include "Animation/Animator.hpp"
#include "BufferManager.hpp"
#include "DataStructs.hpp"
#include "MeshManager.hpp"
#include "Utils/HashedStrings.hpp"

#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/quaternion_float.hpp"
#include "glm/fwd.hpp"
#include "tinygltf/tiny_gltf.h"
#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <glm/gtc/quaternion.hpp>
#include <map>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

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
 *
 *
 *
 */

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
 */
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
  void SetSkeleton(std::shared_ptr<Skeleton> Skeleton) { skeleton = Skeleton; }

  void SetAnimatorID(int id) { animatorID = id; }

  int GetAnimatorID() const { return animatorID; }

  std::shared_ptr<Skeleton> GetSkeleton() const { return skeleton; }

  glm::mat4 GetPositionMat4() { return position; }

private:
  std::vector<MeshID> meshes;
  // std::map<std::string, std::unique_ptr<Joint>> joints;

  // array type shit for the bones

  std::shared_ptr<Skeleton> skeleton;

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

  void LoadAnimation(std::shared_ptr<Skeleton> skeleton, std::string &path,
                     int &r_AnimationID);

  void Update(float deltaTime);

  void SetMeshResidency(MeshID mesh, bool status) {
    meshes[mesh].SetResidencyStatus(status);
  }

  std::shared_ptr<Skeleton> &GetSkeleton(int ID) { return skeletons[ID]; }

  std::shared_ptr<Animator> &GetAnimator(int ID) { return animators[ID]; }

  void AddSubmittedModel(AnimatedModel *model) {
    // TODO: make all model calls be shared pointers , and in static models
    submittedAnimatedModels.push_back(model);
  }

  std::shared_ptr<Animation> GetAnimation(unsigned int animationID) {

    return animations[animationID];
  }

  void Destroy();

private:
  std::vector<MeshID> processNode(aiNode *node);

  Mesh processMesh(aiMesh *mesh);

  void UploadBonesToGPU(BufferRange &range,
                        std::vector<glm::mat4> finalMatrices);

  void BuildBaseSkeleton();

  void SetParentHierarchy(aiNode *node);

  /*




  */
  unsigned int maxID = 0;

  BufferManager *bufferManager;

  std::unordered_map<MeshID, Mesh> meshes;
  std::unordered_map<eHazGraphics_Utils::HashedString, AnimatedModel>
      loadedModels;

  std::unordered_map<MeshID, VertexIndexInfoPair> meshLocations;
  std::vector<AnimatedModel *> submittedAnimatedModels;
  std::vector<std::shared_ptr<Skeleton>> skeletons; // in da closet
  std::vector<std::shared_ptr<Animation>> animations;
  std::vector<std::shared_ptr<Animator>> animators;

  // processing stuff:

  struct VertexBoneData {
    int boneIDs[4];
    float boneWeights[4];
  };

  VertexBoneData GetVertexBoneData(int vertexID, aiMesh *mesh);
  std::map<unsigned int, std::vector<std::pair<int, float>>>
      m_CurrentMeshBoneData;

  void PopulateMeshBoneData(aiMesh *mesh);

  const aiScene *scene;
  Skeleton processingSkeleton;

  std::unordered_map<std::string, int> m_BoneMap;

  Assimp::Importer importer;
};

} // namespace eHazGraphics

#endif
