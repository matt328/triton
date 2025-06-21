#pragma once

#include "bk/Handle.hpp"
#include "buffers/ManagedBuffer.hpp"
#include "img/ManagedImage.hpp"
#include "r3/graph/ImageAlias.hpp"
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

  [[nodiscard]] auto getHandle(ImageAlias alias) const -> LogicalHandle<ManagedImage>;
  [[nodiscard]] auto getHandle(BufferAlias alias) const -> LogicalHandle<ManagedBuffer>;
  [[nodiscard]] auto getHandle(GlobalBufferAlias alias) const -> Handle<ManagedBuffer>;

  auto setHandle(ImageAlias alias, LogicalHandle<ManagedImage> handle) -> void;
  auto setHandle(BufferAlias alias, LogicalHandle<ManagedBuffer> handle) -> void;
  auto setHandle(GlobalBufferAlias alias, Handle<ManagedBuffer> handle) -> void;
  auto reset() -> void;

private:
  std::array<LogicalHandle<ManagedImage>, static_cast<size_t>(ImageAlias::Count)> handles;
  std::array<LogicalHandle<ManagedBuffer>, static_cast<size_t>(BufferAlias::Count)> bufferHandles;
  std::array<Handle<ManagedBuffer>, static_cast<size_t>(GlobalBufferAlias::Count)>
      globalBufferHandles;
};

}
