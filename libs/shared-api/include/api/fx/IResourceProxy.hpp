#pragma once

#include "api/gfx/DDGeometryData.hpp"
#include "api/gw/AssetStructs.hpp"
#include "api/gw/RenderableData.hpp"
#include "api/gw/RenderableResources.hpp"

namespace as {
struct Model;
}

namespace tr {

class IResourceProxy {
public:
  IResourceProxy() = default;
  virtual ~IResourceProxy() = default;

  IResourceProxy(const IResourceProxy&) = default;
  IResourceProxy(IResourceProxy&&) = delete;
  auto operator=(const IResourceProxy&) -> IResourceProxy& = default;
  auto operator=(IResourceProxy&&) -> IResourceProxy& = delete;

  virtual auto uploadModel(const as::Model& model) -> ModelData = 0;
  virtual auto uploadGeometry(DDGeometryData&& data) -> MeshHandle = 0;
  virtual auto registerRenderable(RenderableData&& data) -> RenderableResources = 0;
};

}
