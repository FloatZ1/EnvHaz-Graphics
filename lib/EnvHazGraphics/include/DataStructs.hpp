#ifndef DATA_STRUCTS_HPP
#define DATA_STRUCTS_HPP



#include "BitFlags.hpp"
#include "Utils/HashedStrings.hpp"
#include "stbi_image.h"
#include <SDL3/SDL_log.h>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <utility>
#include <vector>




namespace eHazGraphics
{

#define MBsize(size) ((size) * 1024 * 1024)






struct BufferRange
{
    int OwningBuffer;
    int slot; // if slot is -1 we assume its in the static buffer;
    size_t size;

    size_t offset;
    unsigned int count;

    TypeFlags dataType;
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

    bool operator<(const ShaderComboID &other) const
    {
        if (vertex != other.vertex)
            return vertex < other.vertex;
        return fragment < other.fragment; // compare fragment hash if vertex hash is equal
    }
};







struct Vertex
{
    glm::vec3 Postion;
    glm::vec2 UV;
    glm::vec3 Normal;

    // animation stuff;
    glm::ivec4 boneIDs;
    glm::vec4 boneWeights;
};


class MeshData
{
  public:
    std::vector<Vertex> vertices;

    std::vector<GLuint> indecies;
};

class Texture2D
{
  public:
    GLuint64 TextureHandle;
    GLuint texture;
    int width, height, nrChannel;
    unsigned char *data;


    Texture2D(std::string texturePath, GLenum storageFormat = 0, GLenum imageFormat = 0)
    {



        data = stbi_load(texturePath.c_str(), &width, &height, &nrChannel, 0);

        if (!data)
        {
            std::cerr << "stbi_load failed for " << texturePath << "\n";
            std::cerr << "Reason: " << stbi_failure_reason() << "\n";
            perror("fopen");
        }
        else
        {
            std::cout << "Loaded texture: " << width << "x" << height << " channels: " << nrChannel << std::endl;
        }




        if (storageFormat == 0 && imageFormat == 0)
        {
            switch (nrChannel)
            {
            case 1:
                imageFormat = GL_RED;
                storageFormat = GL_R8; // 8 bits for single channel
                break;
            case 3:
                imageFormat = GL_RGB;
                storageFormat = GL_RGB8; // 8 bits per channel
                break;
            case 4:
                imageFormat = GL_RGBA;
                storageFormat = GL_RGBA8; // 8 bits per channel
                break;
            }
        }


        glCreateTextures(GL_TEXTURE_2D, 1, &texture);

        glTextureStorage2D(texture, 1, storageFormat, width, height);
        glTextureSubImage2D(texture, 0, 0, 0, width, height, imageFormat, GL_UNSIGNED_BYTE, (const void *)&data[0]);
        glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateTextureMipmap(texture);

        TextureHandle = glGetTextureHandleARB(texture);
        if (TextureHandle == 0)
        {
            SDL_Log("Could not load the texturePath: ");
        }
        stbi_image_free(data);
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

        RemoveResidency();
        glDeleteTextures(1, &texture);
    }
};

struct PBRMaterial
{
    // Based on the Hedgehog Engine 2 PBR format
    // https://hedgedocs.com/index.php/Hedgehog_Engine_2_-_Physically_Based_Rendering_(PBR)


    GLuint64 albedo;

    // GLuint64 specular;
    // GLuint64 smoothness;
    // GLuint64 Metalic;
    // GLuint64 AmbientOcclusion;

    GLuint64 prm; // Packed R=Spec, G=Smoothness, B=Metallic, A=AO

    GLuint64 NormalMap;
    GLuint64 Emission;

    // Determines the strength of emision;
    float Luminance = 0.0f;

    float _padding[2]{0.0f, 0.0f};
};

struct DrawElementsIndirectCommand
{
    unsigned int count;
    unsigned int instanceCount;
    unsigned int firstIndex;
    unsigned int baseVertex;
    unsigned int baseInstance;


    bool operator==(const DrawElementsIndirectCommand &other) const
    {
        return count == other.count && instanceCount == other.instanceCount && firstIndex == other.firstIndex &&
               baseVertex == other.baseVertex && baseInstance == other.baseInstance;
    }
};


struct InstanceData
{
    glm::mat4 worldMat;
    uint32_t materialID;
    uint32_t modelMatID;
    uint32_t padding[2];
};

struct DrawRange
{
    size_t startIndex;
    size_t count;
    ShaderComboID shader;
};

}; // namespace eHazGraphics



#endif
