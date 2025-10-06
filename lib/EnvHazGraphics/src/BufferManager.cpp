#include "BufferManager.hpp"
#include "BitFlags.hpp"
#include "DataStructs.hpp"
#include "glad/glad.h"
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_stdinc.h>
#include <algorithm>
#include <cassert>

#include <cstddef>

#include <cstring>
#include <iterator>

#include <utility>
#include <vector>


namespace eHazGraphics
{

StaticBuffer::StaticBuffer()
{
    // initialize members if needed
}

DynamicBuffer::DynamicBuffer()
{
    // initialize members if needed
}




StaticBuffer::StaticBuffer(size_t initialVertexBufferSize, size_t initialIndexBufferSize, int StaticBufferID)
    : VertexBufferSize(initialVertexBufferSize), IndexBufferSize(initialIndexBufferSize), StaticBufferID(StaticBufferID)
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

    // enable the attributes'
}

void StaticBuffer::Destroy()
{
    SDL_Log("\n\n\nSTATIC BUFFER DESTROYED \n\n\n");
    glDeleteBuffers(1, &VertexBufferID);
    glDeleteBuffers(1, &IndexBufferID);
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

VertexIndexInfoPair StaticBuffer::InsertIntoBuffer(const Vertex *vertexData, size_t vertexDataSize,
                                                   const GLuint *indexData, size_t indexDataSize)
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


    if (glIsBuffer(VertexBufferID))
    {
        SDL_Log("STATIC VERTEX BUFFER EXISTS");
    }
    if (glIsBuffer(IndexBufferID))
    {
        SDL_Log("INDEX BUFFER EXISTS");
    }




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

        processedIndecies[i] = *(indexData + i); //(*(indexData + i) + vertexOffset); //remove the index pre-processing
                                                 // since we use the command struct for that
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferID);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, IndexSizeOccupied, processedIndecies.size() * sizeof(GLuint),
                    processedIndecies.data());

    IndexSizeOccupied += processedIndecies.size() * sizeof(GLuint);
    numOfOccupiedIndecies = IndexSizeOccupied / sizeof(typeof(IndexSizeOccupied));
    GLint IndexOffset = (IndexSizeOccupied - indexDataSize) / sizeof(GLuint);


    BufferRange vertexRange;
    BufferRange indexRange;

    vertexRange.OwningBuffer = StaticBufferID;
    vertexRange.offset = vertexOffset;
    vertexRange.size = vertexDataSize;
    vertexRange.slot = VertexBufferID;
    vertexRange.count = vertexDataSize / sizeof(GLuint);


    indexRange.OwningBuffer = StaticBufferID;
    indexRange.offset = IndexOffset;
    indexRange.size = indexDataSize;
    indexRange.count = indexDataSize / sizeof(GLuint);
    indexRange.slot = IndexBufferID;

    return std::pair<BufferRange, BufferRange>(vertexRange, indexRange);
}

void StaticBuffer::ClearBuffer()
{
    VertexSizeOccupied = 0;
    IndexSizeOccupied = 0;
    numOfOccupiedVerts = 0;
}

void StaticBuffer::BindBuffer()
{
    if (glIsBuffer(VertexBufferID))
    {
        SDL_Log("STATIC VERTEX BUFFER EXISTS");
    }
    if (glIsBuffer(IndexBufferID))
    {
        SDL_Log("INDEX BUFFER EXISTS");
    }
    if (glIsBuffer(VertexArrayID))
    {
        SDL_Log("VERTEX ARRAY BUFFER EXISTS");
    }

    glBindVertexArray(VertexArrayID);
    // glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferID);
}


//----------------STATIC BUFFER IMPLEMENTATION END------------------------------------\\



void BufferManager::BindDynamicBuffer(TypeFlags type)
{
    DynamicBuffer *buffer = nullptr;

    switch (type)
    {
    case TypeFlags::BUFFER_DRAW_CALL_DATA:
        buffer = &DrawCommandBuffer;
        break;
    case TypeFlags::BUFFER_INSTANCE_DATA:
        buffer = &InstanceData;
        break;
    case TypeFlags::BUFFER_ANIMATION_DATA:
        buffer = &AnimationMatrices;
        break;
    case TypeFlags::BUFFER_PARTICLE_DATA:
        buffer = &ParticleData;
        break;
    case TypeFlags::BUFFER_TEXTURE_DATA:
        buffer = &TextureHandleBuffer;
        break;
    case TypeFlags::BUFFER_CAMERA_DATA:
        buffer = &cameraMatrices;
        break;
    case TypeFlags::BUFFER_LIGHT_DATA:
        buffer = &LightsBuffer;
        break;
    default:
        SDL_Log("BindDynamicBuffer: Unknown buffer type %d\n", type);
        return;
    }

    // Manager decides which slot to use
    buffer->SetSlot(buffer->GetCurrentSlot());
}






DynamicBuffer::DynamicBuffer(size_t initialBuffersSize, int DynamicBufferID, GLenum target)
    : DynamicBufferID(DynamicBufferID), Target(target)
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


    GLint result = 0;
    glGetNamedBufferParameteriv(BufferSlots[0], GL_BUFFER_SIZE, &result);
    SDL_Log("Buffer %u size = %d", BufferSlots[0], result);

    GLint result1 = 0;
    glGetNamedBufferParameteriv(BufferSlots[1], GL_BUFFER_SIZE, &result1);
    SDL_Log("Buffer %u size = %d", BufferSlots[1], result1);


    GLint result2 = 0;
    glGetNamedBufferParameteriv(BufferSlots[2], GL_BUFFER_SIZE, &result2);
    SDL_Log("Buffer %u size = %d", BufferSlots[2], result2);
}



void DynamicBuffer::Destroy()
{

    for (int i = 0; i < 3; ++i)
    {
        if (slots[i])
        {
            // unmap if mapped
            glUnmapNamedBuffer(BufferSlots[i]);
            slots[i] = nullptr;
        }
        if (fences[i])
        {
            glDeleteSync(fences[i]);
            fences[i] = 0;
        }
        if (glIsBuffer(BufferSlots[i]))
        {
            glDeleteBuffers(1, &BufferSlots[i]);
        }
        BufferSlots[i] = 0;
    }
}




/*void DynamicBuffer::SetSlot(int slot)
{
    currentSlot = slot;

    if (!glIsBuffer(BufferSlots[slot]))
    {
        SDL_Log("SetSlot(): BufferSlots[%d]=%u is not a valid buffer", slot, BufferSlots[slot]);
        return;
    }

    GLint maxBindings = 0;
    glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &maxBindings);
    if (binding < 0 || binding >= maxBindings)
    {
        SDL_Log("SetSlot(): binding index %d out of range (max=%d)", binding, maxBindings);
        return;
    }

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, BufferSlots[slot]);
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        SDL_Log("SetSlot(): glBindBufferBase failed, err=0x%X", err);
    }
} */


void DynamicBuffer::SetSlot(int slot)
{
    currentSlot = slot;

    if (!glIsBuffer(BufferSlots[slot]))
    {
        SDL_Log("SetSlot(): BufferSlots[%d]=%u is not a valid buffer", slot, BufferSlots[slot]);
        return;
    }

    switch (Target)
    {
    case GL_SHADER_STORAGE_BUFFER:
    case GL_UNIFORM_BUFFER:
    case GL_ATOMIC_COUNTER_BUFFER:
    case GL_TRANSFORM_FEEDBACK_BUFFER:
        // Indexed binding targets
        glBindBufferBase(Target, binding, BufferSlots[slot]);
        break;

    case GL_DRAW_INDIRECT_BUFFER:
    case GL_ARRAY_BUFFER:
    case GL_ELEMENT_ARRAY_BUFFER:
        // Non-indexed binding targets
        glBindBuffer(Target, BufferSlots[slot]);
        break;

    default:
        SDL_Log("SetSlot(): Unsupported Target=0x%X", Target);
        break;
    }

    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        SDL_Log("SetSlot(): bind failed for target=0x%X slot=%d buffer=%u err=0x%X", Target, slot, BufferSlots[slot],
                err);
    }
}


/*void DynamicBuffer::SetSlot(int slot)
{
    currentSlot = slot;

    glBindBufferBase(Target, binding, BufferSlots[slot]);
}*/
void DynamicBuffer::SetBinding(int bindingNum)
{
    binding = bindingNum;
}



// buffer resize logic:



/*void DynamicBuffer::ReCreateBuffer()
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
} */






















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

            return {DynamicBufferID, next, slotFullSize[next], 0};
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
    return {DynamicBufferID, currentSlot, slotFullSize[currentSlot], 0};
}





// =================== PATCHED FUNCTIONS ===================


// ---------------------- PATCH START -----------------------

/*void DynamicBuffer::ReCreateBuffer(size_t minimumSize)
{

    for (unsigned int i = 0; i < 3; i++)
    {
        // check if buffer "i" is set to be resized
        if (shouldResize[i] == false)
            continue;
        else
        {

            GLuint newBuffer = 0;


            // if yes then check first if it has a fence object down
            if (waitForSlotFence(i))
            {
                // create the new buffer and allocate it
                size_t newSize = std::max(2 * slotFullSize[i], minimumSize);
                glCreateBuffers(1, &newBuffer);
                glNamedBufferStorage(newBuffer, newSize, nullptr,
                                     GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT |
                                         GL_DYNAMIC_STORAGE_BIT);

                // check if the allocation was successfull
                GLint result = 0;
                glGetNamedBufferParameteriv(newBuffer, GL_BUFFER_SIZE, &result);
                SDL_Log("Buffer %u size = %d needed size: %zu", newBuffer, result, newSize);


                // if the result is that the new buffer has a size of 0 then we re-do this
                if (result == 0)
                {
                }
                else
                { // else we unmap the buffer first, then we copy the old data to the new buffer, then map the new
                  // buffer and check for errors

                    if (glIsBuffer(BufferSlots[i]))
                    {
                        glUnmapNamedBuffer(BufferSlots[i]);
                        slots[i] = nullptr;
                        glCopyNamedBufferSubData(BufferSlots[i], newBuffer, 0, 0, slotOccupiedSize[i]);
                    }
                    else
                    {
                        SDL_Log("ERROR: ReCreateBuffer(): BufferSlots[i] is not a buffer");
                    }





                    slots[i] = glMapNamedBufferRange(newBuffer, 0, slotFullSize[i],
                                                     GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
                    GLenum err = glGetError();
                    if (!slots[i] || err != GL_NO_ERROR)
                    {
                        SDL_Log("ReCreateBuffer()): glMapNamedBufferRange failed for BufferSlots[%d]=%u err=0x%X", i,
                                BufferSlots[i], err);
                        slots[i] = nullptr;
                    }
                    else
                    {
                        // success; caller can write into slots[i]


                        glDeleteBuffers(1, &BufferSlots[i]);

                        BufferSlots[i] = newBuffer;
                        slotFullSize[i] = newSize;
                        shouldResize[i] = false;
                    }
                }
            }
            else
            {
                continue;
            } // wait for fence {
        }
    } // for loop
}*/



void DynamicBuffer::ReCreateBuffer(size_t minimumSize)
{
    for (unsigned int i = 0; i < 3; ++i)
    {
        if (!shouldResize[i])
            continue;

        if (!waitForSlotFence(i))
            continue;

        GLuint newBuffer = 0;
        size_t newSize = std::max(2 * slotFullSize[i], minimumSize);
        if (newSize == 0)
            newSize = 1024; // sanity fallback

        glCreateBuffers(1, &newBuffer);
        glNamedBufferStorage(newBuffer, newSize, nullptr,
                             GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);

        GLint result = 0;
        glGetNamedBufferParameteriv(newBuffer, GL_BUFFER_SIZE, &result);
        SDL_Log("Buffer %u size = %d needed size: %zu", newBuffer, result, newSize);

        if (result == 0)
        {
            SDL_Log("ReCreateBuffer: allocation failed for slot %u (newBuffer=%u)", i, newBuffer);
            glDeleteBuffers(1, &newBuffer);
            continue;
        }

        // Copy old data if valid
        if (glIsBuffer(BufferSlots[i]) && slotOccupiedSize[i] > 0)
        {
            glUnmapNamedBuffer(BufferSlots[i]); // safe even if not mapped
            slots[i] = nullptr;

            glCopyNamedBufferSubData(BufferSlots[i], newBuffer, 0, 0, slotOccupiedSize[i]);
            GLenum err = glGetError();
            if (err != GL_NO_ERROR)
            {
                SDL_Log("ReCreateBuffer: copy failed slot=%u old=%u new=%u err=0x%X", i, BufferSlots[i], newBuffer,
                        err);
                glDeleteBuffers(1, &newBuffer);
                continue;
            }
        }

        // Map new buffer
        slots[i] = glMapNamedBufferRange(newBuffer, 0, newSize,
                                         GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
        GLenum err = glGetError();
        if (!slots[i] || err != GL_NO_ERROR)
        {
            SDL_Log("ReCreateBuffer: map failed newBuffer=%u slot=%u err=0x%X", newBuffer, i, err);
            glDeleteBuffers(1, &newBuffer);
            slots[i] = nullptr;
            continue;
        }

        // Success: swap in new buffer
        if (glIsBuffer(BufferSlots[i]))
            glDeleteBuffers(1, &BufferSlots[i]);

        BufferSlots[i] = newBuffer;
        slotFullSize[i] = newSize;
        shouldResize[i] = false;

        SDL_Log("ReCreateBuffer: slot %u replaced with buffer %u size=%zu (used=%zu)", i, BufferSlots[i], newSize,
                slotOccupiedSize[i]);
    }
}
















void DynamicBuffer::MapAllBufferSlots()
{
    for (unsigned int i = 0; i < 3; i++)
    {

        if (glIsBuffer(BufferSlots[i]))
        {

            slots[i] = glMapNamedBufferRange(BufferSlots[i], 0, slotFullSize[i],
                                             GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
        }
        else
        {
            shouldResize[i] = true;
        }
    }
}


BufferRange DynamicBuffer::InsertNewData(const void *data, size_t size)
{
    for (unsigned int i = 0; i < 3; i++)
    {
        if (slotFullSize[i] >= size + slotOccupiedSize[i]) // allow exact fit
        {
            if (slots[i] != nullptr)
            {
                unsigned char *location = reinterpret_cast<unsigned char *>(slots[i]) + slotOccupiedSize[i];
                SDL_memcpy(location, data, size);

                BufferRange RT;
                RT.OwningBuffer = DynamicBufferID;
                RT.slot = i;
                RT.offset = slotOccupiedSize[i];
                RT.size = size;
                RT.count = 1; // size; // or let caller decide what "count" means
                slotOccupiedSize[i] += size;

                return RT;
            }
            else
            {
                SDL_Log("InsertNewData: slots[%u] of buffer %u is not mapped, attempting recovery", i, DynamicBufferID);
                ReCreateBuffer(size);
                if (slots[i]) // retry once
                    return InsertNewData(data, size);
                continue;
            }
        }
        else
        {
            SDL_Log("InsertNewData: slot %u too small, marking resize", i);
            shouldResize[i] = true;
        }
    }

    // If we get here, no slot worked → force resize and retry
    SDL_Log("InsertNewData: no fitting slot found, forcing resize");
    ReCreateBuffer(size);
    return InsertNewData(data, size);
}

// ---------------------- PATCH END -----------------------

// =====================================END IF PATCH==============================================\\










void DynamicBuffer::UpdateOldData(BufferRange range, const void *data, size_t size)
{
    if (BufferSlots[range.slot] == 0 || !glIsBuffer(BufferSlots[range.slot]))
    {
        SDL_Log("ERROR: trying to use invalid buffer ID %u at slot %d ; called from DynamicBuffer::UpdateOldData()",
                BufferSlots[range.slot], range.slot);
        assert(false);
    }
    if (!slots[range.slot])
    {
        SDL_Log("ERROR: slots[%d] pointer is null even though buffer id %u exists ;  called from "
                "DynamicBuffer::UpdateOldData()",
                range.slot, BufferSlots[range.slot]);
        assert(false);
    }





    void *writeLocation = static_cast<void *>((char *)slots[range.slot] + range.offset);
    SDL_Log("Range size: %zu , size = %zu \n , buffer:%u slot: %u", range.size, size, range.OwningBuffer, range.slot);
    assert(range.size == size);
    SDL_Log("Updating buffer: slot=%u, offset=%zu, size=%zu, buffer=%u", range.slot, range.offset, size,
            range.OwningBuffer);
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

//-----------------------DYNAMIC BUFFER IMPLEMENTATION END-------------------------------------\\



void BufferManager::Initialize()
{
    int d_size = 10;
    int s_size = 16;


    InstanceData = DynamicBuffer(MBsize(d_size), 0);
    DrawCommandBuffer = DynamicBuffer(MBsize(d_size), 1, GL_DRAW_INDIRECT_BUFFER);
    AnimationMatrices = DynamicBuffer(MBsize(d_size), 2);
    TextureHandleBuffer = DynamicBuffer(MBsize(d_size), 3);
    ParticleData = DynamicBuffer(MBsize(d_size), 4);
    StaticMeshInformation = StaticBuffer(MBsize(s_size), MBsize(s_size), 5);
    TerrainBuffer = StaticBuffer(MBsize(s_size), MBsize(s_size), 6);
    cameraMatrices = DynamicBuffer(2 * sizeof(glm::mat4), 7);
    LightsBuffer = DynamicBuffer(MBsize(d_size), 8);


    StaticbufferIDs.push_back(&StaticMeshInformation);
    StaticbufferIDs.push_back(&TerrainBuffer);


    DynamicBufferIDs.push_back(&InstanceData);
    DynamicBufferIDs.push_back(&AnimationMatrices);
    DynamicBufferIDs.push_back(&TextureHandleBuffer);
    DynamicBufferIDs.push_back(&ParticleData);
    DynamicBufferIDs.push_back(&DrawCommandBuffer);
    DynamicBufferIDs.push_back(&cameraMatrices);
    DynamicBufferIDs.push_back(&LightsBuffer);

    InstanceData.SetBinding(0);
    DrawCommandBuffer.SetBinding(1);
    AnimationMatrices.SetBinding(2);
    TextureHandleBuffer.SetBinding(3);
    ParticleData.SetBinding(4);
    cameraMatrices.SetBinding(5);
    LightsBuffer.SetBinding(6);
}
void BufferManager::BeginWritting()
{
    for (auto &buffer : DynamicBufferIDs)
    {
        buffer->BeginWritting();
    }
}
VertexIndexInfoPair BufferManager::InsertNewStaticData(const Vertex *vertexData, size_t vertexDataSize,
                                                       const GLuint *indexData, size_t indexDataSize,
                                                       TypeFlags type = TypeFlags::BUFFER_STATIC_MESH_DATA)
{
    // for now only use the StaticMeshInformation, later implement seperation

    if (type == TypeFlags::BUFFER_STATIC_MESH_DATA)
    {
        return StaticMeshInformation.InsertIntoBuffer(vertexData, vertexDataSize, indexData, indexDataSize);
    }

    if (type == TypeFlags::BUFFER_STATIC_TERRAIN_DATA)
    { // NOTE: this is like this because i dont know how to design the RenderFrame() to account for the different draw
      // indirect commands which are mixed of course i could seperate them, but this should work for now.
      // return TerrainBuffer.InsertIntoBuffer(vertexData, vertexDataSize, indexData, indexDataSize);

        return StaticMeshInformation.InsertIntoBuffer(vertexData, vertexDataSize, indexData, indexDataSize);
    }



    return VertexIndexInfoPair();
}
BufferRange BufferManager::InsertNewDynamicData(const void *data, size_t size, TypeFlags type)
{

    if (type == TypeFlags::BUFFER_INSTANCE_DATA)
    {
        return InstanceData.InsertNewData(data, size);
    }
    if (type == TypeFlags::BUFFER_MATRIX_DATA)
    {
        // guess we dont need it if we have instance data lmao
    }
    if (type == TypeFlags::BUFFER_ANIMATION_DATA)
    {
        return AnimationMatrices.InsertNewData(data, size);
    }
    if (type == TypeFlags::BUFFER_PARTICLE_DATA)
    {
        return ParticleData.InsertNewData(data, size);
    }
    if (type == TypeFlags::BUFFER_TEXTURE_DATA)
    {
        return TextureHandleBuffer.InsertNewData(data, size);
    }
    if (type == TypeFlags::BUFFER_DRAW_CALL_DATA)
    {
        return DrawCommandBuffer.InsertNewData(data, size);
    }
    if (type == TypeFlags::BUFFER_CAMERA_DATA)
    {
        return cameraMatrices.InsertNewData(data, size);
    }
    if (type == TypeFlags::BUFFER_LIGHT_DATA)
    {
        return LightsBuffer.InsertNewData(data, size);
    }

    SDL_Log("DYNAMIC BUFFER INSERTION ERROR: COULD NOT FIND THE DESIRED TYPE!\n");
    return BufferRange();
}
void BufferManager::UpdateData(const BufferRange &range, const void *data, const size_t size)
{

    // this is a stupid ass hack what was i on when i wrote this...
    if (StaticbufferIDs.size() < range.OwningBuffer)
    {

        // nothing since it is a static buffer duhh. Ignore the code below.
        /*  for(auto& buff : StaticbufferIDs){

               if(buff.GetStaticBufferID() == range.OwningBuffer){

               }

           }*/
    }
    else
    {

        for (auto &buffer : DynamicBufferIDs)
        {
            if (buffer->GetDynamicBufferID() == range.OwningBuffer)
            {
                buffer->UpdateOldData(range, data, size);
            }
        }
    }
}
void BufferManager::EndWritting()
{
    for (auto &buffer : DynamicBufferIDs)
    {
        buffer->EndWritting();
    }
}
void BufferManager::Destroy()
{
    for (auto &buffer : DynamicBufferIDs)
    {
        buffer->Destroy();
    }
}



void BufferManager::ClearBuffer(TypeFlags whichBuffer)
{


    if (whichBuffer == TypeFlags::BUFFER_STATIC_MESH_DATA)
    {
        StaticMeshInformation.ClearBuffer();
    }
    if (whichBuffer == TypeFlags::BUFFER_STATIC_TERRAIN_DATA)
    {
        // TerrainBuffer.ClearBuffer();
        StaticMeshInformation.ClearBuffer(); // check the note in the insert function.
    }
    if (whichBuffer == TypeFlags::BUFFER_INSTANCE_DATA)
    {
        InstanceData.ClearBuffer();
    }
    if (whichBuffer == TypeFlags::BUFFER_MATRIX_DATA)
    {
        // guess we dont need it if we have instance data lmao
    }
    if (whichBuffer == TypeFlags::BUFFER_ANIMATION_DATA)
    {
        AnimationMatrices.ClearBuffer();
    }
    if (whichBuffer == TypeFlags::BUFFER_PARTICLE_DATA)
    {
        ParticleData.ClearBuffer();
    }
    if (whichBuffer == TypeFlags::BUFFER_TEXTURE_DATA)
    {
        TextureHandleBuffer.ClearBuffer();
    }
    if (whichBuffer == TypeFlags::BUFFER_DRAW_CALL_DATA)
    {
        DrawCommandBuffer.ClearBuffer();
    }
    if (whichBuffer == TypeFlags::BUFFER_CAMERA_DATA)
    {
        cameraMatrices.ClearBuffer();
    }
    if (whichBuffer == TypeFlags::BUFFER_LIGHT_DATA)
    {
        LightsBuffer.ClearBuffer();
    }
}


BufferManager::BufferManager()
{
}








} // namespace eHazGraphics
