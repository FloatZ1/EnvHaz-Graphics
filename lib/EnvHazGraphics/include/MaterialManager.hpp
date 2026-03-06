#ifndef MATERIAL_MANAGER_HPP
#define MATERIAL_MANAGER_HPP

#include "BitFlags.hpp"
#include "DataStructs.hpp"
#include "Utils/HashedStrings.hpp"
#include "glad/glad.h"
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace eHazGraphics {

struct SMaterialSpec {

  uint32_t generation = 0;

  unsigned int albedoID;
  unsigned int prmID;
  unsigned int NormalMapID;
  unsigned int EmissionID;
  std::string mat_name;
};

class MaterialManager {
public:
  void
  Initialize(); // OPTIONAL: IMPLEMENT , this is incase its needed in the future

  unsigned int LoadTexture(const std::string &path);

  unsigned int
  CreatePBRMaterial(unsigned int albedoID, unsigned int prmID,
                    unsigned int NormalMapID, unsigned int EmissionID,
                    std::string mat_name); // gets the texture ids and creates a
                                           // struct in the pbr materials

  void UpdateMaterial(
      unsigned int MaterialID,
      PBRMaterial replacement); // in case we change a texture of a material

  std::pair<const std::vector<PBRMaterial> &, TypeFlags>
  SubmitMaterials(); // prepares the data to be sent to
                     // BufferManager; then just overwrites the
                     // whole buffer each frame.
  std::optional<PBRMaterial> GetMaterial(const std::string &materialName);

  uint32_t GetTextureID(std::string p_strPath);
  uint32_t GetMaterialID(std::string p_strPath);
  std::string GetMaterialName(uint32_t p_MatName);

  void ClearMaterials();

  void DeleteTexture(uint32_t TextureID);

  void ReloadTexture(uint32_t p_TextureID);

  void ReloadMaterial(uint32_t p_MatID, bool p_bReloadTextures = true);

  GLuint GetTextureGLID(uint32_t p_uiTexture) {
    if (p_uiTexture > LoadedTextures.size() || p_uiTexture <= 0)
      return 0;
    return LoadedTextures[p_uiTexture]->GetTexture();
  }

  void DeleteMaterial(
      unsigned int MaterialID); // probably shouldnt have this here since
                                // again... alignement is a b*%#

  void Destroy(); // OPTIONAL: IMPLEMENT , again nothing much to destroy that
                  // doesnt handle itself

  bool isValidMaterial(std::string p_strPath);
  bool isValidTexture(std::string p_strName);

private:
  std::vector<PBRMaterial> LoadedPBRMaterials;
  std::unordered_map<uint32_t, SMaterialSpec> m_umMaterialSpecs;

  std::vector<std::unique_ptr<Texture2D>> LoadedTextures;
  std::vector<uint32_t> freeTextureIndecies;
  std::vector<unsigned int> freeIndecies;
  std::unordered_map<Texture2DID, uint32_t> TexturePaths;
  std::unordered_map<uint32_t, std::string> m_TexturePathStrings;

  std::unordered_map<MaterialID, uint32_t> MaterialNames;
  std::unordered_map<uint32_t, std::string> m_MaterialNamesString;
};

} // namespace eHazGraphics

#endif
