#ifndef ENVHAZ_ANIMATION_HPP
#define ENVHAZ_ANIMATION_HPP




#include <vector>

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

namespace eHazGraphics
{
struct JointTransform
{

    glm::vec3 scale;
    glm::vec3 position;
    glm::quat rotation;
};
struct KeyFrame
{


    std::vector<JointTransform> transforms;
    float timeStamp;
};
struct Animation
{

    int owningSkeleton;
    std::vector<KeyFrame> frames;
};

} // namespace eHazGraphics



#endif
