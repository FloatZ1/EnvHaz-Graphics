#include "Animation/AnimatedModelManager.hpp"
#include "Animation/Animation.hpp" // For KeyFrame, JointTransform
#include "DataStructs.hpp"
#include "Utils/HashedStrings.hpp"
#include "Utils/Math_Utils.hpp" // For Assimp-to-GLM conversions
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/gtc/quaternion.hpp>
// For glm::slerp
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <map>
#include <memory>
#include <string>
#include <vector>

// Anonymous namespace to keep helper functions private to this file
namespace {

// --- Assimp Animation Baking Helpers ---

/**
 * @brief Finds the index of the keyframe just before the current animation
 * time.
 */
unsigned int FindKeyIndex(float animationTime, unsigned int numKeys,
                          const aiVectorKey *keys) {
  for (unsigned int i = 0; i < numKeys - 1; ++i) {
    // If the next key's time is after our current time, then the current key
    // (i) is our start point.
    if (animationTime < (float)keys[i + 1].mTime) {
      return i;
    }
  }
  // If we're past the second-to-last key, return the last key's index.
  return numKeys - 1;
}

/**
 * @brief Finds the index of the keyframe just before the current animation
 * time. (Quaternion overload)
 */
unsigned int FindKeyIndex(float animationTime, unsigned int numKeys,
                          const aiQuatKey *keys) {
  for (unsigned int i = 0; i < numKeys - 1; ++i) {
    if (animationTime < (float)keys[i + 1].mTime) {
      return i;
    }
  }
  return numKeys - 1;
}

/**
 * @brief Calculates the interpolation factor (alpha) between two keyframes.
 * Returns a value between 0.0 and 1.0.
 */
float GetInterpolationFactor(float lastTimeStamp, float nextTimeStamp,
                             float animationTime) {
  float totalTime = nextTimeStamp - lastTimeStamp;
  // Avoid divide-by-zero if keyframes are at the same time
  if (totalTime <= 0.0f)
    return 0.0f;

  float currentTime = animationTime - lastTimeStamp;
  return currentTime / totalTime;
}

/**
 * @brief Interpolates between two position keys (LERP).
 */
glm::vec3 InterpolatePosition(float animationTime, const aiNodeAnim *channel) {
  if (channel->mNumPositionKeys == 1) {
    return eHazGraphics_Utils::convertAssimpVec3ToGLM(
        channel->mPositionKeys[0].mValue);
  }

  unsigned int p0_idx = FindKeyIndex(animationTime, channel->mNumPositionKeys,
                                     channel->mPositionKeys);
  unsigned int p1_idx = p0_idx + 1;

  // Safety check: If we are at or past the last key, return the last key's
  // value
  if (p1_idx >= channel->mNumPositionKeys) {
    return eHazGraphics_Utils::convertAssimpVec3ToGLM(
        channel->mPositionKeys[p0_idx].mValue);
  }

  float factor = GetInterpolationFactor(
      (float)channel->mPositionKeys[p0_idx].mTime,
      (float)channel->mPositionKeys[p1_idx].mTime, animationTime);

  return glm::mix(eHazGraphics_Utils::convertAssimpVec3ToGLM(
                      channel->mPositionKeys[p0_idx].mValue),
                  eHazGraphics_Utils::convertAssimpVec3ToGLM(
                      channel->mPositionKeys[p1_idx].mValue),
                  factor);
}

/**
 * @brief Interpolates between two rotation keys (SLERP).
 */
glm::quat InterpolateRotation(float animationTime, const aiNodeAnim *channel) {
  if (channel->mNumRotationKeys == 1) {
    return eHazGraphics_Utils::convertAssimpQuatToGLM(
        channel->mRotationKeys[0].mValue);
  }

  unsigned int r0_idx = FindKeyIndex(animationTime, channel->mNumRotationKeys,
                                     channel->mRotationKeys);
  unsigned int r1_idx = r0_idx + 1;

  // Safety check for end of animation
  if (r1_idx >= channel->mNumRotationKeys) {
    return eHazGraphics_Utils::convertAssimpQuatToGLM(
        channel->mRotationKeys[r0_idx].mValue);
  }

  float factor = GetInterpolationFactor(
      (float)channel->mRotationKeys[r0_idx].mTime,
      (float)channel->mRotationKeys[r1_idx].mTime, animationTime);

  return glm::slerp(eHazGraphics_Utils::convertAssimpQuatToGLM(
                        channel->mRotationKeys[r0_idx].mValue),
                    eHazGraphics_Utils::convertAssimpQuatToGLM(
                        channel->mRotationKeys[r1_idx].mValue),
                    factor);
}

/**
 * @brief Interpolates between two scale keys (LERP).
 */
glm::vec3 InterpolateScale(float animationTime, const aiNodeAnim *channel) {
  if (channel->mNumScalingKeys == 1) {
    return eHazGraphics_Utils::convertAssimpVec3ToGLM(
        channel->mScalingKeys[0].mValue);
  }

  unsigned int s0_idx = FindKeyIndex(animationTime, channel->mNumScalingKeys,
                                     channel->mScalingKeys);
  unsigned int s1_idx = s0_idx + 1;

  // Safety check for end of animation
  if (s1_idx >= channel->mNumScalingKeys) {
    return eHazGraphics_Utils::convertAssimpVec3ToGLM(
        channel->mScalingKeys[s0_idx].mValue);
  }

  float factor = GetInterpolationFactor(
      (float)channel->mScalingKeys[s0_idx].mTime,
      (float)channel->mScalingKeys[s1_idx].mTime, animationTime);

  return glm::mix(eHazGraphics_Utils::convertAssimpVec3ToGLM(
                      channel->mScalingKeys[s0_idx].mValue),
                  eHazGraphics_Utils::convertAssimpVec3ToGLM(
                      channel->mScalingKeys[s1_idx].mValue),
                  factor);
}

} // namespace

namespace eHazGraphics {

/**
 * @brief Loads an animation file and "bakes" its channel-based data
 * into a snapshot-based KeyFrame system for a specific skeleton.
 */
void AnimatedModelManager::LoadAnimation(std::shared_ptr<Skeleton> skeleton,
                                         std::string &path,
                                         AnimationID &r_AnimationID) {

  // 1. Load the scene containing the animation
  // We use a new importer for each animation file
  Assimp::Importer animationImporter;
  const aiScene *animationScene = animationImporter.ReadFile(
      path, aiProcess_LimitBoneWeights); // Keep bone weights limited

  if (!animationScene || animationScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      animationScene->mNumAnimations == 0) {
    SDL_Log("ERROR LOADING ANIMATION from %s: %s", path.c_str(),
            animationImporter.GetErrorString());
    r_AnimationID = -1;
    return;
  }

  auto &m_BoneMap = skeleton->m_BoneMap;

  // We assume the file contains at least one animation (index 0)
  aiAnimation *assimpAnimation = animationScene->mAnimations[0];

  // 2. Create the new Animation object
  std::shared_ptr<Animation> newAnimation = std::make_shared<Animation>();
  // newAnimation->owningSkeleton = skeleton.get() ? skeleton->GetID() : -1; //
  // Example

  // 3. Bake Assimp's channel-based data into our KeyFrame system
  float ticksPerSecond = (float)(assimpAnimation->mTicksPerSecond != 0
                                     ? assimpAnimation->mTicksPerSecond
                                     : 25.0f);
  float durationInTicks = (float)assimpAnimation->mDuration;

  newAnimation->SetTicksPerSecond(assimpAnimation->mTicksPerSecond != 0.0
                                      ? assimpAnimation->mTicksPerSecond
                                      : 25.0f);

  // We sample at each tick
  // Note: This can be optimized to sample only at unique keyframe times,
  // but sampling per tick is simpler and very robust.
  for (float tick = 0.0f; tick <= durationInTicks; tick += 1.0f) {
    KeyFrame keyFrame;
    keyFrame.timeStamp = tick;

    // Initialize the pose to the bind pose (identity transforms)
    // This ensures un-animated joints remain static
    keyFrame.transforms.resize(skeleton->m_Joints.size());
    for (size_t i = 0; i < skeleton->m_Joints.size(); ++i) {
      keyFrame.transforms[i].position = glm::vec3(0.0f);
      keyFrame.transforms[i].rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
      keyFrame.transforms[i].scale = glm::vec3(1.0f);
    }

    // Iterate over all Assimp channels (one channel per bone)
    for (unsigned int i = 0; i < assimpAnimation->mNumChannels; ++i) {
      aiNodeAnim *channel = assimpAnimation->mChannels[i];
      std::string boneName = channel->mNodeName.data;

      // Find the joint index in our skeleton using the map
      // We use the AnimatedModelManager's m_BoneMap for this
      if (m_BoneMap.find(boneName) == m_BoneMap.end()) {
        continue; // This animation affects a bone not in our skeleton
      }
      int jointIndex = m_BoneMap[boneName];

      // Get the interpolated TRS for this joint at this specific tick
      glm::vec3 pos = InterpolatePosition(tick, channel);
      glm::quat rot = InterpolateRotation(tick, channel);
      glm::vec3 scale = InterpolateScale(tick, channel);

      // Store the transform
      keyFrame.transforms[jointIndex].position = pos;
      keyFrame.transforms[jointIndex].rotation = rot;
      keyFrame.transforms[jointIndex].scale = scale;
    }

    // Add the fully populated "snapshot" keyframe to the animation
    newAnimation->frames.push_back(keyFrame);
  }
  AnimationID animID =
      eHazGraphics_Utils::computeHash(assimpAnimation->mName.data);
  // 4. Store the new animation and return its ID
  animations.emplace(
      eHazGraphics_Utils::computeHash(assimpAnimation->mName.data),
      newAnimation);
  r_AnimationID = animID;

  // animationImporter goes out of scope here, freeing the aiScene
}

} // namespace eHazGraphics
