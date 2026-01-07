#ifndef EnvHazGraphics_DEBUG_LINES
#define EnvHazGraphics_DEBUG_LINES
#include "glm/glm.hpp"
#include <cstdlib>
#include <glad/glad.h>
#include <vector>
using namespace glm;
//using namespace std;
namespace eHazGraphics_Utils {

class Line {
  int shaderProgram;
  unsigned int VBO, VAO;
  std::vector<float> vertices;
  vec3 startPoint;
  vec3 endPoint;
  mat4 MVP;
  vec3 lineColor;
  float Radius;
  int ID;
  const char *vertexShaderSource =
      "#version 460 core\n"
      "struct VP{mat4 view; mat4 projection;};\n"
      "layout(binding = 5,std430) readonly buffer ssbo5{VP camMats;};\n"
      "layout (location = 0) in vec3 aPos;\n"
      "uniform mat4 MVP;\n"
      "void main()\n"
      "{\n"
      "   gl_Position = inverse(camMats.projection * camMats.view) * "
      "vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
      "}\0";
  const char *fragmentShaderSource = "#version 460 core\n"
                                     "out vec4 FragColor;\n"
                                     "uniform vec3 color;\n"
                                     "void main()\n"
                                     "{\n"
                                     "   FragColor = vec4(color, 1.0f);\n"
                                     "}\n\0";

public:
  Line(vec3 start, vec3 direction, float radius, int id, bool isLine = true) {

    startPoint = start;
    if (isLine == false)
      endPoint = start + (direction * radius);
    else {
      endPoint = direction;
    }
    Radius = radius;
    lineColor = vec3(1, 1, 1);
    MVP = mat4(1.0f);

    // vertex shader
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors

    // fragment shader
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors

    // link shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    vertices = {
        start.x, start.y, start.z, endPoint.x, endPoint.y, endPoint.z,

    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  int setColor(vec3 color) {
    lineColor = color;
    return 1;
  }

  void UpdateInformation(glm::vec3 start, glm::vec3 end, float width,
                         glm::vec3 color) {

    startPoint = start;
    endPoint = end;
    width = width;
    vertices = {
        start.x, start.y, start.z, end.x, end.y, end.z,

    };

    glNamedBufferData(VBO, sizeof(vertices), vertices.data(), GL_DYNAMIC_DRAW);

    setColor(color);
  }

  int draw() {
    glUseProgram(shaderProgram);
    // glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "MVP"), 1,
    // GL_FALSE, &MVP[0][0]); we will use the standart ssbo for the mvp
    glUniform3fv(glGetUniformLocation(shaderProgram, "color"), 1,
                 &lineColor[0]);

    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, 2);
    return 1;
  }

  ~Line() {

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
  }
};
} // namespace eHazGraphics_Utils
#endif
