

#include "Animation/AnimatedModel.hpp"
#include "Animation/AnimatedModelManager.hpp"
#include "DataStructs.hpp"
#include "MeshManager.hpp"
#include "Model.hpp"
#include "ModelPackage.hpp"
#include "Utils/Boost_GLM_Serialization.hpp"

#include <SDL3/SDL_log.h>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/detail/iserializer.hpp>
#include <condition_variable>
#include <fstream>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>
namespace eHazGraphics {

StaticModelPackage MeshManager::LoadSingleModel(const std::string &path) {
  StaticModelPackage pkg;
  std::ifstream ifs(path, std::ios::binary);
  if (!ifs.is_open()) {
    throw std::runtime_error("Failed to open file: " + path);
  }
  boost::archive::binary_iarchive ia(ifs);
  ia >> pkg;
  return pkg;
}
std::vector<ModelID>
MeshManager::LoadHazModelListLimited(const std::vector<std::string> &paths,
                                     size_t maxThreads) {
  std::vector<ModelID> models;
  std::vector<std::future<StaticModelPackage>> futures;
  size_t activeThreads = 0;
  size_t i = 0;

  while (i < paths.size() || activeThreads > 0) {
    // Launch new threads if below limit
    while (i < paths.size() && activeThreads < maxThreads) {
      futures.push_back(
          std::async(std::launch::async, LoadSingleModel, paths[i]));
      ++activeThreads;
      ++i;
    }

    // Check futures for completion
    for (auto it = futures.begin(); it != futures.end();) {
      auto &f = *it;
      auto status = f.wait_for(std::chrono::milliseconds(0));
      if (status == std::future_status::ready) {
        try {
          StaticModelPackage pkg = f.get();

          std::lock_guard<std::mutex> lock(mapMutex);
          for (auto &mesh : pkg.meshes)
            meshes[mesh.GetID()] = mesh;
          models.push_back(pkg.model.GetID());
          loadedModels[pkg.model.GetID()] = std::make_shared<Model>(pkg.model);

        } catch (const std::exception &e) {
          std::cerr << "Error loading model: " << e.what() << std::endl;
        }

        it = futures.erase(it); // remove completed future
        --activeThreads;
      } else {
        ++it;
      }
    }

    std::this_thread::sleep_for(
        std::chrono::milliseconds(1)); // small wait to reduce busy-loop
  }
  return models;
}
void MeshManager::ExportHazModel(std::string exportPath, ModelID modelID) {

  // Find the model
  auto modelIt = loadedModels.find(modelID);
  if (modelIt == loadedModels.end()) {
    throw std::runtime_error("ModelID not found in loadedModels");
  }

  Model &model = *modelIt->second;

  // Gather meshes used by this model
  std::vector<Mesh> modelMeshes;
  for (MeshID meshID : model.GetMeshIDs()) {
    auto meshIt = meshes.find(meshID);
    if (meshIt != meshes.end()) {
      modelMeshes.push_back(meshIt->second);
    } else {
      throw std::runtime_error("MeshID not found in meshes");
    }
  }

  // Build package
  StaticModelPackage pkg;
  pkg.model = model;
  pkg.meshes = std::move(modelMeshes);

  // Serialize to file
  std::ofstream ofs(exportPath, std::ios::binary);
  if (!ofs.is_open()) {
    throw std::runtime_error("Failed to open export path: " + exportPath);
  }

  boost::archive::binary_oarchive oa(ofs);
  oa << pkg;
}

ModelID MeshManager::LoadHazModel(std::string path) {

  StaticModelPackage pkg = LoadSingleModel(path);

  std::lock_guard<std::mutex> lock(mapMutex);

  for (auto &mesh : pkg.meshes) {

    meshes.try_emplace(mesh.GetID(), std::move(mesh));
  }

  ModelID retID = pkg.model.GetID();

  loadedModels.try_emplace(retID, std::make_shared<Model>(pkg.model));

  ValidateLoadedFile(retID);

  return retID;
}
std::vector<ModelID>
MeshManager::LoadHazModelList(std::vector<std::string> paths) {

  auto ids = LoadHazModelListLimited(paths);

  for (auto &id : ids) {
    ValidateLoadedFile(id);
  }

  return ids;
}

std::vector<ModelID>
MeshManager::LoadHazModelList(std::vector<StaticModelPackage> &packages) {
  std::vector<ModelID> models;
  std::lock_guard<std::mutex> lock(mapMutex);

  for (auto &pkg : packages) {

    for (auto &mesh : pkg.meshes) {

      meshes[mesh.GetID()] = std::move(mesh);
    }
    models.push_back(pkg.model.GetID());
    loadedModels[pkg.model.GetID()] = std::make_shared<Model>(pkg.model);
  }

  for (auto &id : models) {
    ValidateLoadedFile(id);
  }

  return models;
}

void MeshManager::ValidateLoadedFile(ModelID model) {

  auto meshIDs = loadedModels[model]->GetMeshIDs();

  for (auto &meshID : meshIDs) {

    Mesh &mesh = meshes[meshID];

    if (!meshTransforms.contains(meshID)) {
      glm::mat4 relMat = mesh.GetRelativeMatrix();
      meshTransforms.emplace(meshID, relMat);

      BufferRange relMatRange = bufferManager->InsertNewDynamicData(
          &relMat, sizeof(relMat), TypeFlags::BUFFER_STATIC_MATRIX_DATA);

      meshTransformRanges.emplace(meshID, relMatRange);
    }

    if (!meshLocations.contains(meshID)) {

      auto l_vertex = mesh.GetVertexData();
      auto l_index = mesh.GetIndexData();
      VertexIndexInfoPair l_viipLocation = bufferManager->InsertNewStaticData(
          l_vertex.first, l_vertex.second, l_index.first, l_index.second,
          TypeFlags::BUFFER_STATIC_MESH_DATA);

      meshLocations.emplace(meshID, l_viipLocation);
    }
  }
}

void MeshManager::ValidateLoadedFiles() {

  for (auto &[modelID, model] : loadedModels) {

    ValidateLoadedFile(modelID);
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
 */

void AnimatedModelManager::ExportAHazModel(std::string exportPath,
                                           ModelID modelID) {
  auto modelIt = loadedModels.find(modelID);
  if (modelIt == loadedModels.end()) {
    throw std::runtime_error("ModelID not found in loadedModels");
  }

  AnimatedModel &model = *modelIt->second;

  std::vector<Mesh> modelMeshes;
  for (MeshID meshID : model.GetMeshIDs()) {
    auto meshIt = meshes.find(meshID);
    if (meshIt != meshes.end()) {

      modelMeshes.push_back(meshIt->second);
    } else {
      throw std::runtime_error("MeshID not found in meshes");
    }
  }

  AnimatedModelPackage pkg;
  pkg.model = model;
  pkg.meshes = std::move(modelMeshes);

  std::ofstream ofs(exportPath, std::ios::binary);
  if (!ofs.is_open()) {
    throw std::runtime_error("Failed to open export path: " + exportPath);
  }

  boost::archive::binary_oarchive oa(ofs);
  oa << pkg;
}
ModelID AnimatedModelManager::LoadAHazModel(std::string path) {

  AnimatedModelPackage pkg = LoadSingleModel(path);

  std::lock_guard<std::mutex> lock(mapMutex);

  for (auto &mesh : pkg.meshes) {

    meshes.try_emplace(mesh.GetID(), std::move(mesh));
  }

  ModelID retID = pkg.model.GetID();

  loadedModels.try_emplace(retID, std::make_shared<AnimatedModel>(pkg.model));

  ValidateLoadedFile(retID);

  return retID;
}
std::vector<ModelID>
AnimatedModelManager::LoadAHazModelList(std::vector<std::string> paths) {

  auto ids = LoadAHazModelListLimited(paths);

  for (auto &id : ids) {
    ValidateLoadedFile(id);
  }

  return ids;
}
std::vector<ModelID> AnimatedModelManager::LoadAHazModelList(
    std::vector<AnimatedModelPackage> &packages) {

  std::vector<ModelID> models;
  std::lock_guard<std::mutex> lock(mapMutex);

  for (auto &pkg : packages) {

    for (auto &mesh : pkg.meshes) {

      meshes[mesh.GetID()] = std::move(mesh);
    }
    models.push_back(pkg.model.GetID());
    loadedModels[pkg.model.GetID()] =
        std::make_shared<AnimatedModel>(pkg.model);
  }

  for (auto &id : models) {
    ValidateLoadedFile(id);
  }

  return models;
}

AnimatedModelPackage
AnimatedModelManager::LoadSingleModel(const std::string &path) {

  AnimatedModelPackage pkg;
  std::ifstream ifs(path, std::ios::binary);
  if (!ifs.is_open()) {
    throw std::runtime_error("Failed to open file: " + path);
  }
  boost::archive::binary_iarchive ia(ifs);
  ia >> pkg;

  for (size_t mi = 0; mi < pkg.meshes.size(); ++mi) {
    const auto &m = pkg.meshes[mi];
    std::cerr << "  mesh[" << mi << "].GetID()=" << std::hex << m.GetID()
              << std::dec << ", verts=" << m.GetMeshData().vertices.size()
              << ", idx=" << m.GetMeshData().indecies.size()
              << ", shader.v=" << m.GetShaderID().vertex
              << ", shader.f=" << m.GetShaderID().fragment << "\n";
  }

  return pkg;
}
std::vector<ModelID> AnimatedModelManager::LoadAHazModelListLimited(
    const std::vector<std::string> &paths, size_t maxThreads) {

  std::vector<ModelID> models;
  std::vector<std::future<AnimatedModelPackage>> futures;
  size_t activeThreads = 0;
  size_t i = 0;

  while (i < paths.size() || activeThreads > 0) {
    // Launch new threads if below limit
    while (i < paths.size() && activeThreads < maxThreads) {
      futures.push_back(
          std::async(std::launch::async, LoadSingleModel, paths[i]));
      ++activeThreads;
      ++i;
    }

    // Check futures for completion
    for (auto it = futures.begin(); it != futures.end();) {
      auto &f = *it;
      auto status = f.wait_for(std::chrono::milliseconds(0));
      if (status == std::future_status::ready) {
        try {
          AnimatedModelPackage pkg = f.get();

          std::lock_guard<std::mutex> lock(mapMutex);
          for (auto &mesh : pkg.meshes)
            meshes[mesh.GetID()] = mesh;
          models.push_back(pkg.model.GetID());
          loadedModels[pkg.model.GetID()] =
              std::make_shared<AnimatedModel>(pkg.model);

        } catch (const std::exception &e) {
          std::cerr << "Error loading animated model: " << e.what()
                    << std::endl;
        }

        it = futures.erase(it); // remove completed future
        --activeThreads;
      } else {
        ++it;
      }
    }

    std::this_thread::sleep_for(
        std::chrono::milliseconds(1)); // small wait to reduce busy-loop
  }
  return models;
}
void AnimatedModelManager::ValidateLoadedFile(ModelID model) {

  auto meshIDs = loadedModels[model]->GetMeshIDs();

  for (auto &meshID : meshIDs) {

    Mesh &mesh = meshes[meshID];

    if (!meshLocations.contains(meshID)) {

      auto l_vertex = mesh.GetVertexData();
      auto l_index = mesh.GetIndexData();
      VertexIndexInfoPair l_viipLocation = bufferManager->InsertNewStaticData(
          l_vertex.first, l_vertex.second, l_index.first, l_index.second,
          TypeFlags::BUFFER_STATIC_MESH_DATA);

      meshLocations.emplace(meshID, l_viipLocation);
    }

    if (!skeletons.contains(model)) {
      if (loadedModels[model]->GetSkeleton()) {

        skeletons.emplace(model, loadedModels[model]->GetSkeleton());

      } else
        SDL_Log("ERROR: animated model has no skeleton, please assign one "
                "ModelID: %zu",
                model);
    }
    if (!animators.contains(model)) {

      animators.emplace(model, std::make_shared<Animator>());

      animators[model]->SetSkeleton(loadedModels[model]->GetSkeleton());

      loadedModels[model]->SetAnimatorID(model);
    }
  }
}
void AnimatedModelManager::ValidateLoadedFiles() {

  for (auto &[modelID, model] : loadedModels) {

    ValidateLoadedFile(modelID);
  }
}

} // namespace eHazGraphics
