#include <algorithm>

#include "ResourceAliasRegistry.hpp"

namespace tr {

[[nodiscard]] auto ResourceAliasRegistry::getHandle(ResourceAlias alias) const
    -> LogicalHandle<ManagedImage> {
  return handles[static_cast<size_t>(alias)];
}

auto ResourceAliasRegistry::setHandle(ResourceAlias alias, LogicalHandle<ManagedImage> handle)
    -> void {
  handles[static_cast<size_t>(alias)] = handle;
}

auto ResourceAliasRegistry::reset() -> void {
  std::ranges::fill(handles, LogicalHandle<ManagedImage>{});
}

}
