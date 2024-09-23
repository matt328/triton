#pragma once

#include "as/ConverterComponents.hpp"

namespace tr::as::gltf {

   class GltfLoaderImpl : public GltfFileLoader {
    public:
      GltfLoaderImpl() = default;
      ~GltfLoaderImpl() noexcept override;

      GltfLoaderImpl(const GltfLoaderImpl&) = delete;
      auto operator=(const GltfLoaderImpl&) -> GltfLoaderImpl& = delete;

      GltfLoaderImpl(GltfLoaderImpl&&) = delete;
      auto operator=(GltfLoaderImpl&&) -> GltfLoaderImpl& = delete;

      auto loadFromFile(tinygltf::Model* model,
                        std::string* err,
                        std::string* warn,
                        const std::string& filename) -> bool override;
   };

} // namespace tr::as::gltf
