#pragma once

#include "bk/Handle.hpp"
#include "img/ManagedImage.hpp"
#include "r3/graph/ResourceAliases.hpp"

namespace tr {

class ResourceAliasRegistry {
public:
  ResourceAliasRegistry() = default;
  ~ResourceAliasRegistry() = default;

  ResourceAliasRegistry(const ResourceAliasRegistry&) = default;
  ResourceAliasRegistry(ResourceAliasRegistry&&) = delete;
  auto operator=(const ResourceAliasRegistry&) -> ResourceAliasRegistry& = default;
  auto operator=(ResourceAliasRegistry&&) -> ResourceAliasRegistry& = delete;

  [[nodiscard]] auto getHandle(ResourceAlias alias) const -> LogicalHandle<ManagedImage>;
  auto setHandle(ResourceAlias alias, LogicalHandle<ManagedImage> handle) -> void;
  auto reset() -> void;

private:
  std::array<LogicalHandle<ManagedImage>, static_cast<size_t>(ResourceAlias::Count)> handles;
};

}
