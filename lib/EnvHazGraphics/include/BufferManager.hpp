#ifndef BUFFER_MANAGER_HPP
#define BUFFER_MANAGER_HPP


#include "BitFlags.hpp"
#include <DataStructs.hpp>
#include <SDL3/SDL_log.h>
#include <cstddef>
#include <glad/glad.h>

#include <vector>


#include "DataStructs.hpp"

namespace eHazGraphics
{










class StaticBuffer
{
  public:
    StaticBuffer();
    StaticBuffer(size_t initialVertexBufferSize, size_t initialIndexBufferSize, int StaticBufferID);



    void ResizeBuffer();


    // inserts data into the vertex and index buffer and returns the buffer ranges for them in the order of
    // <VertexRange, IndexRange>
    VertexIndexInfoPair InsertIntoBuffer(const Vertex *vertexData, size_t vertexDataSize, const GLuint *indexData,
                                         size_t indexDataSize);


    void ClearBuffer();

    void BindBuffer();

    int GetStaticBufferID() const
    {
        return StaticBufferID;
    }

    // Can be called manually or wait for the object to be destroyed
    void Destroy();


    ~StaticBuffer()
    {
        // for some reasong the destructor is called right after the object is initialized, so for now its commented out
        //  Destroy();
    }

  private:
    GLuint VertexBufferID;
    GLuint VertexArrayID;
    GLuint IndexBufferID;

    size_t VertexBufferSize = 0;
    size_t IndexBufferSize = 0;

    int StaticBufferID = 0;




    size_t VertexSizeOccupied = 0;
    size_t IndexSizeOccupied = 0;
    int numOfOccupiedVerts = 0;
    int numOfOccupiedIndecies = 0;
    // Sets the vertex attributes to the currently bound VAO
    void setVertexAttribPointers();
};





class DynamicBuffer
{

  public:
    DynamicBuffer();

    DynamicBuffer(size_t initialBuffersSize, int DynamicBufferID, GLenum target = GL_SHADER_STORAGE_BUFFER,
                  bool trippleBuffer = true);


    void SetSlot(int slot);

    void SetBinding(int bindingNum);



    void ReCreateBuffer(size_t minimumSize = 1024UL);


    void ClearBuffer()
    {

        slotOccupiedSize[0] = 0;
        slotOccupiedSize[1] = 0;
        slotOccupiedSize[2] = 0;
    }

    BufferRange BeginWritting();


    BufferRange InsertNewData(const void *data, size_t size, TypeFlags type);

    void UpdateOldData(BufferRange range, const void *data, size_t size);


    int GetDynamicBufferID() const
    {
        return DynamicBufferID;
    }
    int GetNextSlot() const
    {
        return nextSlot;
    }

    uint GetCurrentSlot()
    {
        return currentSlot;
    }

    void EndWritting();

    void Destroy();


    ~DynamicBuffer()
    {
        // Destroy();
    }

  private:
    GLenum Target = GL_SHADER_STORAGE_BUFFER;
    int DynamicBufferID = 0;
    size_t slotFullSize[3]{0, 0, 0};
    size_t slotOccupiedSize[3]{0, 0, 0};
    // create the initial 3 arrays
    GLuint BufferSlots[3]{0, 0, 0};
    void *slots[3]{nullptr, nullptr, nullptr};
    int binding = 0;
    int currentSlot = 0;
    int nextSlot = 0;
    // add 3 fence variables + a function to initiate it
    GLsync fences[3]{0, 0, 0};
    bool shouldResize[3]{false, false, false};


    bool trippleBuffer = true;

    uint64_t slotTimeline = 0;
    uint64_t slotsAge[3]{0, 0, 0};

    void SetDownFence(int slot);
    void MapAllBufferSlots();
    bool waitForSlotFence(int slot);
};






class BufferManager
{

  public:
    BufferManager();

    BufferManager(BufferManager &&other) noexcept
        : InstanceData(std::move(other.InstanceData)), AnimationMatrices(std::move(other.AnimationMatrices)),
          TextureHandleBuffer(std::move(other.TextureHandleBuffer)), ParticleData(std::move(other.ParticleData)),
          DrawCommandBuffer(std::move(other.DrawCommandBuffer)),
          StaticMeshInformation(std::move(other.StaticMeshInformation)), TerrainBuffer(std::move(other.TerrainBuffer)),
          DynamicBufferIDs(std::move(other.DynamicBufferIDs)), StaticbufferIDs(std::move(other.StaticbufferIDs))
    {
    }





    void Initialize(); // NOTE: MAKE INITIAL SIZE VARY DEPENDING ON LAST SESSIONS MOST MEMORY USED IN EACH

    void BeginWritting();

    void BindDynamicBuffer(TypeFlags type);

    VertexIndexInfoPair InsertNewStaticData(const Vertex *vertexData, size_t vertexDataSize, const GLuint *indexData,
                                            size_t indexDataSize, TypeFlags type);

    BufferRange InsertNewDynamicData(const void *data, size_t size, TypeFlags type);

    void ClearBuffer(TypeFlags whichBuffer);

    void BindStaticBuffer(TypeFlags buffer)
    {

        switch (buffer)
        {
        case TypeFlags::BUFFER_STATIC_MESH_DATA:
            StaticMeshInformation.BindBuffer();
            break;
        case TypeFlags::BUFFER_STATIC_TERRAIN_DATA:
            TerrainBuffer.BindBuffer();
            break;
        default:
            SDL_Log("Failed to bind static buffer, unknown TypeFlag given to BindStaticBuffer()\n");
        }
    }


    void UpdateData(const BufferRange &range, const void *data, const size_t size);


    void EndWritting();

    void UpdateManager();

    void Destroy();

  private:
    DynamicBuffer InstanceData;
    DynamicBuffer AnimationMatrices;
    DynamicBuffer TextureHandleBuffer;
    DynamicBuffer ParticleData;
    DynamicBuffer DrawCommandBuffer;
    DynamicBuffer cameraMatrices;
    DynamicBuffer LightsBuffer;
    DynamicBuffer StaticMatrices;

    StaticBuffer StaticMeshInformation;
    StaticBuffer TerrainBuffer;
    // StaticBuffer StaticMatrices;
    //  Every time a buffer is added update the following functions:
    //  Initialize(), InsertNew*Data() , ClearBuffer() and BitFlags



    // CHANGE THESE EVERYTIME YOU ADD A BUFFER!!!!!!!!!
    unsigned int numOfDynamicBuffers = 8;
    unsigned int numofStaticBuffers = 2;


    std::vector<StaticBuffer *> StaticbufferIDs;
    std::vector<DynamicBuffer *> DynamicBufferIDs;


    // std::unordered_map<MeshID, >
};






} // namespace eHazGraphics









#endif
