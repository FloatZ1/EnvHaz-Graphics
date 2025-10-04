
#include "DataStructs.hpp"
#include "Renderer.hpp"
#include "camera.hpp"
#include "glm/fwd.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_oldnames.h>

#include <cstdint>

#include <vector>



using namespace eHazGraphics;

float deltaTime = 0.0f;
float lastFrame = 0.0f;
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));



#include <SDL3/SDL.h>

void processInput(SDL_Window *window, bool &quit, Camera &camera)
{
    // Delta time calculation using performance counters
    static uint64_t lastCounter = SDL_GetPerformanceCounter();
    uint64_t currentCounter = SDL_GetPerformanceCounter();
    double deltaTime = double(currentCounter - lastCounter) / SDL_GetPerformanceFrequency();
    lastCounter = currentCounter;

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            quit = true;
            break;

        case SDL_EVENT_KEY_DOWN:
            if (event.key.which == SDLK_ESCAPE)
                quit = true;
            break;

        default:
            break;
        }
    }

    // Continuous key input using scancodes
    const auto *state = SDL_GetKeyboardState(nullptr);
    if (state[SDL_SCANCODE_W])
        camera.ProcessKeyboard(FORWARD, static_cast<float>(deltaTime));
    if (state[SDL_SCANCODE_S])
        camera.ProcessKeyboard(BACKWARD, static_cast<float>(deltaTime));
    if (state[SDL_SCANCODE_A])
        camera.ProcessKeyboard(LEFT, static_cast<float>(deltaTime));
    if (state[SDL_SCANCODE_D])
        camera.ProcessKeyboard(RIGHT, static_cast<float>(deltaTime));
}



int main()
{



    eHazGraphics::Renderer rend;
    rend.Initialize();


    rend.p_bufferManager->BeginWritting();

    ShaderComboID shader =
        rend.p_shaderManager->CreateShaderProgramme(RESOURCES_PATH "shader.vert", RESOURCES_PATH "shader.frag");

    SDL_Log("\n\n\n" RESOURCES_PATH "\n\n\n");



    std::string path = RESOURCES_PATH "cube.obj";
    Model cube = rend.p_meshManager->LoadModel(path);
    ShaderComboID temp;


    InstanceData data = {glm::mat4(1.0f), 0};

    rend.p_meshManager->SetModelShader(cube, shader);

    BufferRange instanceData = rend.SubmitDynamicData(&data, sizeof(data), TypeFlags::BUFFER_INSTANCE_DATA);

    rend.SubmitStaticMesh(cube.GetMeshIDs(), instanceData, TypeFlags::BUFFER_STATIC_MESH_DATA);

    auto ranges = rend.p_renderQueue->SubmitRenderCommands();


    glm::mat4 viewTest(1.0f);
    glm::mat4 projtest(1.0f);

    std::vector<glm::mat4> deta{viewTest, projtest};

    BufferRange camDt =
        rend.SubmitDynamicData(deta.data(), deta.size() * sizeof(glm::mat4), TypeFlags::BUFFER_CAMERA_DATA);

    // rend.p_bufferManager->EndWritting();

    while (rend.shouldQuit == false)
    {
        // rend.p_renderQueue->ClearDynamicCommands();


        rend.p_bufferManager->BeginWritting();
        rend.UpdateRenderer();

        processInput(rend.p_window->GetWindowPtr(), rend.shouldQuit, camera);

        glm::mat4 projection =
            glm::perspective(glm::radians(camera.Zoom),
                             (float)rend.p_window->GetWidth() / (float)rend.p_window->GetHeight(), 0.1f, 100.0f);

        std::vector<glm::mat4> cameraSend{camera.GetViewMatrix(), projection};

        rend.UpdateDynamicData(camDt, cameraSend.data(), cameraSend.size() * sizeof(glm::mat4));


        rend.RenderFrame(ranges);
        rend.p_bufferManager->EndWritting();
    }




    return 0;
}
