#ifndef MESH_MANAGER_HPP
#define MESH_MANAGER_HPP





#include "BitFlags.hpp"
#include "DataStructs.hpp"
#include "ShaderManager.hpp"
#include <cstddef>
#include <iterator>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
namespace eHazGraphics
{

class Mesh
{
  private:
    // add variables for used textures and transforms.
    MeshData data;
    ShaderComboID shaderID;

    bool GPUresident = false;

  public:
    Mesh(MeshData data) : data(data) {};

    Mesh(MeshData data, ShaderComboID shaderID) : data(data), shaderID(shaderID) {};

    ShaderComboID GetShaderID() const
    {

        return shaderID;
    }

    std::pair<const Vertex *, const size_t> GetVertexData() const
    {
        return std::pair<const Vertex *, const size_t>{data.vertices.data(), data.vertices.size()};
    }
    std::pair<const int *, const size_t> GetIndexData() const
    {
        return std::pair<const int *, const size_t>{data.indecies.data(), data.indecies.size()};
    }

    bool isResident() const
    {
        return GPUresident;
    }
};








class MeshManager
{
  public:
    void LoadMesh();

    bool SubmitMesh(MeshID ID);



  private:
    std::unordered_map<MeshID, Mesh> meshes;
    std::unordered_map<std::string, MeshID> meshPaths;
};















} // namespace eHazGraphics




#endif
