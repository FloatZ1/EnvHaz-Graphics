#ifndef EnvHazGraphics
#define EnvHazGraphics
#include "Utils/Drawing/Lines.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cstddef>
#include <lib_export.hpp>
#include <map>
#include <memory>
#include <string>
#include <vector>
// temp

#define EHAZ_DEBUG_DRAWING

#include "Animation/AnimatedModelManager.hpp"
#include "BitFlags.hpp"
#include "BufferManager.hpp"
#include "DataStructs.hpp"
#include "MaterialManager.hpp"
#include "MeshManager.hpp"
#include "RenderQueue.hpp"
#include "ShaderManager.hpp"
#include "Window.hpp"

namespace eHazGraphics {
// eHazGAPI
class Renderer {

public:
  static std::unique_ptr<Renderer> r_instance;

  static std::unique_ptr<Window> p_window;

  static std::unique_ptr<ShaderManager> p_shaderManager;
  static std::unique_ptr<AnimatedModelManager> p_AnimatedModelManager;
  static std::unique_ptr<MaterialManager> p_materialManager;
  static std::unique_ptr<MeshManager> p_meshManager;
  static std::unique_ptr<RenderQueue> p_renderQueue;
  static std::unique_ptr<BufferManager> p_bufferManager;

  const SDL_Event &GetEvent() const { return events; }

  bool shouldQuit = false;

  bool Initialize(int width = 1920, int height = 1080, std::string tittle = "",
                  bool fullscreen = false);

  void SubmitStaticModel(Model &model,
                         TypeFlags dataType); // require a an object/container
                                              // from which to unwrap everything
  void SubmitAnimatedModel(AnimatedModel &model);

  BufferRange
  SubmitDynamicData(const void *data, size_t dataSize,
                    TypeFlags dataType); // same, require a container later/
                                         // from a octree node or smth

  void UpdateDynamicData(const BufferRange &location, const void *data,
                         const size_t size);

  void PollInputEvents();

  void ClearRenderCommandBuffer();

  void RenderFrame(std::vector<DrawRange> DrawOrder);

  void UpdateRenderer(float deltaTime);

  // future

#ifdef EHAZ_DEBUG_DRAWING

  // NOTE: HACKY ass debugging fix later

  std::map<int, eHazGraphics_Utils::Line> lines;

  int numLines;

  void DrawLine(glm::vec3 start, glm::vec3 end, float width, glm::vec3 color) {}

  void DrawRay(glm::vec3 start, glm::vec3 direction, float radius, float width,
               glm::vec3 color);

  // TODO: implement

  static void DrawSimpleMesh(glm::vec3 position, SimpleShapes shape,
                             float dimensions[4]);

#endif

  void Destroy();

private:
  SDL_Event events;
  /* Window window;

   ShaderManager shaderManager;
   MaterialManager materialManager;
   MeshManager meshManager;
   RenderQueue renderQueue; */
  // BufferManager bufferManager;
};

} // namespace eHazGraphics

#endif
