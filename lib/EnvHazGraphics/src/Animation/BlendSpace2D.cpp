#include "Animation/Animator.hpp"
#include "Utils/Alghorithms.hpp"

using namespace eHazGraphics_Utils;
namespace eHazGraphics {

KeyFrame BlendSpace2D::GetPoseAt(float time) {

  std::vector<std::pair<std::shared_ptr<Animation>, float>> weightedClips =
      CalculateWeights(HorizontalAxis, VerticalAxis);

  if (weightedClips.empty()) {
    return KeyFrame{};
  }

  // Find the longest clip duration across ALL points (not just weighted ones).
  // This is the same value used in Animator::Update to advance the timer,
  // so time is already fmod'd against this value when it arrives here.
  float maxDuration = 0.0f;
  for (const auto &p : points) {
    float d = p.clip->GetDurationTicks();
    if (d > maxDuration)
      maxDuration = d;
  }

  // Normalize time to a 0-1 progress based on the longest clip,
  // then remap into each individual clip's own duration before sampling.
  // This keeps all clips in sync — a short idle and a long run will both
  // be at the "same point" in their cycle at any given moment.
  struct SampledClip {
    KeyFrame pose;
    float weight;
  };
  std::vector<SampledClip> sampledClips;
  sampledClips.reserve(weightedClips.size());

  for (const auto &pair : weightedClips) {
    const auto &clip = pair.first;
    float clipDur = clip->GetDurationTicks();
    float remappedTime = time;

    // Remap: progress through maxDuration → equivalent tick in this clip
    if (maxDuration > 0.0f && clipDur > 0.0f) {
      float progress = time / maxDuration; // 0.0 to 1.0
      remappedTime = progress * clipDur;   // scaled to this clip
      // clamp to avoid floating point overshoot past the last frame
      if (remappedTime >= clipDur)
        remappedTime = clipDur - 0.001f;
    }

    sampledClips.push_back({clip->GetPoseAt(remappedTime), pair.second});
  }

  size_t jointCount = sampledClips[0].pose.transforms.size();

  KeyFrame blendedPose;
  blendedPose.timeStamp = time;
  blendedPose.transforms.resize(jointCount);

  for (size_t jointIndex = 0; jointIndex < jointCount; ++jointIndex) {

    std::vector<glm::vec3> positions;
    std::vector<glm::quat> rotations;
    std::vector<glm::vec3> scales;
    std::vector<float> blendWeights;

    positions.reserve(sampledClips.size());
    rotations.reserve(sampledClips.size());
    scales.reserve(sampledClips.size());
    blendWeights.reserve(sampledClips.size());

    for (const auto &sc : sampledClips) {
      if (jointIndex >= sc.pose.transforms.size())
        continue;
      const JointTransform &t = sc.pose.transforms[jointIndex];
      positions.push_back(t.position);
      rotations.push_back(t.rotation);
      scales.push_back(t.scale);
      blendWeights.push_back(sc.weight);
    }

    JointTransform &out = blendedPose.transforms[jointIndex];
    out.position = BlendVec3s(positions, blendWeights);
    out.scale = BlendVec3s(scales, blendWeights);
    out.rotation = BlendQuats(rotations, blendWeights);
  }

  return blendedPose;
}

std::vector<std::pair<std::shared_ptr<Animation>, float>>
BlendSpace2D::CalculateWeights(float xIn, float yIn) {

  std::vector<std::pair<std::shared_ptr<Animation>, float>> weightedClips;

  if (points.size() < 3 || topology.empty())
    return weightedClips;

  glm::vec2 P(xIn, yIn);
  const float EPSILON = 0.0001f;

  for (const auto &tri : topology) {

    if (tri.indices[0] >= points.size() || tri.indices[1] >= points.size() ||
        tri.indices[2] >= points.size()) {
      continue;
    }

    const BlendPoint &pA = points[tri.indices[0]];
    const BlendPoint &pB = points[tri.indices[1]];
    const BlendPoint &pC = points[tri.indices[2]];

    glm::vec2 A(pA.x, pA.y);
    glm::vec2 B(pB.x, pB.y);
    glm::vec2 C(pC.x, pC.y);

    float D = (B.y - C.y) * (A.x - C.x) + (C.x - B.x) * (A.y - C.y);

    if (std::abs(D) < EPSILON)
      continue;

    float N_A = (B.y - C.y) * (P.x - C.x) + (C.x - B.x) * (P.y - C.y);
    float W_A = N_A / D;

    float N_B = (C.y - A.y) * (P.x - C.x) + (A.x - C.x) * (P.y - C.y);
    float W_B = N_B / D;

    float W_C = 1.0f - W_A - W_B;

    if (W_A >= -EPSILON && W_B >= -EPSILON && W_C >= -EPSILON) {

      float sum = W_A + W_B + W_C;
      if (sum > EPSILON) {
        W_A /= sum;
        W_B /= sum;
        W_C /= sum;
      }

      weightedClips.push_back({pA.clip, W_A});
      weightedClips.push_back({pB.clip, W_B});
      weightedClips.push_back({pC.clip, W_C});

      return weightedClips;
    }
  }

  // Fallback: give full weight to the nearest point
  float minSqDist = std::numeric_limits<float>::max();
  int closestIndex = -1;
  for (size_t i = 0; i < points.size(); ++i) {
    float dx = points[i].x - xIn;
    float dy = points[i].y - yIn;
    float distSq = dx * dx + dy * dy;
    if (distSq < minSqDist) {
      minSqDist = distSq;
      closestIndex = static_cast<int>(i);
    }
  }

  if (closestIndex != -1) {
    // FIXED: was using map syntax, now uses vector push_back
    weightedClips.push_back({points[closestIndex].clip, 1.0f});
  }

  return weightedClips;
}

void BlendSpace2D::RecalculateTopology() {
  topology.clear();

  if (points.size() < 3) {
    std::cout << "DEBUG: Not enough BlendPoints to form a triangle."
              << std::endl;
    return;
  }

  for (size_t i = 1; i < points.size() - 1; ++i) {
    BlendTriangle tri;
    tri.indices[0] = 0;
    tri.indices[1] = i;
    tri.indices[2] = i + 1;
    topology.push_back(tri);
  }

  std::cout << "DEBUG: BlendSpace topology calculated with " << topology.size()
            << " triangles using simplified fan method." << std::endl;
}

} // namespace eHazGraphics
