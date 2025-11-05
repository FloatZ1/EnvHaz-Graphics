#include "Utils/Alghorithms.hpp"

#include <assimp/scene.h>
namespace eHazGraphics_Utils {

aiMatrix4x4 GetNodeToRootMat4(aiNode *node) {

  aiMatrix4x4 globalTransform = node->mTransformation;

  aiNode *currentNode = node->mParent;
  while (currentNode) {

    globalTransform = currentNode->mTransformation * globalTransform;
    currentNode = currentNode->mParent;
  }

  return globalTransform;
}

glm::quat BlendQuats(const std::vector<glm::quat> &quaternions,
                     const std::vector<float> &weights) {

  // Safety check

  if (quaternions.size() != weights.size() || quaternions.empty()) {
    return glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Return identity quaternion
  }

  // 1. Find the quaternion with the largest weight to use as the starting point
  // (P)
  //    This is important for handling the double-cover problem (q vs -q).
  size_t largestWeightIndex = 0;
  float maxWeight = 0.0f;
  for (size_t i = 0; i < weights.size(); ++i) {
    if (weights[i] > maxWeight) {
      maxWeight = weights[i];
      largestWeightIndex = i;
    }
  }

  // Start with the dominant quaternion
  glm::quat blendedRotation = quaternions[largestWeightIndex];
  float accumulatedWeight = maxWeight;

  // 2. Iteratively blend the remaining quaternions
  for (size_t i = 0; i < quaternions.size(); ++i) {
    if (i == largestWeightIndex || weights[i] <= 0.0001f) {
      continue; // Skip the quaternion we started with or zero-weight quats
    }

    const glm::quat &nextQuat = quaternions[i];
    float nextWeight = weights[i];

    // Ensure we are taking the shortest path (q vs -q)
    if (glm::dot(blendedRotation, nextQuat) < 0.0f) {
      // If the dot product is negative, reverse the sign of the next quaternion
      // to ensure blending is the shortest arc.
      blendedRotation =
          glm::slerp(blendedRotation, -nextQuat,
                     nextWeight / (accumulatedWeight + nextWeight));
    } else {
      blendedRotation =
          glm::slerp(blendedRotation, nextQuat,
                     nextWeight / (accumulatedWeight + nextWeight));
    }

    // Update the accumulated weight
    accumulatedWeight += nextWeight;
  }

  // 3. Renormalize (Slerp should maintain unit length, but good practice)
  return glm::normalize(blendedRotation);
}

glm::vec3 BlendVec3s(const std::vector<glm::vec3> &vectors,
                     const std::vector<float> &weights) {

  // Safety check: The number of vectors must match the number of weights.
  if (vectors.size() != weights.size() || vectors.empty()) {
    // Return identity or throw error (returning identity for safety)
    return glm::vec3(0.0f);
  }

  glm::vec3 result = glm::vec3(0.0f);
  float totalWeight = 0.0f;

  // 1. Calculate the weighted sum
  for (size_t i = 0; i < vectors.size(); ++i) {
    // Accumulate the weighted vector
    result += vectors[i] * weights[i];

    // Accumulate the weight sum for normalization
    totalWeight += weights[i];
  }

  // 2. Normalize the result (crucial if weights don't sum to 1.0)
  if (totalWeight > 0.0001f) {
    // Simple division to ensure the final pose respects the blending ratio
    result /= totalWeight;
  }
  // If totalWeight is zero, result is already zero, which is safe.

  return result;
}

}; // namespace eHazGraphics_Utils
