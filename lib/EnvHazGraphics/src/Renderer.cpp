#include "Renderer.hpp"
#include "BufferManager.hpp"
#include "DataStructs.hpp"
#include "MaterialManager.hpp"
#include "MeshManager.hpp"
#include "RenderQueue.hpp"
#include "ShaderManager.hpp"
#include "Window.hpp"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>
#include <glad/glad.h>
#include <iostream>
#include <memory>
#include <vector>


namespace eHazGraphics
{


std::unique_ptr<Window> Renderer::p_window = nullptr;

std::unique_ptr<ShaderManager> Renderer::p_shaderManager = nullptr;
std::unique_ptr<MaterialManager> Renderer::p_materialManager = nullptr;
std::unique_ptr<MeshManager> Renderer::p_meshManager = nullptr;
std::unique_ptr<RenderQueue> Renderer::p_renderQueue = nullptr;
std::unique_ptr<BufferManager> Renderer::p_bufferManager = nullptr;











// NOTE: refactor when everything works ...


bool Renderer::Initialize()
{



    p_window = std::make_unique<Window>();

    bool success{true};

    if (SDL_Init(SDL_INIT_VIDEO) == false)
    {
        SDL_Log("SDL could not initialize! SDL error: %s\n", SDL_GetError());
        success = false;
    }



    if (p_window->Create(200, 200, false, "yummers"))
    {
        SDL_Log("Window created successfully.\n");
        if (gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
        {
            SDL_Log("GLAD INITIALIZED SUCESSFULLY!");
            glViewport(0, 0, p_window->GetWidth(), p_window->GetHeight());
            glClearColor(0.1f, 0.5f, 0.7f, 1.0f);
        }
        else
        {
            SDL_Log("Failed to initialize GLAD");
        }
    }


    /*   shaderManager = ShaderManager();
       materialManager = MaterialManager();
       // meshManager = MeshManager();
       renderQueue = RenderQueue();
       //  bufferManager = BufferManager();



       //  bufferManager.Initialize();
       renderQueue.Initialize();
       shaderManager.Initialize();
       meshManager.Initialize();
       materialManager.Initialize();


       p_window = std::make_unique<Window>(window);
       p_events = std::make_unique<SDL_Event>(events);
       p_shaderManager = std::make_unique<ShaderManager>(shaderManager);
       p_materialManager = std::make_unique<MaterialManager>(materialManager);
       p_meshManager = std::make_unique<MeshManager>();
       p_renderQueue = std::make_unique<RenderQueue>(renderQueue);
       p_bufferManager = std::make_unique<BufferManager>();
       p_bufferManager->Initialize();
                                    */


    p_shaderManager = std::make_unique<ShaderManager>();
    p_materialManager = std::make_unique<MaterialManager>();
    p_meshManager = std::make_unique<MeshManager>();
    p_renderQueue = std::make_unique<RenderQueue>();
    p_bufferManager = std::make_unique<BufferManager>();

    p_renderQueue->Initialize();
    p_shaderManager->Initialize();
    p_meshManager->Initialize();
    p_materialManager->Initialize();
    p_bufferManager->Initialize();

    assert(p_shaderManager && "ShaderManager is not initialized");
    assert(p_window && "Window is not initialized");





    return success;
    std::cout << "piss\n\n :)))";
}



void Renderer::SubmitStaticMesh(std::vector<MeshID> meshes, BufferRange &instanceData, TypeFlags dataType)
{

    p_bufferManager->ClearBuffer(dataType);


    for (auto &mesh : meshes)
    {

        const Mesh &m_mesh = p_meshManager->GetMesh(mesh);

        const auto &vertexPair = m_mesh.GetVertexData();
        const auto &indexPair = m_mesh.GetIndexData();

        auto range = p_bufferManager->InsertNewStaticData(vertexPair.first, vertexPair.second, indexPair.first,
                                                          indexPair.second, dataType);

        // TODO: INTEGRATE THE INSTANCE DATA, get the data and create it;

        size_t instanceID = instanceData.offset / sizeof(InstanceData);

        int cmdID = p_renderQueue->CreateRenderCommand(range, true, instanceID, m_mesh.GetInstanceCount(),
                                                       m_mesh.GetShaderID());
    }
}
BufferRange Renderer::SubmitDynamicData(const void *data, size_t dataSize, TypeFlags dataType)
{
    BufferRange rt;

    rt = p_bufferManager->InsertNewDynamicData(data, dataSize, dataType);

    return rt;
    // only for new data
}



void Renderer::PollInputEvents()
{
    SDL_PollEvent(&events);
}


void Renderer::RenderFrame(std::vector<DrawRange> DrawOrder)
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Bind the static mesh buffer
    p_bufferManager->BindStaticBuffer(TypeFlags::BUFFER_STATIC_MESH_DATA);

    // Bind the dynamic draw command buffer
    p_bufferManager->BindDynamicBuffer(TypeFlags::BUFFER_DRAW_CALL_DATA);

    if (!p_shaderManager || !p_window)
    {
        SDL_Log("RenderFrame called with uninitialized managers!");
        return;
    }



    for (const auto &range : DrawOrder)
    {
        p_shaderManager->UseProgramme(range.shader);
        GLintptr offset = range.startIndex * sizeof(DrawElementsIndirectCommand);

        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void *)offset, range.count, 0);
    }

    SDL_GL_SwapWindow(p_window->GetWindowPtr());
}

void Renderer::UpdateRenderer()
{
    PollInputEvents();
    if (events.type == SDL_EVENT_QUIT)
        shouldQuit = true;
}

void Renderer::Destroy()
{


    p_meshManager->Destroy();
    p_renderQueue->Destroy();
    //  bufferManager.Destroy();
    p_bufferManager->Destroy();
    p_shaderManager->Destroy();
    p_materialManager->Destroy();
}


void Renderer::UpdateDynamicData(const BufferRange &location, const void *data, const size_t size)
{

    p_bufferManager->UpdateData(location, data, size);
}

} // namespace eHazGraphics
