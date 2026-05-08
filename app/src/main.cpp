#include "Animation/AnimatedModelManager.hpp"
#include "Animation/Animator.hpp"
#include "BitFlags.hpp"
#include "DataStructs.hpp"
#include "Renderer.hpp"
#include "camera.hpp"
#include "glm/ext/matrix_transform.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_scancode.h>
#include <vector>

using namespace eHazGraphics;

float deltaTime = 0.0f;
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float blendX = 0.0f; // speed axis:  0=idle, 0.5=jog, 1.0=run
float blendY = 0.0f; // action axis: 1.0=jump

void processInput(Window *c_window, bool &quit, Animator *anim) {
  static uint64_t lastCounter = SDL_GetPerformanceCounter();
  uint64_t now = SDL_GetPerformanceCounter();
  deltaTime = double(now - lastCounter) / SDL_GetPerformanceFrequency();
  lastCounter = now;

  static bool firstMouse = true;
  static float lastX = 0.0f, lastY = 0.0f;

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
    case SDL_EVENT_MOUSE_MOTION: {
      float xpos = (float)event.motion.x, ypos = (float)event.motion.y;
      if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
      }
      if (event.key.which == SDLK_R)
        camera.ProcessMouseMovement(xpos - lastX, lastY - ypos);
      lastX = xpos;
      lastY = ypos;
      break;
    }
    case SDL_EVENT_MOUSE_WHEEL:
      camera.ProcessMouseScroll((float)event.wheel.y);
      break;
    default:
      break;
    }
  }

  const auto *keys = SDL_GetKeyboardState(nullptr);
  if (keys[SDL_SCANCODE_W])
    camera.ProcessKeyboard(FORWARD, (float)deltaTime);
  if (keys[SDL_SCANCODE_S])
    camera.ProcessKeyboard(BACKWARD, (float)deltaTime);
  if (keys[SDL_SCANCODE_A])
    camera.ProcessKeyboard(LEFT, (float)deltaTime);
  if (keys[SDL_SCANCODE_D])
    camera.ProcessKeyboard(RIGHT, (float)deltaTime);
  if (keys[SDL_SCANCODE_SPACE])
    camera.ProcessKeyboard(UP, (float)deltaTime);
  if (keys[SDL_SCANCODE_LSHIFT])
    camera.ProcessKeyboard(DOWN, (float)deltaTime);
  if (keys[SDL_SCANCODE_KP_PLUS])
    anim->SetSpeed(anim->GetSpeed() + 0.01f);
  if (keys[SDL_SCANCODE_KP_MINUS])
    anim->SetSpeed(anim->GetSpeed() - 0.01f);

  // Arrow keys drive the blend space
  //  Right arrow: idle → jog → run  (hold = jog, double-tap not needed, just
  //               press right once for jog at 0.5, hold shift+right for run
  //               at 1.0)
  blendX = 0.0f;
  blendY = 0.0f;
  if (keys[SDL_SCANCODE_RIGHT] && !keys[SDL_SCANCODE_RSHIFT])
    blendX = 0.5f; // jog
  if (keys[SDL_SCANCODE_RIGHT] && keys[SDL_SCANCODE_RSHIFT])
    blendX = 1.0f; // run
  if (keys[SDL_SCANCODE_UP])
    blendY = 1.0f; // jump
}

int main() {
  eHazGraphics::Renderer rend;
  rend.Initialize();
  rend.p_bufferManager->BeginWritting();

  // ---- Shader ----
  ShaderComboID shader = rend.p_shaderManager->CreateShaderProgramme(
      RESOURCES_PATH "animation.vert", RESOURCES_PATH "shader.frag");

  // ---- Material (using body texture) ----
  unsigned int albedo = Renderer::p_materialManager->LoadTexture(
      RESOURCES_PATH "Sonic/textures/Body-0000-Body-0000.png");
  unsigned int matID = Renderer::p_materialManager->CreatePBRMaterial(
      albedo, albedo, albedo, albedo, "sonic_body");
  auto mat = rend.p_materialManager->SubmitMaterials();
  SBufferRange materials = rend.p_bufferManager->InsertNewDynamicData(
      mat.first.data(), mat.first.size() * sizeof(PBRMaterial),
      TypeFlags::BUFFER_TEXTURE_DATA);

  // ---- Load the model from one of the gltf files (they share the same
  // skeleton) ----
  std::string modelPath = RESOURCES_PATH "Sonic/idle.gltf";
  auto modelID = rend.p_AnimatedModelManager->LoadAnimatedModel(modelPath);
  rend.p_AnimatedModelManager->SetModelShader(modelID, shader);

  auto skeleton = rend.p_AnimatedModelManager->GetModel(modelID)->GetSkeleton();
  auto &anim = rend.p_AnimatedModelManager->GetAnimator(
      rend.p_AnimatedModelManager->GetModel(modelID)->GetAnimatorID());

  // ---- Load the four animation clips ----
  AnimationID idleID, jogID, runID, jumpID;
  std::string idlePath = RESOURCES_PATH "Sonic/idle.gltf";
  std::string jogPath = RESOURCES_PATH "Sonic/jog.gltf";
  std::string runPath = RESOURCES_PATH "Sonic/run.gltf";
  std::string jumpPath = RESOURCES_PATH "Sonic/jump.gltf";

  rend.p_AnimatedModelManager->LoadAnimation(skeleton, idlePath, idleID);
  rend.p_AnimatedModelManager->LoadAnimation(skeleton, jogPath, jogID);
  rend.p_AnimatedModelManager->LoadAnimation(skeleton, runPath, runID);
  rend.p_AnimatedModelManager->LoadAnimation(skeleton, jumpPath, jumpID);

  auto clipIdle = rend.p_AnimatedModelManager->GetAnimation(idleID);
  auto clipJog = rend.p_AnimatedModelManager->GetAnimation(jogID);
  auto clipRun = rend.p_AnimatedModelManager->GetAnimation(runID);
  auto clipJump = rend.p_AnimatedModelManager->GetAnimation(jumpID);

  // ---- Build the blend space ----
  //
  //  Y axis = jump (vertical)
  //  X axis = speed (horizontal): 0=idle, 0.5=jog, 1.0=run
  //
  //        (0, 1) jump
  //           |
  // (0,0) ---+--- (0.5, 0) --- (1.0, 0)
  // idle              jog          run
  //
  // Fan triangulation pivots from index 0 (idle), so point order matters:
  //   tri 0: idle(0) → jog(1) → run(2)
  //   tri 1: idle(0) → run(2) → jump(3)
  // That covers the full convex region.

  std::vector<BlendPoint> points = {
      {clipIdle, 0.0f, 0.0f}, // index 0 — fan center
      {clipJog, 0.5f, 0.0f},  // index 1
      {clipRun, 1.0f, 0.0f},  // index 2
      {clipJump, 0.0f, 1.0f}, // index 3
  };

  int blendSpaceID = anim->RegisterBlendSpace2D(points);

  // ---- Wire the blend space into a layer ----
  // After the Animator.hpp/.cpp fix above, this overload exists:
  int layerIndex = anim->CreateAnimationLayer();
  anim->SetLayerSource(layerIndex, anim->GetBlendSpace(blendSpaceID));
  //  ^ GetBlendSpace() is a one-liner accessor you add to Animator (see below)

  // ---- Transform ----
  glm::mat4 position = glm::mat4(1.0f);
  position = glm::translate(position, glm::vec3(0.0f, 0.0f, -10.0f));
  position =
      glm::rotate(position, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
  position = glm::scale(position, glm::vec3(1.0f));

  rend.UpdateRenderer(deltaTime);
  rend.SubmitAnimatedModel(modelID, position, matID, shader);
  std::vector<DrawRange> ranges = rend.p_renderQueue->SubmitRenderCommands();

  // ---- Loop ----
  while (!rend.shouldQuit) {
    processInput(rend.p_window.get(), rend.shouldQuit, anim.get());

    // This is the only line you need each frame to drive the blend space.
    // SetBlendInput pushes x/y into BlendSpace2D::HorizontalAxis/VerticalAxis.
    anim->SetBlendInput(blendX, blendY);

    glm::mat4 proj = glm::perspective(glm::radians(camera.Zoom),
                                      (float)rend.p_window->GetWidth() /
                                          (float)rend.p_window->GetHeight(),
                                      0.1f, 100.0f);
    rend.SetCameraPosition(camera.Position);
    rend.SetViewProjection(camera.GetViewMatrix(), proj);

    rend.SubmitAnimatedModel(modelID, position, 0, shader);
    ranges = Renderer::p_renderQueue->SubmitRenderCommands();
    rend.UpdateDynamicData(materials, mat.first.data(),
                           mat.first.size() * sizeof(PBRMaterial));
    rend.UpdateRenderer(deltaTime);
    rend.RenderFrame(ranges);
    rend.SwapBuffers();
  }
  return 0;
}
