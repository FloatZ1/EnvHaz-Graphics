
#ifndef ENVHAZ_ANIMATOR_HPP
#define ENVHAZ_ANIMATOR_HPP

#include "Animation/Animation.hpp"
#include "DataStructs.hpp"
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

namespace eHazGraphics {

class Animator {
public:
  Animator() : currentAnimation(0), animationTime(0.0f) {}

  void Update(float deltaTime) {
    if (animations.empty())
      return;
    if (currentAnimation >= animations.size())
      return;

    animationTime += deltaTime;
    const Animation &anim = animations[currentAnimation];
    if (anim.frames.empty())
      return;

    // Loop animation
    float duration = anim.frames.back().timeStamp;
    animationTime = fmod(animationTime, duration);

    // Find current keyframe
    size_t frameIndex = 0;
    while (frameIndex < anim.frames.size() - 1 &&
           animationTime > anim.frames[frameIndex + 1].timeStamp)
      frameIndex++;

    const KeyFrame &frameA = anim.frames[frameIndex];
    const KeyFrame &frameB = anim.frames[(frameIndex + 1) % anim.frames.size()];

    float t = (animationTime - frameA.timeStamp) /
              (frameB.timeStamp - frameA.timeStamp);
    t = glm::clamp(t, 0.0f, 1.0f);

    finalMatrices.clear();
    finalMatrices.reserve(frameA.transforms.size());

    for (size_t i = 0; i < frameA.transforms.size(); ++i) {
      const JointTransform &tA = frameA.transforms[i];
      const JointTransform &tB = frameB.transforms[i];

      glm::vec3 position = glm::mix(tA.position, tB.position, t);
      glm::vec3 scale = glm::mix(tA.scale, tB.scale, t);
      glm::quat rotation =
          glm::normalize(glm::slerp(tA.rotation, tB.rotation, t));

      glm::mat4 mat = glm::translate(glm::mat4(1.0f), position) *
                      glm::mat4_cast(rotation) *
                      glm::scale(glm::mat4(1.0f), scale);

      finalMatrices.push_back(mat);
    }
    // std::reverse(finalMatrices.begin(), finalMatrices.end());
  }

  const std::vector<glm::mat4> &GetSubmissionMatrices4x4() const {
    return finalMatrices;
  }

  void PlayAnimation(unsigned int index) {
    if (index < animations.size()) {
      currentAnimation = index;
      animationTime = 0.0f;
    }
  }

  int AddAnimation(const Animation &anim) {
    animations.push_back(anim);
    return animations.size() - 1;
  }

private:
  std::vector<Animation> animations;
  unsigned int currentAnimation;
  float animationTime;
  std::vector<glm::mat4> finalMatrices;
};

} // namespace eHazGraphics

#endif
