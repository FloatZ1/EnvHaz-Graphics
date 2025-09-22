#include "ShaderManager.hpp"
#include "BitFlags.hpp"
#include "DataStructs.hpp"
#include "Utils/HashedStrings.hpp"
#include "glad/glad.h"



namespace eHazGraphics
{

StandartShaderProgramme::StandartShaderProgramme(Shader &shader1, Shader &shader2)
{

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
    if (!successPR)
    {
        glGetProgramInfoLog(progID, 512, NULL, infoLogPR);

        std::string error("ERROR::SHADER::PROGRAMME::COMPILATION_FAILED\n");
        error += infoLogPR;
        SDL_Log("%s", error.c_str());
    }

    FlipFlags(shader1.Type());
    FlipFlags(shader2.Type());
}


void StandartShaderProgramme::FlipFlags(BitFlag<ShaderManagerFlags> flags)
{

    executionFlags.SetFlagsFrom(flags);
}


void StandartShaderProgramme::UseProgramme()
{
    glUseProgram(progID);
}

////Shader manager--------------------------------------
///
///
///
///


ShaderComboID ShaderManager::CreateShaderProgramme(const std::string &vertexShaderPath,
                                                   const std::string &fragmentShaderPath)
{
    eHazGraphics_Utils::HashedString vs;
    eHazGraphics_Utils::HashedString fs;




    vs = eHazGraphics_Utils::computeHash(vertexShaderPath);
    fs = eHazGraphics_Utils::computeHash(fragmentShaderPath);

    ShaderComboID cmp = ShaderComboID(vs, fs);




    auto vIterator = LoadedShaders.try_emplace(vs, Shader(vertexShaderPath)).first;



    auto fIterator = LoadedShaders.try_emplace(fs, Shader(fragmentShaderPath)).first;



    if (LoadedProgrammes.find(cmp) == LoadedProgrammes.end())
    {
        LoadedProgrammes[cmp] = StandartShaderProgramme(vIterator->second, fIterator->second);
        return cmp;
    }


    return cmp;
}

void ShaderManager::UseProgramme(const ShaderComboID &ShaderProgrammeID)
{

    glUseProgram(LoadedProgrammes[ShaderProgrammeID].GetGLShaderID());
}





} // namespace eHazGraphics
