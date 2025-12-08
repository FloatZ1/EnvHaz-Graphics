#ifndef EHAZ_MESH_HPP
#define EHAZ_MESH_HPP

#include "DataStructs.hpp"
#include <vector>

namespace eHazGraphics {

class Mesh {
private:
  // add variables for used textures and transforms.
  MeshData data{};
  ShaderComboID shaderID{};
  MeshID ID;
  GLuint instances = 1;
  glm::mat4 relativeMatrix = glm::mat4(1.0f);
  bool GPUresident = false;

public:
  Mesh() = default;

  Mesh(MeshData data) : data(data) {};

  Mesh(MeshData data, ShaderComboID shaderID)
      : data(data), shaderID(shaderID) {};

  const glm::mat4 &GetRelativeMatrix() const { return relativeMatrix; }

  void SetID(MeshID id) { ID = id; }

  const MeshID GetID() const { return ID; }

  ShaderComboID GetShaderID() const { return shaderID; }

  void SetInstanceCount(GLuint numInstances) { instances = numInstances; }

  void SetResidencyStatus(bool value) { GPUresident = value; }

  const MeshData &GetMeshData() const { return data; }

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

  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar & ID;
    ar & data;
    ar & shaderID;
    ar & relativeMatrix;
  }
};

} // namespace eHazGraphics

#endif
