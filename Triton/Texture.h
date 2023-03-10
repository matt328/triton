#pragma once

#include <ktx.h>
#include <string_view>


class Texture {
public:
   static ktxResult loadKtxFile(const std::string_view& filename, ktxTexture** target);
};
