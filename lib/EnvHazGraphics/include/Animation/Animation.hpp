#ifndef ENVHAZ_ANIMATION_HPP
#define ENVHAZ_ANIMATION_HPP

#include <vector>

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

namespace eHazGraphics {

struct JointTransform {

  glm::vec3 scale;
  glm::vec3 position;
  glm::quat rotation;
};
struct KeyFrame {

  std::vector<JointTransform> transforms;
  float timeStamp;
};

struct IAnimationSource {
  virtual KeyFrame GetPoseAt(float time) = 0;
};
class Animation : IAnimationSource {
public:
  int owningSkeleton;
  std::vector<KeyFrame> frames;

  KeyFrame GetPoseAt(float time) override;

  JointTransform GetJointTransform(size_t jointIndex, float time);

  size_t GetJointCount() const;

  float ticksPerSecond = 25.0f;

  float GetTicksPerSecond() const { return ticksPerSecond; }
  float GetDurationTicks() const {
    return frames.empty() ? 0.0f : frames.back().timeStamp;
  }

  // These can be set when loading from Assimp
  void SetTicksPerSecond(float tps) { ticksPerSecond = tps; }
};

} // namespace eHazGraphics

#endif
