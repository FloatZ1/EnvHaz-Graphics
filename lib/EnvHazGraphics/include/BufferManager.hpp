#ifndef BUFFER_MANAGER_HPP
#define BUFFER_MANAGER_HPP


#include <DataStructs.hpp>
#include <cstddef>
#include <glad/glad.h>


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




class DynamicBuffer
{
};






class BufferManager
{
};






} // namespace eHazGraphics









#endif
