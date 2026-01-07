#ifndef ANIMATED_MODEL_HPP
#define ANIMATED_MODEL_HPP
#include "Animator.hpp"
#include "Mesh.hpp"
#include <boost/serialization/access.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/utility.hpp> // for std::pair
#include <boost/serialization/vector.hpp>
namespace eHazGraphics {

class AnimatedModel {

public:
  void SetID(MeshID id) { ID = id; }

  MeshID GetID() { return ID; }
  void AddMesh(MeshID ID) { meshes.push_back(ID); }

  const std::vector<MeshID> &GetMeshIDs() const { return meshes; }

  const unsigned int GetMaterialID() const { return materialID; }
  // void SetPositionMat4(glm::mat4 Position) { position = Position; }
  const std::vector<InstanceData> &GetInstances() const { return instanceData; }
  const std::vector<SBufferRange> &GetInstanceRanges() const {
    return instanceRanges;
  }
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

  void SetShaderID(ShaderComboID id) { shader = id; }
  ShaderComboID GetShaderID() const { return shader; }
  void SetSkeleton(std::shared_ptr<Skeleton> Skeleton) { skeleton = Skeleton; }

  void SetAnimatorID(AnimatorID id) { animatorID = id; }

  int GetAnimatorID() const { return animatorID; }

  std::shared_ptr<Skeleton> GetSkeleton() const { return skeleton; }

private:
  friend class boost::serialization::access;

  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar & animatorID;
    ar & materialID;
    ar & meshes;
    ar & skeleton;
    ar & shader;
    ar & ID;

    // skip instanceData, instanceRanges, instanceCount → runtime only
  }

  ModelID ID;

  std::vector<MeshID> meshes;
  // std::map<std::string, std::unique_ptr<Joint>> joints;

  // array type shit for the bones

  std::shared_ptr<Skeleton> skeleton;

  AnimatorID animatorID;

  ShaderComboID shader;

  unsigned int materialID = 0;

  std::vector<InstanceData> instanceData;
  std::vector<SBufferRange> instanceRanges;

  unsigned int instanceCount = 1;
};

} // namespace eHazGraphics

#endif
