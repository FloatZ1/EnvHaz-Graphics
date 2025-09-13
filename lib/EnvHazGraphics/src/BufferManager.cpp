#include "BufferManager.hpp"
#include "DataStructs.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>

#include <cstring>
#include <iterator>
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


DynamicBuffer::DynamicBuffer(size_t initialBuffersSize)
{

    slotFullSize[0] = initialBuffersSize;
    slotFullSize[1] = initialBuffersSize;
    slotFullSize[2] = initialBuffersSize;
    glCreateBuffers(1, &BufferSlots[0]);
    glCreateBuffers(1, &BufferSlots[1]);
    glCreateBuffers(1, &BufferSlots[2]);

    glNamedBufferStorage(BufferSlots[0], initialBuffersSize, nullptr,
                         GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);

    glNamedBufferStorage(BufferSlots[1], initialBuffersSize, nullptr,
                         GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);
    glNamedBufferStorage(BufferSlots[2], initialBuffersSize, nullptr,
                         GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);



    MapAllBufferSlots();
}








void DynamicBuffer::SetSlot(int slot)
{
    currentSlot = slot;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, BufferSlots[slot]);
}
void DynamicBuffer::SetBinding(int bindingNum)
{
    binding = bindingNum;
}



// buffer resize logic:



void DynamicBuffer::ReCreateBuffer()
{
    bool any = false;
    for (auto check : shouldResize)
    {
        if (check == true)
        {
            any = true;
            break;
        }
    }


    if (any)
    {
        GLuint newBuffers[3] = {0, 0, 0};






        if (shouldResize[0])
            glCreateBuffers(1, &newBuffers[0]);
        if (shouldResize[1])
            glCreateBuffers(1, &newBuffers[1]);
        if (shouldResize[2])
            glCreateBuffers(1, &newBuffers[2]);



        for (int i = 0; i < 3; i++)
        {
            if (shouldResize[i])
                slotFullSize[i] = std::max(slotFullSize[i] * 2, 1024UL);
        }

        if (shouldResize[0])
            glNamedBufferStorage(newBuffers[0], slotFullSize[0], nullptr,
                                 GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT |
                                     GL_DYNAMIC_STORAGE_BIT);
        if (shouldResize[1])
            glNamedBufferStorage(newBuffers[1], slotFullSize[1], nullptr,
                                 GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT |
                                     GL_DYNAMIC_STORAGE_BIT);
        if (shouldResize[2])
            glNamedBufferStorage(newBuffers[2], slotFullSize[2], nullptr,
                                 GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT |
                                     GL_DYNAMIC_STORAGE_BIT);

        for (int i = 0; i < 3; i++)
        {
            if (shouldResize[i])
                glCopyNamedBufferSubData(BufferSlots[i], newBuffers[i], 0, 0, slotOccupiedSize[i]);
        }


        if (shouldResize[0])
        {
            if (!waitForSlotFence(0))
            {
                glClientWaitSync(fences[0], GL_SYNC_FLUSH_COMMANDS_BIT, GLuint64(1e9));
            }
            glUnmapNamedBuffer(BufferSlots[0]);
            glDeleteBuffers(1, &BufferSlots[0]);
        }

        if (shouldResize[1])
        {
            if (!waitForSlotFence(1))
            {
                glClientWaitSync(fences[1], GL_SYNC_FLUSH_COMMANDS_BIT, GLuint64(1e9));
            }
            glUnmapNamedBuffer(BufferSlots[1]);
            glDeleteBuffers(1, &BufferSlots[1]);
        }

        if (shouldResize[2])
        {

            if (!waitForSlotFence(2))
            {
                glClientWaitSync(fences[2], GL_SYNC_FLUSH_COMMANDS_BIT, GLuint64(1e9));
            }



            glUnmapNamedBuffer(BufferSlots[2]);

            glDeleteBuffers(1, &BufferSlots[2]);
        }
        for (int i = 0; i < 3; i++)
        {
            if (shouldResize[i])
                BufferSlots[i] = newBuffers[i];
        }
        MapAllBufferSlots();
        for (int i = 0; i < 3; i++)
        {
            shouldResize[i] = false;
        }
    }
}



void DynamicBuffer::ReCreateBuffer(size_t minimumSize)
{

    bool any = false;
    for (auto check : shouldResize)
    {
        if (check == true)
            any = true;
        break;
    }


    if (any)
    {



        GLuint newBuffers[3] = {0, 0, 0};

        // yandere dev Ahhhh code
        //
        //
        //


        if (shouldResize[0])
            glCreateBuffers(1, &newBuffers[0]);
        if (shouldResize[1])
            glCreateBuffers(1, &newBuffers[1]);
        if (shouldResize[2])
            glCreateBuffers(1, &newBuffers[2]);



        for (int i = 0; i < 3; i++)
        {
            if (shouldResize[i])
                slotFullSize[i] = std::max(slotFullSize[i] * 2, minimumSize);
        }

        if (shouldResize[0])
            glNamedBufferStorage(newBuffers[0], slotFullSize[0], nullptr,
                                 GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT |
                                     GL_DYNAMIC_STORAGE_BIT);
        if (shouldResize[1])
            glNamedBufferStorage(newBuffers[1], slotFullSize[1], nullptr,
                                 GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT |
                                     GL_DYNAMIC_STORAGE_BIT);
        if (shouldResize[2])
            glNamedBufferStorage(newBuffers[2], slotFullSize[2], nullptr,
                                 GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT |
                                     GL_DYNAMIC_STORAGE_BIT);

        for (int i = 0; i < 3; i++)
        {
            if (shouldResize[i])
                glCopyNamedBufferSubData(BufferSlots[i], newBuffers[i], 0, 0, slotOccupiedSize[i]);
        }



        if (shouldResize[0])
        {
            if (!waitForSlotFence(0))
            {
                glClientWaitSync(fences[0], GL_SYNC_FLUSH_COMMANDS_BIT, GLuint64(1e9));
            }
            glUnmapNamedBuffer(BufferSlots[0]);
            glDeleteBuffers(1, &BufferSlots[0]);
        }

        if (shouldResize[1])
        {
            if (!waitForSlotFence(1))
            {
                glClientWaitSync(fences[1], GL_SYNC_FLUSH_COMMANDS_BIT, GLuint64(1e9));
            }
            glUnmapNamedBuffer(BufferSlots[1]);
            glDeleteBuffers(1, &BufferSlots[1]);
        }

        if (shouldResize[2])
        {

            if (!waitForSlotFence(2))
            {
                glClientWaitSync(fences[2], GL_SYNC_FLUSH_COMMANDS_BIT, GLuint64(1e9));
            }



            glUnmapNamedBuffer(BufferSlots[2]);

            glDeleteBuffers(1, &BufferSlots[2]);
        }


        for (int i = 0; i < 3; i++)
        {
            if (shouldResize[i])
                BufferSlots[i] = newBuffers[i];
        }
        MapAllBufferSlots();

        for (int i = 0; i < 3; i++)
        {
            shouldResize[i] = false;
        }
    }
}












BufferRange DynamicBuffer::BeginWritting()
{



    ReCreateBuffer();
    for (int i = 0; i < 3; i++)
    {
        int next = (currentSlot + i + 1) % 3;
        if (waitForSlotFence(next))
        {
            currentSlot = next;

            slotOccupiedSize[next] = 0;

            return {next, slotFullSize[next], 0};
        }
    }


    auto max = (std::max_element(slotsAge, (slotsAge + (sizeof(slotsAge) / sizeof(slotsAge[0])))));
    int oldest = std::distance(slotsAge, max);
    glClientWaitSync(fences[oldest], GL_SYNC_FLUSH_COMMANDS_BIT, GLuint64(1e9));
    glDeleteSync(fences[oldest]);
    slotsAge[oldest] = 0;
    fences[oldest] = 0;
    currentSlot = oldest;
    slotOccupiedSize[oldest] = 0;
    return {currentSlot, slotFullSize[currentSlot], 0};
}








BufferRange DynamicBuffer::InsertNewData(void *data, size_t size)
{
    int fittingSlot = -1;
    int slotsToCheck[3]{-1, -1, -1};
    while (fittingSlot == -1)
    {
        for (int i = 0; i < 3; i++)
        {

            if (slotOccupiedSize[i] + size > slotFullSize[i])
            {
                shouldResize[i] = true;
            }
            else
            {
                if (waitForSlotFence(i))
                {
                    fittingSlot = i;
                    break;
                }
                else
                {
                    slotsToCheck[i] = i;
                }
            }
        }
        for (auto &isEmpty : slotsToCheck)
        {
            if (isEmpty == -1)
            {
            }
            else
            {
                fittingSlot = 3; // set to an out of reach value so that we know there are abailable slots, just that
                                 // their fences are still up;
                break;
            }
        }
        if (fittingSlot == -1)
            ReCreateBuffer(size * 2);
    }


    // if we didnt get a free and fitting slot, we check for the oldest of fences


    auto max = (std::max_element(slotsAge, (slotsAge + (sizeof(slotsAge) / sizeof(slotsAge[0])))));
    int oldest = std::distance(slotsAge, max);

    if (oldest ==
            (*(std::find(slotsToCheck, (slotsToCheck + (sizeof(slotsToCheck) / sizeof(slotsToCheck[0]))), oldest))) &&
        fittingSlot == 3)
    {

        if (!waitForSlotFence(oldest))
        {
            glClientWaitSync(fences[oldest], GL_SYNC_FLUSH_COMMANDS_BIT, GLuint64(1e9));
            fittingSlot = oldest;
        }
        else
            fittingSlot = oldest;
    }





    /*  for (auto &slots : slotsToCheck)
      {

          if (slots != -1)

              else if (waitForSlotFence(slots))
              {
                  fittingSlot = slots;
              }
      }*/



    assert(fittingSlot >= 0 && fittingSlot < 3);

    void *writeLocation = static_cast<void *>((char *)slots[fittingSlot] + slotOccupiedSize[fittingSlot]);

    memcpy(writeLocation, data, size);
    slotOccupiedSize[fittingSlot] += size;
    currentSlot = fittingSlot;
    return {fittingSlot, size, slotOccupiedSize[fittingSlot] - size};
}











void DynamicBuffer::UpdateOldData(BufferRange range, void *data, size_t size)
{
    void *writeLocation = static_cast<void *>((char *)slots[range.slot] + range.offset);
    assert(range.size == size);
    memcpy(writeLocation, data, range.size);
}



void DynamicBuffer::EndWritting()
{

    SetDownFence();
}

// privates:

bool DynamicBuffer::waitForSlotFence(int slot)
{

    if (fences[slot] == 0)
        return true;


    GLenum res = glClientWaitSync(fences[slot], 0, 0);

    if (res == GL_CONDITION_SATISFIED || res == GL_ALREADY_SIGNALED)
    {
        glDeleteSync(fences[slot]);
        fences[slot] = 0;

        return true;
    }




    return false;
}
void DynamicBuffer::MapAllBufferSlots()
{

    for (int i = 0; i < 3; i++)
    {

        slots[i] = glMapNamedBufferRange(BufferSlots[i], 0, slotFullSize[i],
                                         GL_MAP_COHERENT_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_WRITE_BIT |
                                             GL_MAP_PERSISTENT_BIT);
    }
}





void DynamicBuffer::SetDownFence()
{
    if (fences[currentSlot])
    {
        glDeleteSync(fences[currentSlot]);
    }
    fences[currentSlot] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    slotsAge[currentSlot] = slotTimeline;
    slotTimeline++;
}




} // namespace eHazGraphics
