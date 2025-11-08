#include "Animation/Animator.hpp"

#include "Utils/Alghorithms.hpp"

using namespace eHazGraphics_Utils;
namespace eHazGraphics {

KeyFrame BlendSpace2D::GetPoseAt(float time) {

  // 1. Get the list of clips and their blend weights based on input axes
  std::map<std::shared_ptr<Animation>, float> weightedClips =
      CalculateWeights(HorizontalAxis, VerticalAxis);

  if (weightedClips.empty()) {
    return KeyFrame{}; // Bind pose fallback
  }

  // Determine joint count (assuming first clip is representative)
  size_t jointCount = weightedClips.begin()->first->GetJointCount();

  KeyFrame blendedPose;
  blendedPose.timeStamp = time;
  blendedPose.transforms.resize(jointCount);

  // 2. Iterate through every joint (J) in the skeleton
  for (size_t jointIndex = 0; jointIndex < jointCount; ++jointIndex) {

    std::vector<glm::vec3> positions;
    std::vector<glm::quat> rotations;
    std::vector<glm::vec3> scales;
    std::vector<float> blendWeights;

    // 3. Sample and Collect: Gather components for joint J from all weighted
    // clips
    for (const auto &pair : weightedClips) {
      std::shared_ptr<Animation> clip = pair.first;
      float weight = pair.second;

      // Sample the specific clip at the current time
      JointTransform sampledTransform =
          clip->GetJointTransform(jointIndex, time);

      positions.push_back(sampledTransform.position);
      rotations.push_back(sampledTransform.rotation);
      scales.push_back(sampledTransform.scale);
      blendWeights.push_back(weight);
    }

    // 4. Blend: Use the Animator's blending utilities
    JointTransform &finalTransform = blendedPose.transforms[jointIndex];

    // Position and Scale: Weighted sum (BlendVec3s)
    finalTransform.position = BlendVec3s(positions, blendWeights);
    finalTransform.scale = BlendVec3s(scales, blendWeights);

    // Rotation: Weighted Slerp (BlendQuats)
    finalTransform.rotation = BlendQuats(rotations, blendWeights);
  }

  return blendedPose;
}
std::map<std::shared_ptr<Animation>, float>
BlendSpace2D::CalculateWeights(float xIn, float yIn) {

  std::map<std::shared_ptr<Animation>, float> weightedClips;

  if (points.size() < 3 || topology.empty())
    return weightedClips;

  // Input point (P)
  glm::vec2 P(xIn, yIn);
  const float EPSILON = 0.0001f;

  // Check all pre-defined triangles in the topology
  for (const auto &tri : topology) {

    // Safety check on indices
    if (tri.indices[0] >= points.size() || tri.indices[1] >= points.size() ||
        tri.indices[2] >= points.size()) {
      continue;
    }

    // Vertices of the current triangle (A, B, C)
    const BlendPoint &pA = points[tri.indices[0]];
    const BlendPoint &pB = points[tri.indices[1]];
    const BlendPoint &pC = points[tri.indices[2]];

    glm::vec2 A(pA.x, pA.y);
    glm::vec2 B(pB.x, pB.y);
    glm::vec2 C(pC.x, pC.y);

    // Calculate Barycentric Coordinates (W1, W2, W3)

    // D is the denominator (twice the area of the triangle ABC)
    float D = (B.y - C.y) * (A.x - C.x) + (C.x - B.x) * (A.y - C.y);

    if (std::abs(D) < EPSILON)
      continue; // Skip degenerate triangle

    // W1 (Weight for A) - calculated using area of P-B-C
    float N_A = (B.y - C.y) * (P.x - C.x) + (C.x - B.x) * (P.y - C.y);
    float W_A = N_A / D;

    // W2 (Weight for B) - calculated using area of A-P-C (rearranged for
    // formula consistency)
    float N_B = (C.y - A.y) * (P.x - C.x) + (A.x - C.x) * (P.y - C.y);
    float W_B = N_B / D;

    // W3 (Weight for C) - derived from W_A + W_B + W_C = 1
    float W_C = 1.0f - W_A - W_B;

    // Check if the input point is inside the triangle
    // Point is inside if all weights are non-negative.
    if (W_A >= -EPSILON && W_B >= -EPSILON && W_C >= -EPSILON) {

      // Found the containing triangle! Normalize weights to ensure sum is
      // exactly 1.0
      float sum = W_A + W_B + W_C;
      if (sum > EPSILON) {
        W_A /= sum;
        W_B /= sum;
        W_C /= sum;
      }

      // Add the weighted clips to the result map
      weightedClips[pA.clip] = W_A;
      weightedClips[pB.clip] = W_B;
      weightedClips[pC.clip] = W_C;

      return weightedClips; // Done!
    }
  }

  // --- Fallback: Closest Point Extrapolation ---
  // If input is outside all triangles, find the nearest point and give it full
  // weight.
  float minSqDist = std::numeric_limits<float>::max();
  int closestIndex = -1;
  for (size_t i = 0; i < points.size(); ++i) {
    float dx = points[i].x - xIn;
    float dy = points[i].y - yIn;
    float distSq = dx * dx + dy * dy;

    if (distSq < minSqDist) {
      minSqDist = distSq;
      closestIndex = i;
    }
  }

  if (closestIndex != -1) {
    weightedClips[points[closestIndex].clip] = 1.0f;
  }

  return weightedClips;
}

void BlendSpace2D::RecalculateTopology() {
  // 1. Clear any existing topology
  topology.clear();

  // 2. Check minimum requirement for a triangle
  if (points.size() < 3) {
    std::cout << "DEBUG: Not enough BlendPoints to form a triangle."
              << std::endl;
    return;
  }

  // --- SIMPLIFIED FAN TRIANGULATION LOGIC (Placeholder) ---
  // This method assumes points are in a convex shape and creates triangles
  // by fanning out from the first point (index 0).
  // This is NOT true Delaunay but achieves the goal of populating 'topology'.

  // Start with the first point (index 0) as the "fan center"
  for (size_t i = 1; i < points.size() - 1; ++i) {
    BlendTriangle tri;
    tri.indices[0] = 0;     // Fan center
    tri.indices[1] = i;     // Current point
    tri.indices[2] = i + 1; // Next point
    topology.push_back(tri);
  }

  std::cout << "DEBUG: BlendSpace topology calculated with " << topology.size()
            << " triangles using simplified fan method." << std::endl;
}
}; // namespace eHazGraphics
