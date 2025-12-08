#ifndef ENVHAZ_ANIMATOR_HPP
#define ENVHAZ_ANIMATOR_HPP

#include "Animation.hpp"
#include "DataStructs.hpp"
#include "Utils/Boost_GLM_Serialization.hpp"
#include <algorithm>
#include <boost/serialization/unordered_map.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>
#include <map>
#include <memory>
#include <string>
#include <vector>
namespace eHazGraphics {

// --- SKELETON DATA STRUCTURES ---

struct Joint {
  std::string m_Name;
  int m_ParentJoint; // Index of parent in the flat Joint array

  // Static Data (Loaded Once from Assimp)
  glm::mat4 mOffsetMatrix = glm::mat4(1.0f);

  // Dynamic Data (Calculated Every Frame)
  glm::mat4 m_GlobalTransform = glm::mat4(1.0f); // M_GlobalBone
  // glm::mat4 m_FinalShaderMatrix = glm::mat4(
  //   1.0f); // M_Final (M_Root * M_GlobalBone * M_Offset * M_Root_Inv)
  glm::mat4 localBindTransform = glm::mat4(1.0f);

private:
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar & m_Name;
    ar & m_ParentJoint;
    ar & mOffsetMatrix;
    ar & localBindTransform;
  }
};

struct Skeleton {
  std::vector<glm::mat4> finalMatrices;
  std::vector<Joint> m_Joints;
  std::vector<int> m_RootJointIndecies;

  glm::mat4 m_InverseRoot = glm::mat4(1.0f);
  glm::mat4 m_RootTransform = glm::mat4(1.0f);
  std::unordered_map<std::string, int> m_BoneMap;

private:
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar & m_Joints;
    ar & m_RootJointIndecies;
    ar & m_InverseRoot;
    ar & m_RootTransform;
    ar & m_BoneMap;
  }
};

// --- ANIMATION SOURCE STRUCTURES ---

struct BlendTriangle {
  int indices[3]; // Indices into the BlendSpace2D::points vector
};

struct BlendPoint {
  // Defines the clip and its position in the 2D plane
  // We reference Animation directly as these are the leaf nodes being blended.
  std::shared_ptr<Animation> clip;
  float x; // Normalized X coordinate (e.g., -1.0 to 1.0)
  float y; // Normalized Y coordinate (e.g., -1.0 to 1.0)
};

class BlendSpace2D : public IAnimationSource {
public:
  std::vector<BlendPoint> points;
  std::vector<BlendTriangle> topology;
  float HorizontalAxis = 0.0f; // Input updated via Animator::SetBlendInput
  float VerticalAxis = 0.0f;   // Input updated via Animator::SetBlendInput

  void RecalculateTopology();

  KeyFrame GetPoseAt(float time) override;

  // Calculates the weights for the active clips based on input
  std::map<std::shared_ptr<Animation>, float> CalculateWeights(float xIn,
                                                               float yIn);
};

// --- LAYER STRUCTURES ---

struct AnimationLayer {

  std::shared_ptr<Animation> activeSource = nullptr;

  float currentTime = 0.0f;
  float weight = 1.0f; // Global weight for this layer
};

// --- ANIMATOR CLASS (THE ORCHESTRATOR) ---

class Animator {
public:
  // === 1. Asset/Source Management ===

  int AddAnimation(std::shared_ptr<Animation> animation) {
    animations.push_back(animation);
    return animations.size() - 1;
  }

  int RegisterBlendSpace2D(std::vector<BlendPoint> blendPoints);

  void ReplaceBlendSpace(int ID, std::vector<BlendPoint> points);

  // === 2. Layer Management ===

  int CreateAnimationLayer();

  // Assigns an Animation (or BlendSpace) asset to a layer
  void SetLayerSource(int layerIndex, std::shared_ptr<Animation> source);

  // === 3. Runtime Control ===

  void SetBlendInput(float x, float y);

  void Update(float deltaTime);

  // === 4. Accessors/Mutators ===

  void SetGPULocation(BufferRange &range) { GPUlocation = range; }

  BufferRange &GetGPULocation() { return GPUlocation; }

  void SetSkeleton(std::shared_ptr<Skeleton> Skeleton) {
    skeleton = Skeleton;
    // Ensure currentPose has space for all joint transforms
    // Assuming KeyFrame::transforms is a vector of JointTransform
    if (skeleton) {
      currentPose.transforms.resize(skeleton->m_Joints.size());
    }
  }
  std::shared_ptr<Skeleton> GetSkeleton() { return skeleton; }
  std::vector<glm::mat4> GetFinalMatrices();

private:
  // --- Data Storage ---
  std::shared_ptr<Skeleton> skeleton;
  std::vector<AnimationLayer> layers;
  // Changed to Animation for consistency with AddAnimation and BlendPoint
  std::vector<std::shared_ptr<Animation>> animations;

  // Storage for all reusable BlendSpace2D assets
  std::vector<std::shared_ptr<BlendSpace2D>> blendSpaces;

  // --- State ---
  KeyFrame currentPose;

  BufferRange GPUlocation; // where the joints are on the buffer
};

} // namespace eHazGraphics

#endif // ENVHAZ_ANIMATOR_HPP
