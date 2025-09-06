#include "BufferManager.hpp"
#include "DataStructs.hpp"
#include <cassert>
#include <cstddef>

#include <vector>


namespace eHazGraphics
{


StaticBuffer::StaticBuffer(size_t initialVertexBufferSize, size_t initialIndexBufferSize)
    : VertexBufferSize(initialVertexBufferSize), IndexBufferSize(initialIndexBufferSize)
{

    // Generate the needed Buffers
    glGenBuffers(1, &VertexBufferID);
    glGenBuffers(1, &IndexBufferID);
    glGenVertexArrays(1, &VertexArrayID);

    // Bind them

    glBindVertexArray(VertexArrayID);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferID);


    // allocate the initial data for the vertex and index buffers
    glBufferData(GL_ARRAY_BUFFER, initialVertexBufferSize, nullptr, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, initialIndexBufferSize, nullptr, GL_STATIC_DRAW);

    // set the offsets
    setVertexAttribPointers();

    // enable the attributes
}



void StaticBuffer::setVertexAttribPointers()
{



    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Postion));

    glEnableVertexAttribArray(0);


    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, UV));

    glEnableVertexAttribArray(1);


    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Normal));

    glEnableVertexAttribArray(2);
}










void StaticBuffer::ResizeBuffer()
{
    GLuint newBuffer, newIndexBuffer;
    glGenBuffers(1, &newBuffer);
    glGenBuffers(1, &newIndexBuffer);


    // bind the buffer to their corresponding targets
    // glBindVertexArray(VertexArrayID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newIndexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, newBuffer);
    // calculate their new size
    //
    //

    assert(VertexSizeOccupied <= VertexBufferSize);
    assert(IndexSizeOccupied <= IndexBufferSize);

    size_t newVertSize = std::max(1024UL, 2 * VertexBufferSize);
    VertexBufferSize = newVertSize;

    size_t newIndexSize = std::max(1024UL, 2 * VertexBufferSize);
    IndexBufferSize = newIndexSize;
    // allocate the new buffers with their new size
    glBufferData(GL_ARRAY_BUFFER, newVertSize, nullptr, GL_STATIC_DRAW);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, newIndexSize, nullptr, GL_STATIC_DRAW);
    // Now we move over the data to the new buffers


    // first the vertex buffer

    glBindBuffer(GL_COPY_READ_BUFFER, VertexBufferID);
    glBindBuffer(GL_COPY_WRITE_BUFFER, newBuffer);

    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, VertexSizeOccupied);
    // then the IndexBuffer


    glBindBuffer(GL_COPY_READ_BUFFER, IndexBufferID);
    glBindBuffer(GL_COPY_WRITE_BUFFER, newIndexBuffer);

    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, IndexSizeOccupied);



    glDeleteBuffers(1, &VertexBufferID);
    glDeleteBuffers(1, &IndexBufferID);


    // the attributes, should switch to vertex attribute format, but later.





    VertexBufferID = newBuffer;
    IndexBufferID = newIndexBuffer;
    glBindVertexArray(VertexArrayID);

    glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferID);
    setVertexAttribPointers();



    // also should remove the binds to GL_COPY_READ_BUFFER and write buffer, and just bind the read buffer to write out
    // to the proper binding to remove extra binds.
}

void StaticBuffer::InsertIntoBuffer(const Vertex *vertexData, size_t vertexDataSize, const GLuint *indexData,
                                    size_t indexDataSize)
{

    // bind->set->bind->set
    bool fits = false;
    while (!fits)
    {
        if (vertexDataSize + VertexSizeOccupied >= VertexBufferSize ||
            indexDataSize + IndexSizeOccupied >= IndexBufferSize)
        {
            ResizeBuffer();
        }
        else
        {
            fits = true;
        }
    }
    assert(vertexDataSize + VertexSizeOccupied <= VertexBufferSize &&
           indexDataSize + IndexSizeOccupied <= IndexBufferSize);


    glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
    glBufferSubData(GL_ARRAY_BUFFER, VertexSizeOccupied, vertexDataSize, vertexData);
    VertexSizeOccupied += vertexDataSize;
    numOfOccupiedVerts = VertexSizeOccupied / sizeof(Vertex);





    // process the indecies since they need to be appended by the count of vertices in store


    // at some point remove the std::vector here and just send in raw data, problem is that i want to keep the pointers
    // const in order to have them pre-loaded and inserted when needed.
    std::vector<GLuint> processedIndecies(indexDataSize / sizeof(GLuint));

    size_t vertexOffset = (VertexSizeOccupied - vertexDataSize) / sizeof(Vertex);
    for (int i = 0; i < (indexDataSize / sizeof(GLuint)); i++)
    {

        processedIndecies[i] = (*(indexData + i) + vertexOffset);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferID);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, IndexSizeOccupied, processedIndecies.size() * sizeof(GLuint),
                    processedIndecies.data());

    IndexSizeOccupied += processedIndecies.size() * sizeof(GLuint);
}

void StaticBuffer::ClearBuffer()
{
    VertexSizeOccupied = 0;
    IndexSizeOccupied = 0;
    numOfOccupiedVerts = 0;
}

void StaticBuffer::BindBuffer()
{

    glBindVertexArray(VertexArrayID);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferID);
}


//----------------STATIC BUFFER IMPLEMENTATION END------------------------------------\\









} // namespace eHazGraphics
