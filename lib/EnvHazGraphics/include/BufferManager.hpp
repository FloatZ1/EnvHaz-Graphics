#ifndef BUFFER_MANAGER_HPP
#define BUFFER_MANAGER_HPP


#include "BitFlags.hpp"
#include <DataStructs.hpp>
#include <SDL3/SDL_log.h>
#include <cstddef>
#include <glad/glad.h>
#include <memory>
#include <unordered_map>
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
        Destroy();
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

    DynamicBuffer(size_t initialBuffersSize, int DynamicBufferID, GLenum target = GL_SHADER_STORAGE_BUFFER);


    virtual void SetSlot(int slot);

    void SetBinding(int bindingNum);

    void ReCreateBuffer();

    void ReCreateBuffer(size_t minimumSize);


    void ClearBuffer()
    {

        slotOccupiedSize[0] = 0;
        slotOccupiedSize[1] = 0;
        slotOccupiedSize[2] = 0;
    }

    BufferRange BeginWritting();


    BufferRange InsertNewData(const void *data, size_t size);

    void UpdateOldData(BufferRange range, const void *data, size_t size);


    int GetDynamicBufferID() const
    {
        return DynamicBufferID;
    }


    uint GetCurrentSlot()
    {
        return currentSlot;
    }

    void EndWritting();

    void Destroy();


    ~DynamicBuffer()
    {
        Destroy();
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
    // add 3 fence variables + a function to initiate it
    GLsync fences[3]{0, 0, 0};
    bool shouldResize[3]{false, false, false};

    uint64_t slotTimeline = 0;
    uint64_t slotsAge[3]{0, 0, 0};

    void SetDownFence();
    void MapAllBufferSlots();
    bool waitForSlotFence(int slot);
};






class BufferManager
{
    // ok so im thinking of having a vector/list full of the meshes,
    // the meshes contain a vector of vertices & indecies and also shader ID
    // to which shader they use,
    // then in the renderer with the fucnions for submitting static/dynamic meshes
    // we route them accordingly ofcourse deletion of particular data is kinda
    // dificult due to fragmentation, so we will keep the mesh data on cpu side
    // as well, i dont think it will be that much of an overhead unless it has like
    // a bagilion triangles lmao 0-0l, so anyways
    //
    // im thinking ill go for a double aproach,
    // have the unordered list to get the BufferRange of each mesh, and then
    // use a vector to sort them based on the shader usage to minimize binding.
    // for updating same stuff, tho im wondering if a dynamic mesh can increase its
    // vertices count, if so damn. buut that probably will be fine
    //
    // ok so we also have a FinishWriting function here as well in which we will store the EndWritting()
    // of all the dynamic buffers
    // and also a BeginWritting();
    // oh we also sort what goes where with a typeof(), we will have a switch for
    // dat, maybe even some bitflags? depends really, ok we can make it so that
    // if there arent any specified flags we sort it, but if there are, we go with that instead yeah.
    //
    // then the binding comes, it will have to be done at the beggining of each frame? unless
    // there wont be any need for that if we are already bound from the previous frame ig.
    // depends on the insertion logic
    //

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



    void Destroy();

  private:
    DynamicBuffer InstanceData;
    DynamicBuffer AnimationMatrices;
    DynamicBuffer TextureHandleBuffer;
    DynamicBuffer ParticleData;
    DynamicBuffer DrawCommandBuffer;
    DynamicBuffer cameraMatrices;
    DynamicBuffer LightsBuffer;


    StaticBuffer StaticMeshInformation;
    StaticBuffer TerrainBuffer;

    // Every time a buffer is added update the following functions:
    // Initialize(), InsertNew*Data() , ClearBuffer() and BitFlags



    // CHANGE THESE EVERYTIME YOU ADD A BUFFER!!!!!!!!!
    unsigned int numOfDynamicBuffers = 7;
    unsigned int numofStaticBuffers = 2;


    std::vector<std::unique_ptr<StaticBuffer>> StaticbufferIDs;
    std::vector<std::unique_ptr<DynamicBuffer>> DynamicBufferIDs;


    // std::unordered_map<MeshID, >
};






} // namespace eHazGraphics









#endif
