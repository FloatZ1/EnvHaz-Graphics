
#include "Animation/Animation.hpp"
#include "Utils/Alghorithms.hpp"
#include "glm/matrix.hpp"
#include <Animation/Animator.hpp>
#include <memory>
#include <vector>

using namespace eHazGraphics_Utils;
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
 *
 *
 *
 *
 */

int Animator::RegisterBlendSpace2D(std::vector<BlendPoint> blendPoints) {

  std::shared_ptr<BlendSpace2D> bs = std::make_shared<BlendSpace2D>();

  bs->points = blendPoints;
  bs->RecalculateTopology();
  blendSpaces.push_back(bs);
  return blendSpaces.size() - 1;
}
void Animator::ReplaceBlendSpace(int ID, std::vector<BlendPoint> points) {

  std::shared_ptr<BlendSpace2D> bs = std::make_shared<BlendSpace2D>();

  bs->points = points;
  bs->RecalculateTopology();
  blendSpaces[ID] = bs;
}
int Animator::CreateAnimationLayer() {

  layers.push_back(AnimationLayer());

  return layers.size() - 1;
}
void Animator::SetLayerSource(int layerIndex,
                              std::shared_ptr<Animation> source) {

  layers[layerIndex].activeSource = source;
}
void Animator::SetBlendInput(float x, float y) {

  for (auto &bs : blendSpaces) {
    bs->HorizontalAxis = x;
    bs->VerticalAxis = y;
  }
}

void CalculateJointTransforms(const KeyFrame &pose,
                              std::shared_ptr<Skeleton> skeleton,
                              int jointIndex,
                              const glm::mat4 &parentTransform) {
  // 1. Get the local transform from the KeyFrame (blended pose)
  if (jointIndex >= pose.transforms.size())
    return;
  const JointTransform &localT = pose.transforms[jointIndex];

  // Convert local scale, rotation, and position into a local 4x4 matrix
  glm::mat4 localMatrix = glm::translate(glm::mat4(1.0f), localT.position);
  localMatrix *= glm::mat4_cast(localT.rotation);
  localMatrix = glm::scale(
      localMatrix, localT.scale); // Assuming scale is part of JointTransform

  // 2. Calculate the Global Transform
  glm::mat4 globalTransform = parentTransform * localMatrix;
  skeleton->m_Joints[jointIndex].m_GlobalTransform = globalTransform;

  // 3. Calculate the Final Shader Matrix
  // Final Matrix = M_GlobalBone * M_OffsetMatrix
  glm::mat4 finalMatrix =
      globalTransform * (skeleton->m_Joints[jointIndex].mOffsetMatrix);

  skeleton->finalMatrices[jointIndex] = finalMatrix;

  // 4. Recursively call for all children
  for (size_t i = 0; i < skeleton->m_Joints.size(); ++i) {
    if (skeleton->m_Joints[i].m_ParentJoint == jointIndex) {
      CalculateJointTransforms(pose, skeleton, i, globalTransform);
    }
  }
}

void Animator::Update(float deltaTime) {
  if (!skeleton) {
    std::cerr << "Animator Error: No skeleton set." << std::endl;
    return;
  }
  if (layers.empty() || !layers[0].activeSource) {
    return;
  }

  // Initialize finalMatrices
  if (skeleton->finalMatrices.size() != skeleton->m_Joints.size()) {
    skeleton->finalMatrices.resize(skeleton->m_Joints.size());
  }

  // Get base layer
  AnimationLayer &baseLayer = layers[0];

  // Advance time in ticks
  float tps = baseLayer.activeSource->GetTicksPerSecond();
  float duration = baseLayer.activeSource->GetDurationTicks();

  baseLayer.currentTime += deltaTime * tps;
  if (duration > 0.0f) {
    baseLayer.currentTime = std::fmod(baseLayer.currentTime, duration);
  }

  KeyFrame finalPose = baseLayer.activeSource->GetPoseAt(baseLayer.currentTime);
  const size_t jointCount = finalPose.transforms.size();

  // Blend additional layers
  for (size_t i = 1; i < layers.size(); ++i) {
    AnimationLayer &layer = layers[i];

    if (!layer.activeSource ||
        layer.weight < std::numeric_limits<float>::epsilon()) {
      continue;
    }

    float layerTps = layer.activeSource->GetTicksPerSecond();
    float layerDuration = layer.activeSource->GetDurationTicks();

    layer.currentTime += deltaTime * layerTps;
    if (layerDuration > 0.0f) {
      layer.currentTime = std::fmod(layer.currentTime, layerDuration);
    }

    KeyFrame currentPose = layer.activeSource->GetPoseAt(layer.currentTime);
    const float w = layer.weight;

    for (size_t j = 0; j < jointCount && j < currentPose.transforms.size();
         ++j) {
      const JointTransform &currentT = currentPose.transforms[j];
      JointTransform &finalT = finalPose.transforms[j];

      finalT.position = glm::mix(finalT.position, currentT.position, w);
      finalT.rotation = glm::slerp(finalT.rotation, currentT.rotation, w);
      finalT.scale = glm::mix(finalT.scale, currentT.scale, w);
    }
  }
  // 3. Apply Forward Kinematics (Convert the final blended pose to Shader
  // Matrices)
  /* int rootJointIndex = 0;
   CalculateJointTransforms(finalPose, skeleton, rootJointIndex,
                            skeleton->m_RootTransform); */

  const glm::mat4 &rootModelTransform = skeleton->m_RootTransform;

  // FIND ALL ROOT JOINTS AND START A RECURSION FOR EACH.
  for (size_t i = 0; i < skeleton->m_Joints.size(); ++i) {
    if (skeleton->m_Joints[i].m_ParentJoint == -1) {
      // Found a root joint! Start a new FK chain from here.

      // Pass the model's root transform as the parent transform
      // for this chain's first joint.
      CalculateJointTransforms(finalPose, skeleton, i, rootModelTransform);
    }
  }
}
std::vector<glm::mat4> Animator::GetFinalMatrices() {

  if (!skeleton) {
    // Return an empty vector or log an error if the skeleton isn't set.
    std::cerr
        << "Animator Error: Cannot get final matrices, skeleton is nullptr."
        << std::endl;
    return {};
  }
  // This vector contains the M_Final matrices calculated in the Update loop.
  return skeleton->finalMatrices;
}

} // namespace eHazGraphics
