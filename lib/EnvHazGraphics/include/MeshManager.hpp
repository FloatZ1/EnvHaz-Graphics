#ifndef MESH_MANAGER_HPP
#define MESH_MANAGER_HPP

#include "BitFlags.hpp"
#include "BufferManager.hpp"
#include "DataStructs.hpp"
#include "Model.hpp"

#include "ModelPackage.hpp"
#include "ShaderManager.hpp"
#include "Utils/HashedStrings.hpp"
#include "glad/glad.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cstddef>
#include <cstdio>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace eHazGraphics {

class MeshManager {
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

    // for (auto&[ID, l_meshTransform] : meshTransformRanges) {
    //     bufferManager->RemoveRange(l_meshTransform);
    // }

    bufferManager->ClearBuffer(TypeFlags::BUFFER_STATIC_MATRIX_DATA);

    meshTransforms.clear();
    meshTransformRanges.clear();
    meshLocations.clear();
    meshes.clear();

    submittedModels.clear();
    loadedModels.clear();
  }

  void Initialize(BufferManager *bufferManager); // TODO: IMPLEMENT
  ModelID LoadModel(std::string path);

  std::shared_ptr<Model> GetModel(ModelID p_ModelID) {
    if (loadedModels.contains(p_ModelID)) {
      return loadedModels[p_ModelID];
    }

    return nullptr;
  }

  AABB GetModelAABB(ModelID model) { return loadedModels[model]->GetAABB(); }
  void EraseModel(ModelID modelID) {
    std::shared_ptr<Model> &model = loadedModels[modelID];

    for (auto &meshID : model->GetMeshIDs()) {
      EraseMesh(meshID);
    }

    loadedModels.erase(modelID);
  }

  void EraseMesh(MeshID mesh);

  const glm::mat4 &GetMeshTransform(MeshID mesh) {

    return meshTransforms[mesh];
  }

  void SetModelShader(ModelID modelID, const ShaderComboID &shader);

  void SetMeshResidency(MeshID mesh, bool value) {
    meshes[mesh].SetResidencyStatus(value);
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

  bool ContainsTransformRange(const MeshID &mesh) {

    return meshTransformRanges.contains(mesh);
  }

  void AddTransformRange(const MeshID &mesh, const SBufferRange &range) {

    meshTransformRanges.try_emplace(mesh, range);
  }

  const SBufferRange GetTransformBufferRange(const MeshID &mesh) {

    return meshTransformRanges[mesh];
  }

  void ClearSubmittedModelInstances() {
    for (auto &model : submittedModels) {
      model->ClearInstances();
    }
    meshTransformRanges.clear();
    submittedModels.clear();
  }

  void AddSubmittedModel(std::shared_ptr<Model> model) {
    submittedModels.push_back(model);
  }

  void UpdateSubmittedMeshes() {
    for (unsigned int i = 0; i < submittedModels.size(); i++) {

      auto Instances = submittedModels[i]->GetInstances();
      auto InstanceRanges = submittedModels[i]->GetInstanceRanges();
      assert(Instances.size() == InstanceRanges.size());

      for (unsigned int i = 0; i < Instances.size(); i++) {

        bufferManager->UpdateData(InstanceRanges[i], &Instances[i],
                                  sizeof(InstanceData));
      }
    }
  }

  void Update() { /*UpdateSubmittedMeshes();*/ }

  const Mesh &GetMesh(MeshID id) { return meshes[id]; }
  std::string GetModelPath(ModelID p_ModelID) {

    if (modelPaths.contains(p_ModelID)) {
      return modelPaths[p_ModelID];
    }

    else
      return "model has no path";
  }
  // ModelPackage import and export

  void ExportHazModel(std::string exportPath, ModelID modelID);

  ModelID LoadHazModel(std::string path);

  std::vector<ModelID> LoadHazModelList(std::vector<std::string> paths);
  std::vector<ModelID>
  LoadHazModelList(std::vector<StaticModelPackage> &packages);

  void Destroy(); // TODO: IMPLEMENT

  bool isLoadedModel(std::string p_strPath) {

    eHazGraphics_Utils::HashedString l_hsHash =
        eHazGraphics_Utils::computeHash(p_strPath);

    if (loadedModels.contains(l_hsHash))
      return true;

    return false;
  }

private:
  AABB GetModelAABB(const aiScene *scene);

  static StaticModelPackage LoadSingleModel(const std::string &path);

  std::vector<ModelID>
  LoadHazModelListLimited(const std::vector<std::string> &paths,
                          size_t maxThreads = 4);
  void ValidateLoadedFile(ModelID model);

  void ValidateLoadedFiles();

  std::vector<MeshID> processNode(aiNode *node, const aiScene *scene);
  Mesh processMesh(aiMesh *mesh, const aiScene *scene);
  /* vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                        string typeName); */

  // unsigned int maxID = 0;

  std::unordered_map<eHazGraphics_Utils::HashedString, std::shared_ptr<Model>>
      loadedModels;
  std::vector<std::shared_ptr<Model>> submittedModels;
  std::unordered_map<MeshID, Mesh> meshes;
  std::unordered_map<ModelID, std::string> modelPaths;
  std::unordered_map<MeshID, glm::mat4> meshTransforms;
  std::unordered_map<MeshID, SBufferRange> meshTransformRanges;
  std::unordered_map<MeshID, VertexIndexInfoPair> meshLocations;

  std::mutex mapMutex;

  BufferManager *bufferManager;
  Assimp::Importer importer;
};

} // namespace eHazGraphics

#endif
