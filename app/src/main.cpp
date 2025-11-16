
#include "Animation/AnimatedModelManager.hpp"
#include "BitFlags.hpp"
#include "DataStructs.hpp"
#include "Renderer.hpp"
#include "camera.hpp"

#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include "glm/trigonometric.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_oldnames.h>

#include <SDL3/SDL_scancode.h>
#include <cstdint>

#include <vector>

using namespace eHazGraphics;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

void processInput(Window *c_window, bool &quit, Camera &camera) {

  SDL_Window *window = c_window->GetWindowPtr();
  // Delta time calculation using performance counters
  static uint64_t lastCounter = SDL_GetPerformanceCounter();
  uint64_t currentCounter = SDL_GetPerformanceCounter();

  deltaTime =
      double(currentCounter - lastCounter) / SDL_GetPerformanceFrequency();
  lastCounter = currentCounter;

  // Static mouse state
  static bool firstMouse = true;
  static float lastX = 0.0f;
  static float lastY = 0.0f;

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_EVENT_QUIT:
      quit = true;
      break;

    case SDL_EVENT_KEY_DOWN:
      if (event.key.which == SDLK_ESCAPE)
        quit = true;
      break;

    case SDL_EVENT_KEY_UP:

      break;

    // --- Added mouse movement support for camera ---
    case SDL_EVENT_MOUSE_MOTION: {
      float xpos = static_cast<float>(event.motion.x);
      float ypos = static_cast<float>(event.motion.y);

      if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
      }

      float xoffset = xpos - lastX;
      float yoffset =
          lastY - ypos; // reversed since y-coordinates go from bottom to top

      lastX = xpos;
      lastY = ypos;

      camera.ProcessMouseMovement(xoffset, yoffset);
      break;
    }

    // --- Added mouse scroll support for zoom ---
    case SDL_EVENT_MOUSE_WHEEL: {
      float yoffset = static_cast<float>(event.wheel.y);
      camera.ProcessMouseScroll(yoffset);
      break;
    }

    // --- Added window resize support (framebuffer_size_callback equivalent)
    // ---
    case SDL_EVENT_WINDOW_RESIZED: {
      int width = event.window.data1;
      int height = event.window.data2;
      c_window->SetDimensions(width, height);
      glViewport(0, 0, width, height);
      break;
    }

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
  if (state[SDL_SCANCODE_SPACE])
    camera.ProcessKeyboard(UP, static_cast<float>(deltaTime));
  if (state[SDL_SCANCODE_LSHIFT])
    camera.ProcessKeyboard(DOWN, static_cast<float>(deltaTime));
  if (state[SDL_SCANCODE_R])
    c_window->ToggleMouseCursor();
}
struct camData {
  glm::mat4 view = glm::mat4(1.0f);
  glm::mat4 projection = glm::mat4(1.0f);
};

int main() {

  eHazGraphics::Renderer rend;
  rend.Initialize();

  rend.p_bufferManager->BeginWritting();

  unsigned int AlbedoTexture =
      Renderer::p_materialManager->LoadTexture(RESOURCES_PATH "rizz.png");

  unsigned int materialID = Renderer::p_materialManager->CreatePBRMaterial(
      AlbedoTexture, AlbedoTexture, AlbedoTexture, AlbedoTexture);

  auto mat = rend.p_materialManager->SubmitMaterials();
  BufferRange materials = rend.p_bufferManager->InsertNewDynamicData(
      mat.first.data(), mat.first.size() * sizeof(PBRMaterial),
      TypeFlags::BUFFER_TEXTURE_DATA);

  ShaderComboID shader = rend.p_shaderManager->CreateShaderProgramme(
      RESOURCES_PATH "animation.vert", RESOURCES_PATH "shader.frag");

  SDL_Log("\n\n\n" RESOURCES_PATH "\n\n\n");

  std::string path = RESOURCES_PATH "animated/rigged_sonic.glb";
  // std::string path = RESOURCES_PATH "cube.obj";
  // Model cube = rend.p_meshManager->LoadModel(path);
  auto model = rend.p_AnimatedModelManager->LoadAnimatedModel(path);
  int animationID;
  rend.p_AnimatedModelManager->LoadAnimation(model->GetSkeleton(), path,
                                             animationID);
  auto &anim = rend.p_AnimatedModelManager->GetAnimator(model->GetAnimatorID());

  int skelAnimID = anim->AddAnimation(
      rend.p_AnimatedModelManager->GetAnimation(animationID));

  // Renderer::p_meshManager->SetModelInstanceCount(cube, 1);

  glm::mat4 position = glm::mat4(1.0f);

  position = glm::translate(position, glm::vec3(0.0f, 0.0f, -10.0f));
  position =
      glm::rotate(position, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
  position = glm::scale(position, glm::vec3(0.3f));
  model->SetPositionMat4(position);

  rend.p_AnimatedModelManager->SetModelShader(model, shader);
  // cube.SetPositionMat4(model);

  // rend.p_meshManager->SetModelShader(cube, shader);

  // BufferRange instanceData = rend.SubmitDynamicData(&data, sizeof(data),
  // TypeFlags::BUFFER_INSTANCE_DATA);

  // rend.SubmitStaticModel(cube, TypeFlags::BUFFER_STATIC_MESH_DATA);

  rend.SubmitAnimatedModel(model);

  auto ranges = rend.p_renderQueue->SubmitRenderCommands();

  // glm::mat4 test(1.0f);
  // test[2][2] = 69.0f;

  // animation stuff here, adding into animator

  const auto &animationClip =
      rend.p_AnimatedModelManager->GetAnimation(animationID);

  // 1. Register the animation with the Animator (as per your current design)
  // The returned index (localAnimationID) is for the Animator's internal list
  // only.
  int localAnimationID = anim->AddAnimation(animationClip);

  // --- Core Animation Setup ---
  // 2. Create the first animation layer (this usually returns 0)
  int layerIndex = anim->CreateAnimationLayer();

  // 3. Set the loaded Animation as the source for that layer
  // The layer will now start playing this animation clip.
  anim->SetLayerSource(layerIndex, animationClip);

  //
  glm::mat4 projection1 = glm::perspective(
      glm::radians(camera.Zoom),
      (float)rend.p_window->GetWidth() / (float)rend.p_window->GetHeight(),
      0.1f, 100.0f);

  camData deta{camera.GetViewMatrix(), projection1};

  BufferRange camDt = rend.SubmitDynamicData(&deta, sizeof(deta),
                                             TypeFlags::BUFFER_CAMERA_DATA);

  // rend.p_bufferManager->EndWritting();

  int frameNum = 0;

  while (rend.shouldQuit == false) {

    processInput(rend.p_window.get(), rend.shouldQuit, camera);
    rend.UpdateRenderer(deltaTime);
    glm::mat4 projection = glm::perspective(
        glm::radians(camera.Zoom),
        (float)rend.p_window->GetWidth() / (float)rend.p_window->GetHeight(),
        0.1f, 100.0f);

    camData camcamdata = {camera.GetViewMatrix(), projection};

    rend.UpdateDynamicData(camDt, &camcamdata, sizeof(camcamdata));

    // TODO: make animator play the fucking animation;

    ranges = Renderer::p_renderQueue->SubmitRenderCommands();
    // rend.p_bufferManager->EndWritting();
    rend.RenderFrame(ranges);
    // Renderer::p_bufferManager->ClearBuffer(TypeFlags::BUFFER_CAMERA_DATA);
    rend.UpdateDynamicData(materials, mat.first.data(),
                           mat.first.size() * sizeof(PBRMaterial));

    frameNum++;

    // printf("END OF FRAME: %u ==============================\n", frameNum);
  }

  return 0;
}
