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

  unsigned int id;

  if (freeTextureIndecies.size() > 0) {

    id = freeTextureIndecies.back();
    freeTextureIndecies.pop_back();

    LoadedTextures[id]->RemoveResidency();

    LoadedTextures[id] = std::make_unique<Texture2D>(path);

  } else {
    LoadedTextures.push_back(std::make_unique<Texture2D>(path));
    id = LoadedTextures.size() - 1;
  }

  LoadedTextures[id]->MakeResident();
  TexturePaths.emplace(h_path, id);
  m_TexturePathStrings.emplace(h_path, path);

  return id;
}

void MaterialManager::ReloadTexture(uint32_t p_TextureID) {

  std::string &l_strPath = m_TexturePathStrings[p_TextureID];

  LoadedTextures[p_TextureID]->RemoveResidency();

  LoadedTextures[p_TextureID].release();
  LoadedTextures[p_TextureID] =
      std::make_unique<Texture2D>(m_TexturePathStrings[p_TextureID]);

  LoadedTextures[p_TextureID]->MakeResident();
}
void MaterialManager::ReloadMaterial(uint32_t p_MatID, bool p_bReloadTextures) {

  SMaterialSpec &l_msSpec = m_umMaterialSpecs[p_MatID];

  if (p_bReloadTextures) {
    ReloadTexture(l_msSpec.EmissionID);
    ReloadTexture(l_msSpec.NormalMapID);
    ReloadTexture(l_msSpec.albedoID);
    ReloadTexture(l_msSpec.prmID);
  }

  PBRMaterial &l_pbrMaterial = LoadedPBRMaterials[p_MatID];

  l_pbrMaterial.Emission =
      LoadedTextures[l_msSpec.EmissionID]->GetTextureHandle();

  l_pbrMaterial.NormalMap =
      LoadedTextures[l_msSpec.NormalMapID]->GetTextureHandle();

  l_pbrMaterial.albedo = LoadedTextures[l_msSpec.albedoID]->GetTextureHandle();

  l_pbrMaterial.prm = LoadedTextures[l_msSpec.prmID]->GetTextureHandle();
}
void MaterialManager::DeleteTexture(uint32_t TextureID) {

  if (LoadedTextures.size() - 1 > TextureID) {

    freeTextureIndecies.push_back(TextureID);
  }
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

  SMaterialSpec newSpec;
  newSpec.EmissionID = EmissionID;
  newSpec.NormalMapID = NormalMapID;
  newSpec.prmID = prmID;
  newSpec.albedoID = albedoID;
  newSpec.mat_name = mat_name;

  PBRMaterial newMat;
  newMat.albedo = LoadedTextures[albedoID]->GetTextureHandle();
  newMat.prm = LoadedTextures[prmID]->GetTextureHandle();
  newMat.NormalMap = LoadedTextures[NormalMapID]->GetTextureHandle();
  newMat.Emission = LoadedTextures[EmissionID]->GetTextureHandle();
  newMat.Luminance = 0.5f;
  if (freeIndecies.size() == 0) {
    LoadedPBRMaterials.push_back(newMat);
    int index = LoadedPBRMaterials.size() - 1;
    MaterialNames.emplace(h_matName, index);
    newSpec.generation = 0;
    m_umMaterialSpecs.emplace(index, newSpec);
    return index;
  } else {
    int index = freeIndecies[0];
    LoadedPBRMaterials[index] = newMat;
    freeIndecies.erase(freeIndecies.begin());
    MaterialNames.emplace(h_matName, index);
    newSpec.generation++;
    m_umMaterialSpecs.emplace(index, newSpec);

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

std::string MaterialManager::GetMaterialName(uint32_t p_MatID) {

  return m_MaterialNamesString[p_MatID];
}
void MaterialManager::Destroy() {}

void MaterialManager::Initialize() {}

bool MaterialManager::isValidMaterial(std::string p_strPath) {
  MaterialID l_midHash = eHazGraphics_Utils::computeHash(p_strPath);

  if (MaterialNames.contains(l_midHash))
    return true;

  return false;
}
bool MaterialManager::isValidTexture(std::string p_strPath) {

  Texture2DID l_midHash = eHazGraphics_Utils::computeHash(p_strPath);

  if (TexturePaths.contains(l_midHash))
    return true;

  return false;
}
uint32_t MaterialManager::GetTextureID(std::string p_strPath) {

  Texture2DID l_tidHash = eHazGraphics_Utils::computeHash(p_strPath);

  return TexturePaths[l_tidHash];
}
uint32_t MaterialManager::GetMaterialID(std::string p_strName) {
  MaterialID l_midHash = eHazGraphics_Utils::computeHash(p_strName);

  return MaterialNames[l_midHash];
}
} // namespace eHazGraphics
