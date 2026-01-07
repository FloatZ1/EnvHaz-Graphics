#ifndef ENVHAZGRAPHICS_MODEL_HPP
#define ENVHAZGRAPHICS_MODEL_HPP

#include "Mesh.hpp"

namespace eHazGraphics {

class Model {
public:
  void AddMesh(MeshID ID) { meshes.push_back(ID); }

  const std::vector<MeshID> &GetMeshIDs() const { return meshes; }

  const unsigned int GetMaterialID() const { return materialID; }

  void SetID(MeshID id) { ID = id; }

  MeshID GetID() { return ID; }

  void SetInstances(const std::vector<InstanceData> &instances,
                    const std::vector<SBufferRange> &InstanceRanges) {
    instanceCount = instances.size();
    instanceRanges = InstanceRanges;
    instanceData = instances;
  }

  void AddInstances(std::vector<InstanceData> &instances,
                    std::vector<SBufferRange> &InstanceRanges) {

    for (int i = 0; i < instances.size(); i++) {

      instanceRanges.push_back(std::move(InstanceRanges[i]));
      instanceData.push_back(std::move(instances[i]));
    }

    instanceCount = instanceData.size();
  }

  void ClearInstances() {
    instanceCount = 0;
    instanceRanges.clear();
    instanceData.clear();
  }

  const std::vector<InstanceData> &GetInstances() const { return instanceData; }
  const std::vector<SBufferRange> &GetInstanceRanges() const {
    return instanceRanges;
  }

  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar & ID;
    ar & materialID;
    ar & meshes;
  }

private:
  ModelID ID;

  std::vector<MeshID> meshes;
  std::vector<InstanceData> instanceData;
  std::vector<SBufferRange> instanceRanges;

  unsigned int materialID = 0;
  unsigned int instanceCount = 1;
};

} // namespace eHazGraphics

#endif
