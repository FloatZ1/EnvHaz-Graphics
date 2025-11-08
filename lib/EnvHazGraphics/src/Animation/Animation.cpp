#include "Animation/Animation.hpp"

#include <cmath>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp> // For glm::lerp/slerp, using GLM's utilities
#include <iostream>
#include <limits>
#include <vector>

namespace eHazGraphics {

KeyFrame Animation::GetPoseAt(float time) {
  if (frames.empty()) {
    return KeyFrame{};
  }

  // Handle looping
  float duration = GetDurationTicks();
  if (duration > 0.0f) {
    time = std::fmod(time, duration);
  }

  // Find surrounding keyframes
  size_t frameIndex = 0;
  for (size_t i = 0; i < frames.size() - 1; ++i) {
    if (time >= frames[i].timeStamp && time < frames[i + 1].timeStamp) {
      frameIndex = i;
      break;
    }
  }

  // If we're at or past the last frame
  if (frameIndex >= frames.size() - 1) {
    return frames.back();
  }

  // Interpolate between frameIndex and frameIndex + 1
  const KeyFrame &frame0 = frames[frameIndex];
  const KeyFrame &frame1 = frames[frameIndex + 1];

  float frameDelta = frame1.timeStamp - frame0.timeStamp;
  float factor =
      (frameDelta > 0.0f) ? (time - frame0.timeStamp) / frameDelta : 0.0f;

  KeyFrame result;
  result.timeStamp = time;
  result.transforms.resize(frame0.transforms.size());

  // Interpolate each joint
  for (size_t i = 0; i < frame0.transforms.size(); ++i) {
    result.transforms[i].position = glm::mix(
        frame0.transforms[i].position, frame1.transforms[i].position, factor);
    result.transforms[i].rotation = glm::slerp(
        frame0.transforms[i].rotation, frame1.transforms[i].rotation, factor);
    result.transforms[i].scale = glm::mix(frame0.transforms[i].scale,
                                          frame1.transforms[i].scale, factor);
  }

  return result;
}

JointTransform Animation::GetJointTransform(size_t jointIndex, float time) {
  KeyFrame pose = GetPoseAt(time);
  if (jointIndex < pose.transforms.size()) {
    return pose.transforms[jointIndex];
  }
  // Return identity transform as fallback
  return JointTransform{
      glm::vec3(1.0f),                  // scale
      glm::vec3(0.0f),                  // position
      glm::quat(1.0f, 0.0f, 0.0f, 0.0f) // rotation
  };
}

size_t Animation::GetJointCount() const {
  return frames.empty() ? 0 : frames[0].transforms.size();
}

} // namespace eHazGraphics
