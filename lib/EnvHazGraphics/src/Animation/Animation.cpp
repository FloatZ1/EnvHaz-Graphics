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
    // Return a default/bind pose if the animation has no data
    return KeyFrame();
  }

  // 1. Handle Looping
  // We assume the duration is the time of the last keyframe for simplicity,
  // but a proper `duration` member should be used for safety.
  float animationDuration = frames.back().timeStamp;
  time = std::fmod(time, animationDuration);
  if (time < 0.0f) {
    time += animationDuration;
  }

  // 2. Find the bracketing keyframes (prevFrame and nextFrame)
  size_t prevFrameIndex = 0;
  size_t nextFrameIndex = 0;

  for (size_t i = 0; i < frames.size(); ++i) {
    // Use timeStamp as defined in your KeyFrame struct
    if (frames[i].timeStamp > time) {
      nextFrameIndex = i;
      // Handle looping back to the last frame if we are between the last frame
      // and time 0
      prevFrameIndex = (i == 0) ? frames.size() - 1 : i - 1;
      break;
    }
  }

  // Safety check for single-frame animations or if time is past the last frame
  if (nextFrameIndex == 0) {
    // Time is past the last frame (loop has been handled) or equals the last
    // frame time. Return the last frame's pose.
    return frames[frames.size() - 1];
  }

  const KeyFrame &prevFrame = frames[prevFrameIndex];
  const KeyFrame &nextFrame = frames[nextFrameIndex];

  // 3. Calculate the Interpolation Factor (alpha)
  float totalTime = nextFrame.timeStamp - prevFrame.timeStamp; // Use timeStamp

  // Handle case where two frames are at the same time
  if (totalTime < std::numeric_limits<float>::epsilon()) {
    return prevFrame;
  }

  // alpha is the normalized time (0.0 to 1.0) between prevFrame and nextFrame
  float alpha = (time - prevFrame.timeStamp) / totalTime;

  // 4. Interpolate Each Joint
  KeyFrame interpolatedPose;
  // Assuming all KeyFrames have the same number of JointTransforms
  size_t jointCount = prevFrame.transforms.size();
  interpolatedPose.transforms.resize(jointCount);

  for (size_t i = 0; i < jointCount; ++i) {
    const JointTransform &prevT = prevFrame.transforms[i];
    const JointTransform &nextT = nextFrame.transforms[i];

    JointTransform &resultT = interpolatedPose.transforms[i];

    // Position (Translation): Linear Interpolation (LERP)
    // Using `position` as defined in your struct
    resultT.position = glm::lerp(prevT.position, nextT.position, alpha);

    // Rotation: Spherical Linear Interpolation (SLERP)
    resultT.rotation = glm::slerp(prevT.rotation, nextT.rotation, alpha);

    // Scale: Linear Interpolation (LERP)
    resultT.scale = glm::lerp(prevT.scale, nextT.scale, alpha);
  }

  interpolatedPose.timeStamp = time;

  return interpolatedPose;
}

JointTransform Animation::GetJointTransform(size_t jointIndex, float time) {
  KeyFrame pose = GetPoseAt(time);
  if (jointIndex < pose.transforms.size()) {
    return pose.transforms[jointIndex];
  }
  return JointTransform(); // Return default transform on error
}

size_t Animation::GetJointCount() const {
  if (frames.empty())
    return 0;
  return frames.front().transforms.size();
}

} // namespace eHazGraphics
