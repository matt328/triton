#pragma once

#include "as/ConverterComponents.hpp"

namespace as {

class GltfModelLoader final : public as::IModelLoader {
public:
  GltfModelLoader() = default;
  ~GltfModelLoader() noexcept override;

  GltfModelLoader(const GltfModelLoader&) = delete;
  auto operator=(const GltfModelLoader&) -> GltfModelLoader& = delete;

  GltfModelLoader(GltfModelLoader&&) = delete;
  auto operator=(GltfModelLoader&&) -> GltfModelLoader& = delete;

  auto load(IGltfFileLoader* loader, const std::filesystem::path& path) const
      -> tinygltf::Model override;
};

}
