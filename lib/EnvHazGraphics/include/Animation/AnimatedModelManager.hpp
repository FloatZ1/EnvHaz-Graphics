
#ifndef ENVHAZ_ANIMATED_MODEL_MANAGER_HPP
#define ENVHAZ_ANIMATED_MODEL_MANAGER_HPP

#include "Animation/AnimatedModel.hpp"
#include "Animation/Animation.hpp"
#include "Animation/Animator.hpp"
#include "BufferManager.hpp"
#include "DataStructs.hpp"
// #include "MeshManager.hpp"
#include "ModelPackage.hpp"
#include "Utils/HashedStrings.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/quaternion_float.hpp"
#include "glm/fwd.hpp"
#include "tinygltf/tiny_gltf.h"
#include <Model.hpp>
#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/scene.h>

#include <glm/gtc/quaternion.hpp>
#include <map>
#include <memory>
#include <mutex>
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

class AnimatedModelManager {
public:
  void ClearEverything() {

    for (auto &[id, mesh] : meshes) {
      mesh.SetResidencyStatus(false);
      VertexIndexInfoPair &meshLoc = meshLocations[id];

      bufferManager->InvalidateStaticRange(meshLoc);
    }
    for (auto &[id, model] : loadedModels) {
      model->ClearInstances();
    }
    // call the function from buffer manager to clear the ranges

    meshes.clear();
    loadedModels.clear();
    animators.clear();
    animations.clear();
    meshLocations.clear();
    submittedAnimatedModels.clear();
  }

  AABB GetModelAABB(ModelID model) { return loadedModels[model]->GetAABB(); }

  void EraseMesh(MeshID mesh) {
    meshes[mesh].SetResidencyStatus(false);

    VertexIndexInfoPair &meshLoc = meshLocations[mesh];

    bufferManager->InvalidateStaticRange(meshLoc);

    meshes.erase(mesh);

    meshLocations.erase(mesh);
  }

  void RemoveModel(ModelID modelID) {

    std::shared_ptr<AnimatedModel> &model = loadedModels[modelID];

    for (auto &meshID : model->GetMeshIDs()) {
      EraseMesh(meshID);
    }

    AnimatorID animatorID = model->GetAnimatorID();
    animators.erase(animatorID);
    skeletons.erase(modelID);
    loadedModels.erase(modelID);
  }

  void RemoveAnimation(AnimationID animationID) {

    animations.erase(animationID);
  }

  void Initialize(BufferManager *bufferManager);

  void AddMeshLocation(const MeshID &mesh, VertexIndexInfoPair &location);

  const VertexIndexInfoPair &GetMeshLocation(const MeshID &mesh);

  void SaveMeshLocation(const MeshID &mesh, const VertexIndexInfoPair &range);

  const Mesh &GetMesh(MeshID id) { return meshes[id]; }

  void SetModelShader(ModelID modelID, ShaderComboID &shader);

  ModelID LoadAnimatedModel(std::string path);

  void LoadAnimation(std::shared_ptr<Skeleton> skeleton, std::string &path,
                     AnimationID &r_AnimationID);

  void Update(float deltaTime);

  void SetMeshResidency(MeshID mesh, bool status);

  std::shared_ptr<Skeleton> &GetSkeleton(ModelID ID) { return skeletons[ID]; }

  std::shared_ptr<Animator> &GetAnimator(AnimatorID ID) {
    return animators[ID];
  }

  std::shared_ptr<AnimatedModel> GetModel(ModelID ID) {
    if (loadedModels.contains(ID))
      return loadedModels[ID];
    else
      return nullptr;
  }

  void AddSubmittedModel(std::shared_ptr<AnimatedModel> model) {

    submittedAnimatedModels.push_back(model);
  }
  void ClearSubmittedModelInstances();

  std::shared_ptr<Animation> GetAnimation(AnimationID animationID) {

    return animations[animationID];
  }

  std::string GetModelPath(ModelID p_ModelID) {

    if (m_umModelPaths.contains(p_ModelID)) {
      return m_umModelPaths[p_ModelID];
    }

    else
      return "model has no path";
  }

  // NOTE: Binary file loading and encoding

  void ExportAHazModel(std::string exportPath, ModelID modelID);

  ModelID LoadAHazModel(std::string path);

  std::vector<ModelID> LoadAHazModelList(std::vector<std::string> paths);
  std::vector<ModelID>
  LoadAHazModelList(std::vector<AnimatedModelPackage> &packages);

  void Destroy();
  bool isLoadedModel(std::string p_strPath) {

    eHazGraphics_Utils::HashedString l_hsHash =
        eHazGraphics_Utils::computeHash(p_strPath);

    if (loadedModels.contains(l_hsHash))
      return true;

    return false;
  }

private:
  static AnimatedModelPackage LoadSingleModel(const std::string &path);

  std::vector<ModelID>
  LoadAHazModelListLimited(const std::vector<std::string> &paths,
                           size_t maxThreads = 4);
  void ValidateLoadedFile(ModelID model);

  void ValidateLoadedFiles();

  std::vector<MeshID> processNode(aiNode *node);

  Mesh processMesh(aiMesh *mesh);

  void UploadBonesToGPU(SBufferRange &range,
                        const std::vector<glm::mat4> &finalMatrices);

  void BuildBaseSkeleton();

  void SetParentHierarchy(aiNode *node);

  void ComputeGlobalBindTransforms(aiNode *node, const glm::mat4 &parentGlobal);

  void ExtractBoneWeightForVertices(std::vector<Vertex> &vertices,
                                    aiMesh *mesh);

  AABB GetModelAABB(const aiScene *scene);

  /*




*/
  // unsigned int maxID = 0;

  std::mutex mapMutex;

  BufferManager *bufferManager;

  std::unordered_map<MeshID, Mesh> meshes;
  std::unordered_map<eHazGraphics_Utils::HashedString,
                     std::shared_ptr<AnimatedModel>>
      loadedModels;

  std::unordered_map<MeshID, VertexIndexInfoPair> meshLocations;
  std::unordered_map<ModelID, std::string> m_umModelPaths;
  std::vector<std::shared_ptr<AnimatedModel>> submittedAnimatedModels;
  std::unordered_map<ModelID, std::shared_ptr<Skeleton>>
      skeletons; // in da closet
  std::unordered_map<AnimationID, std::shared_ptr<Animation>> animations;
  std::unordered_map<AnimatorID, std::shared_ptr<Animator>> animators;

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

  // std::unordered_map<std::string, int> m_BoneMap;
};

} // namespace eHazGraphics

#endif
