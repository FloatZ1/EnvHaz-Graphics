#ifndef BUFFER_MANAGER_HPP
#define BUFFER_MANAGER_HPP


#include <DataStructs.hpp>
#include <cstddef>
#include <glad/glad.h>
#include <iterator>


#include "DataStructs.hpp"

namespace eHazGraphics
{



class StaticBuffer
{
  public:
    StaticBuffer(size_t initialVertexBufferSize, size_t initialIndexBufferSize);



    void ResizeBuffer();



    void InsertIntoBuffer(const Vertex *vertexData, size_t vertexDataSize, const GLuint *indexData,
                          size_t indexDataSize);


    void ClearBuffer();

    void BindBuffer();




  private:
    GLuint VertexBufferID;
    GLuint VertexArrayID;
    GLuint IndexBufferID;

    size_t VertexBufferSize = 0;
    size_t IndexBufferSize = 0;



    size_t VertexSizeOccupied = 0;
    size_t IndexSizeOccupied = 0;
    int numOfOccupiedVerts = 0;
    // Sets the vertex attributes to the currently bound VAO
    void setVertexAttribPointers();
};


struct BufferRange
{

    int slot;
    size_t size;
    size_t offset;
};

class DynamicBuffer
{


    DynamicBuffer(size_t initialBuffersSize);


    void SetSlot(int slot);

    void SetBinding(int bindingNum);

    void ReCreateBuffer();

    void ReCreateBuffer(size_t minimumSize);


    BufferRange BeginWritting();


    BufferRange InsertNewData(void *data, size_t size);

    void UpdateOldData(BufferRange range, void *data, size_t size);







    void EndWritting();




  private:
    size_t slotFullSize[3]{0, 0, 0};
    size_t slotOccupiedSize[3]{0, 0, 0};
    // create the initial 3 arrays
    GLuint BufferSlots[3]{0, 0, 0};
    void *slots[3]{nullptr, nullptr, nullptr};
    int binding = 0;
    int currentSlot = 0;
    // add 3 fence variables + a function to initiate it
    GLsync fences[3];
    bool shouldResize[3]{false, false, false};

    uint64_t slotTimeline = 0;
    uint64_t slotsAge[3]{0, 0, 0};

    void SetDownFence();
    void MapAllBufferSlots();
    bool waitForSlotFence(int slot);
};






class BufferManager
{
};






} // namespace eHazGraphics









#endif
