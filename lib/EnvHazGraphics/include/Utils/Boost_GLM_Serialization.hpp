#ifndef ENVHAZGRAPHICS_GLM_SERIALIZE_HPP
#define ENVHAZGRAPHICS_GLM_SERIALIZE_HPP

#include <boost/serialization/serialization.hpp>
#include <glm/glm.hpp>

namespace boost {
namespace serialization {

// glm::vec2
template <class Archive>
void serialize(Archive &ar, glm::vec2 &v, const unsigned int) {
  ar & v.x & v.y;
}

// glm::vec3
template <class Archive>
void serialize(Archive &ar, glm::vec3 &v, const unsigned int) {
  ar & v.x & v.y & v.z;
}
// glm::ivec3
template <class Archive>
void serialize(Archive &ar, glm::ivec3 &v, const unsigned int) {
  ar & v.x & v.y & v.z;
}
// glm::vec4
template <class Archive>
void serialize(Archive &ar, glm::vec4 &v, const unsigned int) {
  ar & v.x & v.y & v.z & v.w;
}

template <class Archive>
void serialize(Archive &ar, glm::ivec4 &v, const unsigned int) {
  ar & v.x & v.y & v.z & v.w;
}

// glm::mat4
template <class Archive>
void serialize(Archive &ar, glm::mat4 &m, const unsigned int) {
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      ar &m[i][j];
}

} // namespace serialization
} // namespace boost

#endif // ENVHAZGRAPHICS_GLM_SERIALIZE_HPP
