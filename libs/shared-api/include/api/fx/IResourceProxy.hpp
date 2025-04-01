#pragma once

#include "api/gw/AssetStructs.hpp"
#include "as/ColorVertex.hpp"

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

  virtual auto addLines(const std::vector<as::ColorVertex>& vertices) -> void = 0;
};

}
