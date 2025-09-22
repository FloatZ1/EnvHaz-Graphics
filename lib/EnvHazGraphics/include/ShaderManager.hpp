#ifndef SHADER_MANAGER_HPP
#define SHADER_MANAGER_HPP



#include "BitFlags.hpp"
#include "DataStructs.hpp"
#include "Utils/HashedStrings.hpp"
#include "Utils/SDL_HELPERS.hpp"
#include "glad/glad.h"
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_log.h>
#include <climits>
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
namespace eHazGraphics
{






class Shader
{

  public:
    Shader(std::string shaderPath) : shaderSource(shaderPath)
    {
        data = SDLFileReadBuffer(shaderSource);
        type = shaderTypeFromExtension(shaderPath);
    }

    const BitFlag<ShaderManagerFlags> &Type() const
    {
        return shaderFlags;
    }

    std::string GetData() const
    {
        return data.getString();
    }

    // add a function later which reads shader metadata from a file and adds the necessary flags
    bool ReadMetaData();



    GLuint GetGLShaderID()
    {
        if (OpenGL_ShaderExists())
            return shaderID;

        Compile();
        return shaderID;
    }



    bool OpenGL_ShaderExists() const
    {
        return shaderID != 0;
    }


    ~Shader()
    {
        if (OpenGL_ShaderExists())
        {
            glDeleteShader(shaderID);
        }
    }

  private:
    GLenum shaderTypeFromExtension(const std::string &path)
    {

        if (path.ends_with(".vs"))
            return GL_VERTEX_SHADER;
        if (path.ends_with(".fs"))
            return GL_FRAGMENT_SHADER;
        if (path.ends_with(".gs"))
            return GL_GEOMETRY_SHADER;
        if (path.ends_with(".cs"))
            return GL_COMPUTE_SHADER;
        throw std::runtime_error("Unknown shader extension: " + path);
    }

    void Compile()
    {
        int success;
        char infoLog[512];
        shaderID = glCreateShader(type);

        std::string tempSh = data.getString();
        const char *sh = tempSh.c_str();

        glShaderSource(shaderID, 1, &sh, NULL);
        glCompileShader(shaderID);
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);

        if (!success)
        {
            glGetShaderInfoLog(shaderID, 512, NULL, infoLog);

            std::string error("ERROR::SHADER::COMPILATION_FAILED\n");
            error += infoLog;
            SDL_Log("%s", error.c_str());
        }
    }


    SDLFileReadBuffer data;
    GLenum type;
    BitFlag<ShaderManagerFlags> shaderFlags;
    GLuint shaderID = 0;
    std::string shaderSource;
};





class StandartShaderProgramme
{

  public:
    StandartShaderProgramme(Shader &shader1, Shader &shader2);


    GLuint GetGLShaderID() const
    {
        return progID;
    }

    const BitFlag<ShaderManagerFlags> GetFlags() const
    {
        return shaderFlags;
    }


    void UseProgramme();

    ~StandartShaderProgramme()
    {
        glDeleteProgram(progID);
    }

  private:
    void FlipFlags(BitFlag<ShaderManagerFlags> flags);

    BitFlag<ShaderManagerFlags> executionFlags;


    BitFlag<ShaderManagerFlags> shaderFlags;
    unsigned int progID = 0;
    unsigned int vertexShader = 0;
    unsigned int fragmentShader = 0;
};






class ShaderManager
{
  public:
    void Initialize();

    // well this is a bummer i cant load binary shader programmes like in Vulkan unless
    // i use extensions, When i port all this to vulkan ill remake this
    ShaderComboID CreateShaderProgramme(const std::string &vertexShaderPath, const std::string &fragmentShaderPath);

    void UseProgramme(const ShaderComboID &ShaderProgrammeID);




    void Destroy();

  private:
    // Sets the OpenGL flags needed for the shader to work correctly, like for example enable/disable blending etc.
    // TODO: Implement
    void SetdOpenGLFlags(const StandartShaderProgramme &shaderProgramme);



    // convert the paths to HashedString and match them with their shaders
    std::unordered_map<eHazGraphics_Utils::HashedString, Shader> LoadedShaders;
    // brain ache, if performance is really tight in the future, hash the two hashesh toghether to create a single value
    // for lookup
    std::unordered_map<ShaderComboID, StandartShaderProgramme, ShaderComboID::ShaderComboHasher> LoadedProgrammes;
};






} // namespace eHazGraphics







#endif
