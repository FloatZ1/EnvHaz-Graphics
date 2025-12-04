#ifndef MATERIAL_MANAGER_HPP
#define MATERIAL_MANAGER_HPP

#include "BitFlags.hpp"
#include "DataStructs.hpp"
#include "Utils/HashedStrings.hpp"
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace eHazGraphics {

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
                     // whole buffer each frame? or we could
                     // optimize dunno.
  std::optional<PBRMaterial> GetMaterial(const std::string &materialName);

  void ClearMaterials();

  void DeleteMaterial(
      unsigned int MaterialID); // probably shouldnt have this here since
                                // again... alignement is a b*%#

  void Destroy(); // OPTIONAL: IMPLEMENT , again nothing much to destroy that
                  // doesnt handle itself
private:
  std::vector<PBRMaterial> LoadedPBRMaterials;
  std::vector<std::unique_ptr<Texture2D>> LoadedTextures;
  std::vector<unsigned int> freeIndecies;
  std::unordered_map<eHazGraphics_Utils::HashedString, unsigned int>
      TexturePaths;

  std::unordered_map<eHazGraphics_Utils::HashedString, unsigned int>
      MaterialNames;
};

} // namespace eHazGraphics

#endif
