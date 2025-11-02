#include "Animation/AnimatedModelManager.hpp"
#include "DataStructs.hpp"
#include "MeshManager.hpp"
#include "Utils/HashedStrings.hpp"
#include "Utils/Math_Utils.hpp"
#include "glm/fwd.hpp"

#include <assimp/scene.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <set>
#include <string>
#include <vector>

using namespace eHazGraphics_Utils;
namespace eHazGraphics {

AnimatedModel AnimatedModelManager::LoadAnimatedModel(std::string path) {

  std::vector<MeshID> temps;

  eHazGraphics_Utils::HashedString hashedPath =
      eHazGraphics_Utils::computeHash(path);

  if (loadedModels.contains(hashedPath)) {
    return loadedModels[hashedPath];
  }

  const aiScene *scene = importer.ReadFile(
      path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals |
                /*aiProcess_PreTransformVertices |*/ aiProcess_OptimizeMeshes);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {

    SDL_Log("ERROR LOADING THE MODEL: %s", importer.GetErrorString());
  }
}

/*
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */
void AnimatedModelManager::UploadBonesToGPU(
    BufferRange &range, std::vector<glm::mat4> finalMatrices) {

  if (range.OwningBuffer != 2) {
    range = bufferManager->InsertNewDynamicData(
        finalMatrices.data(), finalMatrices.size() * sizeof(glm::mat4),
        TypeFlags::BUFFER_ANIMATION_DATA);
  } else {
    bufferManager->UpdateData(range, finalMatrices.data(),
                              finalMatrices.size() * sizeof(glm::mat4));
  }
}

void AnimatedModelManager::Update(float deltaTime) {
  for (Skeleton &skeleton : skeletons) {
    skeleton.Update(deltaTime);
  }

  // Submit to GPU or instance buffer
  for (AnimatedModel *model : submittedAnimatedModels) {

    auto Instances = model->GetInstances();
    auto InstanceRanges = model->GetInstanceRanges();
    assert(Instances.size() == InstanceRanges.size());

    for (unsigned int i = 0; i < Instances.size(); i++) {

      bufferManager->UpdateData(InstanceRanges[i], &Instances[i],
                                sizeof(InstanceData));
    }

    Skeleton &skeleton = skeletons[model->GetSkeletonID()];
    UploadBonesToGPU(skeleton.GPUlocation,
                     skeleton.finalMatrices); // your existing submission logic
  }
}

} // namespace eHazGraphics
