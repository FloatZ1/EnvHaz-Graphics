#ifndef ENVHAZ_GRAPHICS_UTILS_ALGHORITHMS
#define ENVHAZ_GRAPHICS_UTILS_ALGHORITHMS
#include "glm/glm.hpp"
#include <algorithm>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>
#include <iostream>
#include <vector>
namespace eHazGraphics_Utils {
glm::vec3 BlendVec3s(const std::vector<glm::vec3> &vectors,
                     const std::vector<float> &weights);
glm::quat BlendQuats(const std::vector<glm::quat> &quaternions,
                     const std::vector<float> &weights);

}; // namespace eHazGraphics_Utils

#endif
