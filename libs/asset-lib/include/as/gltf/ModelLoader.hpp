#pragma once

#include "as/ConverterComponents.hpp"

namespace tr::as::gltf {

   class ModelLoader final : public as::ModelLoader {
    public:
      ModelLoader() = default;
      ~ModelLoader() noexcept override;

      ModelLoader(const ModelLoader&) = delete;
      ModelLoader& operator=(const ModelLoader&) = delete;

      ModelLoader(ModelLoader&&) = delete;
      ModelLoader& operator=(ModelLoader&&) = delete;

      auto load(GltfFileLoader* loader, const std::filesystem::path& path) const
          -> tinygltf::Model override;
   };

}
