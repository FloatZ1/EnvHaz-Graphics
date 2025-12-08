#ifndef ENVHAZGRAPHICS_MODELPACKAGE_HPP
#define ENVHAZGRAPHICS_MODELPACKAGE_HPP
#include "Animation/AnimatedModel.hpp"
#include "Model.hpp"
#include <boost/serialization/access.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/utility.hpp> // for std::pair
#include <boost/serialization/vector.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

namespace eHazGraphics {

class StaticModelPackage {

public:
  std::vector<Mesh> meshes;
  Model model;

  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar & meshes;
    ar & model;
  }
};

class AnimatedModelPackage {
public:
  std::vector<Mesh> meshes;
  AnimatedModel model;

  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar & meshes;
    ar & model;
  }
};

} // namespace eHazGraphics

#endif
