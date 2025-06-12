#pragma once

#include "r3/ComponentIds.hpp"
#include "r3/graph/DirectedGraph.hpp"
#include "r3/render-pass/IRenderPass.hpp"

namespace tr {

class PassGraphBuilder {
public:
  PassGraphBuilder() = default;
  ~PassGraphBuilder() = default;

  PassGraphBuilder(const PassGraphBuilder&) = default;
  PassGraphBuilder(PassGraphBuilder&&) = delete;
  auto operator=(const PassGraphBuilder&) -> PassGraphBuilder& = default;
  auto operator=(PassGraphBuilder&&) -> PassGraphBuilder& = delete;

  [[nodiscard]] auto build(const std::unordered_map<PassId, std::unique_ptr<IRenderPass>>& passes)
      const -> DirectedGraph<PassId>;
};

}
