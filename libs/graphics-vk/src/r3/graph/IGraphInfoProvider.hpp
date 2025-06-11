#pragma once

#include "r3/graph/PassGraphInfo.hpp"

namespace tr {

class IGraphInfoProvider {
public:
  IGraphInfoProvider() = default;
  virtual ~IGraphInfoProvider() = default;

  IGraphInfoProvider(const IGraphInfoProvider&) = default;
  IGraphInfoProvider(IGraphInfoProvider&&) = delete;
  auto operator=(const IGraphInfoProvider&) -> IGraphInfoProvider& = default;
  auto operator=(IGraphInfoProvider&&) -> IGraphInfoProvider& = delete;

  [[nodiscard]] virtual auto getGraphInfo() const -> PassGraphInfo = 0;
};

}
