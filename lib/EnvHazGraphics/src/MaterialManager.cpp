#include "MaterialManager.hpp"
#include "Animation/AnimatedModelManager.hpp"
#include "BitFlags.hpp"
#include "DataStructs.hpp"
#include "Utils/HashedStrings.hpp"
#include <SDL3/SDL_log.h>
#include <cstddef>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace eHazGraphics {

void MaterialManager::ClearMaterials() {
  LoadedPBRMaterials.clear();
  LoadedTextures.clear();
  freeIndecies.clear();
  TexturePaths.clear();
  MaterialNames.clear();
}

unsigned int MaterialManager::LoadTexture(const std::string &path) {

  eHazGraphics_Utils::HashedString h_path =
      eHazGraphics_Utils::computeHash(path);

  if (TexturePaths.contains(h_path)) {
    return TexturePaths[h_path];
  }

  LoadedTextures.push_back(std::make_unique<Texture2D>(path));
  unsigned int id = LoadedTextures.size() - 1;
  LoadedTextures[id]->MakeResident();
  TexturePaths.emplace(h_path, id);

  return id;
}
unsigned int MaterialManager::CreatePBRMaterial(unsigned int albedoID,
                                                unsigned int prmID,
                                                unsigned int NormalMapID,
                                                unsigned int EmissionID,
                                                std::string mat_name) {

  eHazGraphics_Utils::HashedString h_matName =
      eHazGraphics_Utils::computeHash(mat_name);

  if (MaterialNames.contains(h_matName))
    return MaterialNames[h_matName];

  PBRMaterial newMat;
  newMat.albedo = LoadedTextures[albedoID]->GetTextureHandle();
  newMat.prm = LoadedTextures[prmID]->GetTextureHandle();
  newMat.NormalMap = LoadedTextures[NormalMapID]->GetTextureHandle();
  newMat.Emission = LoadedTextures[EmissionID]->GetTextureHandle();
  newMat.Luminance = 0.5f;
  if (freeIndecies.size() == 0) {
    LoadedPBRMaterials.push_back(newMat);
    MaterialNames.emplace(h_matName, LoadedPBRMaterials.size() - 1);

    return LoadedPBRMaterials.size() - 1;
  } else {
    int index = freeIndecies[0];
    LoadedPBRMaterials[index] = newMat;
    freeIndecies.erase(freeIndecies.begin());
    MaterialNames.emplace(h_matName, index);
    return index;
  }
}
void MaterialManager::UpdateMaterial(unsigned int MaterialID,
                                     PBRMaterial replacement) {
  LoadedPBRMaterials[MaterialID] = replacement;
}
std::pair<const std::vector<PBRMaterial> &, TypeFlags>
MaterialManager::SubmitMaterials() {

  return std::pair<const std::vector<PBRMaterial> &, TypeFlags>(
      LoadedPBRMaterials, TypeFlags::BUFFER_TEXTURE_DATA);
}
std::optional<PBRMaterial>
MaterialManager::GetMaterial(const std::string &materialName) {
  eHazGraphics_Utils::HashedString temp =
      eHazGraphics_Utils::computeHash(materialName);

  if (MaterialNames.find(temp) != MaterialNames.end()) {
    return LoadedPBRMaterials[MaterialNames[temp]];
  } else {
    SDL_Log("Material Query: NO SUCH MATERIAL ERROR \n");
  }
  return std::nullopt;
}
void MaterialManager::DeleteMaterial(unsigned int MaterialID) {

  freeIndecies.push_back(MaterialID);
}

void MaterialManager::Destroy() {}

void MaterialManager::Initialize() {}

} // namespace eHazGraphics
