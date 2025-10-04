#include "ShaderManager.hpp"
#include "BitFlags.hpp"
#include "DataStructs.hpp"
#include "Utils/HashedStrings.hpp"
#include "glad/glad.h"
#include <memory>



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




    auto vIterator = LoadedShaders.try_emplace(vs, std::make_unique<Shader>(vertexShaderPath)).first;



    auto fIterator = LoadedShaders.try_emplace(fs, std::make_unique<Shader>(fragmentShaderPath)).first;



    if (LoadedProgrammes.find(cmp) == LoadedProgrammes.end())
    {
        LoadedProgrammes.emplace(cmp, StandartShaderProgramme(*vIterator->second, *fIterator->second));
        return cmp;
    }


    return cmp;
}

void ShaderManager::SetOpenGLFlags(const StandartShaderProgramme &shaderProgramme)
{
    BitFlag<ShaderManagerFlags> flags = shaderProgramme.GetFlags();

    if (flags.HasFlag(ShaderManagerFlags::DISABLE_DEPTH_TEST))
    {
        glDisable(GL_DEPTH_TEST);
    }

    if (flags.HasFlag(ShaderManagerFlags::ENABLE_BLEND))
    {
        glEnable(GL_BLEND);
    }

    if (flags.HasFlag(ShaderManagerFlags::DISABLE_CULL_FACE))
    {
        glDisable(GL_CULL_FACE);
    }

    if (flags.HasFlag(ShaderManagerFlags::ENABLE_WIREFRAME))
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    if (flags.HasFlag(ShaderManagerFlags::DISABLE_DEPTH_WRITE))
    {
        glDepthMask(GL_FALSE);
    }

    if (flags.HasFlag(ShaderManagerFlags::BLEND_ALPHA))
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    if (flags.HasFlag(ShaderManagerFlags::BLEND_ADDITIVE))
    {
        glBlendFunc(GL_ONE, GL_ONE);
    }

    if (flags.HasFlag(ShaderManagerFlags::DEPTH_LESS_EQUAL))
    {
        glDepthFunc(GL_LEQUAL);
    }

    if (flags.HasFlag(ShaderManagerFlags::ENABLE_STENCIL_TEST))
    {
        glEnable(GL_STENCIL_TEST);
    }
}
void ShaderManager::UseProgramme(const ShaderComboID &ShaderProgrammeID)
{
    auto it = LoadedProgrammes.find(ShaderProgrammeID);
    if (it != LoadedProgrammes.end())
    {
        SetOpenGLFlags(it->second);
        glUseProgram(it->second.GetGLShaderID());
    }
    else
    {
        SDL_Log("Shader programme not found!");
        // Optionally handle missing shader
    }
}

void ShaderManager::Initialize()
{
}

void ShaderManager::Destroy()
{
}



} // namespace eHazGraphics
