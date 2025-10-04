#include "MaterialManager.hpp"
#include "BitFlags.hpp"
#include "DataStructs.hpp"
#include "Utils/HashedStrings.hpp"
#include <SDL3/SDL_log.h>
#include <cstddef>
#include <optional>
#include <utility>
#include <vector>


namespace eHazGraphics
{

int MaterialManager::LoadTexture(const std::string &path)
{
    LoadedTextures.push_back(Texture2D(path));
    return LoadedTextures.size() - 1;
}
unsigned int MaterialManager::CreatePBRMaterial(unsigned int albedoID, unsigned int prmID, unsigned int NormalMapID,
                                                unsigned int EmissionID)
{
    PBRMaterial newMat;
    newMat.albedo = LoadedTextures[albedoID].GetTextureHandle();
    newMat.prm = LoadedTextures[prmID].GetTextureHandle();
    newMat.NormalMap = LoadedTextures[NormalMapID].GetTextureHandle();
    newMat.Emission = LoadedTextures[EmissionID].GetTextureHandle();
    newMat.Luminance = 0.5f;
    if (freeIndecies.size() == 0)
    {
        LoadedPBRMaterials.push_back(newMat);
        return LoadedPBRMaterials.size() - 1;
    }
    else
    {
        int index = freeIndecies[0];
        LoadedPBRMaterials[index] = newMat;
        freeIndecies.erase(freeIndecies.begin());
        return index;
    }
}
void MaterialManager::UpdateMaterial(unsigned int MaterialID, PBRMaterial replacement)
{
    LoadedPBRMaterials[MaterialID] = replacement;
}
std::pair<const std::vector<PBRMaterial> &, TypeFlags> MaterialManager::SubmitMaterials()
{

    return std::pair<const std::vector<PBRMaterial> &, TypeFlags>(LoadedPBRMaterials, TypeFlags::BUFFER_TEXTURE_DATA);
}
std::optional<PBRMaterial> MaterialManager::GetMaterial(const std::string &materialName)
{
    eHazGraphics_Utils::HashedString temp = eHazGraphics_Utils::computeHash(materialName);


    if (MaterialNames.find(temp) != MaterialNames.end())
    {
        return LoadedPBRMaterials[MaterialNames[temp]];
    }
    else
    {
        SDL_Log("Material Query: NO SUCH MATERIAL ERROR \n");
    }
    return std::nullopt;
}
void MaterialManager::DeleteMaterial(unsigned int MaterialID)
{

    freeIndecies.push_back(MaterialID);
}

void MaterialManager::Destroy()
{
}

void MaterialManager::Initialize()
{
}



} // namespace eHazGraphics
