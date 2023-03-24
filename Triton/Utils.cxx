#include "pch.hpp"

#include "Utils.hpp"

ktxResult Textures::loadKtxFile(const std::string_view& filename, ktxTexture** target) {
   ktxResult result = KTX_SUCCESS;
   result = ktxTexture_CreateFromNamedFile(
       filename.data(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, target);
   return result;
}
