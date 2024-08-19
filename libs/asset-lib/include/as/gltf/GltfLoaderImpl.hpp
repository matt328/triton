#pragma once

#include "as/ConverterComponents.hpp"

namespace tr::as::gltf {

   class GltfLoaderImpl : public GltfFileLoader {
    public:
      GltfLoaderImpl() = default;
      ~GltfLoaderImpl() noexcept override;

      GltfLoaderImpl(const GltfLoaderImpl&) = delete;
      GltfLoaderImpl& operator=(const GltfLoaderImpl&) = delete;

      GltfLoaderImpl(GltfLoaderImpl&&) = delete;
      GltfLoaderImpl& operator=(GltfLoaderImpl&&) = delete;

      bool loadFromFile(tinygltf::Model* model,
                        std::string* err,
                        std::string* warn,
                        const std::string& filename) override;
   };

}
