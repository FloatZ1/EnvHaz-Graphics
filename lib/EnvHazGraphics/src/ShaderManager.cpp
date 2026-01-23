#include "ShaderManager.hpp"
#include "BitFlags.hpp"
#include "DataStructs.hpp"
#include "Utils/HashedStrings.hpp"
#include "glad/glad.h"
#include <SDL3/SDL_log.h>
#include <memory>

namespace eHazGraphics {

void ShaderManager::SetShaderProgrammeFlags(
    ShaderComboID p_Programme, BitFlag<ShaderManagerFlags> p_replacement) {

  if (LoadedProgrammes.contains(p_Programme)) {
    LoadedProgrammes[p_Programme]->SetFlagsFromOther(p_replacement);
    return;
  }

  SDL_Log("Error: No such programme");
}
bool ShaderManager::isValidID(ShaderComboID p_ID) {

  return LoadedProgrammes.contains(p_ID);
}
void ShaderManager::RemoveShaderProgramme(ShaderComboID p_ID) {

  if (isValidID(p_ID))
    LoadedProgrammes.erase(p_ID);
}
void ShaderManager::RecompileProgramme(ShaderComboID p_ID) {
  LoadedProgrammes[p_ID]->Recompile();
}
void StandartShaderProgramme::Recompile() {
  if (computeShader == 0) {

    if (geometryShader == 0) {

      if (glIsShader(vertexShader) && glIsShader(fragmentShader)) {

        glDeleteProgram(progID);

        int successPR;
        char infoLogPR[512];

        progID = glCreateProgram();

        glAttachShader(progID, vertexShader);
        glAttachShader(progID, fragmentShader);
        glLinkProgram(progID);

        glGetProgramiv(progID, GL_LINK_STATUS, &successPR);
        if (!successPR) {
          glGetProgramInfoLog(progID, 512, NULL, infoLogPR);

          std::string error(
              "ERROR::SHADER::PROGRAMME::RE-COMPILATION_FAILED\n");
          error += infoLogPR;
          SDL_Log("%s", error.c_str());
        }
      }
    } else {

      if (glIsShader(vertexShader) && glIsShader(fragmentShader) &&
          glIsShader(geometryShader)) {

        glDeleteProgram(progID);

        int successPR;
        char infoLogPR[512];

        progID = glCreateProgram();

        glAttachShader(progID, vertexShader);
        glAttachShader(progID, geometryShader);
        glAttachShader(progID, fragmentShader);
        glLinkProgram(progID);

        glGetProgramiv(progID, GL_LINK_STATUS, &successPR);
        if (!successPR) {
          glGetProgramInfoLog(progID, 512, NULL, infoLogPR);

          std::string error(
              "ERROR::SHADER::PROGRAMME::RE-COMPILATION_FAILED\n");
          error += infoLogPR;
          SDL_Log("%s", error.c_str());
        }
        return;
      }
    }
  } else {
    if (glIsShader(computeShader)) {
      glDeleteProgram(progID);
      int successPR;
      char infoLogPR[512];

      progID = glCreateProgram();
      glAttachShader(progID, computeShader);
      glLinkProgram(progID);

      glGetProgramiv(progID, GL_LINK_STATUS, &successPR);
      if (!successPR) {
        glGetProgramInfoLog(progID, 512, NULL, infoLogPR);

        std::string error(
            "ERROR::COMPUTE::SHADER::PROGRAMME::RE-COMPILATION_FAILED\n");
        error += infoLogPR;
        SDL_Log("%s", error.c_str());
      }
      return;
    }
  }

  SDL_Log("ERROR INVALID SHADER:LIKELYHOOD OF USE AFTER FREE");
}

StandartShaderProgramme::StandartShaderProgramme(Shader &p_ComputeShader) {

  computeShader = p_ComputeShader.GetGLShaderID();

  int successPR;
  char infoLogPR[512];

  progID = glCreateProgram();

  glAttachShader(progID, computeShader);
  glLinkProgram(progID);

  glGetProgramiv(progID, GL_LINK_STATUS, &successPR);
  if (!successPR) {
    glGetProgramInfoLog(progID, 512, NULL, infoLogPR);

    std::string error("ERROR::SHADER::PROGRAMME::COMPILATION_FAILED\n");
    error += infoLogPR;
    SDL_Log("%s", error.c_str());
  }

  FlipFlags(p_ComputeShader.Flags());
}

StandartShaderProgramme::StandartShaderProgramme(Shader &p_VertexShader,
                                                 Shader &p_GeometryShader,
                                                 Shader &p_FragmentShader) {

  vertexShader = p_VertexShader.GetGLShaderID();
  fragmentShader = p_FragmentShader.GetGLShaderID();
  geometryShader = p_GeometryShader.GetGLShaderID();

  // PROGRAMME creation ---------------------------------------

  int successPR;
  char infoLogPR[512];

  progID = glCreateProgram();

  glAttachShader(progID, vertexShader);
  glAttachShader(progID, geometryShader);
  glAttachShader(progID, fragmentShader);
  glLinkProgram(progID);

  glGetProgramiv(progID, GL_LINK_STATUS, &successPR);
  if (!successPR) {
    glGetProgramInfoLog(progID, 512, NULL, infoLogPR);

    std::string error("ERROR::SHADER::PROGRAMME::COMPILATION_FAILED\n");
    error += infoLogPR;
    SDL_Log("%s", error.c_str());
  }

  FlipFlags(p_VertexShader.Flags());
  FlipFlags(p_GeometryShader.Flags());
  FlipFlags(p_FragmentShader.Flags());
}

StandartShaderProgramme::StandartShaderProgramme(Shader &shader1,
                                                 Shader &shader2) {

  vertexShader = shader1.GetGLShaderID();
  fragmentShader = shader2.GetGLShaderID();

  // PROGRAMME creation ---------------------------------------

  int successPR;
  char infoLogPR[512];

  progID = glCreateProgram();

  glAttachShader(progID, vertexShader);
  glAttachShader(progID, fragmentShader);
  glLinkProgram(progID);

  glGetProgramiv(progID, GL_LINK_STATUS, &successPR);
  if (!successPR) {
    glGetProgramInfoLog(progID, 512, NULL, infoLogPR);

    std::string error("ERROR::SHADER::PROGRAMME::COMPILATION_FAILED\n");
    error += infoLogPR;
    SDL_Log("%s", error.c_str());
  }

  FlipFlags(shader1.Flags());
  FlipFlags(shader2.Flags());
}

void StandartShaderProgramme::SetFlagsFromOther(
    BitFlag<ShaderManagerFlags> p_replacement) {

  executionFlags = p_replacement;
}
void StandartShaderProgramme::FlipFlags(BitFlag<ShaderManagerFlags> flags) {

  executionFlags.SetFlagsFrom(flags);
}

void StandartShaderProgramme::UseProgramme() {
  glUseProgram(progID);
  if (computeShader != 0) {

    // glDispatchCompute

    glMemoryBarrier(GL_ALL_BARRIER_BITS);
  }
}

////Shader manager--------------------------------------
///
///
///
///
std::string ExtractShaderName(const std::string &text) {
  const std::string startTag = "//@@start@@";
  const std::string endTag = "@@end@@";

  size_t startPos = text.find(startTag);
  if (startPos == std::string::npos)
    return ""; // not found

  startPos += startTag.length();
  size_t endPos = text.find(endTag, startPos);
  if (endPos == std::string::npos)
    return ""; // not found

  std::string name = text.substr(startPos, endPos - startPos);

  // Trim whitespace
  size_t first = name.find_first_not_of(" \t");
  size_t last = name.find_last_not_of(" \t");
  if (first == std::string::npos || last == std::string::npos)
    return "";
  return name.substr(first, last - first + 1);
}
ShaderComboID
ShaderManager::CreateShaderProgramme(const std::string &vertexShader,
                                     const std::string &fragmentShader,
                                     bool isPath) {
  eHazGraphics_Utils::HashedString vs;
  eHazGraphics_Utils::HashedString fs;

  if (isPath == true) {

    vs = eHazGraphics_Utils::computeHash(vertexShader);
    fs = eHazGraphics_Utils::computeHash(fragmentShader);

  } else {

    std::string specVS = ExtractShaderName(vertexShader);
    std::string specFS = ExtractShaderName(fragmentShader);

    vs = eHazGraphics_Utils::computeHash(specVS);
    fs = eHazGraphics_Utils::computeHash(specFS);
  }

  decltype(LoadedShaders)::iterator vIterator;
  decltype(LoadedShaders)::iterator fIterator;

  ShaderComboID cmp = ShaderComboID(vs, fs);

  if (LoadedProgrammes.contains(cmp))
    return cmp;

  if (isPath) {

    vIterator =
        LoadedShaders.try_emplace(vs, std::make_unique<Shader>(vertexShader))
            .first;

    fIterator =
        LoadedShaders.try_emplace(fs, std::make_unique<Shader>(fragmentShader))
            .first;
  } else {

    ShaderSpec spec{false, ".vert"};
    vIterator =
        LoadedShaders
            .try_emplace(vs, std::make_unique<Shader>(vertexShader, spec))
            .first;
    ShaderSpec fragSpec{false, ".frag"};
    fIterator =
        LoadedShaders
            .try_emplace(fs, std::make_unique<Shader>(fragmentShader, fragSpec))
            .first;
  }
  if (LoadedProgrammes.find(cmp) == LoadedProgrammes.end()) {
    LoadedProgrammes.emplace(cmp, std::make_unique<StandartShaderProgramme>(
                                      *vIterator->second, *fIterator->second));
    return cmp;
  }

  return cmp;
}

void ShaderManager::SetOpenGLFlags(
    const std::shared_ptr<StandartShaderProgramme> shaderProgramme) {
  BitFlag<ShaderManagerFlags> flags = shaderProgramme->GetFlags();

  // ---- Depth Test ----
  if (flags.HasFlag(ShaderManagerFlags::DEPTH_TEST_DISABLED)) {
    glDisable(GL_DEPTH_TEST);
  } else if (flags.HasFlag(ShaderManagerFlags::DEPTH_TEST_ENABLED)) {
    glEnable(GL_DEPTH_TEST);
  } else {
    glEnable(GL_DEPTH_TEST); // default
  }

  // ---- Depth Write ----
  if (flags.HasFlag(ShaderManagerFlags::DEPTH_WRITE_DISABLED)) {
    glDepthMask(GL_FALSE);
  } else if (flags.HasFlag(ShaderManagerFlags::DEPTH_WRITE_ENABLED)) {
    glDepthMask(GL_TRUE);
  } else {
    glDepthMask(GL_TRUE); // default
  }

  // ---- Depth Func ----
  if (flags.HasFlag(ShaderManagerFlags::DEPTH_LESS_EQUAL)) {
    glDepthFunc(GL_LEQUAL);
  } else if (flags.HasFlag(ShaderManagerFlags::DEPTH_LESS)) {
    glDepthFunc(GL_LESS);
  } else {
    glDepthFunc(GL_LESS); // default
  }

  // ---- Blending ----
  if (flags.HasFlag(ShaderManagerFlags::BLEND_DISABLED)) {
    glDisable(GL_BLEND);
  } else if (flags.HasFlag(ShaderManagerFlags::BLEND_ENABLED)) {
    glEnable(GL_BLEND);
  } else {
    glDisable(GL_BLEND); // default
  }

  // ---- Blend Mode ----
  if (flags.HasFlag(ShaderManagerFlags::BLEND_ALPHA)) {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  } else if (flags.HasFlag(ShaderManagerFlags::BLEND_ADDITIVE)) {
    glBlendFunc(GL_ONE, GL_ONE);
  }

  // ---- Face Culling ----
  if (flags.HasFlag(ShaderManagerFlags::CULL_FACE_DISABLED)) {
    glDisable(GL_CULL_FACE);
  } else if (flags.HasFlag(ShaderManagerFlags::CULL_FACE_ENABLED)) {
    glEnable(GL_CULL_FACE);
  } else {
    glEnable(GL_CULL_FACE); // default
  }

  // ---- Wireframe ----
  if (flags.HasFlag(ShaderManagerFlags::WIREFRAME_ENABLED)) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  } else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // default
  }

  // ---- Stencil Test ----
  if (flags.HasFlag(ShaderManagerFlags::STENCIL_TEST_DISABLED)) {
    glDisable(GL_STENCIL_TEST);
  } else if (flags.HasFlag(ShaderManagerFlags::STENCIL_TEST_ENABLED)) {
    glEnable(GL_STENCIL_TEST);
  } else {
    glDisable(GL_STENCIL_TEST); // default
  }
}

void ShaderManager::UseProgramme(const ShaderComboID &ShaderProgrammeID) {
  auto it = LoadedProgrammes.find(ShaderProgrammeID);
  if (it != LoadedProgrammes.end()) {
    SetOpenGLFlags(it->second);
    glUseProgram(it->second->GetGLShaderID());
  } else {
    SDL_Log("Shader programme not found!");
    // Optionally handle missing shader
  }
}

ShaderID ShaderManager::LoadShader(const std::string &p_strPath) {

  ShaderID l_sIDShader = eHazGraphics_Utils::computeHash(p_strPath);

  if (LoadedShaders.contains(l_sIDShader))
    return l_sIDShader;

  LoadedShaders.try_emplace(l_sIDShader, std::make_unique<Shader>(p_strPath));

  return l_sIDShader;
}

ShaderComboID
ShaderManager::CreateShaderProgramme(const std::string &p_ComputeShaderPath) {

  ShaderID l_sIDShader = eHazGraphics_Utils::computeHash(p_ComputeShaderPath);

  ShaderComboID l_scIdProgramme = {l_sIDShader, l_sIDShader};

  if (LoadedProgrammes.contains(l_scIdProgramme))
    return l_scIdProgramme;

  ShaderID l_shIdComputeShader = LoadShader(p_ComputeShaderPath);

  LoadedProgrammes.emplace(l_scIdProgramme,
                           std::make_unique<StandartShaderProgramme>(
                               *LoadedShaders[l_shIdComputeShader]));

  return l_scIdProgramme;
}
ShaderComboID
ShaderManager::CreateShaderProgramme(const std::string &p_VertexShaderPath,
                                     const std::string &p_FragmentShaderPath,
                                     const std::string &p_GeometryShaderPath) {

  ShaderID vs;
  ShaderID fs;
  ShaderID gs;

  vs = eHazGraphics_Utils::computeHash(p_VertexShaderPath +
                                       p_GeometryShaderPath);
  fs = eHazGraphics_Utils::computeHash(p_FragmentShaderPath);
  gs = eHazGraphics_Utils::computeHash(p_GeometryShaderPath);

  ShaderComboID cmp = ShaderComboID(vs, fs);

  if (LoadedProgrammes.contains(cmp))
    return cmp;

  vs = LoadShader(p_VertexShaderPath);
  fs = LoadShader(p_FragmentShaderPath);
  gs = LoadShader(p_GeometryShaderPath);

  LoadedProgrammes.emplace(
      cmp, std::make_unique<StandartShaderProgramme>(
               *LoadedShaders[vs], *LoadedShaders[gs], *LoadedShaders[fs]));

  return cmp;
}
ShaderComboID
ShaderManager::CreateShaderProgramme(ShaderID p_VertexShaderID,
                                     ShaderID p_FragmentShaderID) {
  ShaderID vs = p_VertexShaderID;
  ShaderID fs = p_FragmentShaderID;

  vs = eHazGraphics_Utils::computeHash(LoadedShaders[vs]->GetSource());

  ShaderComboID cmp = ShaderComboID(vs, fs);

  vs = p_VertexShaderID;

  if (LoadedProgrammes.contains(cmp))
    return cmp;

  LoadedProgrammes.emplace(cmp, std::make_unique<StandartShaderProgramme>(
                                    *LoadedShaders[vs], *LoadedShaders[fs]));

  return cmp;
}
ShaderComboID
ShaderManager::CreateShaderProgramme(ShaderID p_VertexShaderID,
                                     ShaderID p_FragmentShaderID,
                                     ShaderID p_GeometryShaderID) {
  ShaderID vs = p_VertexShaderID;
  ShaderID fs = p_FragmentShaderID;
  ShaderID gs = p_GeometryShaderID;

  vs = eHazGraphics_Utils::computeHash(LoadedShaders[vs]->GetSource() +
                                       LoadedShaders[gs]->GetSource());

  ShaderComboID cmp = ShaderComboID(vs, fs);

  vs = p_VertexShaderID;

  if (LoadedProgrammes.contains(cmp))
    return cmp;

  LoadedProgrammes.emplace(
      cmp, std::make_unique<StandartShaderProgramme>(
               *LoadedShaders[vs], *LoadedShaders[gs], *LoadedShaders[fs]));

  return cmp;
}
ShaderComboID ShaderManager::CreateShaderProgramme(ShaderID p_ComputeShaderID) {

  ShaderComboID l_rID = {p_ComputeShaderID, p_ComputeShaderID};

  if (LoadedProgrammes.contains(l_rID))
    return l_rID;

  LoadedProgrammes.emplace(l_rID, std::make_unique<StandartShaderProgramme>(
                                      *LoadedShaders[p_ComputeShaderID]));
}
void ShaderManager::Initialize() {}

void ShaderManager::Destroy() {}

void ShaderManager::setBool(ShaderComboID shader, const std::string &name,
                            bool value) {

  GLuint ID = LoadedProgrammes[shader]->GetGLShaderID();

  glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
void ShaderManager::setInt(ShaderComboID shader, const std::string &name,
                           int value) {

  GLuint ID = LoadedProgrammes[shader]->GetGLShaderID();
  glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
void ShaderManager::setFloat(ShaderComboID shader, const std::string &name,
                             float value) {

  GLuint ID = LoadedProgrammes[shader]->GetGLShaderID();
  glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}
void ShaderManager::setVec2(ShaderComboID shader, const std::string &name,
                            const glm::vec2 &value) {

  GLuint ID = LoadedProgrammes[shader]->GetGLShaderID();
  glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void ShaderManager::setVec2(ShaderComboID shader, const std::string &name,
                            float x, float y) {

  GLuint ID = LoadedProgrammes[shader]->GetGLShaderID();
  glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}
void ShaderManager::setVec3(ShaderComboID shader, const std::string &name,
                            const glm::vec3 &value) {

  GLuint ID = LoadedProgrammes[shader]->GetGLShaderID();
  glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void ShaderManager::setVec3(ShaderComboID shader, const std::string &name,
                            float x, float y, float z) {

  GLuint ID = LoadedProgrammes[shader]->GetGLShaderID();
  glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}
void ShaderManager::setVec4(ShaderComboID shader, const std::string &name,
                            const glm::vec4 &value) {

  GLuint ID = LoadedProgrammes[shader]->GetGLShaderID();
  glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void ShaderManager::setVec4(ShaderComboID shader, const std::string &name,
                            float x, float y, float z, float w) {

  GLuint ID = LoadedProgrammes[shader]->GetGLShaderID();
  glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}
void ShaderManager::setMat2(ShaderComboID shader, const std::string &name,
                            const glm::mat2 &mat) {

  GLuint ID = LoadedProgrammes[shader]->GetGLShaderID();
  glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE,
                     &mat[0][0]);
}
void ShaderManager::setMat3(ShaderComboID shader, const std::string &name,
                            const glm::mat3 &mat) {

  GLuint ID = LoadedProgrammes[shader]->GetGLShaderID();
  glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE,
                     &mat[0][0]);
}
void ShaderManager::setMat4(ShaderComboID shader, const std::string &name,
                            const glm::mat4 &mat) {

  GLuint ID = LoadedProgrammes[shader]->GetGLShaderID();
  glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE,
                     &mat[0][0]);
}

} // namespace eHazGraphics
