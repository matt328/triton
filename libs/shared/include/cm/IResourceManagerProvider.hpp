#pragma once

#include "Handles.hpp"
#include "IGeometryData.hpp"
#include "as/Model.hpp"

// TODO(matt): Move ImageData into CM
// TODO(matt): Move Vertex Definitions into CM

namespace tr {

class IResourceManagerProvider {
public:
  IResourceManagerProvider() = default;
  virtual ~IResourceManagerProvider() = default;

  IResourceManagerProvider(const IResourceManagerProvider&) = default;
  IResourceManagerProvider(IResourceManagerProvider&&) = delete;
  auto operator=(const IResourceManagerProvider&) -> IResourceManagerProvider& = default;
  auto operator=(IResourceManagerProvider&&) -> IResourceManagerProvider& = delete;

  virtual auto uploadDynamicMesh(const IGeometryData& geometryData) -> MeshHandle = 0;
  virtual auto uploadStaticMesh(const IGeometryData& geometryData) -> MeshHandle = 0;
  virtual auto uploadImage([[maybe_unused]] const as::ImageData& imageData,
                           std::string_view name) -> TextureHandle = 0;
};

}
