#ifndef DATA_STRUCTS_HPP
#define DATA_STRUCTS_HPP



#include "Utils/HashedStrings.hpp"
#include "stbi_image.h"
#include <SDL3/SDL_log.h>
#include <filesystem>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <utility>
#include <vector>

namespace eHazGraphics
{


struct BufferRange
{
    int OwningBuffer;
    int slot; // if slot is -1 we assume its in the static buffer;
    size_t size;
    size_t offset;
};


typedef int MeshID;
using VertexIndexInfoPair = std::pair<BufferRange, BufferRange>;


struct ShaderComboID
{
    ShaderComboID() = default;

    eHazGraphics_Utils::HashedString vertex;
    eHazGraphics_Utils::HashedString fragment;
    ShaderComboID(eHazGraphics_Utils::HashedString vs, eHazGraphics_Utils::HashedString fs)
        : vertex(vs), fragment(fs) {};




    struct ShaderComboHasher
    {
        std::size_t operator()(const ShaderComboID &id) const noexcept
        {
            // simple hash combine
            return id.vertex ^ (id.fragment + 0x9e3779b97f4a7c15ULL + (id.vertex << 6) + (id.vertex >> 2));
        }
    };


    void operator=(const ShaderComboID &other)
    {
        vertex = other.vertex;
        fragment = other.fragment;
    }



    bool operator==(const ShaderComboID &other) const
    {
        return vertex == other.vertex && fragment == other.fragment;
    }
};







struct Vertex
{
    glm::vec3 Postion;
    glm::vec2 UV;
    glm::vec3 Normal;
};


class MeshData
{
  public:
    std::vector<Vertex> vertices;

    std::vector<int> indecies;
};

class Texture2D
{

    GLuint64 TextureHandle;
    GLuint texture;
    int width, height, nrChannel;
    unsigned char *data;

  public:
    Texture2D(std::string texturePath, GLenum storageFormat, GLenum imageFormat)
    {


        data = stbi_load(texturePath.c_str(), &width, &height, &nrChannel, 0);


        glCreateTextures(GL_TEXTURE_2D, 1, &texture);

        glTextureStorage2D(texture, 1, storageFormat, width, height);
        glTextureSubImage2D(texture, 0, 0, 0, width, height, imageFormat, GL_UNSIGNED_BYTE, (const void *)&data[0]);

        glGenerateTextureMipmap(texture);

        TextureHandle = glGetTextureHandleARB(texture);
        if (TextureHandle == 0)
        {
            SDL_Log("Could not load the texturePath: ");
        }
    }


    void MakeResident() const
    {
        glMakeTextureHandleResidentARB(TextureHandle);
    }

    void RemoveResidency() const
    {
        glMakeTextureHandleNonResidentARB(texture);
    }


    int GetWidth() const
    {
        return width;
    }

    int GetHeight() const
    {
        return height;
    }

    int GetNrChannels() const
    {
        return nrChannel;
    }

    int GetTexture() const
    {
        return texture;
    }

    int GetTextureHandle() const
    {
        return TextureHandle;
    }




    ~Texture2D()
    {
        stbi_image_free(data);
        RemoveResidency();
        glDeleteTextures(1, &texture);
    }
};



typedef struct
{
    uint count;
    uint instanceCount;
    uint firstIndex;
    int baseVertex;
    uint baseInstance;
} DrawElementsIndirectCommand;


struct InstanceData
{
    glm::mat4 modelMat;
    unsigned int materialID;
};



}; // namespace eHazGraphics



#endif
