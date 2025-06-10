#include "ResourceAliasRegistry.hpp"

namespace tr {

[[nodiscard]] auto ResourceAliasRegistry::getHandle(ImageAlias alias) const
    -> LogicalHandle<ManagedImage> {
  return handles[static_cast<size_t>(alias)];
}

[[nodiscard]] auto ResourceAliasRegistry::getHandle(BufferAlias alias) const
    -> LogicalHandle<ManagedBuffer> {
  return bufferHandles[static_cast<size_t>(alias)];
}

auto ResourceAliasRegistry::setHandle(ImageAlias alias, LogicalHandle<ManagedImage> handle)
    -> void {
  handles[static_cast<size_t>(alias)] = handle;
}

auto ResourceAliasRegistry::setHandle(BufferAlias alias, LogicalHandle<ManagedBuffer> handle)
    -> void {
  bufferHandles[static_cast<size_t>(alias)] = handle;
}

auto ResourceAliasRegistry::reset() -> void {
  std::ranges::fill(handles, LogicalHandle<ManagedImage>{});
}

}
