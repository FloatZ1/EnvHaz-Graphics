#ifndef MATH_UTILS_HPP
#define MATH_UTILS_HPP
#include <assimp/matrix4x4.h>
#include <glm/glm.hpp>
namespace eHazGraphics_Utils
{
static glm::mat4 convertAssimpMatrixToGLM(aiMatrix4x4 mat)
{

    glm::mat4 result;

    // Assimp uses row-major order, so we transpose while copying
    result[0][0] = mat.a1;
    result[1][0] = mat.a2;
    result[2][0] = mat.a3;
    result[3][0] = mat.a4;
    result[0][1] = mat.b1;
    result[1][1] = mat.b2;
    result[2][1] = mat.b3;
    result[3][1] = mat.b4;
    result[0][2] = mat.c1;
    result[1][2] = mat.c2;
    result[2][2] = mat.c3;
    result[3][2] = mat.c4;
    result[0][3] = mat.d1;
    result[1][3] = mat.d2;
    result[2][3] = mat.d3;
    result[3][3] = mat.d4;

    return result;
}
} // namespace eHazGraphics_Utils




#endif
