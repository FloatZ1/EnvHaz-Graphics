#ifndef MESH_MANAGER_HPP
#define MESH_MANAGER_HPP

#include "BitFlags.hpp"
#include "BufferManager.hpp"
#include "DataStructs.hpp"

#include "ShaderManager.hpp"
#include "Utils/HashedStrings.hpp"
#include "glad/glad.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace eHazGraphics {

class Mesh {
private:
  // add variables for used textures and transforms.
  MeshData data{};
  ShaderComboID shaderID{};
  GLuint instances = 1;
  glm::mat4 relativeMatrix = glm::mat4(1.0f);
  bool GPUresident = false;

public:
  Mesh() = default;

  Mesh(MeshData data) : data(data) {};

  Mesh(MeshData data, ShaderComboID shaderID)
      : data(data), shaderID(shaderID) {};

  ShaderComboID GetShaderID() const { return shaderID; }

  void SetInstanceCount(GLuint numInstances) { instances = numInstances; }

  void SetResidencyStatus(bool value) { GPUresident = value; }

  MeshData GetMeshData() { return data; }

  const GLuint &GetInstanceCount() const { return instances; }

  void SetShader(ShaderComboID &shader) { shaderID = shader; }

  void setRelativeMatrix(const glm::mat4 &mat) { relativeMatrix = mat; }

  std::pair<const Vertex *, const size_t> GetVertexData() const {
    return std::pair<const Vertex *, const size_t>{
        data.vertices.data(), data.vertices.size() * sizeof(Vertex)};
  }
  std::pair<const GLuint *, const size_t> GetIndexData() const {
    return std::pair<const GLuint *, const size_t>{
        data.indecies.data(), data.indecies.size() * sizeof(GLuint)};
  }

  bool isResident() const { return GPUresident; }
};

class Model {
public:
  void AddMesh(MeshID ID) { meshes.push_back(ID); }

  const std::vector<MeshID> &GetMeshIDs() const { return meshes; }

  const unsigned int GetMaterialID() const { return materialID; }
  /*const unsigned int GetInstanceCount() const
  {
      return instanceCount;
  }
  */
  void SetInstances(const std::vector<InstanceData> &instances,
                    const std::vector<BufferRange> &InstanceRanges) {
    instanceCount = instances.size();
    instanceRanges = InstanceRanges;
    instanceData = instances;
  }

  void AddInstances(std::vector<InstanceData> &instances,
                    std::vector<BufferRange> &InstanceRanges) {

    for (int i = 0; i < instances.size(); i++) {

      instanceRanges.push_back((InstanceRanges[i]));
      instanceData.push_back((instances[i]));
    }

    instanceCount = instanceData.size();
  }

  void ClearInstances() {
    instanceCount = 0;
    instanceRanges.clear();
    instanceData.clear();
  }

  const glm::mat4 GetPositionMat4() const { return position; }

  void SetPositionMat4(glm::mat4 Postion) { position = Postion; }
  const std::vector<InstanceData> &GetInstances() const { return instanceData; }
  const std::vector<BufferRange> &GetInstanceRanges() const {
    return instanceRanges;
  }

  // Animation stuff

private:
  std::vector<MeshID> meshes;
  std::vector<InstanceData> instanceData;
  std::vector<BufferRange> instanceRanges;
  glm::mat4 position;
  unsigned int materialID = 0;
  unsigned int instanceCount = 1;
};

class MeshManager {
public:
  void Initialize(BufferManager *bufferManager); // TODO: IMPLEMENT
  std::shared_ptr<Model> LoadModel(std::string &path);

  void EraseMesh(MeshID mesh) {

    // TODO: erase from mesh maps as well
    meshes.erase(mesh);
  }

  const glm::mat4 &GetMeshTransform(MeshID mesh) {

    return meshTransforms[mesh];
  }

  void SetModelShader(std::shared_ptr<Model> model, ShaderComboID &shader) {

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

  void AddTransformRange(const MeshID &mesh, const BufferRange &range) {

    meshTransformRanges.try_emplace(mesh, range);
  }

  const BufferRange GetTransformBufferRange(const MeshID &mesh) {

    return meshTransformRanges[mesh];
  }

  void ClearSubmittedModelInstances() {
    for (auto &model : submittedModels) {
      model->ClearInstances();
    }
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

  void Update() { UpdateSubmittedMeshes(); }

  const Mesh &GetMesh(MeshID id) { return meshes[id]; }

  void Destroy(); // TODO: IMPLEMENT

private:
  std::vector<MeshID> processNode(aiNode *node, const aiScene *scene);
  Mesh processMesh(aiMesh *mesh, const aiScene *scene);
  /* vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                        string typeName); */

  unsigned int maxID = 0;

  std::unordered_map<eHazGraphics_Utils::HashedString, std::shared_ptr<Model>>
      loadedModels;
  std::vector<std::shared_ptr<Model>> submittedModels;
  std::unordered_map<MeshID, Mesh> meshes;
  std::unordered_map<std::string, MeshID> meshPaths;
  std::unordered_map<MeshID, glm::mat4> meshTransforms;
  std::unordered_map<MeshID, BufferRange> meshTransformRanges;
  std::unordered_map<MeshID, VertexIndexInfoPair> meshLocations;
  BufferManager *bufferManager;
  Assimp::Importer importer;
};

} // namespace eHazGraphics

#endif
